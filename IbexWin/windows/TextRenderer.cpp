#include "TextRenderer.h"

#include "../simpleworld_plugin/SimpleWorldRendererPlugin.h"

#include <algorithm>
#include <vector>
#include <string>
#include <math.h>

//GLSLShaderProgram Ibex::TextRenderer::textShaderProgram;

void Ibex::TextRenderer::loadProgram() {
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

	IbexTextAttribLocations[0] = glGetAttribLocation(textShaderProgram.shader.program, "vertexPosition_modelspace");
	IbexTextAttribLocations[1] = glGetAttribLocation(textShaderProgram.shader.program, "vertexUV");
    IbexTextAttribLocations[2] = glGetAttribLocation(textShaderProgram.shader.program, "textColor");


    // reusing emissive shader for text renderer
	if(standardShaderProgram.shader.program == 0) standardShaderProgram.loadShaderProgram(mResourcePath, "/resources/shaders/emissive.v.glsl", "/resources/shaders/emissive.f.glsl");
	glUseProgram(standardShaderProgram.shader.program);

	IbexDisplayFlatUniformLocations[0] = glGetUniformLocation(standardShaderProgram.shader.program, "MVP");
	IbexDisplayFlatUniformLocations[1] = glGetUniformLocation(standardShaderProgram.shader.program, "V");
	IbexDisplayFlatUniformLocations[2] = glGetUniformLocation(standardShaderProgram.shader.program, "M");
	IbexDisplayFlatUniformLocations[3] = glGetUniformLocation(standardShaderProgram.shader.program, "textureIn");
	IbexDisplayFlatUniformLocations[4] = glGetUniformLocation(standardShaderProgram.shader.program, "MV");
    IbexDisplayFlatUniformLocations[5] = glGetUniformLocation(standardShaderProgram.shader.program, "inFade");
    IbexDisplayFlatUniformLocations[6] = glGetUniformLocation(standardShaderProgram.shader.program, "offset");

	IbexDisplayFlatAttribLocations[0] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexPosition_modelspace");
	IbexDisplayFlatAttribLocations[1] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexNormal_modelspace");
	IbexDisplayFlatAttribLocations[2] = glGetAttribLocation(standardShaderProgram.shader.program, "vertexUV");
}

void Ibex::TextRenderer::initializeFont()
{
    unsigned char *ttf_buffer = new unsigned char[1<<22];
	unsigned char *temp_bitmap = new unsigned char[1024*256];//512*512];
    
#ifdef WIN32
	//size_t read = fread(ttf_buffer, 1, 1<<22, fopen("c:/windows/fonts/times.ttf", "rb"));
	//size_t read = fread(ttf_buffer, 1, 1<<22, fopen("c:/windows/fonts/Courbd.ttf", "rb"));
	size_t read = fread(ttf_buffer, 1, 1<<22, fopen("c:/windows/fonts/L_10646.ttf", "rb"));
#else
    //size_t read = fread(ttf_buffer, 1, 1<<22, fopen("/Library/Fonts/Georgia.ttf", "rb"));
    size_t read = fread(ttf_buffer, 1, 1<<22, fopen("/Library/Fonts/Andale Mono.ttf", "rb"));
    //size_t read = fread(ttf_buffer, 1, 1<<22, fopen("/Library/Fonts/OsakaMono.ttf", "rb"));
#endif
    std::cerr << "Read font bytes: " << read << std::endl;
    
	stbtt_fontinfo font;
	stbtt_InitFont(&font, ttf_buffer, 0);
	int r = stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,1024,256, 32,96, cdata); // no guarantee this fits!
    if(r <= 0) {
        std::cerr << "stbtt_BackFontBitmap r: " << r << std::endl;
     exit(0);
    }

	scale = stbtt_ScaleForPixelHeight(&font, 32);
	stbtt_GetFontVMetrics(&font, &ascent,&descent,&lineGap);
	baseline = (int) (ascent*scale);

	// can free ttf_buffer at this point
    if(ftex == 0) glGenTextures(1, &ftex);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)1024);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, ftex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024,256, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
	// can free temp_bitmap at this point
    
    if(!checkForErrors()) {
        exit(1);
    }
    delete []ttf_buffer;
    delete []temp_bitmap;
}

