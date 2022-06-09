#include <stdio.h>

#ifndef _LOGGING_H_
#define _LOGGING_H_

#define LOG_FATAL (1U << 0)
#define LOG_ERROR (1U << 1)
#define LOG_WARN  (1U << 2)
#define LOG_INFO  (1U << 3)
#define LOG_DEBUG (1U << 4)
#define LOG_TRACE (1U << 5)
#define LOG_DUMP  (1U << 6)

#ifndef LOG_LEVELS
#define LOG_LEVELS (LOG_FATAL | LOG_ERROR | LOG_WARN | LOG_INFO)
#endif

#if LOG_LEVELS & LOG_DUMP
#define dump(fmt, ...) log(DUMP_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define dump(fmt, ...)
#endif

#if LOG_LEVELS & LOG_TRACE
#define trace(fmt, ...) log(TRACE_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define trace(fmt, ...)
#endif

#if LOG_LEVELS & LOG_DEBUG
#define debug(fmt, ...) log(DEBUG_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

#if LOG_LEVELS & LOG_INFO
#define info(fmt, ...) log(INFO_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define info(fmt, ...)
#endif

#if LOG_LEVELS & LOG_WARN
#define warn(fmt, ...) log(WARN_STYLE, fmt, "\e[0m", __VA_ARGS__)
#else
#define warn(fmt, ...)
#endif

#if LOG_LEVELS & LOG_ERROR
#define error(fmt, ...) {       \
  log(ERROR_STYLE, fmt, "\e[0m", __VA_ARGS__);  \
  exit(1);                      \
}
#else
#define error(fmt, ...)
#endif

#if LOG_LEVELS & LOG_FATAL
#define fatal(fmt, ...) {       \
  log(FATAL_STYLE, fmt, "\e[0m", __VA_ARGS__);  \
  abort();                      \
}
#else
#define fatal(fmt, ...)
#endif

#define DUMP_STYLE  "\e[36mT\e[90m"
#define TRACE_STYLE "\e[35mT\e[90m"
#define DEBUG_STYLE "\e[34mD\e[90m"
#define INFO_STYLE  "\e[32mI\e[90m"
#define WARN_STYLE  "\e[33mW\e[90m"
#define ERROR_STYLE "\e[91mE\e[90m"
#define FATAL_STYLE "\e[93;41mF\e[90m"

#define log(prx, fmt, sfx, ...) printf(prx " %s:%d %s() " fmt sfx "\n", \
                                       __BASE_FILE__, \
                                       __LINE__, \
                                       __func__ __VA_OPT__(,) __VA_ARGS__)

#endif /* _LOGGING_H_ */
