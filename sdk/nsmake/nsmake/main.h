#pragma once
#include <stdio.h>

#define LogMsg(fmt, ...) do { printf(fmt "\n", __VA_ARGS__); } while (0)
