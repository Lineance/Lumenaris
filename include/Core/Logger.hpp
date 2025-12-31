#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <chrono>

// 编译时控制DEBUG日志输出
#if defined(NDEBUG) || defined(FORCE_RELEASE_MODE)
#define LOG_DEBUG_ENABLED 0
#else
#define LOG_DEBUG_ENABLED 1
#endif

// 编译时控制性能关键路径的日志
#ifndef ENABLE_RENDER_STATS
#define ENABLE_RENDER_STATS 0
#endif

namespace Core {

/**
 * @enum LogLevel
 * @brief 日志级别枚举
 */
enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

/**
 * @enum RotationType
 * @brief 日志轮转类型枚举
 */
enum class RotationType {
    NONE,       ///< 不轮转
    SIZE,       ///< 按文件大小轮转
    DAILY,      ///< 每日轮转
    HOURLY      ///< 每小时轮转
};

/**
 * @struct LogRotationConfig
 * @brief 日志轮转配置结构体
 */
struct LogRotationConfig {
    RotationType type = RotationType::NONE;    ///< 轮转类型
    size_t maxFileSize = 10 * 1024 * 1024;     ///< 最大文件大小（字节，默认10MB）
    int maxFiles = 5;                          ///< 最大历史文件数量
    bool compressOldLogs = false;              ///< 是否压缩旧日志文件
};

/**
 * @struct LogContext
 * @brief 日志上下文信息结构体
 */
struct LogContext {
    std::string renderPass;        ///< 当前渲染阶段 (e.g., "ShadowMap", "Forward", "Deferred")
    int batchIndex = -1;           ///< 当前批次索引
    int triangleCount = 0;         ///< 当前三角形数量
    int drawCallCount = 0;         ///< 当前DrawCall数量
    std::string currentShader;     ///< 当前着色器名称
    std::string currentMesh;       ///< 当前网格名称
};

/**
 * @struct LogStatistics
 * @brief 日志统计信息结构体
 */
struct LogStatistics {
    std::atomic<int> shaderActivations{0};     ///< 着色器激活次数
    std::atomic<int> textureBinds{0};          ///< 纹理绑定次数
    std::atomic<int> drawCalls{0};             ///< DrawCall次数
    std::atomic<int> meshRenders{0};           ///< 网格渲染次数
    std::atomic<int> fpsSamples{0};            ///< FPS采样次数
    std::atomic<double> totalFrameTime{0.0};   ///< 总帧时间

    void Reset() {
        shaderActivations = 0;
        textureBinds = 0;
        drawCalls = 0;
        meshRenders = 0;
        fpsSamples = 0;
        totalFrameTime = 0.0;
    }
};

/**
 * @struct LogEntry
 * @brief 日志条目结构体，用于异步队列
 */
struct LogEntry {
    LogLevel level;
    std::string timestamp;
    std::string levelStr;
    std::string message;
    std::string formattedMessage;

    LogEntry(LogLevel lvl, const std::string& ts, const std::string& lvlStr,
             const std::string& msg, const std::string& fmtMsg)
        : level(lvl), timestamp(ts), levelStr(lvlStr), message(msg), formattedMessage(fmtMsg) {}
};

/**
 * @class Logger
 * @brief 线程安全的日志系统，支持文件输出和控制台输出
 *
 * Logger类提供分级日志记录功能，支持DEBUG、INFO、WARNING、ERROR级别。
 * 可以配置输出到文件和控制台，包含时间戳格式化。
 */
class Logger {
public:
    /**
     * @brief 获取Logger单例实例
     * @return Logger实例的引用
     */
    static Logger& GetInstance();

    /**
     * @brief 初始化Logger
     * @param logFilePath 日志文件路径（可选）
     * @param consoleOutput 是否同时输出到控制台
     * @param minLevel 最小日志级别（低于此级别的日志将被忽略）
     * @param async 是否启用异步写入
     * @param rotationConfig 日志轮转配置
     */
    void Initialize(const std::string& logFilePath = "logs/application.log",
                   bool consoleOutput = true,
                   LogLevel minLevel = LogLevel::DEBUG,
                   bool async = true,
                   const LogRotationConfig& rotationConfig = LogRotationConfig());

    /**
     * @brief 设置最小日志级别
     * @param level 最小日志级别
     */
    void SetMinLevel(LogLevel level);

    /**
     * @brief 设置控制台输出开关
     * @param enabled 是否启用控制台输出
     */
    void SetConsoleOutput(bool enabled);

    /**
     * @brief 记录DEBUG级别日志（仅在Debug版本中输出）
     * @param message 日志消息
     */
#if LOG_DEBUG_ENABLED
    void Debug(const std::string& message);
#else
    inline void Debug(const std::string&) {} // Release版本中空实现
#endif

    /**
     * @brief 记录INFO级别日志
     * @param message 日志消息
     */
    void Info(const std::string& message);

    /**
     * @brief 记录WARNING级别日志
     * @param message 日志消息
     */
    void Warning(const std::string& message);

