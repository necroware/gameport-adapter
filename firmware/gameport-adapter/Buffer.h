// This file is part of Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by // the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <Arduino.h>

template <size_t Size>
struct Buffer {
    static const uint8_t MAX_SIZE{Size};
    uint8_t data[MAX_SIZE]{};
    uint8_t size{};
};

template <size_t S>
class BufferFillerImpl {
public:
    using BufferType = Buffer<S>;

    explicit BufferFillerImpl(BufferType& buffer)
        : m_buffer(buffer) {}

    ~BufferFillerImpl() = default;
    BufferFillerImpl(const BufferFillerImpl&) = delete;
    BufferFillerImpl(BufferFillerImpl&&) = default;
    BufferFillerImpl& operator=(const BufferFillerImpl&) = delete;
    BufferFillerImpl& operator=(BufferFillerImpl&&) = default;

    template <typename T>
    BufferFillerImpl& push(const T& bits) {
        return push(uint32_t(bits), sizeof(bits) * BITS_PER_BYTE);
    }

    BufferFillerImpl& push(uint32_t bits, size_t count) {
        while(count) {
            const auto pushed = fillup(bits, count);
            bits >>= pushed;
            count -= pushed;
        }
        return *this;
    }

    BufferFillerImpl& allign() {
        if (bitsUsed) {
            m_buffer.size++;
            bitsUsed = 0u;
        }        
        return *this;
    }

private:
    static const auto BITS_PER_BYTE{8u};

    BufferType& m_buffer;
    size_t bitsUsed{};

    size_t fillup(byte bits, size_t count) {

        const auto freeBits = BITS_PER_BYTE - bitsUsed;
        if (freeBits == 0) {
            return 0;
        }

        // If the total count is above the free bits available in the current
        // byte, then push just a chunk to fill up the current byte
        if (freeBits < count) {
            return fillup(bits, freeBits);
        }

        if (bitsUsed == 0u) {
          m_buffer.data[m_buffer.size] = 0u;
        }

        // Obviously the total count is less, than the amount of free bytes
        // available in the current byte, so the following block will simply
        // add the bits to the current byte and return
        const auto mask = uint32_t(1u << count) - 1;
        m_buffer.data[m_buffer.size] |= byte(bits & mask) << bitsUsed;
        bitsUsed += count;
        if (bitsUsed == BITS_PER_BYTE) {
            m_buffer.size++;
            bitsUsed = 0u;
        }
        return count;
    }
};

template <size_t S>
BufferFillerImpl<S> BufferFiller(Buffer<S>& buffer) {
    return BufferFillerImpl<S>(buffer);
}

