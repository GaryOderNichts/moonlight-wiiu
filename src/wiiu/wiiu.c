#include "wiiu.h"

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>

#include <whb/proc.h>
#include <whb/gfx.h>
#include <coreinit/fastmutex.h>
#include <gx2/mem.h>
#include <gx2/draw.h>
#include <gx2/registers.h>

#include "shaders/display.h"

#define ATTRIB_SIZE (8 * 2 * sizeof(float))
#define ATTRIB_STRIDE (4 * sizeof(float))

static GX2Sampler screenSamp;
static WHBGfxShaderGroup shaderGroup;

static float* tvAttribs;
static float* drcAttribs;

static float tvScreenSize[2];
static float drcScreenSize[2];

uint32_t currentFrame;
uint32_t nextFrame;

static OSFastMutex queueMutex;
static yuv_texture_t* queueMessages[MAX_QUEUEMESSAGES];
static uint32_t queueWriteIndex;
static uint32_t queueReadIndex;

void wiiu_init(uint32_t width, uint32_t height)
{
  currentFrame = nextFrame = 0;

  OSFastMutex_Init(&queueMutex, "");
  queueReadIndex = queueWriteIndex = 0;

  wiiu_screen_exit();
  WHBGfxInit();

  if (!WHBGfxLoadGFDShaderGroup(&shaderGroup, 0, display_gsh)) {
    printf("Cannot load shader\n");
    wiiu_exit();
  }

  WHBGfxInitShaderAttribute(&shaderGroup, "in_pos", 0, 0, GX2_ATTRIB_FORMAT_FLOAT_32_32);
  WHBGfxInitShaderAttribute(&shaderGroup, "in_texCoord", 0, 8, GX2_ATTRIB_FORMAT_FLOAT_32_32);

  if (!WHBGfxInitFetchShader(&shaderGroup)) {
    printf("cannot init shader\n");
    wiiu_exit();
  }

  GX2InitSampler(&screenSamp, GX2_TEX_CLAMP_MODE_WRAP, GX2_TEX_XY_FILTER_MODE_POINT);

  GX2ColorBuffer* cb = WHBGfxGetTVColourBuffer();
  tvScreenSize[0] = 1.0f / (float) cb->surface.width;
  tvScreenSize[1] = 1.0f / (float) cb->surface.height;

  tvAttribs = memalign(GX2_VERTEX_BUFFER_ALIGNMENT, ATTRIB_SIZE);
  int i = 0;

  tvAttribs[i++] = 0.0f;                      tvAttribs[i++] = 0.0f;
  tvAttribs[i++] = 0.0f;                      tvAttribs[i++] = 0.0f;

  tvAttribs[i++] = (float) cb->surface.width; tvAttribs[i++] = 0.0f;
  tvAttribs[i++] = 1.0f;                      tvAttribs[i++] = 0.0f;

  tvAttribs[i++] = (float) cb->surface.width; tvAttribs[i++] = (float) cb->surface.height;
  tvAttribs[i++] = 1.0f;                      tvAttribs[i++] = 1.0f;

  tvAttribs[i++] = 0.0f;                      tvAttribs[i++] = (float) cb->surface.height;
  tvAttribs[i++] = 0.0f;                      tvAttribs[i++] = 1.0f;
  GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, tvAttribs, ATTRIB_SIZE);

  cb = WHBGfxGetDRCColourBuffer();
  drcScreenSize[0] = 1.0f / (float) cb->surface.width;
  drcScreenSize[1] = 1.0f / (float) cb->surface.height;

  drcAttribs = memalign(GX2_VERTEX_BUFFER_ALIGNMENT, ATTRIB_SIZE);
  i = 0;

  drcAttribs[i++] = 0.0f;                      drcAttribs[i++] = 0.0f;
  drcAttribs[i++] = 0.0f;                      drcAttribs[i++] = 0.0f;

  drcAttribs[i++] = (float) cb->surface.width; drcAttribs[i++] = 0.0f;
  drcAttribs[i++] = 1.0f;                      drcAttribs[i++] = 0.0f;

  drcAttribs[i++] = (float) cb->surface.width; drcAttribs[i++] = (float) cb->surface.height;
  drcAttribs[i++] = 1.0f;                      drcAttribs[i++] = 1.0f;

  drcAttribs[i++] = 0.0f;                      drcAttribs[i++] = (float) cb->surface.height;
  drcAttribs[i++] = 0.0f;                      drcAttribs[i++] = 1.0f;
  GX2Invalidate(GX2_INVALIDATE_MODE_CPU_ATTRIBUTE_BUFFER, drcAttribs, ATTRIB_SIZE);
}

