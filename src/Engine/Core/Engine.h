#pragma once
#include "Window.h"
#include "Types.h"
#include "Logger.h"

namespace Duality {

struct EngineConfig {
    u32 windowWidth = 1920;
    u32 windowHeight = 1080;
    std::string windowTitle = "Duality: Apocalypse";
    bool fullscreen = false;
    bool vsync = true;
    bool enableRayTracing = false;
    u32 targetFPS = 144;
};

class IGameInstance {
public:
    virtual ~IGameInstance() = default;
    virtual void Initialize() {}
    virtual void Shutdown() {}
    virtual void Update(float deltaTime) {}
    virtual void Render() {}
};

class Engine {
public:
    Engine(const EngineConfig& config) : m_config(config) {}
    ~Engine() = default;
    
    bool Initialize() {
        LOG_INFO("Engine initializing...");
        m_window = std::make_unique<Window>();
        WindowConfig winConfig;
        winConfig.width = m_config.windowWidth;
        winConfig.height = m_config.windowHeight;
        winConfig.title = m_config.windowTitle;
        winConfig.fullscreen = m_config.fullscreen;
        winConfig.vsync = m_config.vsync;
        
        if (!m_window->Initialize(winConfig)) {
            return false;
        }
        
        LOG_INFO("Engine initialized");
        return true;
    }
    
    void Shutdown() {
        LOG_INFO("Engine shutting down...");
        if (m_window) m_window->Shutdown();
    }
    
    void Run() {
        LOG_INFO("Starting main loop");
        m_running = true;
        
        while (m_running && !m_window->ShouldClose()) {
            float deltaTime = 1.0f / 60.0f; // Placeholder
            
            m_window->PollEvents();
            
            if (m_gameInstance) {
                m_gameInstance->Update(deltaTime);
                m_gameInstance->Render();
            }
            
            m_window->SwapBuffers();
            
            if (m_window->ShouldClose()) {
                m_running = false;
            }
        }
    }
    
    void SetGameInstance(IGameInstance* game) { m_gameInstance = game; }
    InputSystem* GetInput() { return m_window ? m_window->GetInput() : nullptr; }
    Window* GetWindow() { return m_window.get(); }
    
private:
    EngineConfig m_config;
    std::unique_ptr<Window> m_window;
    IGameInstance* m_gameInstance = nullptr;
    bool m_running = false;
};

} // namespace Duality