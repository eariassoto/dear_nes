// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <string>
#include <variant>

#include "dear_nes_lib/enums.h"

namespace dearnes {

// Forward declaration
class Cartridge;
class CartridgeHeader;
class IMapper;

class CartridgeLoader {
   public:
    std::variant<CartridgeLoaderError, Cartridge*> LoadNewCartridge(
        const std::string& fileName);

    std::variant<CartridgeLoaderError, Cartridge*> LoadNewCartridge(
        std::ifstream& inputStream);

   private:
    bool IsMapperSupported(uint8_t mapperId);

    IMapper* CreateMapper(const CartridgeHeader& header);
};

}  // namespace dearnes