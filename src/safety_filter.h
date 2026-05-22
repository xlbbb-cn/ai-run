#pragma once
#include <string>
#include <vector>

enum class SafetyDecision {
    Safe,
    RequireConfirmation,
    Blocked,
};

struct SafetyResult {
    SafetyDecision decision = SafetyDecision::Safe;
    std::string matched_pattern;
    std::string reason;
};

class SafetyFilter {
public:
    // Load blocklist from file. Each non-empty, non-comment line is a pattern.
    // If the file doesn't exist, the filter is empty (no commands blocked).
    explicit SafetyFilter(const std::string& blocklist_path);

    // Evaluate the command against the blocklist and elevation heuristics.
    // - Blocked: reject before execution
    // - RequireConfirmation: allowed, but should be explicitly confirmed by user
    // - Safe: allowed
    SafetyResult evaluate(const std::string& command) const;

    // Returns true if the command is SAFE to execute.
    // Returns false (and sets matched_pattern) if a blocked pattern is found.
    bool is_safe(const std::string& command, std::string& matched_pattern) const;

    // Convenience overload.
    bool is_safe(const std::string& command) const;

    const std::vector<std::string>& patterns() const { return patterns_; }

private:
    std::vector<std::string> patterns_;

    // Tokenize command on whitespace, |, ;, &&, ||
    static std::vector<std::string> tokenize(const std::string& cmd);

    // True if token matches pattern (case-insensitive prefix match)
    static bool matches(const std::string& token, const std::string& pattern);
};
