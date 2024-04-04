#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb/stb_image_write.h"

static int16_t WritePixelsToFile(
    const char* filename,
    int width, int height,
    int channels,
    const void* pixelData   
)
{
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

#endif 