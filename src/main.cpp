// =============================================================================
// FILE: src/main.cpp
// PURPOSE: Game entry point
// =============================================================================

#include "Engine/Core/Engine.h"
#include "Engine/Core/Logger.h"
#include "Game/ApocalypseGame.h"
#include <iostream>
#include <filesystem>

using namespace Duality;

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "DUALITY: APOCALYPSE" << std::endl;
    std::cout << "Cosmic Alliance Simulation v1.0.0" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        // Create logs directory
        std::filesystem::create_directories("logs");
        
        // Configure logging
        LogConfig logConfig;
        logConfig.level = LogLevel::Info;
        logConfig.logToFile = true;
        logConfig.logToConsole = true;
        logConfig.logPath = "logs/";
        Logger::Initialize(logConfig);
        
        LOG_INFO("Game starting...");
        
        // Create engine configuration
        EngineConfig engineConfig;
        engineConfig.windowWidth = 1920;
        engineConfig.windowHeight = 1080;
        engineConfig.windowTitle = "Duality: Apocalypse";
        engineConfig.fullscreen = false;
        engineConfig.vsync = true;
        engineConfig.enableRayTracing = false; // Set to true if RTX available
        engineConfig.targetFPS = 144;
        
        // Parse command line arguments
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--fullscreen") {
                engineConfig.fullscreen = true;
            } else if (arg == "--windowed") {
                engineConfig.fullscreen = false;
            } else if (arg == "--novsync") {
                engineConfig.vsync = false;
            } else if (arg == "--width" && i + 1 < argc) {
                engineConfig.windowWidth = std::stoi(argv[++i]);
            } else if (arg == "--height" && i + 1 < argc) {
                engineConfig.windowHeight = std::stoi(argv[++i]);
            } else if (arg == "--fps" && i + 1 < argc) {
                engineConfig.targetFPS = std::stoi(argv[++i]);
            } else if (arg == "--help") {
                std::cout << "\nUsage: DualityApocalypse [options]\n"
                          << "Options:\n"
                          << "  --fullscreen    Start in fullscreen mode\n"
                          << "  --windowed      Start in windowed mode\n"
                          << "  --novsync       Disable VSync\n"
                          << "  --width N       Set window width\n"
                          << "  --height N      Set window height\n"
                          << "  --fps N         Set target FPS\n"
                          << "  --help          Show this help\n";
                return 0;
            }
        }
        
        // Create and initialize engine
        Engine engine(engineConfig);
        if (!engine.Initialize()) {
            LOG_ERROR("Failed to initialize engine");
            return -1;
        }
        
        // Create and run game
        ApocalypseGame game;
        engine.SetGameInstance(&game);
        
        LOG_INFO("Entering main game loop...");
        engine.Run();
        
        LOG_INFO("Game shutting down...");
        engine.Shutdown();
        Logger::Shutdown();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        
        try {
            Logger::Critical("Fatal error: {}", e.what());
            Logger::Shutdown();
        } catch (...) {}
        
        return -1;
    }
}