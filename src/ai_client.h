#pragma once
#include <string>

class AIClient {
public:
    virtual ~AIClient() = default;

    // Send (system_prompt, user_query) to the model and return the generated command string.
    // Throws std::runtime_error on network or API errors.
    virtual std::string generate_command(const std::string& system_prompt,
                                         const std::string& user_query) = 0;
};
