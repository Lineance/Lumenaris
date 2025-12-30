# TinyOBJLoader API 完整参考文档

## 📚 概述

TinyOBJLoader 是一个轻量级的 C++ OBJ 文件加载库，支持加载 Wavefront OBJ 格式的 3D 模型文件，包括几何体、材质和纹理信息。

**版本**: 1.0.6+
**许可证**: MIT License
**作者**: Syoyo Fujita

---

## 🔧 数据类型

### 核心数据类型

#### `real_t`
```cpp
#ifdef TINYOBJLOADER_USE_DOUBLE
typedef double real_t;
#else
typedef float real_t;  // 默认使用 float
#endif
```

**描述**: 库内部使用的实数类型，可以通过定义 `TINYOBJLOADER_USE_DOUBLE` 来切换为 double 类型。

#### `texture_type_t`
```cpp
typedef enum {
    TEXTURE_TYPE_NONE,        // 默认值，无纹理
    TEXTURE_TYPE_SPHERE,      // 球形纹理映射
    TEXTURE_TYPE_CUBE_TOP,    // 立方体纹理 - 顶部
    TEXTURE_TYPE_CUBE_BOTTOM, // 立方体纹理 - 底部
    TEXTURE_TYPE_CUBE_FRONT,  // 立方体纹理 - 正面
    TEXTURE_TYPE_CUBE_BACK,   // 立方体纹理 - 背面
    TEXTURE_TYPE_CUBE_LEFT,   // 立方体纹理 - 左侧
    TEXTURE_TYPE_CUBE_RIGHT   // 立方体纹理 - 右侧
} texture_type_t;
```

**描述**: 纹理映射类型枚举，用于指定纹理的投影方式。

### 数据结构

#### `texture_option_t`
```cpp
typedef struct {
    texture_type_t type;        // 纹理类型 (默认 TEXTURE_TYPE_NONE)
    real_t sharpness;           // 纹理锐度增强 (默认 1.0)
    real_t brightness;          // 亮度调整 (默认 0.0)
    real_t contrast;            // 对比度调整 (默认 1.0)
    real_t origin_offset[3];    // 纹理原点偏移 (默认 {0,0,0})
    real_t scale[3];            // 纹理缩放 (默认 {1,1,1})
    real_t turbulence[3];       // 湍流效果 (默认 {0,0,0})
    bool clamp;                 // 是否钳制纹理坐标 (默认 false)
    char imfchan;               // 图像通道 (b=blue, g=green, r=red, m=matte, l=luminance, z=z-depth)
    bool blendu;                // U 方向混合 (默认 true)
    bool blendv;                // V 方向混合 (默认 true)
    real_t bump_multiplier;     // 凹凸贴图倍增器 (默认 1.0)
} texture_option_t;
```

**描述**: 纹理选项结构体，定义纹理的各种映射和处理参数。

