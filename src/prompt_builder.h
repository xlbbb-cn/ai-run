#pragma once
#include <string>

class PromptBuilder {
public:
    // Returns the system prompt telling the AI to output only a shell command.
    static std::string system_prompt();

    // Returns the user message built from the raw natural-language query.
    static std::string user_message(const std::string& query);
};
