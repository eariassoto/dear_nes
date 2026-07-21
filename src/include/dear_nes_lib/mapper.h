// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cstdint>
#include <memory>

namespace dearnes {

/// <summary>
/// Interface class for iNES mapper implementation. Each implementation will define
/// the memory address that belongs to it. This class might be reworked in the future
/// to avoid having virtual functions.
/// </summary>
class IMapper {
   public:
    /// <summary>
    /// Constructor for the mapper. Bank quantity is required, though this\
    /// might change in the future.
    /// </summary>
    /// <param name="prgBanks"></param>
    /// <param name="chrBanks"></param>
    IMapper(uint8_t prgBanks, uint8_t chrBanks);

    virtual ~IMapper() = default;

    /// <summary>
    /// Handle CPU read request, if the address to read belongs to the mapper domain,
    /// save the value in the input param and return true, otherwise return false.
    /// </summary>
    /// <param name="addr"></param>
    /// <param name="mappedAddr"></param>
    /// <returns></returns>
    virtual bool CpuMapRead(uint16_t addr, uint32_t &mappedAddr) = 0;

    /// <summary>
    /// Handle CPU write request, if the address to read belongs to the mapper
    /// domain, write the data and return true, otherwise return false.
    /// </summary>
    /// <param name="addr"></param>
    /// <param name="mappedAddr"></param>
    /// <returns></returns>
    virtual bool CpuMapWrite(uint16_t addr, uint32_t &mappedAddr) = 0;

    /// <summary>
    /// Handle PPU read request, if the address to read belongs to the mapper
    /// domain, save the value in the input param and return true, otherwise
    /// return false.
    /// </summary>
    /// <param name="addr"></param>
    /// <param name="mappedAddr"></param>
    /// <returns></returns>
    virtual bool PpuMapRead(uint16_t addr, uint32_t &mappedAddr) = 0;

    /// <summary>
    /// Handle PPU write request, if the address to read belongs to the mapper
    /// domain, write the data and return true, otherwise return false.
    /// </summary>
    /// <param name="addr"></param>
    /// <param name="mappedAddr"></param>
    /// <returns></returns>
    virtual bool PpuMapWrite(uint16_t addr, uint32_t &mappedAddr) = 0;

   protected:
    /// <summary>
    /// Number of program memory banks
    /// </summary>
    uint8_t m_PrgBanks = 0;

    /// <summary>
    /// Number of character memory banks
    /// </summary>
    uint8_t m_ChrBanks = 0;
};
}  // namespace dearnes