#### `material_t`
```cpp
typedef struct {
    std::string name;                           // 材质名称

    real_t ambient[3];                          // 环境光颜色 (Ka)
    real_t diffuse[3];                          // 漫反射颜色 (Kd)
    real_t specular[3];                         // 高光颜色 (Ks)
    real_t transmittance[3];                    // 透射颜色 (Kt)
    real_t emission[3];                         // 自发光颜色 (Ke)
    real_t shininess;                           // 高光指数 (Ns)
    real_t ior;                                 // 折射率 (Ni)
    real_t dissolve;                            // 溶解度/dissolve (d)
    int illum;                                  // 照明模型 (illum)

    // 纹理文件名
    std::string ambient_texname;                // 环境贴图 (map_Ka)
    std::string diffuse_texname;                // 漫反射贴图 (map_Kd)
    std::string specular_texname;               // 高光贴图 (map_Ks)
    std::string specular_highlight_texname;     // 高光强度贴图 (map_Ns)
    std::string bump_texname;                   // 凹凸贴图 (map_bump, bump)
    std::string displacement_texname;           // 位移贴图 (disp)
    std::string alpha_texname;                  // 透明贴图 (map_d)

    // 纹理选项
    texture_option_t ambient_texopt;
    texture_option_t diffuse_texopt;
    texture_option_t specular_texopt;
    texture_option_t specular_highlight_texopt;
    texture_option_t bump_texopt;
    texture_option_t displacement_texopt;
    texture_option_t alpha_texopt;

    // PBR 扩展属性
    real_t roughness;                           // 粗糙度 [0,1]
    real_t metallic;                            // 金属度 [0,1]
    real_t sheen;                               // 光泽度 [0,1]
    real_t clearcoat_thickness;                 // 清漆层厚度 [0,1]
    real_t clearcoat_roughness;                 // 清漆层粗糙度 [0,1]
    real_t anisotropy;                          // 各向异性 [0,1]
    real_t anisotropy_rotation;                 // 各向异性旋转 [0,1]

    // PBR 纹理
    std::string roughness_texname;              // 粗糙度贴图 (map_Pr)
    std::string metallic_texname;               // 金属度贴图 (map_Pm)
    std::string sheen_texname;                  // 光泽贴图 (map_Ps)
    std::string emissive_texname;               // 自发光贴图 (map_Ke)
    std::string normal_texname;                 // 法线贴图 (norm)

    texture_option_t roughness_texopt;
    texture_option_t metallic_texopt;
    texture_option_t sheen_texopt;
    texture_option_t emissive_texopt;
    texture_option_t normal_texopt;

    std::map<std::string, std::string> unknown_parameter; // 未知参数
} material_t;
```

**描述**: 材质信息结构体，包含完整的材质属性和纹理信息。

#### `tag_t`
```cpp
typedef struct {
    std::string name;                    // 标签名称
    std::vector<int> intValues;          // 整数值列表
    std::vector<real_t> floatValues;     // 浮点数值列表
    std::vector<std::string> stringValues; // 字符串值列表
} tag_t;
```

**描述**: SubD 标签结构体，用于存储细分曲面信息。

#### `index_t`
```cpp
typedef struct {
    int vertex_index;     // 顶点索引 (-1 表示未使用)
    int normal_index;     // 法线索引 (-1 表示未使用)
    int texcoord_index;   // 纹理坐标索引 (-1 表示未使用)
} index_t;
```

**描述**: 面索引结构体，将顶点、法线和纹理坐标索引关联起来。

#### `mesh_t`
```cpp
typedef struct {
    std::vector<index_t> indices;           // 面索引列表
    std::vector<unsigned char> num_face_vertices; // 每个面的顶点数 (3=三角形, 4=四边形)
    std::vector<int> material_ids;          // 每个面的材质ID (-1 表示无材质)
    std::vector<tag_t> tags;                // SubD 标签
} mesh_t;
```

**描述**: 网格结构体，包含面的几何和材质信息。

#### `shape_t`
```cpp
typedef struct {
    std::string name;     // 形状名称
    mesh_t mesh;          // 网格数据
} shape_t;
```

**描述**: 形状结构体，表示OBJ文件中的一个对象。

#### `attrib_t`
```cpp
typedef struct {
    std::vector<real_t> vertices;    // 顶点位置 (x,y,z,x,y,z,...)
    std::vector<real_t> normals;     // 法线向量 (x,y,z,x,y,z,...)
    std::vector<real_t> texcoords;   // 纹理坐标 (u,v,u,v,...)
} attrib_t;
```

**描述**: 顶点属性结构体，存储所有顶点相关数据。

#### `callback_t`
```cpp
typedef struct callback_t_ {
    // 顶点回调 (w 参数可选，默认为 1.0)
    void (*vertex_cb)(void *user_data, real_t x, real_t y, real_t z, real_t w);

    // 法线回调
    void (*normal_cb)(void *user_data, real_t x, real_t y, real_t z);

    // 纹理坐标回调 (y,z 参数可选，默认为 0.0)
    void (*texcoord_cb)(void *user_data, real_t x, real_t y, real_t z);

    // 索引回调 (num_indices = 面的顶点数)
    void (*index_cb)(void *user_data, index_t *indices, int num_indices);

    // 材质使用回调 (material_id = -1 表示材质未找到)
    void (*usemtl_cb)(void *user_data, const char *name, int material_id);

    // 材质库加载回调
    void (*mtllib_cb)(void *user_data, const material_t *materials, int num_materials);

    // 组回调 (num_names = 组名称数量)
    void (*group_cb)(void *user_data, const char **names, int num_names);

    // 对象回调
    void (*object_cb)(void *user_data, const char *name);

    callback_t_() : vertex_cb(NULL), normal_cb(NULL), texcoord_cb(NULL),
                   index_cb(NULL), usemtl_cb(NULL), mtllib_cb(NULL),
                   group_cb(NULL), object_cb(NULL) {}
} callback_t;
```

