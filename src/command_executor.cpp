#include "command_executor.h"
#include <stdexcept>
#include <array>
#include <cstdio>
#include <iostream>

ExecResult CommandExecutor::run(const std::string& cmd, bool stream_output) {
    ExecResult result;

    // Redirect stderr to stdout so we capture both
    std::string full_cmd = cmd + " 2>&1";

    FILE* pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed for command: " + cmd);
    }

    std::array<char, 4096> buf{};
    while (fgets(buf.data(), buf.size(), pipe) != nullptr) {
        std::string line(buf.data());
        result.output += line;
        if (stream_output) {
            std::cout << line << std::flush;
        }
    }

    int status = pclose(pipe);
    // WIFEXITED / WEXITSTATUS are POSIX
    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
    } else {
        result.exit_code = -1;
    }

    return result;
}
