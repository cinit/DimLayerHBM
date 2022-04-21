//
// Created by kinit on 2021-10-24.
//

#ifndef DISP_TUNER_NATIVES_DAEMON_IPC_STRUCT_H
#define DISP_TUNER_NATIVES_DAEMON_IPC_STRUCT_H

#include <cstdint>
#include <cstddef>

#include "../hook/hook_sym_struct.h"

namespace halpatch {

constexpr int HAL_PACKET_MAX_LENGTH = 16384;

enum class HalRequestErrorCode : uint32_t {
    ERR_SUCCESS = 0,
    ERR_BAD_REQUEST = 0x12,
    ERR_UNKNOWN_REQUEST = 0x13,
    ERR_INVALID_ARGUMENT = 0x15,
    ERR_REMOTE_INTERNAL_ERROR = 0x21,
};

enum class TrxnType : uint32_t {
    GENERIC_EVENT = 2,
    IO_EVENT = 0x40,
    IO_REQUEST = 0x41,
    GENERIC_REQUEST = 0x50,
    GENERIC_RESPONSE = 0x51,
};

struct HalTrxnPacketHeader {
    uint32_t length; // including header
    TrxnType type;
};
static_assert(sizeof(HalTrxnPacketHeader) == 8);

struct HalRequest {
    uint32_t id;
    uint32_t requestCode;
    uint32_t argument;
    uint32_t payloadSize;
};
static_assert(sizeof(HalRequest) == 16);

struct HalResponse {
    uint32_t id;
    uint32_t result;
    uint32_t error;
    uint32_t payloadSize;
};
static_assert(sizeof(HalResponse) == 16);

enum class OpType : uint32_t {
    OP_TYPE_IO_OPEN = 1,
    OP_TYPE_IO_CLOSE = 2,
    OP_TYPE_IO_READ = 3,
    OP_TYPE_IO_WRITE = 4,
    OP_TYPE_IO_IOCTL = 5,
    OP_TYPE_IO_SELECT = 6,
};

class SourceType {
public:
    static constexpr uint32_t UNKNOWN = 0;
    static constexpr uint32_t NFC_CONTROLLER = 0x100;
    static constexpr uint32_t SECURE_ELEMENT_EMBEDDED = 0x200;
};

struct IoSyscallInfo {
    int32_t opType;
    int32_t fd;
    int64_t retValue;
    uint64_t directArg1;
    uint64_t directArg2;
    int64_t bufferLength;
};

struct IoSyscallEvent {
    uint32_t uniqueSequence;
    uint32_t sourceType;
    uint32_t sourceSequence;
    uint32_t rfu;
    uint64_t timestamp;
    IoSyscallInfo info;
};
static_assert(sizeof(IoSyscallInfo) == 40);
static_assert(sizeof(IoSyscallEvent) == sizeof(IoSyscallInfo) + 24);
static_assert(sizeof(OriginHookProcedure) == 56);

struct SharedObjectInfo {
    constexpr static uint32_t SO_INFO_MAGIC =
            uint32_t('N') | (uint32_t('S') << 8) | (uint32_t('O') << 16) | (uint32_t('I') << 24);
    uint32_t magic;
    uint32_t structSize;
    uint64_t handle;
    uint64_t initProcAddress;
};
static_assert(sizeof(SharedObjectInfo) == 24);

}

#endif //DISP_TUNER_NATIVES_DAEMON_IPC_STRUCT_H
