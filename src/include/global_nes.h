// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include "include/nes.h"

// Since C++17, inline variables are guaranteed to be identical
// across tranlations units. I.e. the linker will know that this is
// the only instance.
inline cpuemulator::Nes g_Nes;

inline cpuemulator::Nes* g_GetGlobalNes() { return &g_Nes; }
