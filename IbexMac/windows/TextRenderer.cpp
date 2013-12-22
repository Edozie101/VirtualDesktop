#include "TextRenderer.h"

#include <algorithm>
#include <math.h>

GLSLShaderProgram TextRenderer::textShaderProgram;

void TextRenderer::loadProgram() {
	if(textShaderProgram.shader.program == 0) {
		textShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/text.v.glsl", "/resources/shaders/text.f.glsl");
	}
	glUseProgram(textShaderProgram.shader.program);


	IbexTextUniformLocations[0] = glGetUniformLocation(textShaderProgram.shader.program, "MVP");
	IbexTextUniformLocations[1] = glGetUniformLocation(textShaderProgram.shader.program, "V");
	IbexTextUniformLocations[2] = glGetUniformLocation(textShaderProgram.shader.program, "M");
	IbexTextUniformLocations[3] = glGetUniformLocation(textShaderProgram.shader.program, "textureIn");
	IbexTextUniformLocations[4] = glGetUniformLocation(textShaderProgram.shader.program, "MV");
    IbexTextUniformLocations[5] = glGetUniformLocation(textShaderProgram.shader.program, "backgroundColor");
    IbexTextUniformLocations[6] = glGetUniformLocation(textShaderProgram.shader.program, "textColor");

	IbexTextAttribLocations[0] = glGetAttribLocation(textShaderProgram.shader.program, "vertexPosition_modelspace");
	IbexTextAttribLocations[1] = glGetAttribLocation(textShaderProgram.shader.program, "vertexUV");












	if(standardShaderProgram.shader.program == 0) standardShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/emissive.v.glsl", "/resources/shaders/emissive.f.glsl");
	glUseProgram(standardShaderProgram.shader.program);

	IbexDisplayFlatUniformLocations[0] = glGetUniformLocation(standardShaderProgram.shader.program, "MVP");
	IbexDisplayFlatUniformLocations[1] = glGetUniformLocation(standardShaderProgram.shader.program, "V");
	IbexDisplayFlatUniformLocations[2] = glGetUniformLocation(standardShaderProgram.shader.program, "M");
	IbexDisplayFlatUniformLocations[3] = glGetUniformLocation(standardShaderProgram.shader.program, "textureIn");
	IbexDisplayFlatUniformLocations[4] = glGetUniformLocation(standardShaderProgram.shader.program, "MV");

	IbexDisplayFlatAttribLocations[0] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexPosition_modelspace");
	IbexDisplayFlatAttribLocations[1] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexNormal_modelspace");
	IbexDisplayFlatAttribLocations[2] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexUV");
}

void TextRenderer::initializeFont()
{
#ifdef WIN32
	fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
#else
    fread(ttf_buffer, 1, 1<<20, fopen("/Library/Fonts/Georgia.ttf", "rb"));
#endif
	stbtt_fontinfo font;
	stbtt_InitFont(&font, ttf_buffer, 0);
	stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!

	scale = stbtt_ScaleForPixelHeight(&font, 32);
	stbtt_GetFontVMetrics(&font, &ascent,&descent,&lineGap);
	baseline = (int) (ascent*scale);

	// can free ttf_buffer at this point
	glGenTextures(1, &ftex);
	glBindTexture(GL_TEXTURE_2D, ftex);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512,512, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
	// can free temp_bitmap at this point
}

