#include "Renderer/Environment/SkyboxLoader.hpp"
#include <algorithm>
#include <stdexcept>

namespace Renderer
{

    // Maya/Corona约定映射
    // Maya顺序: right, left, top, bottom, back(+Z), front(-Z)
    // OpenGL顺序: right, left, top, bottom, back(+Z), front(-Z)
    // 注意：Maya的back是+Z，front是-Z，与OpenGL相反！
    const int SkyboxLoader::s_mayaMapping[] = {
        0,  // right -> right
        1,  // left -> left
        2,  // top -> top
        3,  // bottom -> bottom
        5,  // front -> back (Maya的front对应OpenGL的back位置)
        4   // back -> front (Maya的back对应OpenGL的front位置)
    };

    // DirectX约定映射
    const int SkyboxLoader::s_directxMapping[] = {
        1,  // left -> right (DirectX的left在OpenGL是right)
        0,  // right -> left
        2,  // top -> top
        3,  // bottom -> bottom
        4,  // front -> back
        5   // back -> front
    };

    SkyboxConfig SkyboxLoader::CreateConfig(
        const std::string& directory,
        CubemapConvention convention,
        const std::string& basename,
        const std::string& extension)
    {
        SkyboxConfig config;
        config.directory = directory;
        config.convention = convention;

        // 标准的cubemap面名称（按不同约定的顺序）
        std::vector<std::string> faces;

        switch (convention)
        {
            case CubemapConvention::OPENGL:
                faces = {"right", "left", "top", "bottom", "back", "front"};
                break;

            case CubemapConvention::MAYA:
            case CubemapConvention::BLENDER:
                faces = {"right", "left", "top", "bottom", "back", "front"};
                break;

            case CubemapConvention::DIRECTX:
                faces = {"left", "right", "top", "bottom", "front", "back"};
                break;

            default:
                faces = {"right", "left", "top", "bottom", "back", "front"};
                break;
        }

        // 添加基础名和扩展名
        for (const auto& face : faces)
        {
            if (basename.empty())
            {
                config.faceFilenames.push_back(directory + "/" + face + extension);
            }
            else
            {
                config.faceFilenames.push_back(directory + "/" + basename + "_" + face + extension);
            }
        }

        return config;
    }

    SkyboxConfig SkyboxLoader::CreateCustomConfig(
        const std::string& directory,
        const std::vector<std::string>& filenames,
        CubemapConvention convention)
    {
        if (filenames.size() != 6)
        {
            throw std::invalid_argument("Skybox requires exactly 6 face filenames");
        }

        SkyboxConfig config;
        config.directory = directory;
        config.convention = convention;

        // 如果不是OpenGL约定，需要转换
        if (convention == CubemapConvention::OPENGL)
        {
            // 已经是OpenGL顺序，直接使用
            for (const auto& filename : filenames)
            {
                config.faceFilenames.push_back(directory + "/" + filename);
            }
        }
        else
        {
            // 需要转换到OpenGL顺序
            auto openglOrder = ConvertToOpenGL(convention, filenames);
            for (const auto& filename : openglOrder)
            {
                config.faceFilenames.push_back(directory + "/" + filename);
            }
        }

        return config;
    }

    std::vector<std::string> SkyboxLoader::ConvertToOpenGL(
        CubemapConvention convention,
        const std::vector<std::string>& inputNames)
    {
        if (inputNames.size() != 6)
        {
            throw std::invalid_argument("Input must contain exactly 6 face names");
        }

        std::vector<std::string> openglOrder(6);

        switch (convention)
        {
            case CubemapConvention::OPENGL:
                // 已经是OpenGL顺序
                return inputNames;

            case CubemapConvention::MAYA:
                // Maya约定：front和back与OpenGL相反
                for (int i = 0; i < 6; ++i)
                {
                    openglOrder[s_mayaMapping[i]] = inputNames[i];
                }
                break;

            case CubemapConvention::DIRECTX:
                // DirectX约定：left/right互换，front/back互换
                for (int i = 0; i < 6; ++i)
                {
                    openglOrder[s_directxMapping[i]] = inputNames[i];
                }
                break;

            case CubemapConvention::BLENDER:
                // Blender通常与OpenGL兼容，但有时Y轴方向不同
                // 这里假设与OpenGL相同
                return inputNames;

            default:
                return inputNames;
        }

        return openglOrder;
    }

    SkyboxConfig SkyboxLoader::CreateFromPattern(
        const std::string& directory,
        const std::string& pattern,
        CubemapConvention convention,
        const std::string& extension)
    {
        // 使用预设的命名方案
        FaceNamingScheme scheme;

        switch (convention)
        {
            case CubemapConvention::MAYA:
                scheme = GetMayaScheme();
                break;
            case CubemapConvention::DIRECTX:
                scheme = GetDirectXScheme();
                break;
            case CubemapConvention::OPENGL:
            case CubemapConvention::BLENDER:
            default:
                scheme = GetOpenGLScheme();
                break;
        }

        return CreateFromCustomScheme(directory, pattern, scheme, convention, extension);
    }

    SkyboxConfig SkyboxLoader::CreateFromCustomScheme(
        const std::string& directory,
        const std::string& pattern,
        const FaceNamingScheme& namingScheme,
        CubemapConvention convention,
        const std::string& extension)
    {
        // 从命名方案获取面名称数组
        std::vector<std::string> faces = namingScheme.ToArray();

        // 生成文件名列表
        std::vector<std::string> filenames;
        filenames.reserve(6);

        for (const auto& face : faces)
        {
            std::string filename = pattern;

            // 替换 {face} 占位符
            size_t pos = filename.find("{face}");
            if (pos != std::string::npos)
            {
                filename.replace(pos, 6, face);
            }
            else
            {
                // 如果没有 {face} 占位符，直接使用面名称
                filename = face;
            }

            // 添加扩展名（如果没有）
            if (!extension.empty() && filename.find('.') == std::string::npos)
            {
                filename += extension;
            }

            filenames.push_back(filename);
        }

        // 使用通用的CreateCustomConfig方法
        return CreateCustomConfig(directory, filenames, convention);
    }

} // namespace Renderer
