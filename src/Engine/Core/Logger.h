// =============================================================================
// FILE: src/Engine/Core/Logger.h
// PURPOSE: High-performance asynchronous logging
// =============================================================================

#pragma once

#include "Types.h"
#include <source_location>

namespace Duality {

enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Critical = 5,
    Off = 6
};

struct LogConfig {
    LogLevel level = LogLevel::Info;
    bool logToFile = true;
    bool logToConsole = true;
    std::string logPath = "logs/";
    bool asyncLogging = true;
    usize maxFileSize = 10 * Constants::MB;
    u32 maxFiles = 5;
    bool flushOnError = true;
    bool showTimestamp = true;
    bool showThreadId = true;
    bool showSourceLocation = false;
};

class Logger {
public:
    static void Initialize(const LogConfig& config);
    static void Shutdown();
    
    static LogLevel LevelFromString(const std::string& str);
    
    template<typename... Args>
    static void Trace(std::format_string<Args...> fmt, Args&&... args) {
        Log(LogLevel::Trace, std::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    static void Debug(std::format_string<Args...> fmt, Args&&... args) {
        Log(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    static void Info(std::format_string<Args...> fmt, Args&&... args) {
        Log(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    static void Warning(std::format_string<Args...> fmt, Args&&... args) {
        Log(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    static void Error(std::format_string<Args...> fmt, Args&&... args) {
        Log(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...));
    }
    
    template<typename... Args>
    static void Critical(std::format_string<Args...> fmt, Args&&... args) {
        Log(LogLevel::Critical, std::format(fmt, std::forward<Args>(args)...));
    }
    
    static void Flush();
    static void SetLevel(LogLevel level);
    
private:
    struct LogEntry {
        LogLevel level;
        std::string message;
        TimePoint timestamp;
        std::thread::id threadId;
        std::source_location location;
    };
    
    static void Log(LogLevel level, const std::string& message,
                   const std::source_location& loc = std::source_location::current());
    static void WorkerThread();
    static void WriteToConsole(const LogEntry& entry);
    static void WriteToFile(const LogEntry& entry);
    static void RotateLogFile();
    static std::string LevelToString(LogLevel level);
    static std::string GetColorCode(LogLevel level);
    static std::string GetTimestampString();
    
    inline static LogConfig s_config;
    inline static std::queue<LogEntry> s_queue;
    inline static std::mutex s_queueMutex;
    inline static std::condition_variable s_cv;
    inline static std::thread s_workerThread;
    inline static std::atomic<bool> s_running{false};
    inline static std::ofstream s_fileStream;
    inline static std::mutex s_fileMutex;
    inline static usize s_currentFileSize = 0;
    inline static std::atomic<LogLevel> s_currentLevel{LogLevel::Info};
    inline static u32 s_fileIndex = 0;
};

#define LOG_TRACE(...) Duality::Logger::Trace(__VA_ARGS__)
#define LOG_DEBUG(...) Duality::Logger::Debug(__VA_ARGS__)
#define LOG_INFO(...) Duality::Logger::Info(__VA_ARGS__)
#define LOG_WARN(...) Duality::Logger::Warning(__VA_ARGS__)
#define LOG_ERROR(...) Duality::Logger::Error(__VA_ARGS__)
#define LOG_CRITICAL(...) Duality::Logger::Critical(__VA_ARGS__)

} // namespace Duality