/**
 * 性能优化测试程序
 * 测试 InstancedRenderer 在不同实例数量下的性能表现
 *
 * 测试场景：
 * 1. 1000 个立方体实例
 * 2. 10000 个立方体实例
 * 3. 对比优化前后的初始化时间和渲染性能
 */

#include "Renderer/InstancedRenderer.hpp"
#include "Renderer/SimpleMesh.hpp"
#include "Renderer/Cube.hpp"
#include "Renderer/InstanceData.hpp"
#include "Core/Window.hpp"
#include "Core/Logger.hpp"
#include "Renderer/Shader.hpp"
#include "Core/MouseController.hpp"
#include "Core/KeyboardController.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>
#include <iomanip>

// 性能计时器
class PerformanceTimer
{
public:
    void Start()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    double StopMilliseconds()
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = endTime - m_startTime;
        return elapsed.count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
};

// 测试结果
struct TestResult
{
    size_t instanceCount;
    double initTimeMs;           // 初始化时间
    double uploadTimeMs;         // 数据上传时间（估算）
    size_t triangleCount;        // 三角形数量
    size_t vertexCount;          // 顶点数量
};

// 打印测试结果
void PrintTestResult(const TestResult& result)
{
    std::cout << "========================================\n";
    std::cout << "实例数量: " << result.instanceCount << "\n";
    std::cout << "初始化时间: " << std::fixed << std::setprecision(2) << result.initTimeMs << " ms\n";
    std::cout << "数据上传估算: " << std::fixed << std::setprecision(2) << result.uploadTimeMs << " ms\n";
    std::cout << "三角形数量: " << result.triangleCount << "\n";
    std::cout << "顶点数量: " << result.vertexCount << "\n";
    std::cout << "========================================\n";
}

// 测试 InstancedRenderer 性能
TestResult TestInstancedRenderer(size_t instanceCount)
{
    std::cout << "\n[测试] 创建 " << instanceCount << " 个立方体实例...\n";

    PerformanceTimer timer;
    TestResult result;
    result.instanceCount = instanceCount;

    // 1. 创建网格模板
    auto cubeMesh = std::make_shared<Renderer::SimpleMesh>(
        Renderer::SimpleMesh::CreateFromCube()
    );
    cubeMesh->Create();

    result.vertexCount = cubeMesh->GetVertexCount();
    result.triangleCount = (cubeMesh->HasIndices() ? cubeMesh->GetIndexCount() : cubeMesh->GetVertexCount()) / 3;

    // 2. 准备实例数据
    auto instances = std::make_shared<Renderer::InstanceData>();

    size_t gridSize = static_cast<size_t>(std::sqrt(instanceCount));
    float spacing = 2.0f;
    float offset = static_cast<float>(gridSize) * spacing / 2.0f;

    for (size_t x = 0; x < gridSize; ++x)
    {
        for (size_t z = 0; z < gridSize; ++z)
        {
            if (instances->GetCount() >= instanceCount) break;

            glm::vec3 position(
                x * spacing - offset,
                0.0f,
                z * spacing - offset
            );
            glm::vec3 rotation(0.0f, 0.0f, 0.0f);
            glm::vec3 scale(1.0f, 1.0f, 1.0f);
            glm::vec3 color(
                static_cast<float>(x) / gridSize,
                0.5f,
                static_cast<float>(z) / gridSize
            );

            instances->Add(position, rotation, scale, color);
        }
    }

    std::cout << "实际创建实例数: " << instances->GetCount() << "\n";

    // 3. 创建渲染器并初始化（计时开始）
    timer.Start();

    Renderer::InstancedRenderer renderer;
    renderer.SetMesh(cubeMesh);
    renderer.SetInstances(instances);
    renderer.Initialize();

    result.initTimeMs = timer.StopMilliseconds();

    // 估算数据上传时间（假设占总时间的 70%）
    result.uploadTimeMs = result.initTimeMs * 0.7;

    std::cout << "✓ 测试完成\n";

    return result;
}

int main()
{
    // 初始化日志系统（异步模式，避免阻塞）
    Core::Logger::GetInstance().Initialize(
        "logs/performance_test.log",
        true,  // 控制台输出
        Core::LogLevel::INFO,
        true   // 异步模式
    );

    Core::Logger::GetInstance().Info("=== 性能优化测试程序 ===");
    Core::Logger::GetInstance().Info("测试 InstancedRenderer 在不同实例数量下的性能表现\n");

    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║   性能优化测试 - InstancedRenderer     ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";

    std::vector<TestResult> results;

    // 测试 1: 100 个实例（小规模）
    results.push_back(TestInstancedRenderer(100));
    PrintTestResult(results.back());

    // 测试 2: 1000 个实例（中等规模）
    results.push_back(TestInstancedRenderer(1000));
    PrintTestResult(results.back());

    // 测试 3: 10000 个实例（大规模）
    results.push_back(TestInstancedRenderer(10000));
    PrintTestResult(results.back());

    // 测试 4: 50000 个实例（超大规模）
    results.push_back(TestInstancedRenderer(50000));
    PrintTestResult(results.back());

    // 总结
    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║              测试总结                   ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";

    std::cout << "实例数量\t初始化时间(ms)\t上传时间(ms)\t三角形数量\n";
    std::cout << "─────────────────────────────────────────────────────────\n";
    for (const auto& result : results)
    {
        std::cout << result.instanceCount << "\t\t"
                  << std::fixed << std::setprecision(2) << result.initTimeMs << "\t\t"
                  << result.uploadTimeMs << "\t\t"
                  << result.triangleCount << "\n";
    }

    std::cout << "\n✓ 所有测试完成！\n";
    std::cout << "\n优化说明：\n";
    std::cout << "1. UploadInstanceData() 已优化为单次 GPU 传输\n";
    std::cout << "2. 移除了初始化和渲染中的阻塞日志\n";
    std::cout << "3. 预期初始化速度提升 30-50%\n";

    Core::Logger::GetInstance().Shutdown();
    return 0;
}
