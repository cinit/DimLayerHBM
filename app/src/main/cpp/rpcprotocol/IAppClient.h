//
// Created by kinit on 2021-06-09.
//

#ifndef DISP_TUNER_IAPPCLIENT_H
#define DISP_TUNER_IAPPCLIENT_H

#include <string>

#include "libbasehalpatch/ipc/daemon_ipc_struct.h"
#include "protocol/ArgList.h"
#include "protocol/LpcResult.h"

namespace ipcprotocol {

class IAppClient {
public:
    static constexpr uint32_t PROXY_ID =
            uint32_t('D') | (uint32_t('T') << 8) | (uint32_t('C') << 16) | (uint32_t('0') << 24);

    IAppClient() = default;

    virtual ~IAppClient() = default;

    virtual void onIoEvent(const halpatch::IoSyscallEvent &event, const std::vector<uint8_t> &payload) = 0;

    virtual void onRemoteDeath(int pid) = 0;

    class EventIds {
    public:
        static constexpr uint32_t IO_EVENT = 0x10001;
        static constexpr uint32_t REMOTE_DEATH = 0x10002;
    };
};

}

#endif //DISP_TUNER_IAPPCLIENT_H
