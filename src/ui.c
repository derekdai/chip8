#include <stdlib.h>
#include <stdbool.h>
#include "ui.h"
#include "config.h"
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

bool ui_key_pressed(Ui *self, Chip8Key key) {
  return self->key_pressed(self, key);
}

void ui_flush(Ui *ui, int width, int height, uint8_t *fb) {
  ui->flush(ui, width, height, fb);
}

void ui_free(Ui *self) {
  if(self) {
    self->destroy(self);
    free(self);
  }
  trace("ui_free()");
}