**描述**: 回调函数结构体，用于自定义OBJ文件解析过程中的事件处理。

---

## 🏗️ 类接口

### `MaterialReader` (抽象基类)
```cpp
class MaterialReader {
public:
    virtual ~MaterialReader();
    virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap,
                          std::string *err) = 0;
};
```

**描述**: 材质读取器抽象基类，用于自定义材质文件的加载方式。

#### 纯虚函数
- `operator()`: 执行材质加载操作

**参数**:
- `matId`: 材质文件名
- `materials`: 输出材质数据
- `matMap`: 材质名称到ID的映射
- `err`: 错误信息输出

**返回值**: 加载成功返回 true，否则返回 false

### `MaterialFileReader` (文件材质读取器)
```cpp
class MaterialFileReader : public MaterialReader {
public:
    explicit MaterialFileReader(const std::string &mtl_basedir);
    virtual ~MaterialFileReader();
    virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap,
                          std::string *err);
};
```

**描述**: 基于文件的材质读取器实现。

**构造函数参数**:
- `mtl_basedir`: 材质文件的基础目录路径

### `MaterialStreamReader` (流材质读取器)
```cpp
class MaterialStreamReader : public MaterialReader {
public:
    explicit MaterialStreamReader(std::istream &inStream);
    virtual ~MaterialStreamReader();
    virtual bool operator()(const std::string &matId,
                          std::vector<material_t> *materials,
                          std::map<std::string, int> *matMap,
                          std::string *err);
};
```

**描述**: 基于输入流的材质读取器实现。

**构造函数参数**:
- `inStream`: 输入流引用

---

## 🔄 函数接口

### `LoadObj` (文件加载 - 标准版本)
```cpp
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string *err,
             const char *filename, const char *mtl_basedir = NULL,
             bool triangulate = true);
```

**描述**: 从文件加载OBJ模型。

**参数**:
- `attrib`: 输出顶点属性数据
- `shapes`: 输出形状数据
- `materials`: 输出材质数据
- `err`: 错误信息字符串
- `filename`: OBJ文件名
- `mtl_basedir`: 材质文件基础目录 (可选，默认为应用程序工作目录)
- `triangulate`: 是否将多边形三角化 (可选，默认 true)

**返回值**: 加载成功返回 true，否则返回 false

**注意**: 如果 mtl_basedir 为 NULL，材质文件将在应用程序工作目录中搜索。

### `LoadObj` (流加载版本)
```cpp
bool LoadObj(attrib_t *attrib, std::vector<shape_t> *shapes,
             std::vector<material_t> *materials, std::string *err,
             std::istream *inStream, MaterialReader *readMatFn = NULL,
             bool triangulate = true);
```

**描述**: 从输入流加载OBJ模型。

**参数**:
- `attrib`: 输出顶点属性数据
- `shapes`: 输出形状数据
- `materials`: 输出材质数据
- `err`: 错误信息字符串
- `inStream`: 输入流指针
- `readMatFn`: 自定义材质读取器 (可选)
- `triangulate`: 是否将多边形三角化 (可选，默认 true)

**返回值**: 加载成功返回 true，否则返回 false

### `LoadObjWithCallback` (回调加载版本)
```cpp
bool LoadObjWithCallback(std::istream &inStream, const callback_t &callback,
                         void *user_data = NULL,
                         MaterialReader *readMatFn = NULL,
                         std::string *err = NULL);
```

**描述**: 使用回调函数加载OBJ模型，提供更细粒度的控制。

