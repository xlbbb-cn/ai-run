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

bool SafetyFilter::matches(const std::string& token, const std::string& pattern) {
    // Case-insensitive: token starts with pattern
    std::string lt = to_lower(token);
    std::string lp = to_lower(pattern);
    // Exact match or token starts with pattern (e.g. "rm" matches "rm -rf")
    return lt == lp || lt.substr(0, lp.size()) == lp;
}

bool SafetyFilter::is_safe(const std::string& command, std::string& matched_pattern) const {
    auto tokens = tokenize(command);
    for (const auto& token : tokens) {
        for (const auto& pat : patterns_) {
            if (matches(token, pat)) {
                matched_pattern = pat;
                return false;
            }
        }
    }
    return true;
}

bool SafetyFilter::is_safe(const std::string& command) const {
    std::string dummy;
    return is_safe(command, dummy);
}
