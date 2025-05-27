//
// Created by aidan on 5/24/2025.
//

#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include <vector>
#include <bits/basic_string.h>

#include "SerializedTextureMap.h"
#include "SerializedTextureMap.h"
#include "Texture.h"


class TextureManager {
    public:
    int addTexture(std::string name);

    void initialize();

    void freeTextures();
    int width;
    int height;
    int channels;
    unsigned char* image;
    std::vector<std::string> textures;
    SerializedTextureMap serialized;
};



#endif //TEXTUREMANAGER_H
