// Copyright (c) 2020 Emmanuel Arias
#include "dear_nes_lib/cartridge_loader.h"

#include <fstream>
#include <vector>

#include "dear_nes_lib/cartridge.h"
#include "dear_nes_lib/cartridge_header.h"
#include "dear_nes_lib/mapper_000.h"

namespace dearnes {

// Forward declaration
class Cartridge;

std::variant<CartridgeLoaderError, Cartridge*>
CartridgeLoader::LoadNewCartridge(const std::string& fileName) {
    std::ifstream ifs;
    ifs.open(fileName, std::ifstream::binary);
    
    return LoadNewCartridge(ifs);
}

std::variant<CartridgeLoaderError, Cartridge*>
CartridgeLoader::LoadNewCartridge(std::ifstream& inputStream) {
    std::variant<CartridgeLoaderError, Cartridge*> result;

    if (!inputStream.is_open()) {
        result = CartridgeLoaderError::FILE_NOT_FOUND;
        return result;
    }

    // This consumes the header from the ifstream
    CartridgeHeader header{inputStream};

    // Check for the mapper type
    if (!IsMapperSupported(header.GetMapperId())) {
        result = CartridgeLoaderError::MAPPER_NOT_SUPPORTED;
        return result;
    }

    IMapper* mapperPtr = CreateMapper(header);

    if (header.HasTrainerData()) {
        inputStream.seekg(512, std::ios_base::cur);
    }

    std::vector<uint8_t> programMemory(header.GetProgramMemorySize());
    inputStream.read(reinterpret_cast<char*>(programMemory.data()),
             programMemory.size());

    std::vector<uint8_t> characterMemory(header.GetCharacterMemorySize());
    inputStream.read(reinterpret_cast<char*>(characterMemory.data()),
             characterMemory.size());

    result =
        new Cartridge{std::move(header), mapperPtr, std::move(programMemory),
                      std::move(characterMemory)};

    return result;
}

bool CartridgeLoader::IsMapperSupported(uint8_t mapperId) {
    if (mapperId == 0x00) {
        return true;
    }
    return false;
}

IMapper* CartridgeLoader::CreateMapper(const CartridgeHeader& header) {
    if (header.GetMapperId() == 0x00) {
        return new Mapper_000(header.GetProgramMemoryBanks(),
                              header.GetCharacterMemoryBanks());
    }
    // This should not be reachable
    return nullptr;
}

}  // namespace dearnes