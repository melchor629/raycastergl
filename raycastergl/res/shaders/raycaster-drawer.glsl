#version 430 core

// Raycaster based on https://lodev.org/cgtutor/raycasting.html
// this file only grabs the calculated line heights, and draws them
// Raycaster for sprites based on https://lodev.org/cgtutor/raycasting3.html
// this file only grabs the calculated sizes and positions and draws them

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

out vec4 FragColor;

in vec2 uvCoord;

layout(std430, binding=2) buffer raycasterOutput {
    readonly xdata res[10000];
};
layout(std430, binding=3) buffer dataOutput {
    readonly spritedata spriteResults[100];
};
layout(rgba32f, binding=1) uniform image2DArray textures;
layout(location=1) uniform ivec2 screenSize;
layout(location=2) uniform vec2 position;
layout(location=3) uniform uint spriteCount;
layout(location=4) uniform vec4 floorTex;
layout(location=5) uniform vec4 ceilTex;

void drawSprite(int spriteNum, float distWall, float widthf, float heightf) {
    spritedata spriteData = spriteResults[spriteNum];

    bool insideX = spriteData.drawX.x <= widthf && widthf <= spriteData.drawX.y;
    bool insideY = spriteData.drawY.x <= heightf && heightf <= spriteData.drawY.y;
    bool validZBuffer = spriteData.transformY > 0 && spriteData.transformY < distWall;
    if(insideX && insideY && validZBuffer) {
        ivec2 texSize = imageSize(textures).xy;
        // here I'm using float calculations because is a bit faster
        int texX = int((widthf - (-spriteData.spriteWidth * 0.5 + spriteData.spriteScreenX)) * texSize.x / spriteData.spriteWidth);
        int vMoveScreen = spriteData.vMoveScreen;
        float d = (heightf - vMoveScreen) - screenSize.y * 0.5 + spriteData.spriteHeight * 0.5;
        int texY = texSize.y - int((d * texSize.y) / spriteData.spriteHeight);

        vec4 color = imageLoad(textures, ivec3(texX, texY, spriteData.texture));
        // i don't know if there is a better way to check if this is black
        if(length(color.rgb) > 0.001) {
            FragColor = color;
        }
    }
}

void main() {
    xdata data = res[int(uvCoord.x * screenSize.x)];
    float heightf = float(screenSize.y) * uvCoord.y;

    // how much to increase the texture coordinate per screen pixel
    float step = data.step;
    // starting texture coordinate
    float texPos = data.texPos + step * (heightf - data.draw.x);

    if(data.draw.x <= heightf && heightf <= data.draw.y) {
        vec4 color;
        int texHeight = 64;
        // coordinates here are Y-inverted !!
        int texY = texHeight - int(texPos) % texHeight;

        color = imageLoad(textures, ivec3(data.texX, texY, data.textureNum));

        // make color darker for y-sides
        if(data.side == 1) color *= 0.75;
        FragColor = color;
    } else {
        if(data.draw.y < 0)
            data.draw.y = screenSize.y;

        // in fact it is not ceil, is floor, because of Y-inverted stuff on OpenGL
        bool isCeil = heightf < data.draw.y;
        // texture here are inverted because of the previous comment about isCeil
        vec4 tex = isCeil ? floorTex : ceilTex;
        if(tex.a == 0.0f) {
            FragColor = vec4(tex.rgb, 1.0f);
        } else {
            float currentDist;
            if(isCeil)
                currentDist = float(screenSize.y) / (2.0f * (float(screenSize.y) - heightf) - float(screenSize.y));
            else
                currentDist = float(screenSize.y) / (2.0f * heightf - float(screenSize.y));
            //float weight = (currentDist - distPlayer) / (distWall - distPlayer); // atm distPlayer is 0.0f
            float weight = currentDist / data.distWall;
            vec2 currentFloor = vec2(
                weight * data.floorWall.x + (1.0f - weight) * position.x,
                weight * data.floorWall.y + (1.0f - weight) * position.y
            );

            // coordinates here are Y-inverted !!
            ivec2 floorTex = ivec2(
                int(currentFloor.x * 64) % 64,
                64 - int(currentFloor.y * 64) % 64
            );

            FragColor = imageLoad(textures, ivec3(floorTex, int(tex.a)));
        }
    }

    // draws sprites after drawing the rest - this is really slow :/
    heightf = float(screenSize.y) * uvCoord.y;
    float widthf = float(screenSize.x) * uvCoord.x;
    for(int spriteNum = 0; spriteNum < spriteCount; spriteNum += 1) {
        drawSprite(spriteNum, data.distWall, widthf, heightf);
    }
}