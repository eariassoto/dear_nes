// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>

namespace cpuemulator {
class FileManager {
   public:
    FileManager() = default;
    ~FileManager() = default;
    static std::string ReadShader(const std::string& filename);
};

}  // namespace cpuemulator
