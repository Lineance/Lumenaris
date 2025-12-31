#include "Core/Logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace Core
{

    Logger::Logger()
        : m_running(false), m_asyncMode(true), m_consoleOutput(true),
          m_minLevel(LogLevel::DEBUG), m_initialized(false),
          m_lastStatsTime(std::chrono::steady_clock::now())
    {
    }

    Logger &Logger::GetInstance()
    {
        static Logger instance;
        return instance;
    }

    void Logger::Initialize(const std::string &logFilePath, bool consoleOutput, LogLevel minLevel,
                            bool async, const LogRotationConfig &rotationConfig)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);

        if (m_initialized)
        {
            return; // 已经初始化过了
        }

        m_baseFilePath = logFilePath;
        m_consoleOutput = consoleOutput;
        m_minLevel = minLevel;
        m_asyncMode = async;
        m_rotationConfig = rotationConfig;
        m_lastRotationTime = std::chrono::system_clock::now();

        // 确保日志目录存在
        EnsureLogDirectoryExists(logFilePath);

        // 打开日志文件
        m_logFile.open(logFilePath, std::ios::out | std::ios::app);
        if (!m_logFile.is_open())
        {
            if (m_consoleOutput)
            {
                std::cerr << "[ERROR] Failed to open log file: " << logFilePath << std::endl;
            }
            return;
        }

        // 如果启用异步模式，启动写入线程
        if (m_asyncMode)
        {
            m_running = true;
            m_writeThread = std::thread(&Logger::AsyncWriteThread, this);
        }

        m_initialized = true;
    }

    void Logger::SetMinLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        m_minLevel = level;
    }

    void Logger::SetConsoleOutput(bool enabled)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        m_consoleOutput = enabled;
    }

#if LOG_DEBUG_ENABLED
    void Logger::Debug(const std::string &message)
    {
        if (m_asyncMode)
        {
            LogAsync(LogLevel::DEBUG, message);
        }
        else
        {
            Log(LogLevel::DEBUG, message);
        }
    }
