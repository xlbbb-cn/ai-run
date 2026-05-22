#include "safety_filter.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <regex>

SafetyFilter::SafetyFilter(const std::string& blocklist_path) {
    std::ifstream f(blocklist_path);
    if (!f.is_open()) return;  // missing file = empty blocklist, not an error

    std::string line;
    while (std::getline(f, line)) {
        // Strip carriage return
        if (!line.empty() && line.back() == '\r') line.pop_back();
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        // Trim leading/trailing whitespace
        auto start = line.find_first_not_of(" \t");
        auto end   = line.find_last_not_of(" \t");
        if (start == std::string::npos) continue;
        patterns_.push_back(line.substr(start, end - start + 1));
    }
}

std::vector<std::string> SafetyFilter::tokenize(const std::string& cmd) {
    // Replace shell separators with spaces, then split
    std::string tmp = cmd;
    // Replace |, ;, &&, || with spaces
    for (size_t i = 0; i < tmp.size(); ) {
        if (i + 1 < tmp.size() && (tmp.substr(i, 2) == "&&" || tmp.substr(i, 2) == "||")) {
            tmp[i] = ' '; tmp[i+1] = ' '; i += 2;
        } else if (tmp[i] == '|' || tmp[i] == ';' || tmp[i] == '&') {
            tmp[i] = ' '; ++i;
        } else {
            ++i;
        }
    }

    std::istringstream ss(tmp);
    std::vector<std::string> tokens;
    std::string tok;
    while (ss >> tok) {
        // Strip leading path component (e.g. /bin/rm → rm)
        auto slash = tok.rfind('/');
        if (slash != std::string::npos)
            tok = tok.substr(slash + 1);
        if (!tok.empty())
            tokens.push_back(tok);
    }
    return tokens;
}

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

static bool is_elevation_token_posix(const std::string& token_lower) {
    // Allow sudo with explicit confirmation (not hard-blocked).
    return token_lower == "sudo";
}

static bool is_elevation_token_windows(const std::string& token_lower) {
    // Best-effort detection; actual execution on Windows may differ.
    // "runas" is the canonical elevation mechanism.
    return token_lower == "runas" || token_lower == "runas.exe";
}

static bool contains_powershell_runas(const std::vector<std::string>& tokens) {
    // Detect: Start-Process ... -Verb RunAs
    for (size_t i = 0; i + 2 < tokens.size(); ++i) {
        if (to_lower(tokens[i]) == "start-process" &&
            to_lower(tokens[i + 1]) == "-verb" &&
            to_lower(tokens[i + 2]) == "runas") {
            return true;
        }
    }
    // Also detect: -Verb RunAs (without Start-Process token in the same command)
    for (size_t i = 0; i + 1 < tokens.size(); ++i) {
        if (to_lower(tokens[i]) == "-verb" && to_lower(tokens[i + 1]) == "runas") {
            return true;
        }
    }
    return false;
}

bool SafetyFilter::matches(const std::string& token, const std::string& pattern) {
    // Case-insensitive: token starts with pattern
    std::string lt = to_lower(token);
    std::string lp = to_lower(pattern);
    // Exact match or token starts with pattern (e.g. "rm" matches "rm -rf")
    return lt == lp || lt.substr(0, lp.size()) == lp;
}

SafetyResult SafetyFilter::evaluate(const std::string& command) const {
    SafetyResult result;
    const auto tokens = tokenize(command);

    bool requires_confirmation = false;
    std::string confirm_reason;

    for (const auto& token : tokens) {
        const std::string lower = to_lower(token);
#if defined(_WIN32)
        if (is_elevation_token_windows(lower) || contains_powershell_runas(tokens)) {
            requires_confirmation = true;
            confirm_reason = "command requests administrator privileges";
            result.matched_pattern = token;
            break;
        }
#else
        if (is_elevation_token_posix(lower)) {
            requires_confirmation = true;
            confirm_reason = "command uses sudo and may prompt for credentials";
            result.matched_pattern = token;
            break;
        }
#endif
    }

    // Blocklist enforcement (but do NOT hard-block elevation patterns we allow with confirmation).
    for (const auto& token : tokens) {
        const std::string token_lower = to_lower(token);
        for (const auto& pat : patterns_) {
            const std::string pat_lower = to_lower(pat);

#if defined(_WIN32)
            if (is_elevation_token_windows(token_lower) && pat_lower == "runas") {
                continue;
            }
#else
            if (is_elevation_token_posix(token_lower) && pat_lower == "sudo") {
                continue;
            }
#endif

            if (matches(token, pat)) {
                result.decision = SafetyDecision::Blocked;
                result.matched_pattern = pat;
                result.reason = "blocked by pattern '" + pat + "'";
                return result;
            }
        }
    }

    if (requires_confirmation) {
        result.decision = SafetyDecision::RequireConfirmation;
        result.reason = confirm_reason;
        return result;
    }

    result.decision = SafetyDecision::Safe;
    return result;
}

bool SafetyFilter::is_safe(const std::string& command, std::string& matched_pattern) const {
    auto eval = evaluate(command);
    if (eval.decision == SafetyDecision::Blocked) {
        matched_pattern = eval.matched_pattern;
        return false;
    }
    return true;
}

bool SafetyFilter::is_safe(const std::string& command) const {
    std::string dummy;
    return is_safe(command, dummy);
}
