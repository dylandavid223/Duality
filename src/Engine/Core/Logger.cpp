// =============================================================================
// FILE: src/Engine/Core/Logger.cpp
// PURPOSE: Logger implementation
// =============================================================================

#include "Logger.h"
#include <iostream>
#include <filesystem>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Duality {

void Logger::Initialize(const LogConfig& config) {
    s_config = config;
    s_currentLevel.store(config.level);
    s_running = true;
    
    // Create log directory
    std::filesystem::create_directories(config.logPath);
    
    // Open log file
    if (config.logToFile) {
        std::string filename = config.logPath + "apocalypse_" + GetTimestampString() + ".log";
        s_fileStream.open(filename, std::ios::out | std::ios::app);
        if (!s_fileStream.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }
    
    // Start worker thread
    if (config.asyncLogging) {
        s_workerThread = std::thread(WorkerThread);
    }
    
    Info("Logger initialized. Level: {}", LevelToString(config.level));
}

void Logger::Shutdown() {
    Info("Logger shutting down...");
    
    s_running = false;
    s_cv.notify_all();
    
    if (s_workerThread.joinable()) {
        s_workerThread.join();
    }
    
    Flush();
    
    if (s_fileStream.is_open()) {
        s_fileStream.close();
    }
}

LogLevel Logger::LevelFromString(const std::string& str) {
    std::string lower = ToLower(str);
    if (lower == "trace") return LogLevel::Trace;
    if (lower == "debug") return LogLevel::Debug;
    if (lower == "info") return LogLevel::Info;
    if (lower == "warn" || lower == "warning") return LogLevel::Warning;
    if (lower == "error") return LogLevel::Error;
    if (lower == "critical") return LogLevel::Critical;
    if (lower == "off") return LogLevel::Off;
    return LogLevel::Info;
}

void Logger::Log(LogLevel level, const std::string& message,
                const std::source_location& loc) {
    if (level < s_currentLevel.load()) return;
    
    LogEntry entry{
        .level = level,
        .message = message,
        .timestamp = std::chrono::high_resolution_clock::now(),
        .threadId = std::this_thread::get_id(),
        .location = loc
    };
    
    if (s_config.asyncLogging && s_running) {
        std::lock_guard<std::mutex> lock(s_queueMutex);
        s_queue.push(std::move(entry));
        s_cv.notify_one();
    } else {
        WriteToConsole(entry);
        if (s_config.logToFile) WriteToFile(entry);
    }
    
    if (s_config.flushOnError && level >= LogLevel::Error) {
        Flush();
    }
}

void Logger::Flush() {
    if (s_config.asyncLogging) {
        std::unique_lock<std::mutex> lock(s_queueMutex);
        while (!s_queue.empty()) {
            auto entry = std::move(s_queue.front());
            s_queue.pop();
            lock.unlock();
            
            WriteToConsole(entry);
            if (s_config.logToFile) WriteToFile(entry);
            
            lock.lock();
        }
    }
    
    if (s_fileStream.is_open()) {
        s_fileStream.flush();
    }
}

void Logger::SetLevel(LogLevel level) {
    s_currentLevel.store(level);
    Info("Log level changed to: {}", LevelToString(level));
}

void Logger::WorkerThread() {
    while (s_running) {
        std::unique_lock<std::mutex> lock(s_queueMutex);
        s_cv.wait(lock, [] { return !s_queue.empty() || !s_running; });
        
        while (!s_queue.empty()) {
            auto entry = std::move(s_queue.front());
            s_queue.pop();
            lock.unlock();
            
            WriteToConsole(entry);
            if (s_config.logToFile) WriteToFile(entry);
            
            lock.lock();
        }
    }
}

void Logger::WriteToConsole(const LogEntry& entry) {
    if (!s_config.logToConsole) return;
    
    std::ostringstream oss;
    
    if (s_config.showTimestamp) {
        auto time = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now());
        oss << std::put_time(std::localtime(&time), "%H:%M:%S") << " ";
    }
    
    if (s_config.showThreadId) {
        oss << "[T" << entry.threadId << "] ";
    }
    
    oss << "[" << LevelToString(entry.level) << "] "
        << entry.message;
    
    if (s_config.showSourceLocation && entry.level >= LogLevel::Warning) {
        oss << " [" << entry.location.file_name() << ":"
            << entry.location.line() << "]";
    }
    
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    std::string color = GetColorCode(entry.level);
    SetConsoleTextAttribute(hConsole, std::stoi(color));
    std::cout << oss.str() << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
#else
    std::cout << GetColorCode(entry.level) << oss.str() << "\033[0m" << std::endl;
#endif
}

void Logger::WriteToFile(const LogEntry& entry) {
    if (!s_fileStream.is_open()) return;
    
    std::lock_guard<std::mutex> lock(s_fileMutex);
    
    auto time = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    
    s_fileStream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                 << " [T" << entry.threadId << "] "
                 << "[" << LevelToString(entry.level) << "] "
                 << entry.message << "\n";
    
    s_currentFileSize += entry.message.length();
    
    if (s_currentFileSize >= s_config.maxFileSize) {
        RotateLogFile();
    }
}

void Logger::RotateLogFile() {
    s_fileStream.close();
    s_currentFileSize = 0;
    
    // Rotate old files
    for (u32 i = s_config.maxFiles; i > 0; --i) {
        std::string oldName = s_config.logPath + "apocalypse_" + 
            std::to_string(i - 1) + ".log";
        std::string newName = s_config.logPath + "apocalypse_" + 
            std::to_string(i) + ".log";
        
        if (std::filesystem::exists(oldName)) {
            std::filesystem::rename(oldName, newName);
        }
    }
    
    // Open new file
    std::string filename = s_config.logPath + "apocalypse_0.log";
    s_fileStream.open(filename, std::ios::out | std::ios::app);
}

std::string Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::GetColorCode(LogLevel level) {
#ifdef _WIN32
    switch (level) {
        case LogLevel::Trace: return "8";
        case LogLevel::Debug: return "7";
        case LogLevel::Info: return "10";
        case LogLevel::Warning: return "14";
        case LogLevel::Error: return "12";
        case LogLevel::Critical: return "79";
        default: return "7";
    }
#else
    switch (level) {
        case LogLevel::Trace: return "\033[90m";
        case LogLevel::Debug: return "\033[37m";
        case LogLevel::Info: return "\033[32m";
        case LogLevel::Warning: return "\033[33m";
        case LogLevel::Error: return "\033[31m";
        case LogLevel::Critical: return "\033[41;37m";
        default: return "\033[0m";
    }
#endif
}

std::string Logger::GetTimestampString() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
    return ss.str();
}

} // namespace Duality