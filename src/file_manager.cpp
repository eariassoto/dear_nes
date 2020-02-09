#include <fstream>
#include <sstream>
#include <helpers/RootDir.h>

#include "include/file_manager.h"
#include "include/logger.h"

namespace cpuemulator {
std::string FileManager::ReadShader(const std::string& filename) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::stringstream file_stream;
    try {
        file.open(ROOT_DIR "res/shaders/" + filename);
        file_stream << file.rdbuf();
        file.close();
    } catch (std::ifstream::failure e) {
        Logger::Get().Log("FILEMAN", "Error reading shader file {}!", filename);
    }
    return file_stream.str();
}

}  // namespace cpuemulator