**参数**:
- `inStream`: 输入流引用
- `callback`: 回调函数结构体
- `user_data`: 用户数据指针 (可选)
- `readMatFn`: 自定义材质读取器 (可选)
- `err`: 错误信息字符串 (可选)

**返回值**: 加载成功返回 true，否则返回 false

### `LoadMtl` (材质文件加载)
```cpp
void LoadMtl(std::map<std::string, int> *material_map,
             std::vector<material_t> *materials, std::istream *inStream,
             std::string *warning);
```

**描述**: 从输入流加载材质定义文件 (.mtl)。

**参数**:
- `material_map`: 输出材质名称到ID的映射
- `materials`: 输出材质数据列表
- `inStream`: 输入流指针
- `warning`: 警告信息字符串

**返回值**: 无

---

## 📋 使用指南

### 基本用法示例

#### 1. 简单文件加载
```cpp
#include "tiny_obj_loader.h"

tinyobj::attrib_t attrib;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
std::string err;

bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "model.obj");

if (!err.empty()) {
    std::cerr << err << std::endl;
}

if (!ret) {
    return false; // 加载失败
}

// 使用加载的数据...
```

#### 2. 带材质的加载
```cpp
std::string err;
bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                           "model.obj", "materials/", true);

if (!ret) {
    std::cerr << "Failed to load: " << err << std::endl;
    return false;
}
```

#### 3. 回调方式加载
```cpp
void vertex_callback(void *user_data, float x, float y, float z, float w) {
    // 处理顶点数据
}

tinyobj::callback_t callback;
callback.vertex_cb = vertex_callback;

std::ifstream ifs("model.obj");
bool ret = tinyobj::LoadObjWithCallback(ifs, callback, nullptr, nullptr, nullptr);
```

### 数据访问示例

#### 访问顶点数据
```cpp
// attrib.vertices 存储为连续的实数数组 [x,y,z,x,y,z,...]
for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
    float x = attrib.vertices[i];
    float y = attrib.vertices[i + 1];
    float z = attrib.vertices[i + 2];
    // 处理顶点 (x,y,z)
}
```

#### 访问面数据
```cpp
for (const auto& shape : shapes) {
    for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
        // 三角形顶点索引
        int idx0 = shape.mesh.indices[i].vertex_index;
        int idx1 = shape.mesh.indices[i + 1].vertex_index;
        int idx2 = shape.mesh.indices[i + 2].vertex_index;

        // 顶点位置
        float v0[3] = {
            attrib.vertices[3 * idx0],
            attrib.vertices[3 * idx0 + 1],
            attrib.vertices[3 * idx0 + 2]
        };
        // ... 处理三角形
    }
}
```

#### 访问材质数据
```cpp
for (const auto& material : materials) {
    std::cout << "Material: " << material.name << std::endl;
    std::cout << "Diffuse: (" << material.diffuse[0] << ", "
              << material.diffuse[1] << ", " << material.diffuse[2] << ")" << std::endl;

    if (!material.diffuse_texname.empty()) {
        std::cout << "Diffuse texture: " << material.diffuse_texname << std::endl;
    }
}
```

### 性能优化建议

1. **预分配内存**: 如果知道模型大小，可以预先 reserve 向量容量
2. **选择性加载**: 只加载需要的属性 (顶点、法线、纹理坐标)
3. **缓存管理**: 对于大型模型，考虑内存映射文件
4. **多线程**: 对于多个模型，可以并行加载

### 错误处理

- 总是检查 `LoadObj` 的返回值
- 检查 `err` 字符串是否为空
- 对于材质文件，检查 `mtl_basedir` 路径是否正确
- 处理文件不存在或权限问题的情况

---

## 🔗 相关链接

- [TinyOBJLoader GitHub](https://github.com/tinyobjloader/tinyobjloader)
- [Wavefront OBJ 格式规范](https://en.wikipedia.org/wiki/Wavefront_.obj_file)
- [Material Template Library](https://en.wikipedia.org/wiki/Wavefront_.obj_file#Material_template_library)

---

*本文档基于 TinyOBJLoader 1.0.6 版本编写* 📚
