#include "config.h"
#include <toml11/toml.hpp>
#include <stdexcept>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

static std::string expand_home(const std::string& path) {
    if (path.size() >= 2 && path[0] == '~' && path[1] == '/') {
        const char* home = std::getenv("HOME");
        if (home) return std::string(home) + path.substr(1);
    }
    return path;
}

Config load_config(const std::string& path) {
    toml::value data;
    try {
        data = toml::parse(path);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse config file '" + path + "': " + e.what());
    }

    Config cfg;

    // [api]
    if (data.contains("api")) {
        const auto& api = toml::find(data, "api");
        cfg.provider         = toml::find_or(api, "provider",          std::string("openai"));
        cfg.openai_api_key   = toml::find_or(api, "openai_api_key",    std::string(""));
        cfg.openai_model     = toml::find_or(api, "openai_model",      std::string("gpt-4o"));
        cfg.anthropic_api_key= toml::find_or(api, "anthropic_api_key", std::string(""));
        cfg.anthropic_model  = toml::find_or(api, "anthropic_model",   std::string("claude-opus-4-5"));
        cfg.base_url         = toml::find_or(api, "base_url",          std::string(""));
    }

    // [behavior]
    if (data.contains("behavior")) {
        const auto& beh = toml::find(data, "behavior");
        cfg.blocklist_file    = toml::find_or(beh, "blocklist_file",    std::string(""));
        cfg.show_command      = toml::find_or(beh, "show_command",      true);
        cfg.confirm_before_run= toml::find_or(beh, "confirm_before_run",false);
    }

    // Defaults
    if (cfg.blocklist_file.empty()) {
        cfg.blocklist_file = expand_home("~/.config/airun/blocklist.txt");
    } else {
        cfg.blocklist_file = expand_home(cfg.blocklist_file);
    }

    // Allow env vars to override api keys
    if (const char* v = std::getenv("OPENAI_API_KEY"); v && *v)
        cfg.openai_api_key = v;
    if (const char* v = std::getenv("ANTHROPIC_API_KEY"); v && *v)
        cfg.anthropic_api_key = v;

    return cfg;
}

Config load_config_auto() {
    // 1. $AIRUN_CONFIG
    if (const char* env = std::getenv("AIRUN_CONFIG"); env && *env) {
        return load_config(env);
    }
    // 2. ./airun.toml
    if (fs::exists("airun.toml")) {
        return load_config("airun.toml");
    }
    // 3. ~/.config/airun/config.toml
    std::string default_path = expand_home("~/.config/airun/config.toml");
    if (fs::exists(default_path)) {
        return load_config(default_path);
    }

    throw std::runtime_error(
        "No config file found. Create one at ~/.config/airun/config.toml\n"
        "See the example at share/airun/airun.toml.example"
    );
}
