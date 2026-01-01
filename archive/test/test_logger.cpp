#include "Core/Logger.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    std::cout << "Testing Logger functionality..." << std::endl;

    // 测试1: 基本异步日志功能
    std::cout << "Test 1: Basic async logging" << std::endl;
    Core::LogRotationConfig rotationConfig;
    rotationConfig.type = Core::RotationType::NONE; // 不轮转

    Core::Logger::GetInstance().Initialize("logs/test.log", true, Core::LogLevel::DEBUG,
                                           true, rotationConfig);

    // 测试不同级别的日志
    Core::Logger::GetInstance().Debug("This is a debug message");
    Core::Logger::GetInstance().Info("This is an info message");
    Core::Logger::GetInstance().Warning("This is a warning message");
    Core::Logger::GetInstance().Error("This is an error message");

    // 等待异步写入完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 测试2: 日志轮转功能
    std::cout << "\nTest 2: Log rotation by size" << std::endl;
    Core::Logger::GetInstance().Shutdown();

    rotationConfig.type = Core::RotationType::SIZE;
    rotationConfig.maxFileSize = 1024; // 1KB
    rotationConfig.maxFiles = 3;

    Core::Logger::GetInstance().Initialize("logs/rotation_test.log", true, Core::LogLevel::INFO,
                                           true, rotationConfig);

    // 生成足够多的日志来触发轮转
    for (int i = 0; i < 100; ++i) {
        Core::Logger::GetInstance().Info("This is log entry number " + std::to_string(i) +
                                         ". This message is designed to be long enough to trigger log rotation. " +
                                         "We need to make sure the file size exceeds the limit.");
    }

    // 等待异步写入和轮转完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 测试3: 同步模式
    std::cout << "\nTest 3: Synchronous logging" << std::endl;
    Core::Logger::GetInstance().Shutdown();

    rotationConfig.type = Core::RotationType::NONE;
    Core::Logger::GetInstance().Initialize("logs/sync_test.log", true, Core::LogLevel::DEBUG,
                                           false, rotationConfig); // 同步模式

    Core::Logger::GetInstance().Info("This is a synchronous log message");
    Core::Logger::GetInstance().Debug("This debug message should appear in sync mode");

    // 关闭Logger
    Core::Logger::GetInstance().Shutdown();

    std::cout << "\nLogger tests completed. Check the following files:" << std::endl;
    std::cout << "- logs/test.log (async logging)" << std::endl;
    std::cout << "- logs/rotation_test.log* (rotation test)" << std::endl;
    std::cout << "- logs/sync_test.log (sync logging)" << std::endl;

    return 0;
}
