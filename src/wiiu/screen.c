#include "wiiu.h"

#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <coreinit/screen.h>
#include <coreinit/cache.h>

static bool isInit = false;

static uint32_t drcSize;
static uint32_t tvSize;
static void* drcBuffer;
static void* tvBuffer;

void wiiu_screen_init(void)
{
  if (isInit) return;

  OSScreenInit();

  drcSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
  tvSize = OSScreenGetBufferSizeEx(SCREEN_TV);
  drcBuffer = memalign(0x100, drcSize);
  tvBuffer = memalign(0x100, tvSize);

  OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
  OSScreenSetBufferEx(SCREEN_TV, tvBuffer);

  OSScreenEnableEx(SCREEN_DRC, TRUE);
  OSScreenEnableEx(SCREEN_TV, TRUE);

  isInit = true;
}

void wiiu_screen_exit(void)
{
  if (!isInit) return;

  free(tvBuffer);
  free(drcBuffer);

  OSScreenShutdown();

  isInit = false;
}

void wiiu_screen_clear(void)
{
  if (!isInit) return;

  OSScreenClearBufferEx(SCREEN_DRC, 0x33226100);
  OSScreenClearBufferEx(SCREEN_TV, 0x33226100);
}

void wiiu_screen_draw(void)
{
  if (!isInit) return;

  DCFlushRange(drcBuffer, drcSize);
  DCFlushRange(tvBuffer, tvSize);

  OSScreenFlipBuffersEx(SCREEN_DRC);
  OSScreenFlipBuffersEx(SCREEN_TV);
}

void wiiu_screen_print(uint32_t x, uint32_t y, char* fmt, ...)
{
  if (!isInit) return;

  va_list va;
  va_start(va, fmt);

  char* tmp = NULL;
  if((vasprintf(&tmp, fmt, va) >= 0) && tmp) {
    OSScreenPutFontEx(SCREEN_TV, x, y, tmp);
    OSScreenPutFontEx(SCREEN_DRC, x, y, tmp);
  }

  va_end(va);
  free(tmp);
}