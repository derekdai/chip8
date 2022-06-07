#include <stdint.h>
#include <stdbool.h>

#ifndef __UI_H_
#define __UI_H_

#define UI_WIDTH (64)
#define UI_HEIGHT (32)

#define UI(p) ((Ui *) (p))

typedef enum _UiKind UiKind;

enum _UiKind {
  UI_TERM,
  UI_SDL,
};

typedef struct _Ui Ui;

struct _Ui {
  uint8_t *fb;
  void (*poll_events)(Ui *self);
  void (*flush)(Ui *self);
  void (*destroy)(Ui *self);
};

Ui *ui_new(UiKind kind, int width, int height, int scale);

void ui_free(Ui *self);

void ui_poll_events(Ui *self);

void ui_flush(Ui *self);

#endif /* __UI_H_ */
