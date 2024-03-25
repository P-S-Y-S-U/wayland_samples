#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <stdio.h>

enum CaptureTarget
{
    DEFAULT_FRAME_BUFFER,
    TEX_ID
};

enum TexDownloadApproach
{
    FBO,
    PBO
};

struct TexReader
{
    GLuint* m_glPBOarray;
    size_t m_pboBufferSizeInBytes;
    uint32_t m_numOfPBOs;
    GLenum m_pbobuffertype;

    uint32_t m_currentDownload;
    uint32_t m_totalDownload;
    uint32_t m_asyncDownloadLimit;

    PFNGLMAPBUFFERRANGEEXTPROC gl_map_buffer_range_EXT;
    PFNGLUNMAPBUFFEROESPROC gl_unmap_buffer_oes;
    uint8_t m_bInitialised;
};

static int16_t DownloadUsingFBO(
    enum CaptureTarget target,
    uint32_t xOffset, uint32_t yOffset,
    uint32_t imgWidth, uint32_t imgHeight,
    GLuint texId, GLenum pixelFormatToPack,
    uint8_t bPackReverse, uint16_t bytespp,
    uint8_t* pCPUpixeldump, size_t pixelDumpSizeInBytes
);

static int16_t DownloadUsingPBO(
    enum CaptureTarget target,
    uint32_t xOffset, uint32_t yOffset,
    uint32_t imgWidth, uint32_t imgHeight,
    GLuint texId, GLenum pixelFormatToPack,
    uint8_t bPackReverse, uint16_t bytespp,
    uint8_t* pCPUpixeldump, size_t pixelDumpSizeInBytes
);

static void AllocatePBOs(
    size_t bufferSizeInBytes
);

static void DestroyPBOs();

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

static struct TexReader* GetTexReaderInstance()
{
    static struct TexReader* pSingletonTexReader = NULL;

    if( !pSingletonTexReader )
    {
        printf("Initialising TexReader Singleton\n");

        pSingletonTexReader = malloc(sizeof(struct TexReader));
        pSingletonTexReader->m_glPBOarray = NULL;
        pSingletonTexReader->m_numOfPBOs = 1;
        pSingletonTexReader->m_pbobuffertype = GL_PIXEL_PACK_BUFFER_NV;
        pSingletonTexReader->m_currentDownload = 0;
        pSingletonTexReader->m_totalDownload = 0;
        pSingletonTexReader->m_asyncDownloadLimit = 2;
        pSingletonTexReader->gl_map_buffer_range_EXT = (void*)( eglGetProcAddress("glMapBufferRangeEXT") );
        pSingletonTexReader->gl_unmap_buffer_oes = (void*)( eglGetProcAddress( "glUnmapBufferOES" ) );
        pSingletonTexReader->m_bInitialised = 1;
    }

    return pSingletonTexReader;
}

static void InitTexReader( 
    uint32_t numOfPBO,
    uint32_t asyncDownloadLimit
)
{
    struct TexReader* pTexReader = GetTexReaderInstance();

    pTexReader->m_numOfPBOs = numOfPBO;
    pTexReader->m_asyncDownloadLimit = asyncDownloadLimit;
}

static int16_t DownloadPixelsFromGPU(
    enum CaptureTarget target,
    enum TexDownloadApproach downloadApproach,
    uint32_t xOffset, uint32_t yOffset,
    uint32_t imgWidth, uint32_t imgHeight,
    GLuint texId, GLenum pixelFormatToPack,
    uint8_t bPackReverse, uint16_t bytespp,
    uint8_t* pCPUpixeldump, size_t pixelDumpSizeInBytes
)
{
    switch(downloadApproach)
    {
    case FBO:
        return DownloadUsingFBO( 
            target, 
            xOffset, yOffset,
            imgWidth, imgHeight,
            texId, pixelFormatToPack,
            bPackReverse, bytespp,
            pCPUpixeldump, pixelDumpSizeInBytes
        );
    case PBO:
        return DownloadUsingPBO(
            target,
            xOffset, yOffset,
            imgWidth, imgHeight,
            texId, pixelFormatToPack,
            bPackReverse, bytespp,
            pCPUpixeldump, pixelDumpSizeInBytes
        );
    default:
        return -1;
    }
}

static void RevertGLState( GLuint* fbo )
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, fbo);
}

