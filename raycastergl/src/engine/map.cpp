#include <engine/map.hpp>
#include <iostream>
#include <glad/glad.h>
#include <yaml-cpp/yaml.h>
#include <opengl/check-error.hpp>

void Map::destroy() {
    delete[] data;
}

std::optional<Map> Map::load(const fs::path& path) {
    fs::path fullPath = fs::path("res/maps") / path;
    std::cout << "> Loading map " << path << std::endl;
    if(!fs::exists(fullPath)) {
        std::cerr << "  Map does not exist!" << std::endl;
        return std::nullopt;
    }

    if(!fs::is_regular_file(fullPath)) {
        std::cerr << "  Map is not a file!" << std::endl;
        return std::nullopt;
    }

    auto mapYaml = YAML::LoadFile(fullPath.string());
    if(!mapYaml["map"]) {
        std::cerr << "  Map file is invalid: does not have map property" << std::endl;
        return std::nullopt;
    }

    std::cout << "  > Loading map data" << std::endl;
    uint32_t mapWidth = mapYaml["map"]["width"].as<uint32_t>();
    uint32_t mapHeight = mapYaml["map"]["height"].as<uint32_t>();
    uint8_t* map = new uint8_t[mapWidth * mapHeight];
    for(uint32_t x = 0; x < mapWidth; x += 1) {
        for(uint32_t y = 0; y < mapHeight; y += 1) {
            map[x * mapWidth + y] = mapYaml["map"]["content"][x][y].as<uint16_t>() & 0xFF;
        }
    }

    std::variant<uint32_t, vec3> floor, ceil;
    if(mapYaml["map"]["floor"].Type() == YAML::NodeType::Sequence) {
        floor = vec3 {
            mapYaml["map"]["floor"][0].as<float>(),
            mapYaml["map"]["floor"][1].as<float>(),
            mapYaml["map"]["floor"][2].as<float>(),
        };
    } else {
        floor = mapYaml["map"]["floor"].as<uint32_t>(3);
    }

    if(mapYaml["map"]["ceil"].Type() == YAML::NodeType::Sequence) {
        ceil = vec3 {
            mapYaml["map"]["ceil"][0].as<float>(),
            mapYaml["map"]["ceil"][1].as<float>(),
            mapYaml["map"]["ceil"][2].as<float>(),
        };
    } else {
        ceil = mapYaml["map"]["ceil"].as<uint32_t>(3);
    }

    vec2 initialPos(
        mapYaml["initial"]["pos"][0].as<float>(),
        mapYaml["initial"]["pos"][1].as<float>()
    );
    vec2 initialDir(
        mapYaml["initial"]["dir"][0].as<float>(),
        mapYaml["initial"]["dir"][1].as<float>()
    );
    vec2 initialPlane(
        mapYaml["initial"]["plane"][0].as<float>(),
        mapYaml["initial"]["plane"][1].as<float>()
    );

    std::cout << "  > Loading sprites data" << std::endl;
    size_t spriteCount = mapYaml["sprites"].size();
    std::vector<Sprite> sprites;
    for(uint32_t i = 0; i < spriteCount; i += 1) {
        Sprite sprite = {
            mapYaml["sprites"][i]["x"].as<float>(),
            mapYaml["sprites"][i]["y"].as<float>(),
            mapYaml["sprites"][i]["texture"].as<uint32_t>(),
            mapYaml["sprites"][i]["uDiv"].as<int32_t>(1),
            mapYaml["sprites"][i]["vDiv"].as<int32_t>(1),
            mapYaml["sprites"][i]["vMove"].as<float>(0.0f),
        };
        sprites.push_back(sprite);
    }

    std::cout << "  > Loading map texture" << std::endl;
    Map mapData {
        map,
        uvec2(mapWidth, mapHeight),
        floor,
        ceil,
        initialPos,
        initialDir,
        initialPlane,
        sprites,
        std::make_shared<Texture>(Texture::Type::_2D),
    };

    mapData.texture->bind();
    mapData.texture->setWrap(Texture::Repeat, Texture::Repeat);
    mapData.texture->setMinFilter(Texture::Nearest);
    mapData.texture->setMagFilter(Texture::Nearest);

    mapData.texture->fillImage2D(
        0,
        Texture::R8UI,
        { mapWidth, mapHeight },
        0,
        Texture::RedInteger,
        Texture::UnsignedByte,
        map
    );

    return mapData;
}
