# airun

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**airun** is a powerful CLI tool that translates natural language queries into Linux shell commands using AI (OpenAI or Anthropic APIs), executes them, and returns the results. It includes built-in safety filters to prevent execution of dangerous commands.

## Features

- 🤖 **AI-Powered Command Generation** - Convert natural language (English or Chinese) to shell commands
- 🔒 **Safety First** - Configurable blocklist to prevent dangerous command execution
- 🎯 **Multiple AI Providers** - Support for both OpenAI and Anthropic APIs
- ⚡ **Fast & Lightweight** - Written in C++17 for optimal performance
- 🛠️ **Flexible Configuration** - TOML-based configuration with environment variable overrides

## Installation

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+)
- CMake 3.15+
- libcurl development libraries

On Debian/Ubuntu:
```bash
sudo apt install build-essential cmake libcurl4-openssl-dev
```

### Build from Source

```bash
# Clone the repository
git clone https://github.com/xlbbb-cn/ai-run.git
cd ai-run

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Install (optional)
cmake --install build --prefix ~/.local
```

## Quick Start

### 1. Configuration

Create a configuration file at `~/.config/airun/config.toml`:

```toml
[api]
provider = "openai"          # "openai" or "anthropic"
openai_api_key = "sk-..."
openai_model = "gpt-4o"
# For Anthropic:
# anthropic_api_key = "sk-ant-..."
# anthropic_model = "claude-opus-4-5"

[behavior]
blocklist_file = "~/.config/airun/blocklist.txt"
show_command = true          # Display command before execution
confirm_before_run = false   # Prompt for confirmation before running
```

### 2. Create Blocklist (Optional)

Create `~/.config/airun/blocklist.txt` with patterns to block:

```
rm
sudo
mkfs
dd
chmod 777
> /dev/
```

### 3. Run airun

```bash
# English query
./build/airun "find the largest directories in /mnt"

# Chinese query
./build/airun "查看/mnt文件夹中，占用最大的目录"
```

## Usage Examples

```bash
# Find large files
airun "find files larger than 100MB in current directory"

# System monitoring
airun "show me the top 10 processes by CPU usage"

# Text processing
airun "count the number of lines in all .cpp files"

# Network diagnostics
airun "check if port 8080 is listening"
```

## Configuration Options

### API Section

| Option | Description | Default |
|--------|-------------|---------|
| `provider` | AI provider: "openai" or "anthropic" | "openai" |
| `openai_api_key` | OpenAI API key | - |
| `openai_model` | OpenAI model name | "gpt-4o" |
| `anthropic_api_key` | Anthropic API key | - |
| `anthropic_model` | Anthropic model name | "claude-opus-4-5" |
| `base_url` | Custom API base URL (optional) | - |

### Behavior Section

| Option | Description | Default |
|--------|-------------|---------|
| `blocklist_file` | Path to command blocklist | "~/.config/airun/blocklist.txt" |
| `show_command` | Display generated command | true |
| `confirm_before_run` | Require confirmation before execution | false |

### Environment Variables

You can override configuration values using environment variables:

- `AIRUN_CONFIG` - Path to custom config file
- `OPENAI_API_KEY` - OpenAI API key
- `ANTHROPIC_API_KEY` - Anthropic API key

## Safety Features

airun includes a safety filter that checks generated commands against a configurable blocklist. Commands containing blocked patterns will not be executed.

The blocklist uses case-insensitive prefix matching on command tokens. For example, blocking `rm` will prevent execution of commands containing `rm`, `rm -rf`, etc.

**Note:** The safety filter is a helpful safeguard but not foolproof. Always review generated commands, especially when `show_command` is enabled.

## Architecture

```
airun
├── src/
│   ├── main.cpp                 # Entry point and orchestration
│   ├── config.cpp/h             # TOML configuration loading
│   ├── ai_client.h              # Abstract AI client interface
│   ├── openai_client.cpp/h      # OpenAI API implementation
│   ├── anthropic_client.cpp/h   # Anthropic API implementation
│   ├── prompt_builder.cpp/h     # System prompt construction
│   ├── safety_filter.cpp/h      # Command safety validation
│   └── command_executor.cpp/h   # Shell command execution
├── third_party/                 # Vendored dependencies
├── config/                      # Example configuration files
└── CMakeLists.txt
```

## Dependencies

- **libcurl** - HTTP client for AI API requests
- **nlohmann/json** - JSON parsing (header-only, vendored)
- **toml11** - TOML configuration parsing (header-only, vendored)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Uses [nlohmann/json](https://github.com/nlohmann/json) for JSON parsing
- Uses [toml11](https://github.com/ToruNiina/toml11) for TOML parsing
- Powered by OpenAI and Anthropic AI models

## Support

If you encounter any issues or have questions, please [open an issue](https://github.com/xlbbb-cn/ai-run/issues) on GitHub.

---

**Warning:** This tool executes shell commands on your system. Always review generated commands when possible, and use the safety features (blocklist, confirmation) to prevent unintended actions.