static int16_t DownloadUsingFBO(
    enum CaptureTarget target,
    uint32_t xOffset, uint32_t yOffset,
    uint32_t imgWidth, uint32_t imgHeight,
    GLuint texId, GLenum pixelFormatToPack,
    uint8_t bPackReverse, uint16_t bytespp,
    uint8_t* pCPUpixeldump, size_t pixelDumpSizeInBytes
)
{
    int16_t downloadReslt = -1;

    GLenum status;
    GLuint fbo;

    if( target == DEFAULT_FRAME_BUFFER )
    {
        status = glCheckFramebufferStatus( GL_FRAMEBUFFER );

        if (status != GL_FRAMEBUFFER_COMPLETE) {
		    printf("fbo error: %d\n", status);
            return downloadReslt;
	    }

        if (bPackReverse)
    		glPixelStorei(GL_PACK_REVERSE_ROW_ORDER_ANGLE, GL_FALSE);
	    glPixelStorei(GL_PACK_ALIGNMENT, bytespp);
        glReadPixels(
            xOffset, yOffset,
            imgWidth, imgHeight,
            pixelFormatToPack,
            GL_UNSIGNED_BYTE,
            pCPUpixeldump
        );

        if( glCheckError() )
        {
            printf("Surface Dump Using FBO failed\n");
            return -1;
        }

        downloadReslt = 1;
    }
    else if( target == TEX_ID )
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindTexture(GL_TEXTURE_2D, texId);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);

        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	    if (status != GL_FRAMEBUFFER_COMPLETE) {
	    	printf("fbo error: %d\n", status);
            RevertGLState( &fbo );
            return downloadReslt;
	    }

        if (bPackReverse)
    		glPixelStorei(GL_PACK_REVERSE_ROW_ORDER_ANGLE, GL_FALSE);
	    glPixelStorei(GL_PACK_ALIGNMENT, bytespp);
        glReadPixels(
            xOffset, yOffset,
            imgWidth, imgHeight,
            pixelFormatToPack,
            GL_UNSIGNED_BYTE,
            pCPUpixeldump
        );

        if( glCheckError() )
        {
            printf("Surface Dump Using FBO failed\n");
            RevertGLState( &fbo );
            return downloadReslt;
        }

        RevertGLState( &fbo );
        downloadReslt = 1;
    }

    return downloadReslt;
}

static void TriggerCaptureUsingPBO(
    uint32_t xOffset, uint32_t yOffset,
    uint32_t imgWidth, uint32_t imgHeight,
    GLenum pixelFormatToPack,
    uint8_t bPackReverse, uint16_t bytespp,
    uint8_t* pCPUpixeldump, size_t pixelDumpSizeInBytes
)
{
    struct TexReader* pTexReader = GetTexReaderInstance();

    uint8_t* pMappedBuffer = NULL;

    if( pTexReader->m_totalDownload < pTexReader->m_numOfPBOs )
    {
        glBindBuffer( pTexReader->m_pbobuffertype, pTexReader->m_glPBOarray[pTexReader->m_currentDownload] );

        if (bPackReverse)
    		glPixelStorei(GL_PACK_REVERSE_ROW_ORDER_ANGLE, GL_FALSE);
	    glPixelStorei(GL_PACK_ALIGNMENT, bytespp);

        glReadPixels(
            xOffset, yOffset,
            imgWidth, imgHeight,
            pixelFormatToPack,
            GL_UNSIGNED_BYTE,
            0
        );

        printf("glReadPixels() with pbo: %d\n", pTexReader->m_glPBOarray[pTexReader->m_currentDownload]);
    }
    else 
    {
        glBindBuffer( pTexReader->m_pbobuffertype, pTexReader->m_glPBOarray[pTexReader->m_currentDownload] );
            
        pMappedBuffer = (uint8_t*)( pTexReader->gl_map_buffer_range_EXT(
            pTexReader->m_pbobuffertype,
            0,
            pTexReader->m_pboBufferSizeInBytes,
            GL_MAP_READ_BIT_EXT
        ));

        if( pMappedBuffer )
        {
            memcpy( pCPUpixeldump, pMappedBuffer, pixelDumpSizeInBytes );
            pTexReader->gl_unmap_buffer_oes( pTexReader->m_pbobuffertype );
        }
        else 
        {
           printf("Failed to Map the Buffer\n");
        }

        printf("Trigger For Next Capture\n");
        printf("glReadPixels() with pbo: %d\n", pTexReader->m_glPBOarray[pTexReader->m_currentDownload]);

        if (bPackReverse)
    		glPixelStorei(GL_PACK_REVERSE_ROW_ORDER_ANGLE, GL_FALSE);
	    glPixelStorei(GL_PACK_ALIGNMENT, bytespp);
        glReadPixels(
            xOffset, yOffset,
            imgWidth, imgHeight,
            pixelFormatToPack,
            GL_UNSIGNED_BYTE,
            0
        );
    }

    pTexReader->m_currentDownload++;
    pTexReader->m_currentDownload = pTexReader->m_currentDownload % pTexReader->m_numOfPBOs;
    pTexReader->m_totalDownload++;

    if(pTexReader->m_totalDownload == UINT32_MAX )
    {
        pTexReader->m_totalDownload = pTexReader->m_numOfPBOs;
    }
}

