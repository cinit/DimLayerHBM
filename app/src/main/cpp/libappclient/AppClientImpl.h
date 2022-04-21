//
// Created by kinit on 2021-11-22.
//

#ifndef DISP_TUNER_NATIVES_NCICLIENTIMPL_H
#define DISP_TUNER_NATIVES_NCICLIENTIMPL_H

#include "rpcprotocol/IAppClient.h"
#include "rpcprotocol/protocol/BaseIpcObject.h"

namespace ipcprotocol {

class AppClientImpl : public IAppClient, public BaseIpcObject {
public:
    AppClientImpl() = default;

    ~AppClientImpl() override = default;

    AppClientImpl(const AppClientImpl &) = delete;

    AppClientImpl &operator=(const AppClientImpl &) = delete;

    [[nodiscard]] uint32_t getProxyId() const override;

    bool dispatchLpcInvocation(const IpcTransactor::LpcEnv &env, LpcResult &result,
                               uint32_t funcId, const ArgList &args) override;

    bool dispatchEvent(const IpcTransactor::LpcEnv &env, uint32_t eventId, const ArgList &args) override;

    void onIoEvent(const halpatch::IoSyscallEvent &event, const std::vector<uint8_t> &payload) override;

    void onRemoteDeath(int pid) override;
};

}

void AppClientImpl_forwardRemoteIoEvent(const halpatch::IoSyscallEvent &event, const std::vector<uint8_t> &payload);

void AppClientImpl_forwardRemoteDeathEvent(int pid);

#endif //DISP_TUNER_NATIVES_NCICLIENTIMPL_H