void Ibex::TextRenderer::precompileText(float x, float y, const std::vector<std::string> &lines, const std::vector<bool> &highlighted, int maxChars)
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
	//for(std::string line : lines) {
    for(int i = 0; i < lines.size(); ++i) {
        std::string line = lines[i];
		if(maxChars > 0 && line.length() > maxChars) {
			line.resize(maxChars);
		}
        const GLfloat *color = (highlighted.size() > i && highlighted[i]) ? highlightedTextColor : textColor;
		x = 0;
		y = -lineNum * (ascent-descent+lineGap)*scale;
		unsigned char *text = (unsigned char *)line.data();
		while (*text) {
			if (*text >= 32 && *text < 128) {
				stbtt_aligned_quad q;
				stbtt_GetBakedQuad(cdata, 1024,256, *text-32, &x,&y,&q,1);

				// bottom right triangle
				vertices.push_back(q.x0);
				vertices.push_back((baseline-q.y0));
				vertices.push_back(0);
				vertices.push_back(q.s0);
				vertices.push_back(q.t0);
                vertices.push_back(color[0]);
                vertices.push_back(color[1]);
                vertices.push_back(color[2]);
                vertices.push_back(color[3]);

				vertices.push_back(q.x1);
				vertices.push_back((baseline-q.y0));
				vertices.push_back(0);
				vertices.push_back(q.s1);
				vertices.push_back(q.t0);
                vertices.push_back(color[0]);
                vertices.push_back(color[1]);
                vertices.push_back(color[2]);
                vertices.push_back(color[3]);

				vertices.push_back(q.x1);
				vertices.push_back((baseline-q.y1));
				vertices.push_back(0);
				vertices.push_back(q.s1);
				vertices.push_back(q.t1);
                vertices.push_back(color[0]);
                vertices.push_back(color[1]);
                vertices.push_back(color[2]);
                vertices.push_back(color[3]);

				// top left triangle
				vertices.push_back(q.x0);
				vertices.push_back((baseline-q.y0));
				vertices.push_back(0);
				vertices.push_back(q.s0);
				vertices.push_back(q.t0);
                vertices.push_back(color[0]);
                vertices.push_back(color[1]);
                vertices.push_back(color[2]);
                vertices.push_back(color[3]);
                
				vertices.push_back(q.x1);
				vertices.push_back((baseline-q.y1));
				vertices.push_back(0);
				vertices.push_back(q.s1);
				vertices.push_back(q.t1);
                vertices.push_back(color[0]);
                vertices.push_back(color[1]);
                vertices.push_back(color[2]);
                vertices.push_back(color[3]);

				vertices.push_back(q.x0);
				vertices.push_back((baseline-q.y1));
				vertices.push_back(0);
				vertices.push_back(q.s0);
				vertices.push_back(q.t1);
                vertices.push_back(color[0]);
                vertices.push_back(color[1]);
                vertices.push_back(color[2]);
                vertices.push_back(color[3]);

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

	if(vaoTextRenderer == 0) glGenVertexArrays(1,&vaoTextRenderer);
	if(!checkForErrors()) {
		exit(1);
	}

	glBindVertexArray(vaoTextRenderer);
	if(vboTextVertices == 0) glGenBuffers(1, &vboTextVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboTextVertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	if(!checkForErrors()) {
		exit(1);
	}
	glEnableVertexAttribArray(IbexTextAttribLocations[0]);
	glVertexAttribPointer(IbexTextAttribLocations[0], 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*9, 0);
	glEnableVertexAttribArray(IbexTextAttribLocations[1]);
	glVertexAttribPointer(IbexTextAttribLocations[1], 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*9, (GLvoid*) (sizeof(GLfloat)*3));
    glEnableVertexAttribArray(IbexTextAttribLocations[2]);
	glVertexAttribPointer(IbexTextAttribLocations[2], 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*9, (GLvoid*) (sizeof(GLfloat)*5));
	if(!checkForErrors()) {
		exit(1);
	}
	if(vboTextIndices == 0) glGenBuffers(1, &vboTextIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboTextIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	if(!checkForErrors()) {
		exit(1);
	}
    glBindVertexArray(0);


	//////////////////////////////////////////////////////////////////////////////
	static GLfloat IbexDisplayFlatVertices[] = {
		-1.0, -1.0, 0.0, 0, 1,
		1.0, -1.0, 0.0, 1, 1,
		1.0,  1.0, 0.0, 1, 0,
		-1.0,  1.0, 0.0, 0, 0
	};

	static GLushort IbexDisplayFlatIndices[] = {
		0, 2, 1,
		0, 3, 2
	};

    if(vaoTextTextureRenderer == 0) {
        if(vaoTextTextureRenderer == 0) glGenVertexArrays(1,&vaoTextTextureRenderer);
        if(!checkForErrors()) {
            exit(1);
        }

        glBindVertexArray(vaoTextTextureRenderer);
        if(vboTextTextureVertices == 0) glGenBuffers(1, &vboTextTextureVertices);
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
        if(vboTextTextureIndices == 0) glGenBuffers(1, &vboTextTextureIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboTextTextureIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IbexDisplayFlatIndices), IbexDisplayFlatIndices, GL_STATIC_DRAW);
        if(!checkForErrors()) {
            exit(1);
        }
        glBindVertexArray(0);
    }
}

void Ibex::TextRenderer::bindTextFBO() {
	glBindFramebuffer(GL_FRAMEBUFFER, fboText);
	glViewport(0,0,maxX-minX,maxY-minY);
    
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Ibex::TextRenderer::generateTextFBO()
{
	if(fboText == 0) glGenFramebuffers(1, &fboText);
	glBindFramebuffer(GL_FRAMEBUFFER, fboText);
    
	if(textTextureId == 0) glGenTextures(1, &textTextureId);
	glBindTexture(GL_TEXTURE_2D, textTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

void Ibex::TextRenderer::renderTextToFramebuffer(float x, float y, const std::vector<std::string> &lines, const std::vector<bool> &highlighted, int maxChars) {
    precompileText(x, y, lines, highlighted, maxChars);
    
    generateTextFBO();
	bindTextFBO();
    
	glm::mat4 orth = glm::ortho(minX,maxX,minY,maxY,-100.0f,100.0f);

	glUseProgram(textShaderProgram.shader.program);
	if(IbexTextUniformLocations[0] >= 0) {
        glUniformMatrix4fv(IbexTextUniformLocations[0], 1, GL_FALSE, &orth[0][0]);
    }

	if(IbexTextUniformLocations[3] >= 0)  {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ftex);
		glUniform1i(IbexTextUniformLocations[3], 0);
	}
    if(IbexTextUniformLocations[5] >= 0) {
        glUniform4fv(IbexTextUniformLocations[5], 1, backgroundColor);
    }

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(vaoTextRenderer);
    glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);	
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void Ibex::TextRenderer::renderTextDirect(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP)
{
	glm::mat4 orth = MVP;

	if(shadowPass) {
		return;
	} else {
		glUseProgram(textShaderProgram.shader.program);
		if(IbexTextUniformLocations[0] >= 0) glUniformMatrix4fv(IbexTextUniformLocations[0], 1, GL_FALSE, &orth[0][0]);

		if(IbexTextUniformLocations[3] >= 0)  {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ftex);
			glUniform1i(IbexTextUniformLocations[3], 0);
		}
        if(IbexTextUniformLocations[5] >= 0) {
            glUniform4fv(IbexTextUniformLocations[5], 1, backgroundColor);
        }
	}

	glBindVertexArray(vaoTextRenderer);
	glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);	
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void Ibex::TextRenderer::renderText(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP, const GLfloat &fade, const bool &sizeToFit)
{
    if(!initialized) return;
    
	glm::mat4 orth = MVP;

	if(shadowPass) {
		return;
	} else {
		if(sizeToFit) {
			orth = orth * glm::scale(glm::mat4(), glm::vec3(1.0f, (maxY-minY)/(maxX-minX), 1.0f));
		}
		glUseProgram(standardShaderProgram.shader.program);
		if(IbexDisplayFlatUniformLocations[0] >= 0) glUniformMatrix4fv(IbexDisplayFlatUniformLocations[0], 1, GL_FALSE, &orth[0][0]);

		if(IbexDisplayFlatUniformLocations[3] >= 0)  {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textTextureId);
			glUniform1i(IbexDisplayFlatUniformLocations[3], 0);
		}
        
        if(IbexDisplayFlatUniformLocations[5] >= 0) glUniform1f(IbexDisplayFlatUniformLocations[5], fade);
        
        if(IbexDisplayFlatUniformLocations[6] >= 0) {
            glUniform2f(IbexDisplayFlatUniformLocations[6], 0,0);
        }
	}

	glBindVertexArray(vaoTextTextureRenderer);
	glDrawElements(GL_TRIANGLES,  6, GL_UNSIGNED_SHORT, 0);
}

Ibex::TextRenderer::TextRenderer() : initialized(false),
	vaoTextRenderer(0),
	vboTextVertices(0),
	vboTextIndices(0),
	fboText(0),
	textTextureId(0),
	textTextureWidth(0),
	textTextureHeight(0),
    vaoTextTextureRenderer(0),
    vboTextTextureVertices(0),
    vboTextTextureIndices(0),
    ftex(0)
{
	memset(IbexTextUniformLocations,0,sizeof(IbexTextUniformLocations));
	memset(IbexTextAttribLocations,0,sizeof(IbexTextAttribLocations));

	backgroundColor[0] = 0.1;
    backgroundColor[1] = 0.5;
    backgroundColor[2] = 0.8;
    backgroundColor[3] = 0.7;
    
    textColor[0] = 1;
    textColor[1] = 1;
    textColor[2] = 1;
    textColor[3] = 1;
    
    highlightedTextColor[0] = 1;
    highlightedTextColor[1] = 0;
    highlightedTextColor[2] = 0;
    highlightedTextColor[3] = 1;
}


Ibex::TextRenderer::~TextRenderer()
{
	if(vaoTextRenderer) glDeleteVertexArrays(1, &vaoTextRenderer);
	if(vboTextVertices) glDeleteBuffers(1, &vboTextVertices);
	if(vboTextIndices) glDeleteBuffers(1, &vboTextIndices);
    
    if(vaoTextTextureRenderer) glDeleteVertexArrays(1, &vaoTextTextureRenderer);
	if(vboTextTextureVertices) glDeleteBuffers(1, &vboTextTextureVertices);
	if(vboTextTextureIndices) glDeleteBuffers(1, &vboTextTextureIndices);
    
    if(ftex) glDeleteTextures(1, &ftex);
    if(textTextureId) glDeleteTextures(1, &textTextureId);
}
