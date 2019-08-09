#pragma once

#define ESC_START     "["
#define ESC_END       "]"
#define COLOR_FATAL   "31;40;5m"
#define COLOR_ALERT   "31;40;1m"
#define COLOR_CRIT    "31;40;1m"
#define COLOR_ERROR   "31;40;1m"
#define COLOR_WARN    "33;40;1m"
#define COLOR_NOTICE  "34;40;1m"
#define COLOR_INFO    "32;40;1m"
#define COLOR_DEBUG   "36;40;1m"
#define COLOR_TRACE   "37;40;1m"

#define LogI(format, ...) (printf("[I]-[%s]:" format, __FUNCTION__, ##__VA_ARGS__))
#define LogD(format, ...) (printf("[D]-[%s]:" format, __FUNCTION__, ##__VA_ARGS__))
#define LogW(format, ...) (printf("[W]-[%s]:" format, __FUNCTION__, ##__VA_ARGS__))
#define LogE(format, ...) (printf("[E]-[%s]:" format, __FUNCTION__, ##__VA_ARGS__))
