#pragma once
#include <qopengl.h>

struct tomoThreadParams {
  void* pParent;
  float projWidth;
  float projHeight;
  int   nProjections;
  float rotationCenter;
  float tiltAngle;
  int   filterType;
};

bool initFBO(GLuint* pFbo);
bool initFBOTextures(int nTextures, GLuint* TexId, int texSizeX, int texSizeY, GLenum* attachmentpoints);
bool initProjTextures(int nProjections, GLuint* TexID);

void fft(float *a, float *b, int m, int iopt);
bool buildFilter(float* wfilt, int nfft, int rowWidth, int filterType);
void DoParallelTomo(struct tomoThreadParams params);
