// Copyright (c) 2020 Emmanuel Arias
#include "dear_nes_lib/dma.h"

#include <cassert>

#include "dear_nes_lib/bus.h"

namespace dearnes {

void Dma::SetBus(Bus* bus) {
    assert(bus != nullptr);
    m_Bus = bus;
}

void Dma::StartTransfer(uint8_t dmaPageHighByte) {
    m_DmaPage = dmaPageHighByte;
    m_DmaAddress = 0x00;
    m_DmaTransfer = true;
}

void Dma::StopWaiting() { m_DmaWait = false; }

void Dma::ReadData() {
    m_DmaData = m_Bus->CpuRead(m_DmaPage << 8 | m_DmaAddress);
}

std::pair<uint8_t, uint8_t> Dma::GetLastReadData() {
    uint8_t lastAddr = m_DmaAddress;
    if (++m_DmaAddress == 0x00) {
        FinishTransfer();
    }
    return {lastAddr, m_DmaData};
}

void Dma::FinishTransfer() {
    m_DmaTransfer = false;
    m_DmaWait = true;
}

void Dma::Reset() {
    m_DmaPage = 0x00;
    m_DmaAddress = 0x00;
    m_DmaData = 0x00;
    m_DmaWait = true;
    m_DmaTransfer = false;
}

}  // namespace dearnes