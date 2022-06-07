#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include "ui.h"
#include "logging.h"

typedef struct _SdlUi SdlUi;

struct _SdlUi {
  Ui user_iface;
  SDL_Surface *surf;
  SDL_Window *win;
  SDL_Renderer *rend;
};

static void sdl_ui_poll_events(Ui *self) {
  SDL_Event ev;
  while(SDL_PollEvent(&ev)) {
    switch(ev.type) {
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        SDL_KeyboardEvent *kbev = (SDL_KeyboardEvent *) &ev;
        info("scancode=%s", SDL_GetScancodeName(kbev->keysym.scancode));
        break;
      }
      case SDL_QUIT:
        exit(0);
        break;
    }
  }
}

static void sdl_ui_free(Ui *self) {
}

static void sdl_ui_flush(Ui *self) {
}

Ui *sdl_ui_new(int width, int height) {
  if(!SDL_WasInit(0)) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
      fatal("unable to initialize SDL: %s", SDL_GetError());
    }
    atexit(SDL_Quit);
  }

  SDL_Window *win;
  SDL_Renderer *rend;
  if(SDL_CreateWindowAndRenderer(width, height, 0, &win, &rend)) {
    fatal("unable to create window or renderer: %s", SDL_GetError());
  }

  SdlUi *self = malloc(sizeof(SdlUi));
  UI(self)->poll_events = sdl_ui_poll_events;
  UI(self)->destroy = sdl_ui_free;
  UI(self)->flush = sdl_ui_flush;
  self->win = win;
  self->rend = rend;

  return UI(self);
}
