# raycastergl

> A Wolf3D-like raycaster using OpenGL shaders

This is a hobby project to make a raycaster using the GPU power with Compute and Fragment Shaders. It is based on the [Lode's Computer Graphics Tutorial](https://lodev.org/cgtutor/raycasting.html) (the raycast series), which uses a full CPU solution.

## How it works

The engine uses three steps for the rendering:

1. **raycaster**: performs the raycasting calculations for each screen column (DDL, texture size, perspective...)
2. **spritecaster**: performs the raycasting calculations for each sprite (sprite size, draw position, transformations...)
3. **drawer**: draws the results from the previous steps into the screen (walls, ceiling, floor and sprites)

The first two are [Compute Shaders][compute-shaders], and the last one is a [Fragment Shader][fragment-shader].

The player position and direction are handled in the CPU side, and the values are sent to the shaders each frame.

The map uses the same format as in the tutorials, but it is stored internally as a 2D texture with only red component in 8-bit unsigned int format (the map is accessed as `map[x][y]` which is not the common way to do it).

All textures are stored in a 2D Texture Array, where each layer is a different texture.

### raycaster shader

The shader runs in parallel calculations for each column of the screen (width). The input is the map texture as a `uimage2D` and the output is an array of structs with some data that will be used in the fragment shader. The struct has this look:

```c++
struct xdata {
    ivec2 draw;
    int side;
    uint textureNum;
    int texX;
    float step;
    float texPos;
    float distWall;
    vec2 floorWall;
};
```

Variables receive the same name as in tutorial (with some modifications if they are vectors).

The output uses a [Shared Storage Buffer Object][ssbo] that allows to allocate some space in the GPUs memory to read and write arbitrary data, and can be shared with shaders. The input, instead, is bound to the shader as a image (instead of texture) so the shader can read precisely the contents of the texture using `xy` coords (not `uv` coords, which is the common way to access textures).

The shader also receives as input the player `position`, the `direction` it looks at, the `plane` for the direction and the `screenSize` (the visible section, not the whole window).

With this input, the shader runs for each column of the screen (width) in parallel. Each instance calculates the values for that column and puts the result in the array of `struct xdata`. Uses the vertical version of the algorithm.

Currently, a screen cannot have more than 10000px, it is hardcoded like this :(

> Note: _the screen is always 4:3 aspect ratio, so your screen can be more than 10000px, but what cannot be true is `height * 4 / 3 > 10000`._

### spritecaster shader

While the previous shader is running, this other shader prepares its run (and may even run in parallel with the raycaster). The shader runs in parallel some calculations for each sprite in the map. The input is an array of `Sprite`s sorted by position (first the further sprites), and the output is an array of the result calculations. The two structs look like this:

```c++
// input
struct sprite {
    float x;
    float y;
    uint texture;
    int uDiv;
    int vDiv;
    float vMove;
};

// output
struct spritedata {
    int spriteWidth;
    int spriteHeight;
    float transformY;
    int spriteScreenX;
    ivec2 drawX;
    ivec2 drawY;
    int vMoveScreen;
    uint texture;
};
```

The shader also receives as input the player `position`, the `direction` it looks at, the `plane` for the direction and the `screenSize` (the visible section, not the whole window). Basically, the same `uniform`s as in the previous shader.

The input and output data are also [Shared Storage Buffer Object][ssbo]s. The first one is filled from the CPU side using a `std::vector<Sprite>` sorted by distance (first further ones) - update process is done once per second.

Each instance of the shader, calculates the position and size of a sprite and puts the result in the output buffer, so the Fragment Shader can read the results.

A map cannot have more than 100 sprites, it is limited to that (it is also hardocded sorry).

### raycaster drawer shader

This shader draws into a plane the results. The plane is located in front of the "camera" so it will always occupy the whole viewport. This shader receives the two shared buffers from the previous shaders and the 2D texture array that contains all textures, and draws into the plane.

Takes care of drawing the walls with its texture or the ceiling and floor, and then the sprites over. The sprites step tries to draw each sprite for each pixel. There is a `if` to prevent trying to draw the sprite directly, but the loop is there (this could be optimized). The sprite drawing in the tutorial used integer operations to avoid float calculations, in the shader floats are being used because it is faster.

### Map loader

The game without a map is useless. Maps are stored as yaml files and contain the map itself (which will converted into a texture) and its size, the initial player position and direction, and the sprites. There is an example of map in the maps folder.

> Note: _map loader does not have validations, so if the schema of the yaml is wrong, the engine could crash_

The first section of the file is the `map` which describes how it looks like. The `contents` contains how the map is. Each row is a `x` coord and each column is a `y` coord (remember the map is flipped from normal texture usage). Each value of the map represents a texture that will be drawed into the screen (the value must substract 1 to get the texture ID, so `1` will point to texture ID `0`). `0` means no wall, so the player can move through this section. There are some two optional values `floor` and `ceil` that modifies what is rendered in the floor and ceiling. The accepted values are a texture ID (`3` just the number) or a color (`[ 0.2, 0.2, 0.2 ]` rgb color as floats).

The second section is the `initial`s values for the player position, direction and plane. When the engine is loaded, will set these values to the ones in the yaml.

The third, and last, section is the `sprites` list. Each value of the list points to a sprite that will be placed in the position and the texture to draw.

### Texture loader

As mentioned several times, the textures are stored in a 2D Texture Array. This is the list of textures (and its ID) that are loaded into the engine:

| ID | texture | Purpose |
|----|---------|---------|
| 0  | `eagle.png` | Wall |
| 1  | `redbrick.png` | Wall |
| 2  | `purplestone.png` | Wall |
| 3  | `greystone.png` | Wall |
| 4  | `bluestone.png` | Wall |
| 5  | `mossy.png` | Wall |
| 6  | `wood.png` | Wall |
| 7  | `colorstone.png` | Wall |
| 8  | `barrel.png` | Sprite |
| 9  | `pillar.png` | Sprite |
| 10 | `greenlight.png` | Sprite |

The repository does not have the textures, they will be downloaded from the [tutorial][the-tutorial] during build from a download link almost at the end.

## How to build

The project uses git submodules and [CMake][cmake]. To clone the repository use:

```sh
git clone --recursive https://github.com/melchor629/raycastergl
```

The required external dependency is [GLFW][glfw]. On Linux, you can run:

```sh
#Arch/Manjaro
sudo pacman -S glfw-x11

#Debian/Ubuntu
sudo apt install -y libglfw3-dev libglfw3

#Windows (using vcpkg - for a manual installation of GLFW3 see below section)
vcpkg install glfw3
```

The project uses C++17, so ensure to have a very recent compiler (like GNU GCC 9, Clang 9, or MSVC 15 \[2019]).

Once all requirements are met and installed, you can now use `cmake` or `cmake-gui` to configure the project. An example for Linux:

```sh
mkdir build
cmake -S . -B build
cd build
make -j2
./raycastergl
```

And for Windows (using vcpkg):

```powershell
mkdir build
# If using VS 2017
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 15 2017 Win64"
# If using VS 2019
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 16 2019" -A Win64
# you can also open the solution in Visual Studio or open the folder in Visual Studio Code with cmake extension
cd build
cmake --build .
.\raycastergl
```

> Please use 64-bit version for Windows instead of 32-bit. All code is tested under 64-bit and some nasty bug can appear when using 32-bit...

### Manual installation of GLFW3 for Windows

1. Go to [GLFW3][glfw] page and download the latest source code (or the latest of version 3)
2. Extract the zip and open a terminal there
3. Create the folder `build`
4. Run `cmake -S . -B build -G "Visual Studio 15 2017 Win64" -DCMAKE_PREFIX_INSTALL="${PWD}\output"` or `cmake -S . -B build -G "Visual Studio 16 2019" -A Win64 -DCMAKE_PREFIX_INSTALL="${PWD}\output"`
5. `cd build`
6. `cmake --build .` to build the project
7. `cmake --install .` to install the project (it will be installed in a folder called `output`)
8. When running the `cmake` to configure the project, use `-DGLFW3_DIR=C:/Path/To/Glfw3/output/lib/cmake/glfw3` instead of `-DCMAKE_TOOLCHAIN_FILE=...`

## Running the engine

By default, the engine will run the `default.yaml` map at resolution 1333x1000, with V-Sync enabled. All of this can be changed with arguments. Run `./raycastergl --help` to change these options.

The controls are:

- `W`, `S`, `UP Arrow` and `DOWN Arrow` to move forward and backwards
- `A`, `D`, `LEFT Arrow` and `RIGHT Arrow` to rotate the camera
- `ESC` to close the game
- `F` to enter or exit fullscreen mode
- The mouse also works to move and rotate the camera

  [the-tutorial]: https://lodev.org/cgtutor/raycasting.html
  [compute-shaders]: https://www.khronos.org/opengl/wiki/Compute_Shader
  [fragment-shader]: https://www.khronos.org/opengl/wiki/Fragment_Shader
  [ssbo]: https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object
  [cmake]: https://cmake.org/
  [glfw]: https://www.glfw.org/
