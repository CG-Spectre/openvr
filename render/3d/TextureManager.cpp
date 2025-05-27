//
// Created by aidan on 5/24/2025.
//
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "TextureManager.h"

#include <iostream>

#include "stb_image.h"
#include "stb_image_write.h"

int TextureManager::addTexture(std::string name) {
    this->textures.push_back(name);
    return textures.size() - 1;
}



void TextureManager::initialize() {

    std::string dir = "textures";
    bool success = true;
    std::cout << "Initializing textures." << std::endl;
    int outWidth = 0;
    int outHeight = 0;
    int outChannels = 4;
    for (int i = 0; i < textures.size(); i++) {
        int width1, height1, channels1;
        unsigned char* image = stbi_load((dir+"/"+textures[i]).c_str(), &width1, &height1, &channels1, 0);
        outHeight = std::max(outHeight, height1);
        this->serialized.uvSerialized.push_back(outWidth);
        this->serialized.widthsSerialized.push_back(width1);
        this->serialized.heightsSerialized.push_back(height1);
        outWidth += width1;
        outChannels = std::max(outChannels, channels1);
        stbi_image_free(image);
    }
    //std::cout << "Image width: " << outChannels << std::endl;
    this->image = new unsigned char[outWidth * outHeight * outChannels];
    int count = 0;
    int currentWidth = 0;
    for (int i = 0; i < textures.size(); i++) {
        count++;
        int width1, height1, channels1;
        unsigned char* image1 = stbi_load((dir+"/"+textures[i]).c_str(), &width1, &height1, &channels1, 0);

        for (int y = 0; y < height1; y++) {
            for (int x = 0; x < width1; x++) {
                for (int c = 0; c < outChannels; c++) {
                    if (c == 3 && channels1 < 4) {
                        this->image[(y * outWidth + (x + currentWidth)) * outChannels + c] = 255;
                    }else {
                        //std::cout << image1[(y * width1 + x) * channels1 + c] << std::endl;
                        this->image[(y * outWidth + (x + currentWidth)) * outChannels + c] =
                        image1[(y * width1 + x) * channels1 + c];
                    }

                }
            }
        }
        currentWidth += width1;
        stbi_image_free(image1);
    }
    width = outWidth;
    height = outHeight;
    channels = outChannels;
    //this->image = output;
    if (width == 0) {
        success = false;
    }
    std::cout << "Successfully initialized " << count << " textures.";
    stbi_write_png((dir+"/map/"+"textures.png").c_str(), outWidth, outHeight, outChannels, this->image, outWidth * outChannels);
    /*int width1, width2, height1, height2, channels1, channels2;
    unsigned char* image1 = stbi_load((dir+"/grass.jpeg").c_str(), &width1, &height1, &channels1, 3);
    unsigned char* image2 = stbi_load((dir+"/turbo.png").c_str(), &width2, &height2, &channels2, 4);
    int outWidth = width1 + width2;
    int outHeight = std::max(height1, height2);
    int outChannels = std::max(channels1, channels2);
    std::cout << outChannels << std::endl;
    unsigned char* output = new unsigned char[outWidth * outHeight * outChannels];
    // Copy img1
    for (int y = 0; y < height1; y++) {
        for (int x = 0; x < width1; x++) {
            for (int c = 0; c < outChannels; c++) {
                if (c == 3) {
                    output[(y * outWidth + x) * outChannels + c] = 255;
                }else {
                    //std::cout << image1[(y * width1 + x) * channels1 + c] << std::endl;
                    output[(y * outWidth + x) * outChannels + c] =
                    image1[(y * width1 + x) * channels1 + c];
                }

            }
        }
    }

    // Copy img2 right after img1
    for (int y = 0; y < height2; y++) {
        for (int x = 0; x < width2; x++) {
            for (int c = 0; c < channels2; c++) {
                if (c == 3) {
                    //::cout << static_cast<int>(image2[(y * width2 + x) * channels2 + c]) << std::endl;
                    output[(y * outWidth + (x + width1)) * outChannels + c] = image2[(y * width2 + x) * channels2 + c];
                }else {
                    output[(y * outWidth + (x + width1)) * outChannels + c] =
                    image2[(y * width2 + x) * channels2 + c];
                }
            }
        }
    }*/
    //stbi_write_png("textures/map.png", outWidth, outHeight, outChannels, output, outWidth * outChannels);

}

void TextureManager::freeTextures() {
    stbi_image_free(image);
}
