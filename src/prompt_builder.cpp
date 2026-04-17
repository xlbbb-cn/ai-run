#include "prompt_builder.h"
#include <sys/utsname.h>
#include <sstream>

static std::string get_os_info() {
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        return "Linux";  // fallback
    }

    std::ostringstream oss;
    oss << buffer.sysname << " " << buffer.release << " (" << buffer.machine << ")";
    return oss.str();
}

std::string PromptBuilder::system_prompt() {
    std::string os_info = get_os_info();

    std::ostringstream prompt;
    prompt << "You are a Linux command-line expert assistant.\n"
           << "The user will describe a task in natural language (possibly in Chinese or English).\n"
           << "You must respond with ONLY a single, complete Linux shell command that accomplishes the task.\n"
           << "\n"
           << "Operating System: " << os_info << "\n"
           << "\n"
           << "Rules:\n"
           << "- Output ONLY the raw shell command. No explanation, no markdown, no code fences, no extra text.\n"
           << "- The command must be a single line, suitable for passing directly to `sh -c`.\n"
           << "- Use standard Linux utilities (bash, coreutils, awk, sed, grep, find, du, df, etc.).\n"
           << "- If the task is ambiguous, choose the most common/safe interpretation.\n"
           << "- Do not include `sudo` or any command that requires root unless absolutely necessary and stated by the user.\n"
           << "- Do not include destructive commands like `rm -rf`, `mkfs`, `dd` without explicit user instruction.\n";

    return prompt.str();
}

std::string PromptBuilder::user_message(const std::string& query) {
    return "Task: " + query;
}
