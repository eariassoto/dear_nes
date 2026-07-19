// Copyright (c) 2020 Emmanuel Arias
#include "dear_nes_lib/mapper.h"

namespace dearnes {
IMapper::IMapper(uint8_t prgBanks, uint8_t chrBanks)
    : m_PrgBanks{prgBanks}, m_ChrBanks{chrBanks} {}
}  // namespace dearnes