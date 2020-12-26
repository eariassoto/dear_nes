// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include "dear_nes_lib/nes.h"

// Since C++17, inline variables are guaranteed to be identical
// across tranlations units. I.e. the linker will know that this is
// the only instance.
inline dearnes::Nes g_Nes;

inline dearnes::Nes* g_GetGlobalNes() { return &g_Nes; }
