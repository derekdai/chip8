#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef __UTILS_H_
#define __UTILS_H_

#define Auto(t, f) __attribute__((cleanup(f))) t

#define AutoFree(t) Auto(t, _free)

#define move(v) ({      \
  typeof(v) t = v;      \
  if(v) { v = NULL; }   \
  t;                    \
})

static inline void _free(void **p) { if(*p) { free(*p); *p = NULL; } }

#endif /* __UTILS_H_ */
