#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include "config.h"
#include "ui.h"
#include "logging.h"

typedef struct _SdlUi SdlUi;

struct _SdlUi {
  Ui user_iface;
  int scale;
  SDL_Surface *surf;
  SDL_Window *win;
  SDL_Renderer *rend;
  SDL_Texture *text;
  uint16_t keys;
  uint8_t pixbuf[];
};

static Chip8Key to_chip8_key(int scancode) {
  switch(scancode) {
    case SDL_SCANCODE_1:
      return C8_KEY_1;
    case SDL_SCANCODE_2:
      return C8_KEY_2;
    case SDL_SCANCODE_3:
      return C8_KEY_3;
    case SDL_SCANCODE_4:
      return C8_KEY_C;
    case SDL_SCANCODE_Q:
      return C8_KEY_4;
    case SDL_SCANCODE_W:
      return C8_KEY_5;
    case SDL_SCANCODE_E:
      return C8_KEY_6;
    case SDL_SCANCODE_R:
      return C8_KEY_D;
    case SDL_SCANCODE_A:
      return C8_KEY_7;
    case SDL_SCANCODE_S:
      return C8_KEY_8;
    case SDL_SCANCODE_D:
      return C8_KEY_9;
    case SDL_SCANCODE_F:
      return C8_KEY_E;
    case SDL_SCANCODE_Z:
      return C8_KEY_A;
    case SDL_SCANCODE_X:
      return C8_KEY_0;
    case SDL_SCANCODE_C:
      return C8_KEY_B;
    case SDL_SCANCODE_V:
      return C8_KEY_F;
  }
  return C8_KEY_NIL;
}

static void sdl_ui_poll_events(Ui *ui) {
  SdlUi *self = (SdlUi *) ui;
  SDL_Event ev;
  while(SDL_PollEvent(&ev)) {
    switch(ev.type) {
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        SDL_KeyboardEvent *kbev = (SDL_KeyboardEvent *) &ev;
        info("key: %s", SDL_GetScancodeName(kbev->keysym.scancode));
        if(kbev->keysym.scancode == SDL_SCANCODE_ESCAPE) {
          exit(0);
        } else if(!kbev->repeat) {
          Chip8Key k = to_chip8_key(kbev->keysym.scancode);
          if(k == C8_KEY_NIL) {
            break;
          }
          self->keys ^= 1 << k;
          info("keys: 0x%hx", self->keys);
        }
        break;
      }
      case SDL_QUIT:
        exit(0);
        break;
    }
  }
}

static bool sdl_ui_key_pressed(Ui *ui, Chip8Key key) {
  SdlUi *self = (SdlUi *) ui;
  return !!(self->keys & key);
}

static void sdl_ui_destroy(Ui *ui) {
  SdlUi *self = (SdlUi *) ui;
  SDL_DestroyTexture(self->text);
  SDL_DestroyRenderer(self->rend);
  SDL_DestroyWindow(self->win);
}

//static const char *to_bin(uint8_t v, char *buf) {
//  int i;
//  for(i = 0; i < 8; ++ i) {
//    buf[7 - i] = v & (1 << i) ? '#' : ' ';
//  }
//  buf[8] = '\0';
//  return buf;
//}

static void sdl_ui_flush(Ui *ui, int width, int height, uint8_t *fb) {
  int x, y;
  SdlUi *self = (SdlUi *) ui;
  uint8_t *pbstart = self->pixbuf;
  uint8_t *fbstart = fb;

  trace("");

  for(y = 0; y < height; ++ y) {
    for(x = 0; x < width; ++ x) {
      *(pbstart + x) = (fbstart[x >> 3] >> (7 - (x & 0x7))) & 1 ? 0xff : 0;
    }

    pbstart += width;
    fbstart += width >> 3;
  }

  SDL_UpdateTexture(self->text, NULL, self->pixbuf, width);
  SDL_RenderCopy(self->rend, self->text, NULL, NULL);
  SDL_RenderPresent(self->rend);

  //int r, c;
  //for(r = 0; r < 32; r ++) {
  //  for(c = 0; c < 8; c ++) {
  //    printf("%s", to_bin(fb[r * (UI_WIDTH >> 3) + c], (char[9]){}));
  //  }
  //  printf("\n");
  //}
}

Ui *sdl_ui_new(int width, int height, int scale) {
  SDL_Window *win;
  SDL_Renderer *rend;
  SDL_Texture *text;
  SdlUi *self;

  if(!SDL_WasInit(0)) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
      fatal("unable to initialize SDL: %s", SDL_GetError());
    }
    atexit(SDL_Quit);
  }

  scale = 1;
  if(SDL_CreateWindowAndRenderer(width * scale, height * scale, 0, &win, &rend)) {
    fatal("unable to create window or renderer: %s", SDL_GetError());
  }

  SDL_SetRenderDrawColor(rend, 0, 0, 0, 0);
  SDL_RenderClear(rend);
  SDL_RenderPresent(rend);

  text = SDL_CreateTexture(rend,
                           SDL_PIXELFORMAT_RGB332,
                           SDL_TEXTUREACCESS_STATIC,
                           width * scale,
                           height * scale);
  if(!text) {
    fatal("unable to create texture: %s", SDL_GetError());
  }

  self = malloc(sizeof(SdlUi) + (width * height));
  UI(self)->poll_events = sdl_ui_poll_events;
  UI(self)->key_pressed = sdl_ui_key_pressed;
  UI(self)->destroy = sdl_ui_destroy;
  UI(self)->flush = sdl_ui_flush;
  self->win = win;
  self->rend = rend;
  self->text = text;
  self->scale = scale;
  self->keys = 0;
  memset(self->pixbuf, 0, width * height);

  return UI(self);
}

