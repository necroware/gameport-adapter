// This file is part of Necroware's GamePort adapter firmware.
// Copyright (C) 2021 Necroware
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
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

#include "DigitalPin.h"
#include "GamePort.h"
#include "Joystick.h"
#include "Utilities.h"

class Logitech : public Joystick {
public:
  bool init() override {
    enableDigitalMode();
    if (!readMetaData()) {
      return false;
    }

    // Create joystick description
    m_description.name = m_metaData.deviceName;
    m_description.numAxes = min(Joystick::MAX_AXES, m_metaData.num10bitAxes + m_metaData.num8bitAxes);
    m_description.numButtons = m_metaData.numPrimaryButtons + m_metaData.numSecondaryButtons;
    m_description.numHats = min(Joystick::MAX_HATS, m_metaData.numHats);

    // Initialize axes centers
    for (auto i = 0u; i < m_description.numAxes; i++) {
      const auto mid = (i < m_metaData.num10bitAxes) ? 512 : 128;
      m_limits[i].min = mid - 50;
      m_limits[i].max = mid + 50;
    }

    return true;
  }

  bool update() override {

    // === Status packet format ===
    //
    // Offset Bits  Description
    // --------------------------------------------------------
    // 0      4     Low nibble of the Device ID
    // 4      4     High nibble of the Device ID
    // 8      10*N  10bit axes (N is number of axes)
    // ?      8*N   8bit axes (N is number of axes)
    // ?      1*N   Buttons (N is number of buttons)
    // ?      R*N   Hats (R is resolution, N is number of hats)
    // ?      1*N   Secondary buttons (N is number of buttons)

    // Cyberman 2 seems not to work properly if the packets are
    // read too fast. Following delay will ensure, that after the
    // last read at least 5ms passed to ensure, that the joystick
    // cooled down again.
    delayMicroseconds(5000);

    const auto packet = readPacket();

    if (packet.size != m_metaData.packageSize) {
      return false;
    }

    const auto packetID = getBits(packet, 0, 4) | getBits(packet, 4, 4) << 4;
    if (packetID != m_metaData.deviceID) {
      return false;
    }

    State state;
    uint16_t offset = 8u;

    for (auto i = 0u; i < m_description.numAxes; i++) {
      const auto bits = (i < m_metaData.num10bitAxes) ? 10 : 8;
      const uint16_t value = getBits(packet, offset, bits);
      if (value < m_limits[i].min) {
        m_limits[i].min = value;
      } else if (value > m_limits[i].max) {
        m_limits[i].max = value;
      }
      state.axes[i] = map(value, m_limits[i].min, m_limits[i].max, 0, 1023);
      offset += bits;
    }

    uint16_t button = 0u;
    for (auto i = 0u; i < m_metaData.numPrimaryButtons; i++) {
      state.buttons |= getBits(packet, offset++, 1) << button++;
    }

    auto hatResolution = [](uint16_t value) {
      uint8_t result = 0u;
      while (value) {
        value >>= 1;
        result++;
      }
      return result;
    }(m_metaData.numHatDirections);

    for (auto i = 0u; i < m_description.numHats; i++) {
      const auto value = getBits(packet, offset, hatResolution);
      state.hats[i] = map(value, 0, m_metaData.numHatDirections, 0, 8);
      offset += hatResolution;
    }

    for (auto i = 0u; i < m_metaData.numSecondaryButtons; i++) {
      state.buttons |= getBits(packet, offset++, 1) << button++;
    }

    m_state = state;
    return true;
  }

  const State &getState() const override {
    return m_state;
  }

  const Description &getDescription() const override {
    return m_description;
  }

private:
  struct MetaData {
    char deviceName[32]{};
    uint8_t deviceID{};
    uint8_t packageSize{};
    uint8_t num8bitAxes{};
    uint8_t num10bitAxes{};
    uint8_t numPrimaryButtons{};
    uint8_t numSecondaryButtons{};
    uint8_t numHats{};
    uint8_t numHatDirections{};
  };

  struct Limits {
    uint16_t min, max;
  };

  /// Internal bit structure which is filled by reading from the joystick.
  using Packet = Buffer<128>;

  static uint16_t getBits(const Packet& packet, uint8_t offset, uint8_t count) {
    uint16_t result = 0u;
    if (offset < packet.size && count <= (packet.size - offset)) {
      for (auto i = 0u; i < count; i++) {
        result = (result << 1) | packet.data[offset + i];
      }
    }
    return result;
  }

