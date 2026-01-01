#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Renderer
{
    /**
     * Cubemap约定枚举
     *
     * 不同的工具和引擎使用不同的cubemap面顺序和命名约定
     */
    enum class CubemapConvention
    {
        OPENGL,        // OpenGL标准: right, left, top, bottom, front(+Z), back(-Z)
        DIRECTX,       // DirectX: left, right, top, bottom, front, back
        MAYA,          // Maya/Corona: right, left, top, bottom, back(+Z), front(-Z)
        BLENDER,       // Blender: right, left, top, bottom, front, back (但方向可能不同)
        CUSTOM         // 自定义映射
    };

    /**
     * Skybox配置结构
     */
    struct SkyboxConfig
    {
        std::string directory;                           // 天空盒纹理目录
        std::vector<std::string> faceFilenames;         // 6个面的文件名（按OpenGL顺序）
        CubemapConvention convention;                   // 使用的约定
        bool flipVertically;                            // 是否垂直翻转纹理
        bool generateMipmaps;                           // 是否生成mipmaps

        SkyboxConfig()
            : convention(CubemapConvention::OPENGL)
            , flipVertically(false)
            , generateMipmaps(true)
        {}
    };

    /**
     * Cubemap面命名方案
     *
     * 定义6个面的名称后缀，支持任意命名格式
     * 顺序必须遵循对应约定的标准顺序
     */
    struct FaceNamingScheme
    {
        std::string right;   // +X 面
        std::string left;    // -X 面
        std::string top;     // +Y 面
        std::string bottom;  // -Y 面
        std::string back;    // +Z 面
        std::string front;   // -Z 面

        FaceNamingScheme(
            const std::string& r = "right",
            const std::string& l = "left",
            const std::string& t = "top",
            const std::string& b = "bottom",
            const std::string& bk = "back",
            const std::string& f = "front")
            : right(r), left(l), top(t), bottom(b), back(bk), front(f)
        {}

        /**
         * 转换为字符串数组（按约定顺序）
         */
        std::vector<std::string> ToArray() const
        {
            return {right, left, top, bottom, back, front};
        }
    };

    /**
     * SkyboxLoader - 天空盒加载工具类
     *
     * 功能：
     * - 支持多种cubemap约定
     * - 自动检测并转换不同的面顺序
     * - 提供预设配置
     * - 灵活的文件命名支持
     */
    class SkyboxLoader
    {
    public:
        /**
         * 从标准命名约定创建配置
         * @param directory 纹理目录路径
         * @param convention 使用的约定
         * @param basename 文件基础名（如 "skybox" 或 "corona"）
         * @param extension 文件扩展名（如 ".png" 或 ".jpg"）
         */
        static SkyboxConfig CreateConfig(
            const std::string& directory,
            CubemapConvention convention,
            const std::string& basename = "",
            const std::string& extension = ".png"
        );

        /**
         * 从自定义文件名创建配置
         * @param directory 纹理目录路径
         * @param filenames 6个面的文件名（按约定顺序）
         * @param convention 文件名使用的约定（默认为OpenGL标准）
         *
         * 使用示例：
         * - 标准命名：CreateCustomConfig(dir, {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "back.jpg", "front.jpg"})
         * - Corona命名：CreateCustomConfig(dir, {"corona_rt.png", "corona_lf.png", ...}, MAYA)
         * - HDR Lab：CreateCustomConfig(dir, {"px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png"})
         */
        static SkyboxConfig CreateCustomConfig(
            const std::string& directory,
            const std::vector<std::string>& filenames,
            CubemapConvention convention = CubemapConvention::OPENGL
        );

        /**
         * 从文件模式创建配置（支持通配符）
         * @param directory 纹理目录路径
         * @param pattern 文件名模式，支持占位符：
         *                - {face} 替换为面名称
         * @param convention 使用的约定
         * @param extension 文件扩展名（如 ".png"）
         *
         * 使用示例：
         * - CreateFromPattern(dir, "skybox_{face}", OPENGL, ".jpg")  → skybox_right.jpg, skybox_left.jpg, ...
         * - CreateFromPattern(dir, "{face}", MAYA, ".png")           → rt.png, lf.png, ...
         */
        static SkyboxConfig CreateFromPattern(
            const std::string& directory,
            const std::string& pattern,
            CubemapConvention convention,
            const std::string& extension = ".png"
        );

        /**
         * 从自定义面命名方案创建配置（完全自定义）
         * @param directory 纹理目录路径
         * @param pattern 文件名模式，支持占位符：
         *                - {face} 替换为自定义面名称
         * @param namingScheme 自定义面命名方案
         * @param convention 使用的约定（用于面顺序转换）
         * @param extension 文件扩展名（如 ".png"）
         *
         * 使用示例：
         * - FaceNamingScheme custom("rt", "lf", "up", "dn", "bk", "ft");
         * - CreateFromCustomScheme(dir, "corona_{face}", custom, MAYA, ".png")
         */
        static SkyboxConfig CreateFromCustomScheme(
            const std::string& directory,
            const std::string& pattern,
            const FaceNamingScheme& namingScheme,
            CubemapConvention convention,
            const std::string& extension = ".png"
        );

        /**
         * 获取常用命名方案的预设
         */
        static FaceNamingScheme GetOpenGLScheme() {
            return FaceNamingScheme("right", "left", "top", "bottom", "back", "front");
        }
        static FaceNamingScheme GetMayaScheme() {
            return FaceNamingScheme("rt", "lf", "up", "dn", "bk", "ft");
        }
        static FaceNamingScheme GetDirectXScheme() {
            return FaceNamingScheme("left", "right", "top", "bottom", "front", "back");
        }
        static FaceNamingScheme GetHDRLabScheme() {
            return FaceNamingScheme("px", "nx", "py", "ny", "pz", "nz");
        }

        /**
         * 获取约定对应的OpenGL标准顺序的文件名
         * @param convention 输入约定
         * @param inputNames 输入文件名（按约定顺序）
         * @return OpenGL顺序的文件名
         */
        static std::vector<std::string> ConvertToOpenGL(
            CubemapConvention convention,
            const std::vector<std::string>& inputNames
        );

    private:
        // 各种约定的面顺序映射（索引对应OpenGL顺序）
        // OpenGL顺序: 0=right, 1=left, 2=top, 3=bottom, 4=back(+Z), 5=front(-Z)
        static const int s_mayaMapping[];   // Maya约定映射
        static const int s_directxMapping[]; // DirectX约定映射
    };

} // namespace Renderer
