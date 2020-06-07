#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include "utils/files.hpp"

using namespace std;
namespace fs = std::filesystem;

optional<string> readFile(const fs::path& path) {
    if(!fs::is_regular_file(path)) {
        return nullopt;
    }

    ifstream stream(path);
    if(!stream.is_open()) {
        return nullopt;
    }

    stringstream stringStream;
    for(string line; getline(stream, line); ) {
        stringStream << line << endl;
    }

    stream.close();
    return stringStream.str();
}

std::optional<BinaryData> readFileBinary(const fs::path& path) {
    if(!fs::is_regular_file(path)) {
        return nullopt;
    }

    ifstream stream(path);
    if(!stream.is_open()) {
        return nullopt;
    }

    auto fileSize = size_t(fs::file_size(path));
    shared_ptr<uint8_t[]> data(new uint8_t[fileSize]);
    stream.read((char*) (void*) data.get(), fileSize);

    stream.close();
    return {{ data, fileSize }};
}
