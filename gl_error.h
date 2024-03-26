#ifndef GL_ERROR_H
#define GL_ERROR_H

#include <string.h>
#include <GLES2/gl2.h>

static GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        char error[256];
        switch (errorCode)
        {
            case GL_INVALID_ENUM:
            {
                memcpy( error, "INVALID_ENUM", 13 );
            }
            break;
            case GL_INVALID_VALUE:
            {
                memcpy( error, "INVALID_VALUE", 14 );
            }  
            break;
            case GL_INVALID_OPERATION:
            {
                memcpy( error, "INVALID_OPERATION", 18 );
            }
            break;
            case GL_OUT_OF_MEMORY:
            {
                memcpy( error, "OUT_OF_MEMORY", 14 );
            }
            break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            { 
                memcpy( error, "INVALID_FRAMEBUFFER_OPERATION", 30 );
            }
            break;
        }
        printf("GL Error : %s | %s ( %d )\n", error, file, line);
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

#endif 