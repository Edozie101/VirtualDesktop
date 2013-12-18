#include "TextRenderer.h"

#include <vector>

#include "../opengl_helpers.h"
#include "../GLSLShaderProgram.h"
#include "../simpleworld_plugin/SimpleWorldRendererPlugin.h"

static unsigned char ttf_buffer[1<<20];
static unsigned char temp_bitmap[512*512];

static stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
static GLuint ftex;

static GLint IbexTextUniformLocations[5] = { 0, 0, 0, 0, 0};
static GLint IbexTextAttribLocations[2] = { 0, 0 };

static GLuint vaoTextRenderer = 0;
static std::vector<GLfloat> vertices;
static std::vector<GLuint> indices;
static GLuint vboTextVertices = 0;
static GLuint vboTextIndices = 0;

static GLSLShaderProgram textShaderProgram;
void loadProgram() {
	textShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/text.v.glsl", "/resources/shaders/text.f.glsl");
    glUseProgram(textShaderProgram.shader.program);
        
        
    IbexTextUniformLocations[0] = glGetUniformLocation(textShaderProgram.shader.program, "MVP");
    IbexTextUniformLocations[1] = glGetUniformLocation(textShaderProgram.shader.program, "V");
    IbexTextUniformLocations[2] = glGetUniformLocation(textShaderProgram.shader.program, "M");
    IbexTextUniformLocations[3] = glGetUniformLocation(textShaderProgram.shader.program, "textureIn");
    IbexTextUniformLocations[4] = glGetUniformLocation(textShaderProgram.shader.program, "MV");
        
    IbexTextAttribLocations[0] = glGetAttribLocation(textShaderProgram.shader.program, "vertexPosition_modelspace");
    IbexTextAttribLocations[1] = glGetAttribLocation(textShaderProgram.shader.program, "vertexUV");
}

int ascent,baseline,ch=0;
float scale, xpos=0;
void my_stbtt_initfont(void)
{
	fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
	stbtt_fontinfo font;
	stbtt_InitFont(&font, ttf_buffer, 0);
	stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!

	scale = stbtt_ScaleForPixelHeight(&font, 32);
	stbtt_GetFontVMetrics(&font, &ascent,0,0);
	baseline = (int) (ascent*scale);

	// can free ttf_buffer at this point
	glGenTextures(1, &ftex);
	glBindTexture(GL_TEXTURE_2D, ftex);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
	// can free temp_bitmap at this point
}

void my_stbtt_generate(float x, float y, char *text)
{
	checkForErrors();
	
	x = 0;
	y = 0;

	// assume orthographic projection with units = screen pixels, origin at top left
	int index = 0;
	while (*text) {
		if (*text >= 32 && *text < 128) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl,0=old d3d
			//glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y0);
			//glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y0);
			//glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y1);
			//glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y1);

			// bottom right triangle
			vertices.push_back(q.x0);
			vertices.push_back((baseline-q.y0));
			vertices.push_back(0);
			vertices.push_back(q.s0);
			vertices.push_back(q.t0);

			vertices.push_back(q.x1);
			vertices.push_back((baseline-q.y0));
			vertices.push_back(0);
			vertices.push_back(q.s1);
			vertices.push_back(q.t0);
			
			vertices.push_back(q.x1);
			vertices.push_back((baseline-q.y1));
			vertices.push_back(0);
			vertices.push_back(q.s1);
			vertices.push_back(q.t1);

			// top left triangle
			vertices.push_back(q.x0);
			vertices.push_back((baseline-q.y0));
			vertices.push_back(0);
			vertices.push_back(q.s0);
			vertices.push_back(q.t0);

			vertices.push_back(q.x1);
			vertices.push_back((baseline-q.y1));
			vertices.push_back(0);
			vertices.push_back(q.s1);
			vertices.push_back(q.t1);

			vertices.push_back(q.x0);
			vertices.push_back((baseline-q.y1));
			vertices.push_back(0);
			vertices.push_back(q.s0);
			vertices.push_back(q.t1);

			for(int i = 0; i < 6; ++i) {
				indices.push_back(index++);
			}
		}
		++text;
	}

	glGenVertexArrays(1,&vaoTextRenderer);
	if(!checkForErrors()) {
		exit(1);
	}

	glBindVertexArray(vaoTextRenderer);
	glGenBuffers(1, &vboTextVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboTextVertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	if(!checkForErrors()) {
		exit(1);
	}
	glEnableVertexAttribArray(IbexTextAttribLocations[0]);
	glVertexAttribPointer(IbexTextAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, 0);
	glEnableVertexAttribArray(IbexTextAttribLocations[1]);
	glVertexAttribPointer(IbexTextAttribLocations[1], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, (GLvoid*) (sizeof(GLfloat) * 3));
	if(!checkForErrors()) {
		exit(1);
	}
	glGenBuffers(1, &vboTextIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboTextIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	if(!checkForErrors()) {
		exit(1);
	}
}

void renderText(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{
	if(shadowPass) {
		//glUseProgram(shadowProgram.shader.program);
		//glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
		return;
	} else {
		glUseProgram(textShaderProgram.shader.program);
		if(IbexTextUniformLocations[0] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
		if(IbexTextUniformLocations[1] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[1], 1, GL_FALSE, &V[0][0]);
		if(IbexTextUniformLocations[2] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[2], 1, GL_FALSE, &M[0][0]);
		if(IbexTextUniformLocations[4] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[4], 1, GL_FALSE, &(V*M)[0][0]);

		if(IbexTextUniformLocations[3] >= 0)  {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ftex);
			glUniform1i(IbexTextUniformLocations[3], 0);
		}
	}

	glBindVertexArray(vaoTextRenderer);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);	
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

TextRenderer::TextRenderer(void)
{
}


TextRenderer::~TextRenderer(void)
{
}
