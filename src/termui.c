#include <stdlib.h>
#include "ui.h"

typedef struct _TermDisp TermDisp;

struct _TermDisp {
  Ui ui;
};

Ui *term_ui_new(int width, int height, int scale) {
  TermDisp *self = malloc(sizeof(TermDisp));

  return UI(self);
}

