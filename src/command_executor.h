#pragma once
#include <string>

struct ExecResult {
    std::string output;   // combined stdout + stderr
    int         exit_code = 0;
};

class CommandExecutor {
public:
    // Execute cmd with /bin/sh -c and return combined output + exit code.
    // Streams output to stdout in real time as well as capturing it.
    static ExecResult run(const std::string& cmd, bool stream_output = true);
};
