#include "config.h"
#include "ai_client.h"
#include "openai_client.h"
#include "anthropic_client.h"
#include "safety_filter.h"
#include "command_executor.h"
#include "prompt_builder.h"

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <cstdlib>

static void print_usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <natural language query>\n"
              << "       " << prog << " --version\n"
              << "       " << prog << " --help\n\n"
              << "Environment:\n"
              << "  AIRUN_CONFIG      Path to config file\n"
              << "  OPENAI_API_KEY    Override OpenAI API key\n"
              << "  ANTHROPIC_API_KEY Override Anthropic API key\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string first_arg = argv[1];
    if (first_arg == "--help" || first_arg == "-h") {
        print_usage(argv[0]);
        return 0;
    }
    if (first_arg == "--version" || first_arg == "-v") {
        std::cout << "airun 0.1.0\n";
        return 0;
    }

    // Join all arguments into one query string
    std::ostringstream oss;
    for (int i = 1; i < argc; ++i) {
        if (i > 1) oss << ' ';
        oss << argv[i];
    }
    std::string query = oss.str();

    // ── Load config ────────────────────────────────────────────────────────
    Config cfg;
    try {
        cfg = load_config_auto();
    } catch (const std::exception& e) {
        std::cerr << "[airun] Config error: " << e.what() << "\n";
        return 2;
    }

    // ── Build AI client ────────────────────────────────────────────────────
    std::unique_ptr<AIClient> client;
    try {
        if (cfg.provider == "anthropic") {
            client = std::make_unique<AnthropicClient>(
                cfg.anthropic_api_key, cfg.anthropic_model, cfg.base_url);
        } else {
            // default: openai
            client = std::make_unique<OpenAIClient>(
                cfg.openai_api_key, cfg.openai_model, cfg.base_url);
        }
    } catch (const std::exception& e) {
        std::cerr << "[airun] Failed to create AI client: " << e.what() << "\n";
        return 2;
    }

    // ── Generate command ───────────────────────────────────────────────────
    std::string command;
    try {
        std::string sys = PromptBuilder::system_prompt();
        std::string usr = PromptBuilder::user_message(query);
        command = client->generate_command(sys, usr);
    } catch (const std::exception& e) {
        std::cerr << "[airun] AI error: " << e.what() << "\n";
        return 3;
    }

    if (command.empty()) {
        std::cerr << "[airun] AI returned an empty command.\n";
        return 3;
    }

    // ── Safety check ──────────────────────────────────────────────────────
    SafetyFilter filter(cfg.blocklist_file);
    std::string matched;
    if (!filter.is_safe(command, matched)) {
        std::cerr << "[airun] \033[1;31mBLOCKED\033[0m: The generated command contains a "
                  << "restricted pattern '" << matched << "'.\n"
                  << "  Command: " << command << "\n"
                  << "  Edit " << cfg.blocklist_file << " to adjust the blocklist.\n";
        return 4;
    }

    // ── Show command ───────────────────────────────────────────────────────
    if (cfg.show_command) {
        std::cout << "\033[1;36m[airun]\033[0m Generated command:\n"
                  << "  \033[1m" << command << "\033[0m\n\n";
    }

    // ── Confirm before run ─────────────────────────────────────────────────
    if (cfg.confirm_before_run) {
        std::cout << "Execute? [y/N] ";
        std::string answer;
        std::getline(std::cin, answer);
        if (answer.empty() || (answer[0] != 'y' && answer[0] != 'Y')) {
            std::cout << "[airun] Aborted.\n";
            return 0;
        }
    }

    // ── Execute ───────────────────────────────────────────────────────────
    ExecResult result;
    try {
        result = CommandExecutor::run(command, /*stream_output=*/true);
    } catch (const std::exception& e) {
        std::cerr << "[airun] Execution error: " << e.what() << "\n";
        return 5;
    }

    if (result.exit_code != 0) {
        std::cerr << "\n[airun] Command exited with code " << result.exit_code << "\n";
        return result.exit_code;
    }

    return 0;
}
