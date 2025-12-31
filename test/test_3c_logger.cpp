#include "Core/Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Testing 3C Logger Principles..." << std::endl;

    // 初始化Logger（启用异步和轮转）
    Core::LogRotationConfig rotationConfig;
    rotationConfig.type = Core::RotationType::SIZE;
    rotationConfig.maxFileSize = 1024; // 1KB for testing
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize("logs/3c_test.log", true, Core::LogLevel::INFO,
                                           true, rotationConfig);

    // 测试1: Contextual（上下文化）
    std::cout << "\n=== Test 1: Contextual Logging ===" << std::endl;

    // 设置渲染上下文
    Core::LogContext renderContext;
    renderContext.renderPass = "ShadowMap";
    renderContext.batchIndex = 1;
    renderContext.triangleCount = 4200;
    renderContext.drawCallCount = 12;
    renderContext.currentShader = "depth_shader";
    Core::Logger::GetInstance().SetContext(renderContext);

    // 记录上下文感知的日志
    Core::Logger::GetInstance().Info("Render pass started");
    Core::Logger::GetInstance().LogShaderActivation(3); // 着色器激活
    Core::Logger::GetInstance().LogDrawCall(1400);      // 绘制调用

    // 测试直接的上下文日志
    Core::Logger::GetInstance().Info("Context-aware render operation completed");

    // 测试2: Condensed（压缩）- 模拟高频事件
    std::cout << "\n=== Test 2: Condensed Logging ===" << std::endl;

    // 模拟多个渲染帧
    for (int frame = 0; frame < 10; ++frame) {
        // 更新上下文
        renderContext.batchIndex = frame;
        Core::Logger::GetInstance().SetContext(renderContext);

        // 模拟每帧的渲染活动
        Core::Logger::GetInstance().LogShaderActivation(3);
        Core::Logger::GetInstance().LogTextureBind(1);
        Core::Logger::GetInstance().LogDrawCall(1400);

        // 设置FPS
        Core::Logger::GetInstance().SetFPS(60);

        // 每帧等待一下，然后触发统计摘要
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 手动触发统计摘要
        if (frame == 4 || frame == 9) {
            // 强制输出统计摘要
            const auto& stats = Core::Logger::GetInstance().GetStatistics();
            auto fps = Core::Logger::GetInstance().GetFPS();
            auto shaderCount = stats.shaderActivations.load();
            auto drawCalls = stats.drawCalls.load();
            auto meshRenders = stats.meshRenders.load();

            std::string summary = "FrameSummary: FPS=" + std::to_string(fps) +
                                 ", Shaders=" + std::to_string(shaderCount) +
                                 ", DrawCalls=" + std::to_string(drawCalls) +
                                 ", Meshes=" + std::to_string(meshRenders);

            Core::Logger::GetInstance().Info(summary);
            Core::Logger::GetInstance().ResetStatistics();
        }
    }

    // 测试3: Critical（分级）- DEBUG日志控制
    std::cout << "\n=== Test 3: Critical Logging (Debug Level) ===" << std::endl;

    // 这些DEBUG日志在Release版本中会被完全编译掉
    Core::Logger::GetInstance().Debug("This DEBUG message should not appear in Release builds");
    Core::Logger::GetInstance().Debug("Detailed shader compilation info");
    Core::Logger::GetInstance().Debug("Memory allocation details");

    // INFO和WARNING日志正常显示
    Core::Logger::GetInstance().Info("This INFO message appears in all builds");
    Core::Logger::GetInstance().Warning("This WARNING message appears in all builds");

    // 等待异步日志处理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 关闭Logger
    Core::Logger::GetInstance().Shutdown();

    std::cout << "\n3C Logger tests completed. Check logs/3c_test.log* files." << std::endl;
    std::cout << "Notice how logs now include context information and statistics!" << std::endl;

    return 0;
}
