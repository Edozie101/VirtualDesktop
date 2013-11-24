#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

/*
 * Boring, non-OpenGL-related utility functions
 */

void *
file_contents(const char *filename, GLint *length)
{
  FILE *f = fopen(filename, "r");
  void *buffer;

  if (!f) {
    fprintf(stderr, "Unable to open %s for reading\n", filename);
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  *length = (GLint)ftell(f);
  fseek(f, 0, SEEK_SET);

  buffer = malloc(*length + 1);
  *length = (GLint)fread(buffer, 1, *length, f);
  fclose(f);
  ((char*) buffer)[*length] = '\0';

  return buffer;
}


