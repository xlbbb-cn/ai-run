#pragma once
#include "ai_client.h"
#include <string>

class AnthropicClient : public AIClient {
public:
    explicit AnthropicClient(const std::string& api_key,
                             const std::string& model    = "claude-opus-4-5",
                             const std::string& base_url = "");

    std::string generate_command(const std::string& system_prompt,
                                 const std::string& user_query) override;

private:
    std::string api_key_;
    std::string model_;
    std::string base_url_;
};
