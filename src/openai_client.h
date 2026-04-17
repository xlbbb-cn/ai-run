#pragma once
#include "ai_client.h"
#include <string>

class OpenAIClient : public AIClient {
public:
    explicit OpenAIClient(const std::string& api_key,
                          const std::string& model    = "gpt-4o",
                          const std::string& base_url = "");

    std::string generate_command(const std::string& system_prompt,
                                 const std::string& user_query) override;

private:
    std::string api_key_;
    std::string model_;
    std::string base_url_;  // e.g. "https://api.openai.com/v1"
};
