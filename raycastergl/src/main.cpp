#include <glad/glad.h>
#ifdef WIN32
#include <GLFW/glfw3.h>
#else
#include <GLFW/glfw3.h>
#endif
#include <cmath>
#include <iostream>
#include <stb_image.h>
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>
#include <arguments.hpp>
#include <engine/map.hpp>
#include <opengl/shader-program.hpp>
#include <opengl/buffer-geometry.hpp>
#include <opengl/texture.hpp>
#include <opengl/check-error.hpp>

#include "utils/defer.hpp"

using namespace std::literals;
using namespace glm;

struct MainContext {
    std::function<void(int, int)> onFramebufferSizeChanged;
    std::function<void(dvec2 pos)> onMousePositionChanged;
};

static Texture generateTextures();

int main(int argc, const char* const argv[]) {
    MainContext mainCtx;
    Arguments arguments;

    if(!arguments.parseArguments(argc, argv)) {
        return 1;
    }

#ifndef NDEBUG
    std::cout << "[!!] DEBUG enabled" << std::endl;
#endif

    std::cout << "> Creating window and OpenGL context" << std::endl;
    glfwInit();

    glfwSetErrorCallback([] (int code, const char* message) {
        std::cerr << "  GLFW Error [" << code << "]: " << message << std::endl;
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = arguments.initialWindowSize.x, height = arguments.initialWindowSize.y;
    auto window = glfwCreateWindow(width, height, "raycastergl", nullptr, nullptr);
    DEFER({
        if(window) glfwDestroyWindow(window);
        glfwTerminate();
    });
    if(window == nullptr) {
        std::cerr << "  Failed to create GLFW window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "  Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwGetFramebufferSize(window, &width, &height);
    glfwSetWindowUserPointer(window, &mainCtx);

    // GLFW Callbacks
    glfwSetFramebufferSizeCallback(window, [] (auto window, int width, int height) {
        // good trick to use lambda function in C code :)
        MainContext* ctx = (MainContext*) glfwGetWindowUserPointer(window);
        if(ctx->onFramebufferSizeChanged) {
            ctx->onFramebufferSizeChanged(width, height);
        }
    });

    glfwSetCursorPosCallback(window, [] (auto window, double xPos, double yPos) {
        MainContext* ctx = (MainContext*) glfwGetWindowUserPointer(window);
        if(ctx->onMousePositionChanged) {
            ctx->onMousePositionChanged({ xPos, yPos });
        }
    });

    glfwSetKeyCallback(window, [] (auto window, int key, int, int action, int) {
        if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        if((key == GLFW_KEY_F || key == GLFW_KEY_F11) && action == GLFW_PRESS) {
            static ivec2 oldPos(0, 0), oldSize(0, 0);
            static int monitorNumber = 0;
            int count;
            auto* monitor = glfwGetWindowMonitor(window);
            auto monitors = glfwGetMonitors(&count);
            if(monitor == nullptr) {
                glfwGetWindowPos(window, &oldPos.x, &oldPos.y);
                glfwGetWindowSize(window, &oldSize.x, &oldSize.y);
                monitorNumber = 0;
                monitor = monitors[monitorNumber];
                auto* videoMode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
            } else if(monitorNumber + 1 < count) {
                monitorNumber += 1;
                monitor = monitors[monitorNumber];
                auto* videoMode = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(window, monitor, 0, 0, videoMode->width, videoMode->height, videoMode->refreshRate);
            } else {
                glfwSetWindowMonitor(window, nullptr, oldPos.x, oldPos.y, oldSize.x, oldSize.y, 0);
            }
        }
    });

    glfwSetMouseButtonCallback(window, [] (auto window, int button, int action, int) {
        if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            if(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }
    });

    // window icon :)
    GLFWimage icon;
    icon.pixels = stbi_load("pics/eagle.png", &icon.width, &icon.height, nullptr, 4);
    glfwSetWindowIcon(window, 1, &icon);
    free(icon.pixels);

    // loading game resources
    Shader vertexShader(Shader::Vertex);
    Shader raycasterDrawerShader(Shader::Fragment);
    Shader raycasterShader(Shader::Compute);
    Shader spritecasterShader(Shader::Compute);
    if(
        !vertexShader.loadAndCompile("vert.glsl") ||
        !raycasterDrawerShader.loadAndCompile("raycaster-drawer.glsl") ||
        !raycasterShader.loadAndCompile("raycaster.glsl") ||
        !spritecasterShader.loadAndCompile("spritecaster.glsl")
    ) {
        return -1;
    }

    ShaderProgram raycasterDrawProgram("raycaster-draw");
    ShaderProgram raycasterComputeProgram("raycaster");
    ShaderProgram spritecasterComputeProgram("spritecaster");
    if(
        !raycasterDrawProgram.link({ &vertexShader, &raycasterDrawerShader }) ||
        !raycasterComputeProgram.link({ &raycasterShader }) ||
        !spritecasterComputeProgram.link({ &spritecasterShader })
    ) {
        return -1;
    }

    std::cout << "> Generating plane" << std::endl;
    BufferGeometry screenPlane;
    screenPlane.addAttribute(BufferAttribute(
        {
             1.0f,  1.0f, 0.0f, // top right
             1.0f, -1.0f, 0.0f, // bottom right
            -1.0f, -1.0f, 0.0f, // bottom left
            -1.0f,  1.0f, 0.0f, // top left
        },
        3
    ));
    screenPlane.addAttribute(BufferAttribute(
        {
            1.0f, 1.0f, // top right
            1.0f, 0.0f, // bottom right
            0.0f, 0.0f, // bottom left
            0.0f, 1.0f, // top left
        },
        2
    ));
    screenPlane.setIndices(BufferAttribute(
        {
            0u, 1u, 3u,  // first Triangle
            1u, 2u, 3u,  // second Triangle
        },
        3
    ));

    auto maybeMap = Map::load(arguments.map);
    if(maybeMap == std::nullopt) {
        return 1;
    }

    auto& map = maybeMap.value();
    DEFER(map.destroy());

    std::cout << "> Allocating raycaster output buffer" << std::endl;
    Buffer raycastResultBuffer(
        Buffer::ShaderStorageBuffer,
        10000 * (5 * sizeof(int32_t) + 5 * sizeof(float))
    );
    raycastResultBuffer.bind();

    std::cout << "> Allocating spritecaster output buffer" << std::endl;
    Buffer spritecastResultBuffer(
        Buffer::ShaderStorageBuffer,
        map.sprites.size() * (8 * sizeof(int32_t) + 1 * sizeof(float) + 1 * sizeof(uint32_t))
    );
    spritecastResultBuffer.bind();

    std::cout << "> Allocating spritecaster input buffer" << std::endl;
    Buffer spritecastInputBuffer(
        Buffer::ShaderStorageBuffer,
        map.sprites.size() * sizeof(Sprite),
        Buffer::DynamicCopy
    );
    spritecastInputBuffer.bind();

    // generates the texture array from the pngs
    auto glTextures = generateTextures();

    // another functions and callbacks
    auto framebufferSizeChanged = [
        &raycasterComputeProgram,
        &raycasterDrawProgram,
        &spritecasterComputeProgram
    ] (uvec2 size, uvec2 pos) {
        std::cout << "\rFramebuffer set to (" << size.x << ", " << size.y
            << "), position (" << pos.x << ", " << pos.y << ")" << std::endl;
        glViewport(pos.x, pos.y, size.x, size.y);
        raycasterComputeProgram.use();
        raycasterComputeProgram.setUniform("screenSize", size.x, size.y);
        raycasterDrawProgram.use();
        raycasterDrawProgram.setUniform("screenSize", size.x, size.y);
        spritecasterComputeProgram.use();
        spritecasterComputeProgram.setUniform("screenSize", size.x, size.y);
    };

    auto handleWindowSize = [&framebufferSizeChanged] (uvec2 size) {
        double aspectRatio = double(size.x) / double(size.y);
        if(aspectRatio > 1.333) {
            uint32_t limitedWidth = uint32_t(floor(size.y * 1.33333333333));
            uint32_t diffx = size.x - limitedWidth;
            framebufferSizeChanged({ limitedWidth, size.y }, { diffx / 2, 0 });
        } else {
            uint32_t limitedHeight = uint32_t(floor(size.x / 1.33333333333));
            uint32_t diffy = size.y - limitedHeight;
            framebufferSizeChanged({ size.x, limitedHeight }, { 0, diffy / 2 });
        }
    };

    handleWindowSize({ width, height });

    // define here our resize listener so the result texture can be properly resized
    mainCtx.onFramebufferSizeChanged = [&width, &height, &handleWindowSize] (int newWidth, int newHeight) {
        width = newWidth;
        height = newHeight;
        std::cout << "\rWindow resized to " << width << 'x' << height << std::endl;

        handleWindowSize({ uint32_t(width), uint32_t(height) });
    };

    // define here our mouse position listener alongside with the player variables, so we can modify them here
    vec2 mouseDirection(0, 0);
    mainCtx.onMousePositionChanged = [window, &mouseDirection] (dvec2 pos) {
        static dvec2 oldPos(pos);

        if(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            dvec2 diff = pos - oldPos;
            mouseDirection = diff;
        }

        oldPos = pos;
    };

    // sorted sprites list
    std::vector<Sprite> sortedSprites(map.sprites);
    raycasterDrawProgram.use();
    raycasterDrawProgram.setUniform("spriteCount", sortedSprites.size());
    if(std::holds_alternative<vec3>(map.floor)) {
        raycasterDrawProgram.setUniform("floorTex", vec4(std::get<vec3>(map.floor), 0.f));
    } else {
        raycasterDrawProgram.setUniform("floorTex", vec4(0.f, 0.f, 0.f, std::get<uint32_t>(map.floor)));
    }

    if(std::holds_alternative<vec3>(map.ceil)) {
        raycasterDrawProgram.setUniform("ceilTex", vec4(std::get<vec3>(map.ceil), 0.f));
    } else {
        raycasterDrawProgram.setUniform("ceilTex", vec4(0.f, 0.f, 0.f, std::get<uint32_t>(map.ceil)));
    }

    std::cout << "> Game loaded" << std::endl;
    vec2 pos = map.initialPos;
    vec2 dir = map.initialDir;
    vec2 plane = map.initialPlane;
    double previousTime = glfwGetTime() - 1.0 / 60.0;
    double lastFpsTick = glfwGetTime();
    uint32_t fps = 0;
    bool initialSpriteFill = false;
    while(!glfwWindowShouldClose(window)) {
        //start computing rays
        // note: binds the texture into the computer shader
        map.texture->bindImage(1);
        // note: binds the shared storage into the computer shader
        raycastResultBuffer.bindBase(2);
        raycasterComputeProgram.use();
        raycasterComputeProgram.setUniform("position", pos);
        raycasterComputeProgram.setUniform("direction", dir);
        raycasterComputeProgram.setUniform("plane", plane);
        raycasterComputeProgram.dispatchCompute(width);

        // start computing sprites positions and sizes
        spritecasterComputeProgram.use();
        spritecastInputBuffer.bindBase(1);
        spritecastResultBuffer.bindBase(2);
        spritecasterComputeProgram.setUniform("position", pos);
        spritecasterComputeProgram.setUniform("direction", dir);
        spritecasterComputeProgram.setUniform("plane", plane);
        spritecasterComputeProgram.dispatchCompute(map.sprites.size());

        checkGlError(glClearColor(0, 0, 0, 1));
        checkGlError(glClear(GL_COLOR_BUFFER_BIT));

        //wait until computer shaders finish
        checkGlError(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));

        // draw the raycaster result to the screen using the drawing shader
        // also draws sprites
        raycasterDrawProgram.use();
        // texture arrays are layered
        glTextures.bindImage(1, 0, false, 0);
        raycastResultBuffer.bindBase(2);
        spritecastResultBuffer.bindBase(3);
        raycasterDrawProgram.setUniform("position", pos);
        screenPlane.draw();

        // this code writes the raycaster result 2.0 into a file (not so slow as the texture version)
        // raycastResultBuffer._writeContentsToFile("yes.bin");

        glfwSwapInterval(arguments.vsync);
        glfwSwapBuffers(window);
        glfwPollEvents();

        const float currentTime = glfwGetTime();
        const float delta = currentTime - previousTime;
        const float moveSpeed = delta * 3.5f;
        const float rotationSpeed = delta * 2.5f;
        float movement = 0.0f;
        float rotation = 0.0f;
        // move forward
        const bool forwardPressed = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS
            || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        if(forwardPressed) {
            movement += moveSpeed;
        }
        // move backward
        const bool backwardPressed = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS
            || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        if(backwardPressed) {
            movement -= moveSpeed;
        }
        // move forward or backward (with the mouse)
        if(!(forwardPressed || backwardPressed) && abs(mouseDirection.y) > 0.001f) {
            movement = -delta * mouseDirection.y * 1.75f;
        }
        if(abs(movement) > 0.00000001f) {
            // move only if the player does not collide with some wall
            if(map.at(int(pos.x + dir.x * movement), int(pos.y)) == 0)
                pos.x += dir.x * movement;
            if(map.at(int(pos.x), int(pos.y + dir.y * movement)) == 0)
                pos.y += dir.y * movement;
        }
        // rotate camera to the right
        const bool rotateRightPressed = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS
            || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        if(rotateRightPressed) {
            rotation -= rotationSpeed;
        }
        // rotate camera to the left
        const bool rotateLeftPressed = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS
            || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        if(rotateLeftPressed) {
            rotation += rotationSpeed;
        }
        // rotate camera to the right or left
        if(!(rotateRightPressed || rotateLeftPressed) && abs(mouseDirection.x) > 0.001f) {
            rotation = -mouseDirection.x * delta;
        }
        if(abs(rotation) > 0.00000001f) {
            double oldDirX = dir.x;
            dir.x = dir.x * cos(rotation) - dir.y * sin(rotation);
            dir.y = oldDirX * sin(rotation) + dir.y * cos(rotation);
            double oldPlaneX = plane.x;
            plane.x = plane.x * cos(rotation) - plane.y * sin(rotation);
            plane.y = oldPlaneX * sin(rotation) + plane.y * cos(rotation);
        }

        // update sprites order depending on player's position
        if(currentTime - lastFpsTick >= 1 || !initialSpriteFill) {
            initialSpriteFill = true;
            // sort sprites
            std::sort(sortedSprites.begin(), sortedSprites.end(), [pos] (auto& a, auto& b) {
                const float distA = distance(pos, (vec2) a);
                const float distB = distance(pos, (vec2) b);
                return distA > distB;
            });

            spritecastInputBuffer.bind();
            spritecastInputBuffer.setData(sortedSprites.data(), sortedSprites.size());
        }

        previousTime = currentTime;
        mouseDirection = { 0, 0 };

        if(currentTime - lastFpsTick >= 1) {
            lastFpsTick = currentTime;
            printf("\r                                     ");
            printf("\rfps: %i (%.2f, %.2f)", fps, pos.x, pos.y);
            fflush(stdout);
            fps = 0;
        } else {
            fps += 1;
        }
    }

    printf("\n");
    return 0;
}

static Texture generateTextures() {
    typedef struct {
        stbi_uc* data;
        int width;
        int height;
        int components;
        const char* path;
    } stbiLoadStruct;

    const auto stbiLoad = [] (const char* path, const int c) -> stbiLoadStruct {
        std::cout << "> Reading texture " << path << std::endl;
        stbiLoadStruct res;
        res.path = path;
        res.data = stbi_load(path, &res.width, &res.height, &res.components, c);
        return res;
    };

    stbiLoadStruct textures[] = {
        stbiLoad("pics/eagle.png", 3),
        stbiLoad("pics/redbrick.png", 3),
        stbiLoad("pics/purplestone.png", 3),
        stbiLoad("pics/greystone.png", 3),
        stbiLoad("pics/bluestone.png", 3),
        stbiLoad("pics/mossy.png", 3),
        stbiLoad("pics/wood.png", 3),
        stbiLoad("pics/colorstone.png", 3),

        stbiLoad("pics/barrel.png", 3),
        stbiLoad("pics/pillar.png", 3),
        stbiLoad("pics/greenlight.png", 3),
    };

    Texture glTextures(Texture::Array2D);
    glTextures.bind();
    glTextures.setWrap(Texture::Repeat, Texture::Repeat);
    glTextures.setMinFilter(Texture::Nearest);
    glTextures.setMagFilter(Texture::Nearest);
    glTextures.reserveStorage3D(Texture::RGBA32F, { 64, 64, 11 });

    for(size_t i = 0; i < 11; i += 1) {
        const auto& texture = textures[i];
        std::cout << "> Loading texture " << texture.path << std::endl;
        glTextures.fillSubImage3D(
            0,
            { 0, 0, i },
            { texture.width, texture.height, 1 },
            Texture::RGB,
            Texture::UnsignedByte,
            texture.data
        );

        free(texture.data);
    }

    return glTextures;
}
