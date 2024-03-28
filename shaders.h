#ifndef SHADERS_H
#define SHADERS_H

static const char* vertex_color_vertex_shader_text = 
	"#version 300 es\n"
    "uniform mat4 mvp;\n"
    "in vec4 pos;\n"
    "in vec4 color;\n"
	"out vec4 vertex_color;\n"
	"void main() {\n"
	" gl_Position = mvp * pos;\n"
	" vertex_color = color;\n"
	"}\n";

static const char* vertex_color_frag_shader_text = 
	"#version 300 es\n"
	"precision highp float;\n"
	"in vec4 vertex_color;\n"
	"out vec4 fragColor;\n"
	"void main() {\n"
	"  fragColor = vertex_color;\n"
	"}\n";

static const char* sample_texture_vertex_shader_text = 
	"#version 300 es\n"
    "uniform mat4 mvp;\n"
    "in vec4 pos;\n"
    "in vec2 texcoord;\n"
    "out vec2 out_texcoord;\n"
	"void main() {\n"
	" gl_Position = mvp * pos;\n"
    " out_texcoord = texcoord;\n"
	"}\n";

static const char* sample_texture_frag_shader_text = 
	"#version 300 es\n"
	"uniform sampler2D texSampler;\n"
	"precision highp float;\n"
    "in vec2 out_texcoord;\n"
	"out vec4 fragColor;\n"
	"void main() {\n"
	"  fragColor = texture2D(texSampler, out_texcoord);\n"
	"}\n";

static GLuint createShader( const char* source, GLenum shaderType )
{
	GLuint shader;
	GLint status;

	shader = glCreateShader(shaderType);

	glShaderSource(shader, 1, (const char **) &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetShaderInfoLog(shader, 1000, &len, log);
		fprintf(stderr, "Error: compiling %s: %*s\n",
			shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment",
			len, log);
		exit(1);
	}

	return shader;
}

static GLuint createGPUProgram( const char* vertex_shader_source, const char* frag_shader_source )
{
	GLuint frag, vert;
	GLuint program;
	GLint status;

	frag = createShader( frag_shader_source, GL_FRAGMENT_SHADER);
	vert = createShader( vertex_shader_source, GL_VERTEX_SHADER);

	program = glCreateProgram();
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetProgramInfoLog(program, 1000, &len, log);
		fprintf(stderr, "Error: linking:\n%*s\n", len, log);
		exit(1);
	}

	return program;
}

#endif 