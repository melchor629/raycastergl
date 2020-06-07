#pragma once

#include <filesystem>
#include <optional>
#include <string>

struct BinaryData {
    std::shared_ptr<uint8_t[]> data;
    size_t length;
};

std::optional<std::string> readFile(const std::filesystem::path& path);
std::optional<BinaryData> readFileBinary(const std::filesystem::path& path);