//
// Created by kinit on 2021-11-18.
//
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <sys/fcntl.h>

#include "ipc_requests.h"
#include "../../libbasehalpatch/ipc/request_handler.h"
#include "../../libbasehalpatch/hook/hook_proc_symbols.h"

using namespace halpatch;
using namespace halpatch::opdispfeature;

static volatile bool sInitialized = false;

bool sForceDimLayerHBM = false;

static constexpr auto DEV_NULL_PATH = "/dev/null";
static constexpr auto SYS_HBM_PATH = "/sys/kernel/oplus_display/dimlayer_hbm";
static constexpr int IOCTL_SET_DIM_LAYER_HBM = 0x40046f1f;

void beforeOpenHook(HookParams<int, const char *, int, uint32_t> &params) {
    if (!sForceDimLayerHBM) {
        return;
    }
    auto[path, flags, mode] = params.getArgumentsAsTuple();
    if (path != nullptr && strcmp(path, SYS_HBM_PATH) == 0) {
        // only intercept if opened for writing
        int accessMode = (flags & O_ACCMODE);
        if (accessMode == O_WRONLY || accessMode == O_RDWR) {
            params.setArgv<0>(DEV_NULL_PATH);
        }
    }
}

void beforeIoctlHook(HookParams<int, int, unsigned long int, uint64_t> &params) {
    if (!sForceDimLayerHBM) {
        return;
    }
    auto[fd, request, arg] = params.getArgumentsAsTuple();
    if (request == IOCTL_SET_DIM_LAYER_HBM) {
        // block the ioctl
        params.setResult(-1);
        params.setErrno(EACCES);
    }
}

int isDimLayerHbmEnabled() {
    int fd = open(SYS_HBM_PATH, O_RDONLY);
    if (fd < 0) {
        return -errno;
    }
    std::array<char, 2> buf = {};
    if (auto ret = read(fd, buf.data(), 2); ret < 0) {
        int err = errno;
        close(fd);
        return -err;
    }
    close(fd);
    return buf[0] == '1';
}

int setDimLayerHbmEnabled(bool enabled) {
    int fd = open(SYS_HBM_PATH, O_WRONLY);
    if (fd < 0) {
        return -errno;
    }
    std::array<char, 2> buf = {enabled ? '1' : '0', '\n'};
    if (auto ret = write(fd, buf.data(), 2); ret < 0) {
        int err = errno;
        close(fd);
        return -err;
    }
    close(fd);
    return 0;
}

void handleGetVersionRequest(uint32_t requestId, const void *, uint32_t) {
    const char *version = DISPLAY_TUNER_VERSION;
    HalResponse response = {requestId, 0, 0, static_cast<uint32_t>(strlen(version))};
    sendResponsePacket(response, version);
}

void handleGetHookStatusRequest(uint32_t requestId, const void *, uint32_t) {
    HalResponse response = {requestId, sInitialized, 0, 0};
    sendResponsePacket(response, nullptr);
}

void handleInitPltHookRequest(uint32_t requestId, const void *payload, uint32_t payloadSize) {
    HalResponse response = {requestId, 0, 0, 0};
    const auto *originHookProcedure = reinterpret_cast<const OriginHookProcedure *>(payload);
    if (payloadSize != sizeof(OriginHookProcedure) || originHookProcedure == nullptr
        || originHookProcedure->struct_size != sizeof(OriginHookProcedure)) {
        sendResponseError(requestId, HalRequestErrorCode::ERR_INVALID_ARGUMENT,
                          "invalid payload OriginHookProcedure");
    } else {
        int result = hook_sym_init(originHookProcedure);
        if (result == 0) {
            sInitialized = true;
            halpatchhook::callback::setOpenCallback(&beforeOpenHook, nullptr);
            halpatchhook::callback::setIoctlCallback(&beforeIoctlHook, nullptr);
        }
        response.result = result;
        sendResponsePacket(response, nullptr);
    }
}

void handleGetDeviceStatusRequest(uint32_t requestId, const void *, uint32_t) {
    OpDisplayFeatureStatus deviceStatus = {};
    int v = isDimLayerHbmEnabled();
    deviceStatus.opDisplayFeatureServiceStatus = v < 0 ? -1 : 0;
    deviceStatus.deviceFileDescriptor = -1;
    HalResponse response = {requestId, 0, 0, sizeof(OpDisplayFeatureStatus)};
    sendResponsePacket(response, &deviceStatus);
}

void handleGetDimLayerHbmRequest(uint32_t requestId, const void *, uint32_t) {
    HalResponse response = {requestId, static_cast<uint32_t>(isDimLayerHbmEnabled()), 0, 0};
    sendResponsePacket(response, nullptr);
}

