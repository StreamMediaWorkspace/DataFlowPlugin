#pragma once
#include <stdio.h>
#define LogI(format, ...) (printf("[I][%s]:" format, __FUNCTION__, ##__VA_ARGS__))
#define LogD(format, ...) (printf("[D][%s]:" format, __FUNCTION__, ##__VA_ARGS__))
#define LogW(format, ...) (printf("[W][%s]:" format, __FUNCTION__, ##__VA_ARGS__))
#define LogE(format, ...) (printf("[E][%s]:" format, __FUNCTION__, ##__VA_ARGS__))
