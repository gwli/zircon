// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <ddk/binding.h>
#include <ddk/debug.h>
#include <ddk/device.h>
#include <ddk/platform-defs.h>
#include <ddk/protocol/i2c-impl.h>
#include <ddk/protocol/platform-bus.h>
#include <ddk/protocol/platform-device.h>

#include <fbl/alloc_checker.h>
#include <fbl/auto_call.h>
#include <fbl/string_printf.h>

#include <lib/zx/time.h>

#include <zircon/assert.h>
#include <zircon/types.h>

#include "mt8167-i2c-regs.h"
#include "mt8167-i2c.h"

namespace mt8167_i2c {

constexpr size_t kMaxTransferSize = UINT16_MAX - 1; // More than enough

uint32_t Mt8167I2c::I2cImplGetBusCount() {
    return bus_count_;
}

zx_status_t Mt8167I2c::I2cImplGetMaxTransferSize(uint32_t bus_id, size_t* out_size) {
    *out_size = kMaxTransferSize;
    return ZX_OK;
}

zx_status_t Mt8167I2c::I2cImplSetBitRate(uint32_t bus_id, uint32_t bitrate) {
    // TODO(andresoportus): Support changing frequencies
    return ZX_ERR_NOT_SUPPORTED;
}

zx_status_t Mt8167I2c::I2cImplTransact(uint32_t bus_id, i2c_impl_op_t* ops, size_t count) {
    zx_status_t status = ZX_OK;
    for (size_t i = 0; i < count; ++i) {
        if (ops[i].address > 0xFF) {
            return ZX_ERR_NOT_SUPPORTED;
        }
        if (ops[i].is_read) {
            status = Read(static_cast<uint8_t>(ops[i].address), ops[i].buf, ops[i].length,
                          ops[i].stop);
        } else {
            status = Write(static_cast<uint8_t>(ops[i].address), ops[i].buf, ops[i].length,
                           ops[i].stop);
        }
        if (status != ZX_OK) {
            Reset();
            return status;
        }
    }
    return ZX_OK;
}

zx_status_t Mt8167I2c::WaitFor(Wait type) {
/*
    zx::time timeout = zx::deadline_after(zx::msec(10));
    while (zx::clock::get_monotonic() < timeout) {
        switch (type) {
        case Wait::kIdle:
            if (!StatusReg::Get().ReadFrom(mmio_.get()).bus_busy()) {
                return ZX_OK;
            }
            break;
        case Wait::kBusy:
            if (StatusReg::Get().ReadFrom(mmio_.get()).bus_busy()) {
                return ZX_OK;
            }
            break;
        case Wait::kInterruptPending:
            if (StatusReg::Get().ReadFrom(mmio_.get()).interrupt_pending()) {
                return ZX_OK;
            }
            break;
        }
        // TODO(andresoportus): Use interrupts instead of polling
        zx::nanosleep(zx::deadline_after(zx::usec(10)));
    }
    zxlogf(ERROR, "Mt8167I2c::WaitFor: %s timedout\n", WaitStr(type));
    ControlReg::Get().ReadFrom(mmio_.get()).Print();
    StatusReg::Get().ReadFrom(mmio_.get()).Print();
*/
    return ZX_ERR_TIMED_OUT;
}

zx_status_t Mt8167I2c::Start() {
/*
    ControlReg::Get().ReadFrom(mmio_.get()).set_master(1).set_transmit(1).WriteTo(mmio_.get());
*/
    return WaitFor(Wait::kBusy);
}

void Mt8167I2c::Stop() {
/*
    ControlReg::Get().ReadFrom(mmio_.get()).set_master(0).set_transmit(0).WriteTo(mmio_.get());
*/
}

void Mt8167I2c::Reset() {
    zxlogf(INFO, "Mt8167I2c::Reset: reseting...\n");
/*
    ControlReg::Get().FromValue(0).WriteTo(mmio_.get()); // Implies set_enable(0).
    StatusReg::Get().FromValue(0).WriteTo(mmio_.get());
    ControlReg::Get().FromValue(0).set_enable(1).WriteTo(mmio_.get());
*/
    WaitFor(Wait::kIdle); // No check for error from it
}

zx_status_t Mt8167I2c::RxData(uint8_t* buf, size_t length, bool stop) {
//    zx_status_t status;
    if (length == 0) {
        return ZX_OK;
    }

/*
    // Switch to Rx mode
    auto control = ControlReg::Get().ReadFrom(mmio_.get()).set_transmit(0).set_tx_ack_disable(0);
    if (length == 1) {
        // If length is 1 then we need to no ACK (to finish RX) immediately
        control.set_tx_ack_disable(1);
    }
    control.WriteTo(mmio_.get());

    StatusReg::Get().ReadFrom(mmio_.get()).set_interrupt_pending(0).WriteTo(mmio_.get());
    // Required dummy read, per reference manual:
    // "If Master Receive mode is required, then I2C_I2CR[MTX] should be toggled and a dummy read
    // of the I2C_I2DR register must be executed to trigger receive data."
    DataReg::Get().ReadFrom(mmio_.get()).data();

    for (size_t i = 0; i < length; ++i) {

        // Wait for and check Rx transfer completed
        status = WaitFor(Wait::kInterruptPending);
        if (status != ZX_OK) {
            return status;
        }
        if (!StatusReg::Get().ReadFrom(mmio_.get()).transfer_complete()) {
            return ZX_ERR_IO;
        }
        StatusReg::Get().ReadFrom(mmio_.get()).set_interrupt_pending(0).WriteTo(mmio_.get());
        if (i == length - 2) {
            // Set TX_ACK_DISABLE two bytes before last
            ControlReg::Get().ReadFrom(mmio_.get()).set_tx_ack_disable(1).WriteTo(mmio_.get());
        }
        if (i == length - 1) {
            if (stop) {
                Stop(); // Set STOP one byte before the last
            }
        }
        buf[i] = DataReg::Get().ReadFrom(mmio_.get()).data();
    }
*/
    return ZX_OK;
}

zx_status_t Mt8167I2c::TxData(const uint8_t* buf, size_t length, bool stop) {
    for (size_t i = 0; i < length; ++i) {
        if (i == length - 1 && stop) {
            Stop(); // Set STOP one byte before the last
        }
/*
        StatusReg::Get().ReadFrom(mmio_.get()).set_interrupt_pending(0).WriteTo(mmio_.get());
        DataReg::Get().FromValue(0).set_data(buf[i]).WriteTo(mmio_.get());

        // Wait for and check Tx transfer completed
        zx_status_t status = WaitFor(Wait::kInterruptPending);
        if (status != ZX_OK) {
            return status;
        }
        if (!StatusReg::Get().ReadFrom(mmio_.get()).transfer_complete()) {
            return ZX_ERR_IO;
        }
*/
    }

    return ZX_OK;
}

zx_status_t Mt8167I2c::TxAddress(uint8_t addr, bool is_read) {
    uint8_t data = static_cast<uint8_t>((addr << 1) | static_cast<uint8_t>(is_read));
    return TxData(&data, 1, false);
}

zx_status_t Mt8167I2c::Read(uint8_t addr, void* buf, size_t len, bool stop) {
/*
    ControlReg::Get().ReadFrom(mmio_.get()).set_repeat_start(1).WriteTo(mmio_.get());
    zx_status_t status = TxAddress(addr, true);
    if (status != ZX_OK) {
        return status;
    }
*/
    return RxData(static_cast<uint8_t*>(buf), len, stop);
}

zx_status_t Mt8167I2c::Write(uint8_t addr, const void* buf, size_t len, bool stop) {
    zx_status_t status = Start();
    if (status != ZX_OK) {
        return status;
    }
    status = TxAddress(addr, false);
    if (status != ZX_OK) {
        return status;
    }
    return TxData(static_cast<const uint8_t*>(buf), len, stop);
}

void Mt8167I2c::DdkRelease() {
    delete this;
}

Mt8167I2c::~Mt8167I2c() {
    // Do this in the destructor in case we fail before device is added to devmgr.
    for (mmio_buffer_t& mmio : mmios_) {
        mmio_buffer_release(&mmio);
    }
    mmio_buffer_release(&xo_mmio_);
}

zx_status_t Mt8167I2c::Init() {
    platform_device_protocol_t pdev;
    if (device_get_protocol(parent(), ZX_PROTOCOL_PLATFORM_DEV, &pdev) != ZX_OK) {
        zxlogf(ERROR, "Mt8167I2c::Bind: ZX_PROTOCOL_PLATFORM_DEV not available\n");
        return ZX_ERR_NOT_SUPPORTED;
    }
    platform_bus_protocol_t pbus;
    if (device_get_protocol(parent(), ZX_PROTOCOL_PLATFORM_BUS, &pbus) != ZX_OK) {
        zxlogf(ERROR, "Mt8167I2c::Bind: ZX_PROTOCOL_PLATFORM_BUS not available\n");
        return ZX_ERR_NOT_SUPPORTED;
    }

    pdev_device_info_t info;
    auto status = pdev_get_device_info(&pdev, &info);
    if (status != ZX_OK) {
        zxlogf(ERROR, "mt8167_i2c_bind: pdev_get_device_info failed\n");
        return ZX_ERR_NOT_SUPPORTED;
    }

    // Last MMIO is for XO clock.
    bus_count_ = info.mmio_count - 1;

    fbl::AllocChecker ac;
    mmios_.reset(new (&ac) mmio_buffer_t[bus_count_], bus_count_);
    if (!ac.check()) {
        return ZX_ERR_NO_MEMORY;
    }
 
    for (uint32_t i = 0; i < bus_count_; i++) {
        status = pdev_map_mmio_buffer2(&pdev, i, ZX_CACHE_POLICY_UNCACHED_DEVICE, &mmios_[i]);
        if (status == ZX_OK) {
            return status;
        }  
    }      
    status = pdev_map_mmio_buffer2(&pdev, bus_count_, ZX_CACHE_POLICY_UNCACHED_DEVICE, &xo_mmio_);
    if (status == ZX_OK) {
        return status;
    }  

    status = DdkAdd("mt8167-i2c");
    if (status != ZX_OK) {
        zxlogf(ERROR, "Mt8167I2c::Bind: DdkAdd failed: %d\n", status);
        return status;
    }

    i2c_impl_protocol_t i2c_proto = {
        .ops = &ops_,
        .ctx = this,
    };
    status = pbus_register_protocol(&pbus, ZX_PROTOCOL_I2C_IMPL, &i2c_proto, NULL, NULL);
    if (status != ZX_OK) {
        zxlogf(ERROR, "Mt8167I2c::Bind: pbus_register_protocol failed: %d\n", status);
        return status;
    }

    return ZX_OK;
}

} // namespace mt8167_i2c

extern "C" zx_status_t mt8167_i2c_bind(void* ctx, zx_device_t* parent) {
    fbl::AllocChecker ac;
    auto dev = fbl::make_unique_checked<mt8167_i2c::Mt8167I2c>(&ac, parent);
    if (!ac.check()) {
        zxlogf(ERROR, "mt8167_i2c_bind: ZX_ERR_NO_MEMORY\n");
        return ZX_ERR_NO_MEMORY;
    }

    auto status = dev->Init();
    if (status != ZX_OK) {
        return status;
    }

    // devmgr is now in charge of the memory for dev
    __UNUSED auto ptr = dev.release();

    return ZX_OK;
}
