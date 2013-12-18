#ifndef __IBEX_TEXT_RENDERER_H__
#define __IBEX_TEXT_RENDERER_H__

#include "stb_truetype.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#ifdef __cplusplus
extern "C" {
#endif

void loadProgram();
void my_stbtt_initfont(void);
void my_stbtt_generate(float x, float y, char *text);
void renderText(const glm::mat4 &MVP, const glm::mat4 &V, const glm::mat4 &M, bool shadowPass, const glm::mat4 &depthMVP);

#ifdef __cplusplus
}
#endif

class TextRenderer
{
public:
	TextRenderer(void);
	~TextRenderer(void);
};

#endif // __IBEX_TEXT_RENDERER_H__
