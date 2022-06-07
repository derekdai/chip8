#include <stdlib.h>
#include "ui.h"
#include "logging.h"

extern Ui *term_ui_new(int width, int height, int scale);
extern Ui *sdl_ui_new(int width, int height, int scale);

Ui *ui_new(UiKind kind, int width, int height, int scale) {
  trace("ui_new()");
  if(kind == UI_SDL) {
    return sdl_ui_new(width, height, scale);
  } else {
    return term_ui_new(width, height, scale);
  }
}

void ui_poll_events(Ui *self) {
  self->poll_events(self);
}

void ui_flush(Ui *self) {
}

void ui_free(Ui *self) {
  if(self) {
    self->destroy(self);
    free(self);
  }
}
