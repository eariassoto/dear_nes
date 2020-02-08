// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <fmt/core.h>
#include <iostream>
#include <string>

namespace cpuemulator {

class Logger {
   public:
    template <typename... Ts>
    void Log(const std::string& module, const std::string& formatString,
             Ts&&... args);

    static Logger& Get();
};
template <typename... Ts>
inline void Logger::Log(const std::string& module,
                        const std::string& formatString, Ts&&... args) {
    std::cout << '[' << module << "]: " << fmt::format(formatString, std::forward<Ts>(args)...) << '\n';
}

}  // namespace cpuemulator