static int16_t DownloadUsingPBO(
    enum CaptureTarget target,
    uint32_t xOffset, uint32_t yOffset,
    uint32_t imgWidth, uint32_t imgHeight,
    GLuint texId, GLenum pixelFormatToPack,
    uint8_t bPackReverse, uint16_t bytespp,
    uint8_t* pCPUpixeldump, size_t pixelDumpSizeInBytes
)
{
    struct TexReader* pTexReader = GetTexReaderInstance();

    int16_t downloadReslt = -1;

    GLenum status;
    GLuint fbo;

    printf("TexReader Pointer %x\n", pTexReader);
    printf("%ld vs %ld\n", pixelDumpSizeInBytes, pTexReader->m_pboBufferSizeInBytes );

    if( pixelDumpSizeInBytes != pTexReader->m_pboBufferSizeInBytes  )
    {
        if( pTexReader->m_pboBufferSizeInBytes  != 0 )
            DestroyPBOs();
        AllocatePBOs( pixelDumpSizeInBytes );
    }

    if( target == TEX_ID )
    {
        glGenFramebuffers(1, &fbo);
        glCheckError();
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, texId);
        glCheckError();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
        glCheckError();

        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	    if (status != GL_FRAMEBUFFER_COMPLETE) {
	    	printf("fbo error: %d\n", status);

            RevertGLState( &fbo );
            return downloadReslt;
	    }
    }

    for( uint32_t i = 0; i < pTexReader->m_asyncDownloadLimit; i++ )
    {
        TriggerCaptureUsingPBO(
            xOffset, yOffset,
            imgWidth, imgHeight, 
            pixelFormatToPack,
            bPackReverse, bytespp,
            pCPUpixeldump, pixelDumpSizeInBytes
        );
    }

    downloadReslt = 1;

    if( target == TEX_ID )
    {
        RevertGLState( &fbo );
    }

    return downloadReslt;
}

static void AllocatePBOs(
    size_t bufferSizeInBytes
)
{
    printf("Allocating PBOs\n");

    struct TexReader* pTexReader = GetTexReaderInstance();

    pTexReader->m_glPBOarray = malloc( sizeof(GLuint) * pTexReader->m_numOfPBOs );
    pTexReader->m_currentDownload = 0;
    pTexReader->m_totalDownload = 0;
    pTexReader->m_pboBufferSizeInBytes = bufferSizeInBytes;

    glGenBuffers( pTexReader->m_numOfPBOs, pTexReader->m_glPBOarray );
    glCheckError();
    for( uint32_t i = 0; i < pTexReader->m_numOfPBOs; i++ )
    {
        glBindBuffer( pTexReader->m_pbobuffertype, pTexReader->m_glPBOarray[i]);
        glCheckError();
        glBufferData( 
            pTexReader->m_pbobuffertype, 
            pTexReader->m_pboBufferSizeInBytes,
            NULL,
            GL_STREAM_DRAW
        );
        glCheckError();

       printf("Binded PBO[%d] = %d" ", with %ld bytes\n", i, pTexReader->m_glPBOarray[i], bufferSizeInBytes);
    }

    glBindBuffer( pTexReader->m_pbobuffertype, 0 );
}

static void DestroyPBOs()
{
    printf("Deleting PBOs\n");

    struct TexReader* pTexReader = GetTexReaderInstance();

    glBindBuffer( pTexReader->m_pbobuffertype, 0 );
    glDeleteBuffers( pTexReader->m_numOfPBOs, pTexReader->m_glPBOarray );
    free(pTexReader->m_glPBOarray);
    pTexReader->m_glPBOarray = NULL;
}