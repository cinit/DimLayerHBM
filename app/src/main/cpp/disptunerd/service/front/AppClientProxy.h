//
// Created by kinit on 2021-11-25.
//

#ifndef DISP_TUNER_NATIVES_NCICLIENTPROXY_H
#define DISP_TUNER_NATIVES_NCICLIENTPROXY_H

#include "libbasehalpatch/ipc/daemon_ipc_struct.h"
#include "rpcprotocol/protocol/IpcTransactor.h"
#include "rpcprotocol/protocol/BaseIpcProxy.h"
#include "rpcprotocol/IAppClient.h"

namespace ipcprotocol {

class AppClientProxy : public IAppClient, public BaseIpcProxy {
public:
    AppClientProxy() = default;

    ~AppClientProxy() override = default;

    [[nodiscard]] uint32_t getProxyId() const override;

    AppClientProxy(const AppClientProxy &) = delete;

    AppClientProxy &operator=(const AppClientProxy &) = delete;

    void onIoEvent(const halpatch::IoSyscallEvent &event, const std::vector<uint8_t> &payload) override;

    void onRemoteDeath(int pid) override;
};

}

#endif //DISP_TUNER_NATIVES_NCICLIENTPROXY_H
