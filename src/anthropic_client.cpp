#include "anthropic_client.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <stdexcept>

using json = nlohmann::json;

namespace {

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* buf = static_cast<std::string*>(userdata);
    buf->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string do_post(const std::string& url,
                    const std::string& api_key,
                    const std::string& body) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize libcurl");

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth    = "x-api-key: " + api_key;
    std::string version = "anthropic-version: 2023-06-01";
    headers = curl_slist_append(headers, auth.c_str());
    headers = curl_slist_append(headers, version.c_str());

    curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,     headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        60L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
        throw std::runtime_error(std::string("curl error: ") + curl_easy_strerror(res));

    return response;
}

} // namespace

AnthropicClient::AnthropicClient(const std::string& api_key,
                                  const std::string& model,
                                  const std::string& base_url)
    : api_key_(api_key)
    , model_(model)
    , base_url_(base_url.empty() ? "https://api.anthropic.com" : base_url)
{}

std::string AnthropicClient::generate_command(const std::string& system_prompt,
                                               const std::string& user_query) {
    if (api_key_.empty())
        throw std::runtime_error("Anthropic API key is not configured");

    json payload = {
        {"model",      model_},
        {"system",     system_prompt},
        {"max_tokens", 256},
        {"messages", json::array({
            {{"role", "user"}, {"content", user_query}}
        })}
    };

    std::string url = base_url_ + "/v1/messages";
    std::string raw = do_post(url, api_key_, payload.dump());

    json resp;
    try {
        resp = json::parse(raw);
    } catch (...) {
        throw std::runtime_error("Anthropic: invalid JSON response: " + raw);
    }

    if (resp.contains("error")) {
        throw std::runtime_error("Anthropic API error: " +
            resp["error"].value("message", resp["error"].dump()));
    }

    std::string cmd = resp["content"][0]["text"].get<std::string>();
    // Trim leading/trailing whitespace and backticks
    auto start = cmd.find_first_not_of(" \t\n\r`");
    auto end   = cmd.find_last_not_of(" \t\n\r`");
    if (start == std::string::npos) return "";
    return cmd.substr(start, end - start + 1);
}
