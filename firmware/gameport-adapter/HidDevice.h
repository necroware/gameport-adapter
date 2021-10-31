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

#include <HID.h>

class HidDevice : public PluggableUSBModule {
public:

  explicit HidDevice()
  : PluggableUSBModule(1, 1, epType) {
    PluggableUSB().plug(this);
  }

  void AppendDescriptor(HIDSubDescriptor *node) {

    if (rootNode == nullptr) {
      rootNode = node;
    } else {
      auto current = rootNode;
      while (current->next) {
        current = current->next;
      }
      current->next = node;
    }
    descriptorSize += node->length;
  }

  int SendReport(uint8_t id, const void *data, int len) const {

    const auto ret = USB_Send(pluggedEndpoint, &id, 1);
    if (ret < 0) {
      return ret;
    }

    const auto ret2 = USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, len);
    if (ret2 < 0) {
      return ret2;
    }

    return ret + ret2;
  }

protected:

  int getInterface(uint8_t *interfaceCount) override {
    *interfaceCount += 1; // uses 1
    HIDDescriptor hidInterface {
        D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
        D_HIDREPORT(descriptorSize),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
  }

  int getDescriptor(USBSetup &setup) override {

    // Check if this is a HID Class Descriptor request
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
      return 0;
    }
    if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
      return 0;
    }

    // In a HID Class Descriptor wIndex cointains the interface number
    if (setup.wIndex != pluggedInterface) {
      return 0;
    }

    int total = 0;
    for (auto node = rootNode; node; node = node->next) {
      const auto res = USB_SendControl(0, node->data, node->length);
      if (res < 0) {
        return -1;
      }
      total += res;
    }

    // Reset the protocol on reenumeration. Normally the host should not 
    // assume the state of the protocol due to the USB specs, but Windows
    // and Linux just assumes its in report mode.
    protocol = HID_REPORT_PROTOCOL;

    return total;
  }

  uint8_t getShortName(char *name) override {
    name[0] = 'H';
    name[1] = 'I';
    name[2] = 'D';
    name[3] = 'A' + (descriptorSize & 0x0F);
    name[4] = 'A' + ((descriptorSize >> 4) & 0x0F);
    return 5;
  }

  bool setup(USBSetup &setup) override {

    if (pluggedInterface != setup.wIndex) {
      return false;
    }

    const auto request = setup.bRequest;
    const auto requestType = setup.bmRequestType;

    if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
      if (request == HID_GET_REPORT) {
        // TODO: HID_GetReport();
        return true;
      }
      if (request == HID_GET_PROTOCOL) {
        // TODO: Send8(protocol);
        return true;
      }
      if (request == HID_GET_IDLE) {
        // TODO: Send8(idle);
      }
    }

    if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
      if (request == HID_SET_PROTOCOL) {
        // The USB Host tells us if we are in boot or report mode.
        // This only works with a real boot compatible device.
        protocol = setup.wValueL;
        return true;
      }
      if (request == HID_SET_IDLE) {
        idle = setup.wValueL;
        return true;
      }
      if (request == HID_SET_REPORT) {
        // uint8_t reportID = setup.wValueL;
        // uint16_t length = setup.wLength;
        // uint8_t data[length];
        // Make sure to not read more data than USB_EP_SIZE.
        // You can read multiple times through a loop.
        // The first byte (may!) contain the reportID on a multreport.
        // USB_RecvControl(data, length);
      }
    }

    return false;
  }

private:
  uint8_t epType[1]{EP_TYPE_INTERRUPT_IN};
  HIDSubDescriptor *rootNode{nullptr};
  uint16_t descriptorSize{0};
  uint8_t protocol{HID_REPORT_PROTOCOL};
  uint8_t idle{1};
};