  DigitalOutput<GamePort<3>::pin> m_trigger;
  DigitalInput<GamePort<2>::pin, true> m_data0;
  DigitalInput<GamePort<7>::pin, true> m_data1;
  MetaData m_metaData;
  Description m_description;
  State m_state;
  Limits m_limits[Joystick::MAX_AXES];

  void enableDigitalMode() const {
    static const uint16_t seq[] = {4, 2, 3, 10, 6, 11, 7, 9, 11, 0};
    const InterruptStopper noirq;
    for (auto i = 0u; seq[i]; i++) {
      m_trigger.pulse(10u);
      delayMicroseconds(seq[i] * 1000u);
    }
  }

  byte readData() const {
    const auto b0 = m_data0.read();
    const auto b1 = m_data1.read();
    return bool(b0) | bool(b1) << 1;
  }

  Packet readPacket() const {
    static constexpr auto TIMEOUT = 32u;
    auto timeout = TIMEOUT;
    auto first = true;
    Packet packet;
    const InterruptStopper noirq;
    auto last = readData();
    m_trigger.setHigh();
    while (timeout-- && packet.size < Packet::MAX_SIZE) {
      const auto next = readData();
      const auto edge = last ^ next;
      if (edge) {
        if (first) {
          first = false;
        } else {
          // Normally both data bits should never flip simultaneously.
          // We should get either 10, when data1 has flipped, or 01,
          // when data0 has flipped. So if we just shift the edge to
          // the right once, we will get the needed 1 or 0 bit value.
          packet.data[packet.size++] = edge >> 1;
        }
        last = next;
        timeout = TIMEOUT;
      }
    }
    m_trigger.setLow();
    return packet;
  }

  bool readMetaData() {

    // === Metadata packet format ===
    //
    // Offset Bits Description
    // --------------------------------------------------------
    // 0      10   Metadata package size
    // 10     4    Low nibble of the Device ID
    // 14     4    High nibble of the Device ID
    // 18     4    Feature flags
    // 22     10   Status package size
    // 32     4    Number of axes (0..15)
    // 36     6    Number of buttons (0..63)
    // 42     6    Number of hat directions (0..63)
    // 48     6    Number of secondary buttons (0..63)
    // 54     4    Number of secondary hats (0..15)
    // 58     4    Number of 8-bit axes (0..15)
    // 62     4    Length of the cname in bytes (0..15)
    // 66     8*N  Characters of the cname (N is the length)

    // === Flags ===
    //
    // Bit Description
    // ---------------------
    // 1   Reserved
    // 2   Reserved
    // 3   Has Hat
    // 4   Has 10-bit axes

    const auto packet = readPacket();

    const auto metaSize = getBits(packet, 0, 10);
    if (metaSize != packet.size) {
      log("Meta data package size mismatch, expected %d but got %d", packet.size, metaSize);
      return false;
    }

    m_metaData.deviceID = getBits(packet, 10, 4) | getBits(packet, 14, 4) << 4;
    const auto flags = getBits(packet, 18, 4);
    m_metaData.numHats = (flags & 4) ? 1 : 0;
    m_metaData.packageSize = getBits(packet, 22, 10);
    const auto numTotalAxes = getBits(packet, 32, 4);
    m_metaData.numPrimaryButtons = getBits(packet, 36, 6);
    m_metaData.numHatDirections = getBits(packet, 42, 6);
    m_metaData.numSecondaryButtons = getBits(packet, 48, 6);
    m_metaData.numHats += getBits(packet, 54, 4);
    const auto num8bitAxes = getBits(packet, 58, 4);
    if (flags & 8) {
      m_metaData.num10bitAxes = numTotalAxes - num8bitAxes;
      m_metaData.num8bitAxes = num8bitAxes;
    } else {
      m_metaData.num8bitAxes = numTotalAxes;
      m_metaData.num10bitAxes = 0u;
    }
    const auto cnameLength = getBits(packet, 62, 4);
    m_metaData.deviceName[cnameLength] = 0;
    for (auto i = 0u; i < cnameLength; i++) {
      m_metaData.deviceName[i] = getBits(packet, 66 + 8 * i, 8);
    }

    return true;
  }
};
