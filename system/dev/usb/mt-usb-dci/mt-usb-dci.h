// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ddktl/device.h>
#include <ddktl/protocol/platform-device.h>
#include <ddktl/protocol/usb-dci.h>
#include <fbl/macros.h>
#include <lib/zx/handle.h>

namespace mt_usb_dci {

class MtUsbDci;
using MtUsbDciType = ddk::Device<MtUsbDci>;

class MtUsbDci : public MtUsbDciType, public ddk::UsbDciProtocol<MtUsbDci> {
public:
    explicit MtUsbDci(zx_device_t* parent, platform_device_protocol_t* pdev, zx_handle_t bti)
        : MtUsbDciType(parent), pdev_(pdev), bti_(bti) {}

    static zx_status_t Create(zx_device_t* parent);

    // Device protocol implementation.
    void DdkRelease();

    // USB DCI protocol implementation.
     void UsbDciRequestQueue(usb_request_t* req);
     zx_status_t UsbDciSetInterface(const usb_dci_interface_t* interface);
     zx_status_t UsbDciConfigEp(const usb_endpoint_descriptor_t* ep_desc, const
                                usb_ss_ep_comp_descriptor_t* ss_comp_desc);
     zx_status_t UsbDciDisableEp(uint8_t ep_address);
     zx_status_t UsbDciEpSetStall(uint8_t ep_address);
     zx_status_t UsbDciEpClearStall(uint8_t ep_address);
     zx_status_t UsbDciGetBti(zx_handle_t* out_bti);

private:
    DISALLOW_COPY_ASSIGN_AND_MOVE(MtUsbDci);

    ddk::PlatformDevProtocolProxy pdev_;
    usb_dci_interface_t dci_intf_ = {};
    zx::handle bti_;
};

} // namespace mt_usb_dci

__BEGIN_CDECLS
zx_status_t mt_usb_dci_bind(void* ctx, zx_device_t* parent);
__END_CDECLS
