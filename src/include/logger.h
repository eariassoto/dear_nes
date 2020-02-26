// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <fmt/core.h>

#include <iostream>
#include <mutex>
#include <queue>
#include <string>

namespace cpuemulator {

class Logger {
   public:
    template <typename... Ts>
    void Log(const std::string& module, const std::string& formatString,
             Ts&&... args);

    static Logger& Get();

    void Start();
    void Stop();

   private:
    std::mutex m_QueueMutex;
    std::queue<std::string> m_EventQueue;
    bool m_StopSignal = false;

    static constexpr char MSG_FMT[] = "[{}]: ";

    void ProcessorThread();
};
template <typename... Ts>
inline void Logger::Log(const std::string& module,
                        const std::string& formatString, Ts&&... args) {
    std::string message =
        fmt::format(MSG_FMT + formatString, module, std::forward<Ts>(args)...);
    std::lock_guard<std::mutex> guard(m_QueueMutex);
    m_EventQueue.emplace(std::move(message));
}

}  // namespace cpuemulator
