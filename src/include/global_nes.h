// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include "include/nes.h"

inline cpuemulator::Nes g_Nes;

inline cpuemulator::Nes* g_GetGlobalNes() { return &g_Nes; }
