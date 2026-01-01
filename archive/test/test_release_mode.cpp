#include "Core/Logger.hpp"
#include <iostream>

int main() {
    std::cout << "Testing Release Mode Logger Behavior..." << std::endl;
    std::cout << "LOG_DEBUG_ENABLED = " << LOG_DEBUG_ENABLED << std::endl;

    // 初始化Logger（Release模式）
    Core::LogRotationConfig rotationConfig;
    rotationConfig.type = Core::RotationType::SIZE;
    rotationConfig.maxFileSize = 1024; // 1KB for testing
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize("logs/release_test.log", true, Core::LogLevel::DEBUG,
                                           true, rotationConfig);

    // 测试不同级别的日志
    std::cout << "\n--- Testing different log levels ---" << std::endl;

    // DEBUG日志 - 在Release模式下应该被完全移除
    std::cout << "Calling Debug() - should be no-op in Release mode" << std::endl;
    Core::Logger::GetInstance().Debug("This DEBUG message should NOT appear in Release builds");
    Core::Logger::GetInstance().Debug("Detailed rendering pipeline info");
    Core::Logger::GetInstance().Debug("Memory allocation tracking");

    // INFO日志 - 应该正常工作
    std::cout << "Calling Info() - should work in all modes" << std::endl;
    Core::Logger::GetInstance().Info("This INFO message should appear in all builds");

    // WARNING日志 - 应该正常工作
    std::cout << "Calling Warning() - should work in all modes" << std::endl;
    Core::Logger::GetInstance().Warning("This WARNING message should appear in all builds");

    // ERROR日志 - 应该正常工作
    std::cout << "Calling Error() - should work in all modes" << std::endl;
    Core::Logger::GetInstance().Error("This ERROR message should appear in all builds");

    // 测试上下文感知功能
    std::cout << "\n--- Testing contextual logging ---" << std::endl;
    Core::LogContext context;
    context.renderPass = "MainRender";
    context.batchIndex = 1;
    context.triangleCount = 1500;
    Core::Logger::GetInstance().SetContext(context);

    Core::Logger::GetInstance().Info("Contextual render operation");
    Core::Logger::GetInstance().LogShaderActivation(5);
    Core::Logger::GetInstance().LogDrawCall(500);

    // 等待异步日志处理
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 关闭Logger
    Core::Logger::GetInstance().Shutdown();

    std::cout << "\nRelease mode test completed." << std::endl;
    std::cout << "Check logs/release_test.log for results." << std::endl;
    std::cout << "DEBUG logs should be completely absent in Release mode!" << std::endl;

    return 0;
}
