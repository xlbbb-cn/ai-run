cmake_minimum_required(VERSION 3.15)
project(airun VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies ────────────────────────────────────────────────────────────
find_package(CURL REQUIRED)

# ── Sources ─────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/config.cpp
    src/openai_client.cpp
    src/anthropic_client.cpp
    src/safety_filter.cpp
    src/command_executor.cpp
    src/prompt_builder.cpp
)

add_executable(airun ${SOURCES})

target_include_directories(airun PRIVATE
    src/
    third_party/
)

target_link_libraries(airun PRIVATE CURL::libcurl)

target_compile_options(airun PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
    $<$<CONFIG:Release>:-O2 -Wall>
)

# ── Install ──────────────────────────────────────────────────────────────────
install(TARGETS airun DESTINATION bin)
install(FILES config/airun.toml.example config/blocklist.txt.example
        DESTINATION share/airun)
