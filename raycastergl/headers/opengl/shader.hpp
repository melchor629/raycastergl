#pragma once

#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

class Shader {
    uint32_t shader = 0;
    fs::path path;

    friend class ShaderProgram;

public:
    enum Type {
        Vertex,
        Fragment,
        Compute,
    };

    Shader(Type type);
    ~Shader();

    bool load(const fs::path& path);
    bool compile() const;

    bool loadAndCompile(const fs::path& path) {
        return load(path) && compile();
    }

    static std::optional<Shader> load(const fs::path& path, Type type);
};
