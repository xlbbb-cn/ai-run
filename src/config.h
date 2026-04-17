#pragma once
#include <string>

struct Config {
    // [api]
    std::string provider;           // "openai" or "anthropic"
    std::string openai_api_key;
    std::string openai_model;
    std::string anthropic_api_key;
    std::string anthropic_model;
    std::string base_url;           // optional override

    // [behavior]
    std::string blocklist_file;
    bool        show_command       = true;
    bool        confirm_before_run = false;
};

// Load config from file path. Throws std::runtime_error on failure.
Config load_config(const std::string& path);

// Search standard locations and return the first found config.
// Order: $AIRUN_CONFIG env var → ./airun.toml → ~/.config/airun/config.toml
// Throws std::runtime_error if none found.
Config load_config_auto();
