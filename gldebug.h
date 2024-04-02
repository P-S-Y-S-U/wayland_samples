#ifndef GL_DEBUG_H
#define GL_DEBUG_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>

static PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR = NULL;

const char* GetTypeEnum( GLenum type )
{
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR_KHR:
        return "ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR:
        return "DEPRECATED BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR:
        return "UNDEFINED BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY_KHR:
        return "PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE_KHR:
        return "PERFORMANCE";
    case GL_DEBUG_TYPE_OTHER_KHR:
        return "OTHER";
    case GL_DEBUG_TYPE_MARKER_KHR:
        return "MARKER";
    default:
        return "unknown type";
    }
}

const char* GetSeverity( GLenum severity )
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_KHR:
        return "HIGH";
    case GL_DEBUG_SEVERITY_MEDIUM_KHR:
        return "MEDIUM";
    case GL_DEBUG_SEVERITY_LOW_KHR:
        return "LOW";
    case GL_DEBUG_SEVERITY_NOTIFICATION_KHR:
        return "NOTIFICATION";
    default:
        return "unknown";
    }
}

const char* GetSource( GLenum source )
{
    switch (source)
    {
    case GL_DEBUG_SOURCE_API_KHR:
        return "SOURCE API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR:
        return "WINDOW SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER_KHR:
        return "SHADER COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY_KHR:
        return "THIRD PARTY";
    case GL_DEBUG_SOURCE_APPLICATION_KHR:
        return "APPLICATION";
    case GL_DEBUG_SOURCE_OTHER_KHR:
        return "OTHER";
    default:
        return "unknown";
    }
}

static void GL_APIENTRY DebugMessageCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity, GLsizei len,
    const GLchar* message, 
    const void* userParam
)
{
    printf(
        "GL CALLBACK: [%s] source = %s, severity = %s, message = %s\n",
        GetTypeEnum(type), GetSource(source), GetSeverity(severity),
        message
    );
}

void InitDebugMessenger()
{
    glDebugMessageCallbackKHR = (PFNGLDEBUGMESSAGECALLBACKKHRPROC) eglGetProcAddress("glDebugMessageCallbackKHR");

	if( !glDebugMessageCallbackKHR )
	{
		printf("Failed to get glDebugMessageCallbackKHR\n");
	}

	glDebugMessageCallbackKHR(
		(GLDEBUGPROCKHR) DebugMessageCallback, 
		NULL
	);
	glEnable(GL_DEBUG_OUTPUT_KHR);

	GLboolean param;
	printf("DEBUG %s\n", glIsEnabled(GL_DEBUG_OUTPUT_KHR) ? "ENABLED" : "DISABLED");
}

#endif 