#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define VERSION_STRING "v1.5"

enum {
  STATE_INVALID,
  STATE_DISCONNECTED,
  STATE_CONNECTING,
  STATE_CONNECTED,
  STATE_PAIRING,
  STATE_START_STREAM,
  STATE_STOP_STREAM,
  STATE_STREAMING,
};

#include <Limelight.h>
#include <stdint.h>
#include <stdbool.h>
#include "font.h"

#include <gx2/texture.h>

extern int state;
extern int is_error;
extern char message_buffer[1024];

#define FRAME_BUFFER 12

void wiiu_stream_init(uint32_t width, uint32_t height);
void wiiu_stream_draw(void);
void wiiu_stream_fini(void);
void wiiu_setup_renderstate(void);

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

// input
void wiiu_input_init(void);
void wiiu_input_update(void); // this is only relevant while streaming
uint32_t wiiu_input_num_controllers(void);
uint32_t wiiu_input_buttons_triggered(void); // only really used for the menu
void start_input_thread(void);
void stop_input_thread(void);

// proc
void wiiu_proc_init(void);
void wiiu_proc_shutdown(void);
void wiiu_proc_register_home_callback(void);
int wiiu_proc_running(void);
void wiiu_proc_stop_running(void);
void wiiu_proc_set_home_enabled(int enabled);
