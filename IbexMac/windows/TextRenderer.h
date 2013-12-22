#ifndef __IBEX_TEXT_RENDERER_H__
#define __IBEX_TEXT_RENDERER_H__

#include "stb_truetype.h"

#include <string>
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
	
	void bindTextFBO();
	void generateTextFBO();
	void precompileText(float x, float y, std::vector<std::string> lines);
	void renderTextToFrameBuffer();
	void renderText(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
	void renderTextDirect(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);
private:
	bool initialized;

	unsigned char ttf_buffer[1<<20];
	unsigned char temp_bitmap[512*512];

	stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
	GLuint ftex;

	GLint IbexTextUniformLocations[7];
	GLint IbexTextAttribLocations[2];

	GLuint vaoTextRenderer;
	std::vector<GLfloat> vertices;
	std::vector<GLuint> indices;

	GLuint vaoTextTextureRenderer;
	GLuint vboTextVertices;
	GLuint vboTextIndices;
	GLuint vboTextTextureVertices;
	GLuint vboTextTextureIndices;

	static GLSLShaderProgram textShaderProgram;
	float minX, maxX, minY,maxY;

	int ascent,baseline,descent,lineGap;
	float scale;


	/////////////
	GLint IbexDisplayFlatUniformLocations[5];
    GLint IbexDisplayFlatAttribLocations[3];
    
	GLuint fboText;
	GLuint textTextureId;
	GLuint textTextureWidth;
	GLuint textTextureHeight;
    
    GLfloat backgroundColor[4];
    GLfloat textColor[4];
};

#endif // __IBEX_TEXT_RENDERER_H__