void TextRenderer::precompileText(float x, float y, std::vector<std::string> lines)
{
	if(!initialized) {
		initialized = true;
		loadProgram();
		initializeFont();
	}
	checkForErrors();

	vertices.clear();
	indices.clear();
    minX = minY = INT_MAX;
    maxX = maxY = INT_MIN;

	// assume orthographic projection with units = screen pixels, origin at top left
	int index = 0;
	int lineNum = (int)lines.size()-1;
	for(std::string line : lines) {
		x = 0;
		y = -lineNum * (ascent-descent+lineGap)*scale;
		unsigned char *text = (unsigned char *)line.data();
		while (*text) {
			if (*text >= 32 && *text < 128) {
				stbtt_aligned_quad q;
				stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);

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

				minX = std::min(minX,std::min(q.x0,q.x1));
				maxX = std::max(maxX,std::max(q.x0,q.x1));
				minY = std::min(minY,std::min(baseline-q.y0,baseline-q.y1)+y);
				maxY = std::max(maxY,std::max(baseline-q.y0,baseline-q.y1));

				for(int i = 0; i < 6; ++i) {
					indices.push_back(index++);
				}
			}
			++text;
		}
		--lineNum;

		if(minX > maxX) std::swap(minX,maxX);
		if(minY > maxY) std::swap(minY,maxY);
	}

	if(!vaoTextRenderer) glGenVertexArrays(1,&vaoTextRenderer);
	if(!checkForErrors()) {
		exit(1);
	}

	glBindVertexArray(vaoTextRenderer);
	if(!vboTextVertices) glGenBuffers(1, &vboTextVertices);
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
	if(!vboTextIndices) glGenBuffers(1, &vboTextIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboTextIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	if(!checkForErrors()) {
		exit(1);
	}
    glBindVertexArray(0);


	//////////////////////////////////////////////////////////////////////////////
	static GLfloat IbexDisplayFlatVertices[] = {
		-1.0, -1.0, 0.0, 0, 0,
		1.0, -1.0, 0.0, 1, 0,
		1.0,  1.0, 0.0, 1, 1,
		-1.0,  1.0, 0.0, 0, 1
	};

	static GLushort IbexDisplayFlatIndices[] = {
		0, 1, 2,
		0, 2, 3
	};

    if(vaoTextTextureRenderer == 0) {
        if(!vaoTextTextureRenderer) glGenVertexArrays(1,&vaoTextTextureRenderer);
        if(!checkForErrors()) {
            exit(1);
        }

        glBindVertexArray(vaoTextTextureRenderer);
        if(!vboTextTextureVertices) glGenBuffers(1, &vboTextTextureVertices);
        glBindBuffer(GL_ARRAY_BUFFER, vboTextTextureVertices);
        glBufferData(GL_ARRAY_BUFFER, sizeof(IbexDisplayFlatVertices), IbexDisplayFlatVertices, GL_STATIC_DRAW);
        if(!checkForErrors()) {
            exit(1);
        }
        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[0]);
        glVertexAttribPointer(IbexDisplayFlatAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, 0);
        glEnableVertexAttribArray(IbexDisplayFlatAttribLocations[2]);
        glVertexAttribPointer(IbexDisplayFlatAttribLocations[2], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, (GLvoid*) (sizeof(GLfloat) * 3));
        if(!checkForErrors()) {
            exit(1);
        }
        if(!vboTextTextureIndices) glGenBuffers(1, &vboTextTextureIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboTextTextureIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IbexDisplayFlatIndices), IbexDisplayFlatIndices, GL_STATIC_DRAW);
        if(!checkForErrors()) {
            exit(1);
        }
        glBindVertexArray(0);
    }
}

