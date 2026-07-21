// Copyright (c) 2020 Emmanuel Arias
#pragma once
#include <cinttypes>
#include <utility>

namespace dearnes {

class Bus;


/// <summary>
/// Module to handle the DMA transfer process. In this process, the game can copy sprite
/// data to the PPU OAM memory.
/// 
/// For more info refer to: https://wiki.nesdev.com/w/index.php/PPU_OAM#DMA
/// </summary>
class Dma {
   public:

    /// <summary>
    /// Set the reference pointer for the Bus.
    /// </summary>
    /// <param name="bus"></param>
    void SetBus(Bus *bus);

    /// <summary>
    /// Start the DMA transfer process. This function will set up a flag
    /// to indicate that the CPU wants to tranfer to the PPU OAM memory.
    /// This will copy the starting page to read from. The starting address
    /// will be set as:
    /// High byte: dmaPageHighByte
    /// Low byte: 0x00 (DMA offset 0x00 - 0xFF)
    /// 
    /// </summary>
    /// <param name="dmaPageHighByte"></param>
    void StartTransfer(uint8_t dmaPageHighByte);

    /// <summary>
    /// The DMA transfer process needs to wait for a even frame. This might be refactor
    /// to be transparent.
    /// </summary>
    void StopWaiting();

    /// <summary>
    /// Read data from the current address, defined by:
    /// High byte: dmaPageHighByte
    /// Low byte: DMA offset 0x00 - 0xFF
    /// </summary>
    void ReadData();

    /// <summary>
    /// Return the current DMA address to be copy to the OAM memory, and the
    /// data to be copied.
    /// If the low byte offset reaches 0xFF, the DMA transfer process will be terminated.
    /// </summary>
    /// <returns></returns>
    std::pair<uint8_t, uint8_t> GetLastReadData();

    /// <summary>
    /// Reset the DMA registers.
    /// </summary>
    void Reset();

    /// <summary>
    /// Informs is there is a DMA transfer requested/in progress.
    /// </summary>
    /// <returns></returns>
    inline bool IsTranferInProgress() const { return m_DmaTransfer; }

    /// <summary>
    /// Informs if the DMA transfer process is waiting for a even cycle.
    /// </summary>
    /// <returns></returns>
    inline bool IsInWaitState() const { return m_DmaWait; }

   private:
    Bus *m_Bus = nullptr;
    uint8_t m_DmaPage = 0x00;
    uint8_t m_DmaAddress = 0x00;
    uint8_t m_DmaData = 0x00;

    bool m_DmaTransfer = false;
    bool m_DmaWait = true;

    void FinishTransfer();
};

}  // namespace dearnes