void handleSetDimLayerHbmRequest(uint32_t requestId, const void *payload, uint32_t payloadSize) {
    const auto *value = reinterpret_cast<const uint32_t *>(payload);
    if (payloadSize != 4 || payload == nullptr) {
        sendResponseError(requestId, HalRequestErrorCode::ERR_INVALID_ARGUMENT,
                          "invalid payload SetDimLayerHbmRequest");
    } else {
        uint32_t v = *value;
        if (v != 0 && v != 1) {
            sendResponseError(requestId, HalRequestErrorCode::ERR_INVALID_ARGUMENT,
                              "invalid payload SetDimLayerHbmRequest, expected 0 or 1");
        } else {
            int result = setDimLayerHbmEnabled(v);
            HalResponse response = {requestId, static_cast<uint32_t>(result), static_cast<uint32_t>(result), 0};
            sendResponsePacket(response, nullptr);
        }
    }
}

void handleGetDimLayerHbmForcedRequest(uint32_t requestId, const void *, uint32_t) {
    HalResponse response = {requestId, sForceDimLayerHBM, 0, 0};
    sendResponsePacket(response, nullptr);
}

void handleSetDimLayerHbmForcedRequest(uint32_t requestId, const void *payload, uint32_t payloadSize) {
    const auto *value = reinterpret_cast<const uint32_t *>(payload);
    if (payloadSize != 4 || payload == nullptr) {
        sendResponseError(requestId, HalRequestErrorCode::ERR_INVALID_ARGUMENT,
                          "invalid payload SetDimLayerHbmForced");
    } else {
        uint32_t v = *value;
        if (v != 0 && v != 1) {
            sendResponseError(requestId, HalRequestErrorCode::ERR_INVALID_ARGUMENT,
                              "invalid payload SetDimLayerHbmForced, expected 0 or 1");
        } else {
            sForceDimLayerHBM = v;
            if (sForceDimLayerHBM) {
                setDimLayerHbmEnabled(true);
            }
            HalResponse response = {requestId, 0, 0, 0};
            sendResponsePacket(response, nullptr);
        }
    }
}

void handleDeviceWriteRequest(uint32_t requestId, const void *payload, uint32_t payloadSize) {
    const auto *requestBody = static_cast<const DeviceWriteRequest *>(payload);
    if (payloadSize < 4 || payload == nullptr || payloadSize != 4 + requestBody->length) {
        sendResponseError(requestId, HalRequestErrorCode::ERR_INVALID_ARGUMENT,
                          "invalid payload DeviceWriteRequest");
    } else {
        sendResponseError(requestId, HalRequestErrorCode::ERR_REMOTE_INTERNAL_ERROR,
                          "not implemented");
    }
}

void handleDeviceIoctl0Request(uint32_t requestId, const void *payload, uint32_t payloadSize) {
    if (payloadSize != sizeof(DeviceIoctl0Request) || payload == nullptr) {
        sendResponseError(requestId, HalRequestErrorCode::ERR_INVALID_ARGUMENT,
                          "invalid payload DeviceIoctl0Request");
    } else {
        sendResponseError(requestId, HalRequestErrorCode::ERR_REMOTE_INTERNAL_ERROR,
                          "not implemented");
    }
}

int handleRequestPacket(const HalRequest &request, const void *payload) {
    uint32_t requestId = request.id;
    auto requestCode = static_cast<RequestId>(request.requestCode);
    uint32_t payloadSize = request.payloadSize;
    switch (requestCode) {
        case RequestId::GET_VERSION: {
            handleGetVersionRequest(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::GET_HOOK_STATUS: {
            handleGetHookStatusRequest(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::INIT_PLT_HOOK: {
            handleInitPltHookRequest(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::GET_DEVICE_STATUS: {
            handleGetDeviceStatusRequest(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::DEVICE_OPERATION_WRITE: {
            handleDeviceWriteRequest(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::DEVICE_OPERATION_IOCTL0: {
            handleDeviceIoctl0Request(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::OP_DISP_GET_DIM_LAYER_HBM: {
            handleGetDimLayerHbmRequest(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::OP_DISP_SET_DIM_LAYER_HBM: {
            handleSetDimLayerHbmRequest(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::OP_DISP_GET_DIM_LAYER_HBM_FORCE_STATUS: {
            handleGetDimLayerHbmForcedRequest(requestId, payload, payloadSize);
            return 0;
        }
        case RequestId::OP_DISP_SET_DIM_LAYER_HBM_FORCE_STATUS: {
            handleSetDimLayerHbmForcedRequest(requestId, payload, payloadSize);
            return 0;
        }
        default:
            return -1;
    }
}
