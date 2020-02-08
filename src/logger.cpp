#include "include/logger.h"

namespace cpuemulator {
Logger& Logger::Get() {
    static Logger logger;
    return logger;
}
}  // namespace cpuemulator