#endif

    void Logger::Info(const std::string &message)
    {
        if (m_asyncMode)
        {
            LogAsync(LogLevel::INFO, message);
        }
        else
        {
            Log(LogLevel::INFO, message);
        }
    }

    void Logger::Warning(const std::string &message)
    {
        if (m_asyncMode)
        {
            LogAsync(LogLevel::WARNING, message);
        }
        else
        {
            Log(LogLevel::WARNING, message);
        }
    }

    void Logger::Error(const std::string &message)
    {
        if (m_asyncMode)
        {
            LogAsync(LogLevel::ERROR, message);
        }
        else
        {
            Log(LogLevel::ERROR, message);
        }
    }

    void Logger::Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_configMutex);

        if (!m_initialized)
        {
            return;
        }

        // 停止异步写入线程
        if (m_asyncMode && m_running)
        {
            m_running = false;
            m_queueCondition.notify_one(); // 唤醒等待的线程

            if (m_writeThread.joinable())
            {
                m_writeThread.join();
            }
        }

        // 确保所有队列中的日志都被写入
        if (m_asyncMode && !m_logQueue.empty())
        {
            std::queue<std::unique_ptr<LogEntry>> tempQueue;
            {
                std::lock_guard<std::mutex> queueLock(m_queueMutex);
                std::swap(tempQueue, m_logQueue);
            }

            while (!tempQueue.empty())
            {
                WriteLogEntry(*tempQueue.front());
                tempQueue.pop();
            }
        }

        if (m_logFile.is_open())
        {
            // 直接写入日志而不调用Log方法，避免递归调用
            std::string timestamp = GetTimestamp();
            std::string formattedMessage = "[" + timestamp + "] [INFO] Logger shutting down";
            m_logFile << formattedMessage << std::endl;
            m_logFile.flush();
            m_logFile.close();
        }

        m_initialized = false;
    }

    Logger::~Logger()
    {
        Shutdown();
    }

    void Logger::Log(LogLevel level, const std::string &message)
    {
        // 检查是否已初始化
        if (!m_initialized)
        {
            return;
        }

        // 检查日志级别
        if (static_cast<int>(level) < static_cast<int>(m_minLevel))
        {
            return;
        }

        // 格式化日志消息（包含上下文）
        std::string formattedMessage = FormatMessageWithContext(level, message);

        // 创建日志条目
        std::string timestamp = GetTimestamp();
        std::string levelStr = LevelToString(level);
        LogEntry entry(level, timestamp, levelStr, message, formattedMessage);

        // 写入日志
        WriteLogEntry(entry);
    }

    void Logger::LogAsync(LogLevel level, const std::string &message)
    {
        // 检查是否已初始化
        if (!m_initialized)
        {
            return;
        }

        // 检查日志级别
        if (static_cast<int>(level) < static_cast<int>(m_minLevel))
        {
            return;
        }

        // 创建日志条目并放入队列（不在主线程格式化，让写入线程处理以确保上下文正确）
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_logQueue.push(std::make_unique<LogEntry>(level, "", "", message, ""));
        }

        // 通知写入线程
        m_queueCondition.notify_one();
    }

    void Logger::AsyncWriteThread()
    {
        while (m_running)
        {
            std::unique_ptr<LogEntry> entry;

            // 从队列获取日志条目
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCondition.wait(lock, [this]()
                                      { return !m_running || !m_logQueue.empty(); });

                if (!m_running && m_logQueue.empty())
                {
                    break;
                }

                if (!m_logQueue.empty())
                {
                    entry = std::move(m_logQueue.front());
                    m_logQueue.pop();
                }
            }

            // 写入日志条目
            if (entry)
            {
                WriteLogEntry(*entry);
            }
        }
    }

    void Logger::WriteLogEntry(const LogEntry &entry)
    {
        // DEBUG 级别的日志完全跳过，不写入文件也不输出到控制台
        if (entry.level == LogLevel::DEBUG)
        {
            return;
        }

        // 检查轮转
        CheckRotation();

        // 如果消息还没有格式化（异步模式），现在格式化
        std::string finalMessage = entry.formattedMessage;
        if (finalMessage.empty()) {
            finalMessage = FormatMessageWithContext(entry.level, entry.message);
        }

        // 输出到文件
        if (m_logFile.is_open())
        {
            m_logFile << finalMessage << std::endl;
            m_logFile.flush();
        }

        // 输出到控制台
        if (m_consoleOutput)
        {
            std::ostream &stream = (entry.level == LogLevel::ERROR) ? std::cerr : std::cout;
            stream << finalMessage << std::endl;
        }
    }

    void Logger::FlushLogFile()
    {
        if (m_logFile.is_open())
        {
            m_logFile.flush();
        }
    }

    std::string Logger::GetTimestamp() const
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
           << "." << std::setfill('0') << std::setw(3) << ms.count();

        return ss.str();
    }

    std::string Logger::LevelToString(LogLevel level) const
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    void Logger::CheckRotation()
    {
        if (m_rotationConfig.type == RotationType::NONE)
        {
            return;
        }

        bool shouldRotate = false;

        if (m_rotationConfig.type == RotationType::SIZE)
        {
            // 检查文件大小
            if (m_logFile.is_open())
            {
                auto currentPos = m_logFile.tellp();
                if (static_cast<size_t>(currentPos) >= m_rotationConfig.maxFileSize)
                {
                    shouldRotate = true;
                }
            }
        }
        else
        {
            // 检查时间
            auto now = std::chrono::system_clock::now();
            auto timeDiff = now - m_lastRotationTime;

            if (m_rotationConfig.type == RotationType::DAILY)
            {
                shouldRotate = timeDiff >= std::chrono::hours(24);
            }
            else if (m_rotationConfig.type == RotationType::HOURLY)
            {
                shouldRotate = timeDiff >= std::chrono::hours(1);
            }
        }

        if (shouldRotate)
        {
            RotateLogFile();
        }
    }

    void Logger::RotateLogFile()
    {
        if (!m_logFile.is_open())
        {
            return;
        }

        // 关闭当前文件
        m_logFile.close();

        // 重命名现有文件
        try
        {
            // 删除最旧的文件（如果超过最大数量）
            for (int i = m_rotationConfig.maxFiles; i >= 1; --i)
            {
                std::string oldFile = GenerateRotatedFileName(i);
                if (fs::exists(oldFile))
                {
                    if (i == m_rotationConfig.maxFiles)
                    {
                        fs::remove(oldFile);
                    }
                    else
                    {
                        std::string newFile = GenerateRotatedFileName(i + 1);
                        fs::rename(oldFile, newFile);
                    }
                }
            }

            // 重命名当前文件为.1
            if (fs::exists(m_baseFilePath))
            {
                std::string rotatedFile = GenerateRotatedFileName(1);
                fs::rename(m_baseFilePath, rotatedFile);
            }

            // 重新打开日志文件
            m_logFile.open(m_baseFilePath, std::ios::out | std::ios::app);
            if (!m_logFile.is_open())
            {
                if (m_consoleOutput)
                {
                    std::cerr << "[ERROR] Failed to reopen log file after rotation: " << m_baseFilePath << std::endl;
                }
            }

            m_lastRotationTime = std::chrono::system_clock::now();
        }
        catch (const std::exception &e)
        {
            if (m_consoleOutput)
            {
                std::cerr << "[ERROR] Failed to rotate log file: " << e.what() << std::endl;
            }
            // 尝试重新打开原文件
            m_logFile.open(m_baseFilePath, std::ios::out | std::ios::app);
        }
    }

    std::string Logger::GenerateRotatedFileName(int index) const
    {
        fs::path basePath(m_baseFilePath);
        std::string stem = basePath.stem().string();
        std::string extension = basePath.extension().string();

        return basePath.parent_path().string() + "/" + stem + "." + std::to_string(index) + extension;
    }

    std::string Logger::FormatMessageWithContext(LogLevel level, const std::string &message)
    {
        std::string timestamp = GetTimestamp();
        std::string levelStr = LevelToString(level);

        std::string contextStr;
        if (!m_contextStack.empty())
        {
            const auto &context = m_contextStack.back();
            if (!context.renderPass.empty())
            {
                contextStr += "[" + context.renderPass + "]";
            }
            if (context.batchIndex >= 0)
            {
                contextStr += " Batch:" + std::to_string(context.batchIndex);
            }
            if (context.triangleCount > 0)
            {
                contextStr += " Tri:" + std::to_string(context.triangleCount) + "k";
            }
            if (context.drawCallCount > 0)
            {
                contextStr += " DrawCalls:" + std::to_string(context.drawCallCount);
            }
        }
        else if (level == LogLevel::INFO && message.find("Render pass") != std::string::npos)
        {
            // 调试：上下文栈为空
            contextStr += "[DEBUG: No context]";
        }

        std::string result = "[" + timestamp + "] [" + levelStr + "]";
        if (!contextStr.empty())
        {
            result += contextStr;
        }
        result += " " + message;

        return result;
    }

    bool Logger::ShouldLogStatistics()
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastStatsTime).count();
        return elapsed >= 5; // 每5秒输出一次统计
    }

    void Logger::SetContext(const LogContext &context)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        if (m_contextStack.empty())
        {
            m_contextStack.push_back(context);
        }
        else
        {
            m_contextStack.back() = context;
        }
    }

    const LogContext &Logger::GetContext() const
    {
        static LogContext emptyContext;
        return m_contextStack.empty() ? emptyContext : m_contextStack.back();
    }

    void Logger::PushContext(const LogContext &context)
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        m_contextStack.push_back(context);
    }

    void Logger::PopContext()
    {
        std::lock_guard<std::mutex> lock(m_configMutex);
        if (!m_contextStack.empty())
        {
            m_contextStack.pop_back();
        }
    }

    void Logger::LogShaderActivation(unsigned int shaderId)
    {
        m_statistics.shaderActivations++;
        // DEBUG 级别的日志已被删除
    }

    void Logger::LogTextureBind(unsigned int textureId)
    {
        m_statistics.textureBinds++;
        // 纹理绑定通常过于频繁，减少日志输出
    }

    void Logger::LogDrawCall(int triangleCount)
    {
        m_statistics.drawCalls++;
        m_statistics.meshRenders++;
        // DrawCall通常也比较频繁，使用统计摘要
    }

    void Logger::LogStatisticsSummary()
    {
        if (!ShouldLogStatistics())
        {
            return;
        }

        auto fps = m_frameCount.load();
        auto shaderCount = m_statistics.shaderActivations.load();
        auto drawCalls = m_statistics.drawCalls.load();
        auto meshRenders = m_statistics.meshRenders.load();

        std::string summary = "FrameSummary: FPS=" + std::to_string(fps) +
                              ", Shaders=" + std::to_string(shaderCount) +
                              ", DrawCalls=" + std::to_string(drawCalls) +
                              ", Meshes=" + std::to_string(meshRenders);

        Info(summary);

        // 重置统计数据
        ResetStatistics();
        m_frameCount = 0;
        m_lastStatsTime = std::chrono::steady_clock::now();
    }

    void Logger::ResetStatistics()
    {
        m_statistics.Reset();
    }

    void Logger::SetFPS(int fps)
    {
        m_frameCount = fps;
    }

    const LogStatistics &Logger::GetStatistics() const
    {
        return m_statistics;
    }

    int Logger::GetFPS() const
    {
        return m_frameCount.load();
    }

    void Logger::EnsureLogDirectoryExists(const std::string &filePath)
    {
        try
        {
            fs::path logPath(filePath);
            fs::path dirPath = logPath.parent_path();

            if (!dirPath.empty() && !fs::exists(dirPath))
            {
                fs::create_directories(dirPath);
            }
        }
        catch (const std::exception &e)
        {
            if (m_consoleOutput)
            {
                std::cerr << "[ERROR] Failed to create log directory: " << e.what() << std::endl;
            }
        }
    }

} // namespace Core
