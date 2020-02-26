// Copyright (c) 2020 Emmanuel Arias
#include "include/logger.h"

#include <iostream>
#include <thread>

namespace cpuemulator {
Logger& Logger::Get() {
    static Logger logger;
    return logger;
}

void Logger::Start() {
    std::thread t(&Logger::ProcessorThread, this);
    t.detach();
}

void Logger::Stop() { m_StopSignal = true; }

void Logger::ProcessorThread() {
    while (!m_StopSignal) {
        {
            std::lock_guard<std::mutex> guard(m_QueueMutex);
            if (!m_EventQueue.empty()) {
                const std::string& msg = m_EventQueue.front();
                std::cout << msg << '\n';
                m_EventQueue.pop();
            }
        }
    }
}

}  // namespace cpuemulator