void TextRenderer::bindTextFBO() {
	glBindFramebuffer(GL_FRAMEBUFFER, fboText);
	glViewport(0,0,maxX-minX,maxY-minY);

    backgroundColor[0] = 0.1;
    backgroundColor[1] = 0.5;
    backgroundColor[2] = 0.8;
    backgroundColor[3] = 0.7;
    
    textColor[0] = 1;
    textColor[1] = 1;
    textColor[2] = 1;
    textColor[3] = 1;
    
	//glClearColor(0.01, 0.1,0.3,0.7);
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void TextRenderer::generateTextFBO()
{
	if(fboText == 0) glGenFramebuffers(1, &fboText);
	glBindFramebuffer(GL_FRAMEBUFFER, fboText);
    
	if(textTextureId == 0) glGenTextures(1, &textTextureId);
	glBindTexture(GL_TEXTURE_2D, textTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxX-minX, maxY-minY, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textTextureId, 0);
	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	glBindTexture(GL_TEXTURE_2D, 0);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Problem generating textTextureFBO" << std::endl;
		exit(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}				


void TextRenderer::renderTextToFrameBuffer()
{
	generateTextFBO();
	bindTextFBO();
    
	glm::mat4 orth = glm::ortho(minX,maxX,minY,maxY,-100.0f,100.0f);//-512.0f,512.0f,-512.0f,512.0f,-100.0f,100.0f);
    //glm::mat4 orth = glm::ortho(-maxX,maxX,-maxY,maxY,-100.0f,100.0f);//-512.0f,512.0f,-512.0f,512.0f,-100.0f,100.0f);
	//glm::mat4 orth = glm::ortho(-512.0f,512.0f,-512.0f,512.0f,-100.0f,100.0f);
    //glm::mat4 orth = glm::ortho(-1024.0f,1024.0f,-1024.0f,1024.0f,-1024.0f,1024.0f);
    //glm::mat4 orth = glm::ortho(-2.0f,2.0f,-2.0f,2.0f,-100.0f,100.0f);

	glUseProgram(textShaderProgram.shader.program);
	if(IbexTextUniformLocations[0] >= 0) {
        glUniformMatrix4fv(IbexTextUniformLocations[0], 1, GL_FALSE, &orth[0][0]);//MVP[0][0]);
    }

	if(IbexTextUniformLocations[3] >= 0)  {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ftex);
		glUniform1i(IbexTextUniformLocations[3], 0);
	}
    if(IbexTextUniformLocations[5] >= 0) {
        glUniform4fv(IbexTextUniformLocations[5], 1, backgroundColor);
    }
    if(IbexTextUniformLocations[6] >= 0) {
        glUniform4fv(IbexTextUniformLocations[6], 1, textColor);
    }

    //glDisable(GL_BLEND);
	glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vaoTextRenderer);
    glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);
    
    //glBindTexture(GL_TEXTURE_2D, ftex);//desktopTexture);
    //glBindVertexArray(vaoTextTextureRenderer);
	//glDrawElements(GL_TRIANGLES,  6, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);	
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void TextRenderer::renderTextDirect(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{

	glm::mat4 orth = glm::ortho(minX,maxX,minY,maxY,-100.0f,100.0f);//-512.0f,512.0f,-512.0f,512.0f,-100.0f,100.0f);

	if(shadowPass) {
		//glUseProgram(shadowProgram.shader.program);
		//glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
		return;
	} else {
		glUseProgram(textShaderProgram.shader.program);
		if(IbexTextUniformLocations[0] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[0], 1, GL_FALSE, &orth[0][0]);//MVP[0][0]);
		//if(IbexTextUniformLocations[1] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[1], 1, GL_FALSE, &V[0][0]);
		//if(IbexTextUniformLocations[2] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[2], 1, GL_FALSE, &M[0][0]);
		//if(IbexTextUniformLocations[4] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[4], 1, GL_FALSE, &(V*M)[0][0]);

		if(IbexTextUniformLocations[3] >= 0)  {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ftex);
			glUniform1i(IbexTextUniformLocations[3], 0);
		}
        if(IbexTextUniformLocations[5] >= 0) {
            glUniform4fv(IbexTextUniformLocations[5], 1, backgroundColor);
        }
        if(IbexTextUniformLocations[6] >= 0) {
            glUniform4fv(IbexTextUniformLocations[6], 1, textColor);
        }
	}

	glBindVertexArray(vaoTextRenderer);
	glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);	
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void TextRenderer::renderText(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{
	glm::mat4 orth = glm::ortho(-2.0f,2.0f,-2.0f,2.0f,-2.0f,2.0f);

	if(shadowPass) {
		//glUseProgram(shadowProgram.shader.program);
		//glUniformMatrix4fv(ShadowUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
		return;
	} else {
		glUseProgram(standardShaderProgram.shader.program);
		if(IbexDisplayFlatUniformLocations[0] >= 0) glUniformMatrix4fv(IbexDisplayFlatUniformLocations[0], 1, GL_FALSE, &orth[0][0]);//MVP[0][0]);
		/*if(IbexDisplayFlatUniformLocations[1] >= 0) glUniformMatrix4fv(IbexDisplayFlatUniformLocations[1], 1, GL_FALSE, &V[0][0]);
		if(IbexDisplayFlatUniformLocations[2] >= 0) glUniformMatrix4fv(IbexDisplayFlatUniformLocations[2], 1, GL_FALSE, &M[0][0]);
		if(IbexDisplayFlatUniformLocations[4] >= 0) glUniformMatrix4fv(IbexDisplayFlatUniformLocations[4], 1, GL_FALSE, &(V*M)[0][0]);*/

		if(IbexDisplayFlatUniformLocations[3] >= 0)  {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textTextureId);
			glUniform1i(IbexDisplayFlatUniformLocations[3], 0);
		}
	}

	glBindVertexArray(vaoTextTextureRenderer);
	glDrawElements(GL_TRIANGLES,  6, GL_UNSIGNED_SHORT, 0);

	//glBindVertexArray(0);	
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glUseProgram(0);
}

TextRenderer::TextRenderer() : initialized(false),
	vaoTextRenderer(0),
	vboTextVertices(0),
	vboTextIndices(0),
	fboText(0),
	textTextureId(0),
	textTextureWidth(0),
	textTextureHeight(0)
{
	memset(IbexTextUniformLocations,0,sizeof(IbexTextUniformLocations));
	memset(IbexTextAttribLocations,0,sizeof(IbexTextAttribLocations));
}


TextRenderer::~TextRenderer()
{
	if(vaoTextRenderer) glDeleteVertexArrays(1, &vaoTextRenderer);
	if(vboTextVertices) glDeleteBuffers(1, &vboTextVertices);
	if(vboTextIndices) glDeleteBuffers(1, &vboTextIndices);
}
