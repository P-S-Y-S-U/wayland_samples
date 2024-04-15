#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#include <GLES2/gl2.h>

static uint8_t* LoadPixelsFromFile(
    const char* filePath,
    int* imgWidth, int* imgHeight,
    int* numOfChannels
)
{
    stbi_set_flip_vertically_on_load(1);

    return stbi_load(
        filePath,
        imgWidth, imgHeight,
        numOfChannels,
        STBI_rgb_alpha
    );
}

static int16_t WritePixelsToFile(
    const char* filename,
    int width, int height,
    int channels,
    const void* pixelData   
)
{
    stbi_flip_vertically_on_write(1);

    int16_t reslt = 0;
    if( !pixelData )
        printf("Pixel Not Dumped\n");
    else
    {
        reslt = stbi_write_png(
            filename,
            width, height,
            channels,
            pixelData,
            width * channels
        );
        if( reslt == 0 )
            printf("Failed to write image\n");
    }

    return reslt;
}

static void GenerateTextureFromImage(
    const char* filename,
    int* imgWidth, int* imgHeight,
    GLuint* texture,
    GLuint minFilter, GLuint magFilter
)
{
    int numOfChannels = 0;
    uint8_t* imgData = LoadPixelsFromFile(
        filename, 
        imgWidth, imgHeight,
        &numOfChannels
    );
    glGenTextures( 1, texture);
    glBindTexture( GL_TEXTURE_2D, *texture );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    glTexImage2D(
        GL_TEXTURE_2D, 0,
        GL_RGBA,
        *imgWidth, *imgHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        imgData
    );

    stbi_image_free(imgData);
}

static uint32_t uiClamp( uint32_t value, uint32_t min, uint32_t max ){
    const uint32_t t = t < min ? min : value;
    return t > max ? max : t;
};

static int iClamp( int value, int min, int max ){
    const int t = t < min ? min : value;
    return t > max ? max : t;
};

static float fClamp( float value, float min, float max ){
    const float t = t < min ? min : value;
    return t > max ? max : t;
};

#endif 