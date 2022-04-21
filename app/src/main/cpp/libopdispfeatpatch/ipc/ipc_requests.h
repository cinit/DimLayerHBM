//
// Created by kinit on 2021-11-18.
//

#ifndef DISP_TUNER_NATIVES_IPC_REQUESTS_H
#define DISP_TUNER_NATIVES_IPC_REQUESTS_H

#include <cstdint>

namespace halpatch::opdispfeature {

enum class RequestId : uint32_t {
    GET_VERSION = 0x1,
    GET_HOOK_STATUS = 0x40,
    INIT_PLT_HOOK = 0x41,
    GET_DEVICE_STATUS = 0x50,
    DEVICE_OPERATION_WRITE = 0x51,
    DEVICE_OPERATION_IOCTL0 = 0x52,
    OP_DISP_GET_DIM_LAYER_HBM = 0x70,
    OP_DISP_SET_DIM_LAYER_HBM = 0x71,
    OP_DISP_GET_DIM_LAYER_HBM_FORCE_STATUS = 0x72,
    OP_DISP_SET_DIM_LAYER_HBM_FORCE_STATUS = 0x73,
};

struct OpDisplayFeatureStatus {
    static constexpr uint8_t STATUS_UNKNOWN = 0;
    static constexpr uint8_t STATUS_ENABLED = 1;
    static constexpr uint8_t STATUS_DISABLED = 2;

    uint8_t opDisplayFeatureServiceStatus;
    uint8_t unused_0_0;
    uint8_t unused1_0;
    uint8_t unused1_1;
    uint32_t deviceFileDescriptor;
    uint32_t unused4_2;
    uint32_t unused4_3;
};
static_assert(sizeof(OpDisplayFeatureStatus) == 16, "OpDisplayFeatureStatus size error");

struct DeviceIoctl0Request {
    uint64_t request;
    uint64_t arg;
};
static_assert(sizeof(DeviceIoctl0Request) == 16, "DeviceIoctl0Request size error");

struct DeviceWriteRequest {
    uint32_t length;
    uint8_t data[];
};
// no size static assert...

}

#endif //DISP_TUNER_NATIVES_IPC_REQUESTS_H
