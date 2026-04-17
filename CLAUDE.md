# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**airun** is a C++ CLI tool that translates natural language queries into Linux shell commands using AI (OpenAI or Anthropic APIs), executes those commands, and returns the results. It also intercepts a configurable list of sensitive/dangerous commands before execution.

Example usage:
```bash
airun 查看/mnt文件夹中，占用最大的目录
# → AI generates: du -sh /mnt/* | sort -rh | head -5
# → Executes command and prints output
```

## Build System

Uses **CMake** (minimum 3.15) with C++17.

```bash
# Configure (from project root)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Debug build
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j$(nproc)

# Run
./build/airun "查看/mnt文件夹中占用最大的目录"

# Install (optional)
cmake --install build --prefix ~/.local
```

## Architecture

### Key Components

```
src/
  main.cpp                  - Entry point, argument parsing, orchestration
  config.cpp/.h             - Config file loading (TOML format)
  ai_client.h               - Abstract AI client interface (pure virtual)
  openai_client.cpp/.h      - OpenAI API implementation (HTTP + JSON)
  anthropic_client.cpp/.h   - Anthropic API implementation
  command_executor.cpp/.h   - Shell command execution via popen()
  safety_filter.cpp/.h      - Sensitive command interception against blocklist
  prompt_builder.cpp/.h     - Builds system/user prompts for the AI
third_party/
  nlohmann/json.hpp         - Header-only JSON library (vendored)
  toml11/                   - Header-only TOML parser (vendored)
config/
  airun.toml.example        - Example configuration file
  blocklist.txt.example     - Example sensitive command blocklist
CMakeLists.txt
```

### Data Flow

1. **main** parses CLI args → natural language query string
2. **config** loads `~/.config/airun/config.toml` (or `./airun.toml` as fallback)
3. **prompt_builder** constructs the system prompt (instructs AI to output only a single shell command, no explanation)
4. **ai_client** (OpenAI or Anthropic, selected by config `provider` field) sends the prompt and returns the generated command string
5. **safety_filter** tokenizes the command and checks each token against the blocklist; aborts with an error message if a match is found
6. **command_executor** runs the command via `popen()` and streams stdout/stderr back to the terminal

### Configuration File (`~/.config/airun/config.toml`)

```toml
[api]
provider = "openai"          # "openai" or "anthropic"
openai_api_key = "sk-..."
openai_model = "gpt-4o"
anthropic_api_key = "sk-ant-..."
anthropic_model = "claude-opus-4-5"
base_url = ""                # optional API base URL override

[behavior]
blocklist_file = "~/.config/airun/blocklist.txt"
show_command = true          # print the generated command before executing
confirm_before_run = false   # prompt user y/n before executing
```

### Blocklist File

Plain text, one pattern per line. Each token in the generated command is checked against these patterns (case-insensitive prefix match). Example:

```
rm
sudo
mkfs
dd
chmod 777
> /dev/
```

### Dependencies

- **libcurl** – HTTP requests to AI APIs (system package: `libcurl4-openssl-dev`)
- **nlohmann/json** (header-only, vendored in `third_party/`) – JSON parsing
- **toml11** (header-only, vendored in `third_party/`) – Config file parsing

Install system deps on Debian/Ubuntu:
```bash
sudo apt install build-essential cmake libcurl4-openssl-dev
```

## Prompt Engineering

The system prompt sent to the AI instructs it to:
- Output **only** the shell command, with no explanation or markdown
- Produce a single line suitable for passing to `sh -c`
- Target OS: Linux

## Safety Filter Logic

`safety_filter` splits the generated command on whitespace, pipes (`|`), semicolons (`;`), `&&`, and `||`, then checks each resulting token against the blocklist. Matching is case-insensitive prefix match. If any token matches, execution is blocked and an error is printed to stderr.

## AI Client Interface

`ai_client.h` defines an abstract base class:
```cpp
class AIClient {
public:
    virtual ~AIClient() = default;
    virtual std::string generate_command(const std::string& user_query) = 0;
};
```

`OpenAIClient` and `AnthropicClient` both implement this interface, so `main.cpp` selects the concrete implementation based on config and works against the interface.
