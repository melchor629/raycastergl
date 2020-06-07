#version 430 core

// Raycaster based on https://lodev.org/cgtutor/raycasting.html
// this file only calculates the line heights, which are stored into a shared buffer xD

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

layout(local_size_x=1, local_size_y=1) in;
layout(r8ui, binding=1) uniform uimage2D map;
layout(std430, binding=2) buffer dataOutput {
    restrict xdata res[10000];
};
layout(location=1) uniform vec2 position;
layout(location=2) uniform vec2 direction;
layout(location=3) uniform vec2 plane;
layout(location=4) uniform ivec2 screenSize;

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    uint x = pixelCoords.x;
    uint w = screenSize.x;
    int height = screenSize.y;
    ivec3 pixel = ivec3(0, 0, 0);
    ivec2 mapSize = imageSize(map);

    // x-coord in camera space
    float cameraX = 2 * float(x) / float(w) - 1;
    vec2 rayDir = direction + plane * cameraX;

    // where we are now (the box from the map)
    ivec2 mapPos = ivec2(position);
    // length of the ray from one x/y-side to the next x/y-side (simplified formula)
    vec2 deltaDist = vec2(abs(1 / rayDir.x), abs(1 / rayDir.y));
    // direction of the ray
    ivec2 step;
    // length of the ray from current position to the next x/y-side
    vec2 sideDist;

    // calculate initial step and sideDist values
    if(rayDir.x < 0) {
        step.x = -1;
        sideDist.x = (position.x - mapPos.x) * deltaDist.x;
    } else {
        step.x = 1;
        sideDist.x = (mapPos.x + 1.0f - position.x) * deltaDist.x;
    }

    if(rayDir.y < 0) {
        step.y = -1;
        sideDist.y = (position.y - mapPos.y) * deltaDist.y;
    } else {
        step.y = 1;
        sideDist.y = (mapPos.y + 1.0f - position.y) * deltaDist.y;
    }

    // perform DDA
    int side = 0;
    bool hit = false;
    while(!hit) {
        if(sideDist.x < sideDist.y) {
            sideDist.x += deltaDist.x;
            mapPos.x += step.x;
            side = 0;
        } else {
            sideDist.y += deltaDist.y;
            mapPos.y += step.y;
            side = 1;
        }

        // map coords are reversed!
        uint mapValue = imageLoad(map, mapPos.yx).r;
        hit = mapValue > 0;
    }

    // distance between the camera and the wall (perpendicullar not euclidean)
    float perpWallDist;
    if(side == 0) {
        perpWallDist = (mapPos.x - position.x + (1.0f - step.x) / 2.0f) / rayDir.x;
    } else {
        perpWallDist = (mapPos.y - position.y + (1.0f - step.y) / 2.0f) / rayDir.y;
    }

    // calculate the height of the line to draw
    int lineHeight = int(height / perpWallDist);

    // calculate lowest and highest pixel to fill in current stripe
    int drawStart = -lineHeight / 2 + height / 2;
    if(drawStart < 0) {
        drawStart = 0;
    }
    int drawEnd = lineHeight / 2 + height / 2;
    if(drawEnd >= height) {
        drawEnd = height - 1;
    }

    // texturing calculations (map coords are reversed)
    uint texNum = imageLoad(map, mapPos.yx).r - 1;

    // calculate value of wallX - where exactly the wall was hit
    float wallX;
    if(side == 0) {
        wallX = position.y + perpWallDist * rayDir.y;
    } else {
        wallX = position.x + perpWallDist * rayDir.x;
    }
    wallX -= floor(wallX);

    // x coordinate on the texture
    int texWidth = 64; // TODO not hardcode texWidth and texHeight
    int texHeight = 64;
    int texX = int(wallX * float(texWidth));
    if(side == 0 && rayDir.x > 0) texX = texWidth - texX - 1;
    if(side == 1 && rayDir.y < 0) texX = texWidth - texX - 1;

    // floow/ceil casting (vertical version to take advantage of this loop)
    vec2 floorWall;

    // 4 different wall directions possible
    if(side == 0 && rayDir.x > 0) {
        floorWall.x = mapPos.x;
        floorWall.y = mapPos.y + wallX;
    } else if(side == 0 && rayDir.x < 0) {
        floorWall.x = mapPos.x + 1.0f;
        floorWall.y = mapPos.y + wallX;
    } else if(side == 1 && rayDir.y > 0) {
        floorWall.x = mapPos.x + wallX;
        floorWall.y = mapPos.y;
    } else {
        floorWall.x = mapPos.x + wallX;
        floorWall.y = mapPos.y + 1.0f;
    }

    float distWall = perpWallDist;

    // store information for the fragment shader
    res[x].draw = ivec2(drawStart, drawEnd);
    res[x].side = side;
    res[x].textureNum = texNum;
    res[x].texX = texX;
    res[x].step = float(texHeight) / float(lineHeight);
    res[x].texPos = float(drawStart - height / 2 + lineHeight / 2) * res[x].step;
    res[x].distWall = distWall; // <- zbuffer
    res[x].floorWall = floorWall;
}