    /**
     * @brief 记录ERROR级别日志
     * @param message 日志消息
     */
    void Error(const std::string& message);

    /**
     * @brief 设置当前日志上下文
     * @param context 上下文信息
     */
    void SetContext(const LogContext& context);

    /**
     * @brief 获取当前日志上下文
     * @return 当前上下文信息
     */
    const LogContext& GetContext() const;

    /**
     * @brief 推送上下文（支持嵌套）
     * @param context 新的上下文信息
     */
    void PushContext(const LogContext& context);

    /**
     * @brief 弹出上下文
     */
    void PopContext();

    /**
     * @brief 记录上下文感知的着色器激活日志
     * @param shaderId 着色器ID
     */
    void LogShaderActivation(unsigned int shaderId);

    /**
     * @brief 记录上下文感知的纹理绑定日志
     * @param textureId 纹理ID
     */
    void LogTextureBind(unsigned int textureId);

    /**
     * @brief 记录上下文感知的绘制调用日志
     * @param triangleCount 三角形数量
     */
    void LogDrawCall(int triangleCount);

    /**
     * @brief 输出统计摘要
     */
    void LogStatisticsSummary();

    /**
     * @brief 重置统计数据
     */
    void ResetStatistics();

    /**
     * @brief 设置当前FPS值
     * @param fps FPS值
     */
    void SetFPS(int fps);

    /**
     * @brief 获取统计数据（用于测试）
     * @return 统计数据引用
     */
    const LogStatistics& GetStatistics() const;

    /**
     * @brief 获取当前FPS值
     * @return FPS值
     */
    int GetFPS() const;

    /**
     * @brief 关闭Logger并清理资源
     */
    void Shutdown();

private:
    Logger();
    ~Logger();

    // 禁止拷贝和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief 内部日志记录方法
     * @param level 日志级别
     * @param message 日志消息
     */
    void Log(LogLevel level, const std::string& message);

    /**
     * @brief 异步日志记录方法
     * @param level 日志级别
     * @param message 日志消息
     */
    void LogAsync(LogLevel level, const std::string& message);

    /**
     * @brief 获取当前时间戳字符串
     * @return 格式化的时间戳字符串
     */
    std::string GetTimestamp() const;

    /**
     * @brief 将LogLevel转换为字符串
     * @param level 日志级别
     * @return 日志级别字符串
     */
    std::string LevelToString(LogLevel level) const;

    /**
     * @brief 确保日志目录存在
     * @param filePath 文件路径
     */
    void EnsureLogDirectoryExists(const std::string& filePath);

    /**
     * @brief 检查是否需要轮转日志文件
     */
    void CheckRotation();

    /**
     * @brief 执行日志文件轮转
     */
    void RotateLogFile();

    /**
     * @brief 生成轮转文件名
     * @param index 轮转文件索引
     * @return 轮转文件名
     */
    std::string GenerateRotatedFileName(int index) const;

    /**
     * @brief 异步写入线程函数
     */
    void AsyncWriteThread();

    /**
     * @brief 同步写入日志条目
     * @param entry 日志条目
     */
    void WriteLogEntry(const LogEntry& entry);

    /**
     * @brief 刷新日志文件
     */
    void FlushLogFile();

    /**
     * @brief 生成包含上下文的格式化消息
     * @param level 日志级别
     * @param message 原始消息
     * @return 格式化后的消息
     */
    std::string FormatMessageWithContext(LogLevel level, const std::string& message);

    /**
     * @brief 检查是否应该输出统计摘要
     * @return 是否应该输出统计
     */
    bool ShouldLogStatistics();

private:
    // 异步写入相关
    std::queue<std::unique_ptr<LogEntry>> m_logQueue;    ///< 日志队列
    std::thread m_writeThread;                           ///< 写入线程
    std::mutex m_queueMutex;                             ///< 队列互斥锁
    std::condition_variable m_queueCondition;            ///< 队列条件变量
    std::atomic<bool> m_running;                          ///< 线程运行标志
    bool m_asyncMode;                                     ///< 是否异步模式

    // 文件和轮转相关
    std::ofstream m_logFile;                              ///< 日志文件流
    std::string m_baseFilePath;                           ///< 基础文件路径
    LogRotationConfig m_rotationConfig;                   ///< 轮转配置
    std::chrono::system_clock::time_point m_lastRotationTime; ///< 上次轮转时间

    // 配置相关
    std::mutex m_configMutex;                             ///< 配置互斥锁
    bool m_consoleOutput;                                 ///< 是否输出到控制台
    LogLevel m_minLevel;                                  ///< 最小日志级别
    bool m_initialized;                                   ///< 是否已初始化

    // 上下文和统计相关
    std::vector<LogContext> m_contextStack;               ///< 上下文栈
    LogStatistics m_statistics;                           ///< 统计信息
    std::chrono::steady_clock::time_point m_lastStatsTime; ///< 上次统计输出时间
    std::atomic<int> m_frameCount{0};                     ///< 帧计数器
};

} // namespace Core
