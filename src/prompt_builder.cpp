#include "prompt_builder.h"
#include <sstream>
#include <string>

#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#include <sys/utsname.h>
#endif

#if defined(__linux__)
#include <fstream>
#endif

namespace {

#if defined(__linux__)
std::string strip_quotes(const std::string& value) {
    if (value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\''))) {
        return value.substr(1, value.size() - 2);
    }

    return value;
}
#endif

#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
std::string get_uname_info() {
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        return "";
    }

    std::ostringstream oss;
    oss << buffer.release << " (" << buffer.machine << ")";
    return oss.str();
}
#endif

#if defined(__linux__)
std::string get_linux_distribution() {
    std::ifstream os_release("/etc/os-release");
    if (!os_release) {
        return "";
    }

    std::string line;
    std::string name;
    std::string version_id;

    while (std::getline(os_release, line)) {
        if (line.rfind("PRETTY_NAME=", 0) == 0) {
            return strip_quotes(line.substr(12));
        }

        if (line.rfind("NAME=", 0) == 0) {
            name = strip_quotes(line.substr(5));
            continue;
        }

        if (line.rfind("VERSION_ID=", 0) == 0) {
            version_id = strip_quotes(line.substr(11));
        }
    }

    if (!name.empty() && !version_id.empty()) {
        return name + " " + version_id;
    }

    return name;
}
#endif

std::string get_os_info() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    const std::string uname_info = get_uname_info();
    return uname_info.empty() ? "macOS" : "macOS " + uname_info;
#elif defined(__linux__)
    const std::string distro = get_linux_distribution();
    const std::string uname_info = get_uname_info();

    if (!distro.empty() && !uname_info.empty()) {
        return distro + " / Linux " + uname_info;
    }

    if (!distro.empty()) {
        return distro;
    }

    return uname_info.empty() ? "Linux" : "Linux " + uname_info;
#elif defined(__unix__)
    const std::string uname_info = get_uname_info();
    return uname_info.empty() ? "Unix" : "Unix " + uname_info;
#else
    return "Unknown OS";
#endif
}

std::string get_shell_name() {
#if defined(_WIN32)
    return "PowerShell";
#else
    return "POSIX shell";
#endif
}

}  // namespace

std::string PromptBuilder::system_prompt() {
    std::string os_info = get_os_info();
    std::string shell_name = get_shell_name();

    std::ostringstream prompt;
    prompt << "You are a command-line expert assistant.\n"
           << "The user will describe a task in natural language (possibly in Chinese or English).\n"
           << "You must respond with ONLY a single, complete command that matches the current operating system.\n"
           << "\n"
           << "Operating System: " << os_info << "\n"
           << "Preferred Shell: " << shell_name << "\n"
           << "\n"
           << "Rules:\n"
           << "- Output ONLY the raw shell command. No explanation, no markdown, no code fences, no extra text.\n"
           << "- The command must be a single line, suitable for passing directly to the current shell.\n"
           << "- Use commands and syntax native to the current operating system.\n"
           << "- On macOS and Linux, prefer standard POSIX utilities (sh, bash, coreutils, awk, sed, grep, find, du, df, etc.).\n"
           << "- On Windows, prefer PowerShell cmdlets and native commands.\n"
           << "- If the task is ambiguous, choose the most common/safe interpretation.\n"
           << "- Do not include `sudo` or any command that requires root unless absolutely necessary and stated by the user.\n"
           << "- Do not include destructive commands like `rm -rf`, `mkfs`, `dd` without explicit user instruction.\n";

    return prompt.str();
}

std::string PromptBuilder::user_message(const std::string& query) {
    return "Task: " + query;
}
