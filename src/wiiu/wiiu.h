#pragma once

#include <Limelight.h>
#include <stdint.h>
#include <stdbool.h>

#include <gx2/texture.h>

#define MAX_CHANNEL_COUNT 6
#define FRAME_SIZE 240
#define FRAME_BUFFER 12

void wiiu_init(uint32_t width, uint32_t height);
void wiiu_loop(void);
void wiiu_fini(void);
void wiiu_exit(void);
void wiiu_error_exit(char* fmt, ...);

#define NUM_BUFFERS 2
#define MAX_QUEUEMESSAGES 16

typedef struct {
  GX2Texture yTex;
  GX2Texture uvTex;
} yuv_texture_t;

void* get_frame(void);
void add_frame(yuv_texture_t* msg);

extern uint32_t nextFrame;

extern AUDIO_RENDERER_CALLBACKS audio_callbacks_wiiu;
extern DECODER_RENDERER_CALLBACKS decoder_callbacks_wiiu;

// screen
void wiiu_screen_init(void);
void wiiu_screen_exit(void);
void wiiu_screen_clear(void);
void wiiu_screen_draw(void);
void wiiu_screen_print(uint32_t x, uint32_t y, char* fmt, ...);

// input
void wiiu_input_init(void);
void wiiu_input_update(void); // this is only relevant while streaming
uint32_t wiiu_input_num_controllers(void);
uint32_t wiiu_input_buttons_triggered(void); // only really used for the menu
