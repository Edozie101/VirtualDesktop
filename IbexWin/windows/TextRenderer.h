#ifndef __IBEX_TEXT_RENDERER_H__
#define __IBEX_TEXT_RENDERER_H__

#include "stb_truetype.h"

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "../opengl_helpers.h"
#include "../GLSLShaderProgram.h"
#include "../simpleworld_plugin/SimpleWorldRendererPlugin.h"

class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	void loadProgram();
	void initializeFont(void);

	void precompileText(float x, float y, char *text);
	void renderText(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
private:
	bool initialized;

	unsigned char ttf_buffer[1<<20];
	unsigned char temp_bitmap[512*512];

	stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
	GLuint ftex;

	GLint IbexTextUniformLocations[5];
	GLint IbexTextAttribLocations[2];

	GLuint vaoTextRenderer;
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;
	GLuint vboTextVertices;
	GLuint vboTextIndices;

	static GLSLShaderProgram textShaderProgram;
};

#endif // __IBEX_TEXT_RENDERER_H__