void wiiu_loop(void)
{
  while(WHBProcIsRunning()) {
    wiiu_input_update();
    yuv_texture_t* tex = get_frame();
    if (tex) {
      if (++currentFrame <= nextFrame - NUM_BUFFERS) {
        // display thread is behind decoder, skip frame
      }
      else {
        WHBGfxBeginRender();

        // TV
        WHBGfxBeginRenderTV();
        WHBGfxClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        GX2SetColorControl(GX2_LOGIC_OP_COPY, 0xFF, FALSE, TRUE);
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
          GX2_BLEND_MODE_SRC_ALPHA, GX2_BLEND_MODE_INV_SRC_ALPHA,
          GX2_BLEND_COMBINE_MODE_ADD,
          TRUE,
          GX2_BLEND_MODE_ONE, GX2_BLEND_MODE_INV_SRC_ALPHA,
          GX2_BLEND_COMBINE_MODE_ADD
        );
        GX2SetDepthOnlyControl(FALSE, FALSE, GX2_COMPARE_FUNC_ALWAYS);

        GX2SetPixelTexture(&tex->yTex, 0);
        GX2SetPixelTexture(&tex->uvTex, 1);
        GX2SetPixelSampler(&screenSamp, 0);
        GX2SetPixelSampler(&screenSamp, 1);

        GX2SetFetchShader(&shaderGroup.fetchShader);
        GX2SetVertexShader(shaderGroup.vertexShader);
        GX2SetPixelShader(shaderGroup.pixelShader);

        GX2SetVertexUniformReg(0, 2, tvScreenSize);
        GX2SetAttribBuffer(0, ATTRIB_SIZE, ATTRIB_STRIDE, tvAttribs);
        GX2DrawEx(GX2_PRIMITIVE_MODE_QUADS, 4, 0, 1);

        WHBGfxFinishRenderTV();

        // DRC
        WHBGfxBeginRenderDRC();
        WHBGfxClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        GX2SetColorControl(GX2_LOGIC_OP_COPY, 0xFF, FALSE, TRUE);
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
          GX2_BLEND_MODE_SRC_ALPHA, GX2_BLEND_MODE_INV_SRC_ALPHA,
          GX2_BLEND_COMBINE_MODE_ADD,
          TRUE,
          GX2_BLEND_MODE_ONE, GX2_BLEND_MODE_INV_SRC_ALPHA,
          GX2_BLEND_COMBINE_MODE_ADD
        );
        GX2SetDepthOnlyControl(FALSE, FALSE, GX2_COMPARE_FUNC_ALWAYS);

        GX2SetPixelTexture(&tex->yTex, 0);
        GX2SetPixelTexture(&tex->uvTex, 1);
        GX2SetPixelSampler(&screenSamp, 0);
        GX2SetPixelSampler(&screenSamp, 1);

        GX2SetFetchShader(&shaderGroup.fetchShader);
        GX2SetVertexShader(shaderGroup.vertexShader);
        GX2SetPixelShader(shaderGroup.pixelShader);

        GX2SetVertexUniformReg(0, 2, drcScreenSize);
        GX2SetAttribBuffer(0, ATTRIB_SIZE, ATTRIB_STRIDE, drcAttribs);
        GX2DrawEx(GX2_PRIMITIVE_MODE_QUADS, 4, 0, 1);
        
        WHBGfxFinishRenderDRC();

        WHBGfxFinishRender();
      }
    }
  }

  wiiu_fini();
}

void wiiu_fini(void)
{
  free(tvAttribs);
  free(drcAttribs);

  WHBGfxFreeShaderGroup(&shaderGroup);
  WHBGfxShutdown();
}

void wiiu_exit(void)
{
  WHBProcStopRunning();

  while (WHBProcIsRunning());

  WHBProcShutdown();
  exit(0);
}

void wiiu_error_exit(char* fmt, ...)
{
  wiiu_screen_init();
  wiiu_screen_clear();

  va_list args;
  va_start(args, fmt);

  char* tmp = NULL;
  if((vasprintf(&tmp, fmt, args) >= 0) && tmp) {
    wiiu_screen_print(0, 0, tmp);
  }

  va_end(args);
  free(tmp);

  wiiu_screen_draw();

  // Re-enable home button detection for exit
  WHBProcInit();
  while (WHBProcIsRunning());

  wiiu_screen_exit();
  WHBProcShutdown();
  exit(0);
}

void* get_frame(void)
{
  OSFastMutex_Lock(&queueMutex);

  uint32_t elements_in = queueWriteIndex - queueReadIndex;
  if(elements_in == 0) {
    OSFastMutex_Unlock(&queueMutex);
    return NULL; // framequeue is empty
  }

  uint32_t i = (queueReadIndex)++ & (MAX_QUEUEMESSAGES - 1);
  yuv_texture_t* message = queueMessages[i];

  OSFastMutex_Unlock(&queueMutex);
  return message;
}

void add_frame(yuv_texture_t* msg)
{
  OSFastMutex_Lock(&queueMutex);

  uint32_t elements_in = queueWriteIndex - queueReadIndex;
  if (elements_in == MAX_QUEUEMESSAGES) {
    OSFastMutex_Unlock(&queueMutex);
    return; // framequeue is full
  }

  uint32_t i = (queueWriteIndex)++ & (MAX_QUEUEMESSAGES - 1);
  queueMessages[i] = msg;

  OSFastMutex_Unlock(&queueMutex);
}
