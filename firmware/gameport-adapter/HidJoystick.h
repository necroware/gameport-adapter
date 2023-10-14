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

#include "Buffer.h"
#include "HidDevice.h"
#include "Joystick.h"
#include "Utilities.h"
#include <Arduino.h>

class HidJoystick {
public:
  bool init(Joystick *joystick) {
    if (!joystick || !joystick->init()) {
      return false;
    }

    m_joystick = joystick;
    m_hidDescription = createDescription(*m_joystick);
    m_subDescriptor = new HIDSubDescriptor{m_hidDescription.data, m_hidDescription.size};
    m_hidDevice.AppendDescriptor(m_subDescriptor);

    log("Detected device: %s", joystick->getDescription().name);
    return true;
  }

  bool update() {
    if (!m_joystick || !m_joystick->update()) {
      return false;
    }

    const auto packet = createPacket(*m_joystick);
    m_hidDevice.SendReport(DEVICE_ID, packet.data, packet.size);
    return true;
  }

private:
  using BufferType = Buffer<255>;
  static const uint8_t DEVICE_ID{3};

  static BufferType createDescription(const Joystick &joystick) {

    enum class ID : uint8_t {
      application = 0x01,
      button = 0x09,
      collection = 0xa1,
      end_collection = 0xc0,
      generic_desktop = 0x01,
      hat_switch = 0x39,
      input = 0x81,
      input_const = 0x03,
      input_data = 0x02,
      joystick = 0x04,
      logical_max = 0x26,
      logical_min = 0x15,
      report_count = 0x95,
      report_id = 0x85,
      report_size = 0x75,
      simulation_controls = 0x02,
      throttle = 0xbb,
      usage = 0x09,
      usage_max = 0x29,
      usage_min = 0x19,
      usage_page = 0x05,
    };

    const auto &desc = joystick.getDescription();
    BufferType buffer;
    auto filler = BufferFiller(buffer);

    auto pushData = [&filler](uint8_t size, uint8_t count) -> uint32_t {
      filler.push(ID::report_size).push(size);
      filler.push(ID::report_count).push(count);
      filler.push(ID::input).push(ID::input_data);
      const auto padding = (size * count) % 8u;
      if (padding) {
        filler.push(ID::report_size).push<uint8_t>(8u - padding);
        filler.push(ID::report_count).push<uint8_t>(1);
        filler.push(ID::input).push(ID::input_const);
      }
    };

    filler.push(ID::usage_page).push(ID::generic_desktop);
    filler.push(ID::usage).push(ID::joystick);
    filler.push(ID::collection).push(ID::application);
    filler.push(ID::report_id).push<uint8_t>(DEVICE_ID);

    // Push axes
    if (desc.numAxes > 0) {
      for (auto i = 0u; i < desc.numAxes; i++) {
        static constexpr uint8_t x_axis = 0x30;
        filler.push(ID::usage).push<uint8_t>(x_axis + i);
      }
      filler.push(ID::logical_min).push<uint8_t>(0);
      filler.push(ID::logical_max).push<uint16_t>(1023);
      pushData(10, desc.numAxes);
    }

    // Push hat
    if (desc.hasHat) {
      filler.push(ID::usage).push(ID::hat_switch);
      filler.push(ID::logical_min).push<uint8_t>(1);
      filler.push(ID::logical_max).push<uint16_t>(8);
      pushData(4, 1);
    }

    // Push buttons
    if (desc.numButtons > 0) {
      filler.push(ID::usage_page).push(ID::button);
      filler.push(ID::usage_min).push<uint8_t>(1);
      filler.push(ID::usage_max).push<uint8_t>(desc.numButtons);
      filler.push(ID::logical_min).push<uint8_t>(0);
      filler.push(ID::logical_max).push<uint16_t>(1);
      pushData(1, desc.numButtons);
    }

    filler.push(ID::end_collection);
    return buffer;
  }

  static BufferType createPacket(const Joystick &joystick) {

    const auto &state = joystick.getState();
    const auto &description = joystick.getDescription();
    BufferType buffer;
    auto filler = BufferFiller(buffer);

    for (auto i = 0u; i < description.numAxes; i++) {
      filler.push(state.axes[i], 10);
    }
    filler.allign();

    if (description.hasHat) {
      filler.push(state.hat, 4);
      filler.allign();
    }

    if (description.numButtons) {
      filler.push(state.buttons, description.numButtons);
      filler.allign();
    }

    return buffer;
  }

  Joystick *m_joystick{};
  BufferType m_hidDescription{};
  HIDSubDescriptor *m_subDescriptor{};
  HidDevice m_hidDevice;
};

