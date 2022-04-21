//
// Created by kinit on 2021-11-22.
//

#include "rpcprotocol/protocol/LpcArgListExtractor.h"
#include "rpcprotocol/log/Log.h"

#include "AppClientImpl.h"

using namespace ipcprotocol;
using namespace halpatch;

constexpr const char *LOG_TAG = "NciClientImpl";

uint32_t AppClientImpl::getProxyId() const {
    return PROXY_ID;
}

bool AppClientImpl::dispatchLpcInvocation(const IpcTransactor::LpcEnv &,
                                          LpcResult &, uint32_t, const ArgList &) {
    // we have no functions to dispatch
    return false;
}

bool AppClientImpl::dispatchEvent(const IpcTransactor::LpcEnv &env, uint32_t eventId, const ArgList &args) {
    using T = AppClientImpl;
    using R = LpcArgListExtractor<T>;
    using Ids = IAppClient::EventIds;
    switch (eventId) {
        case Ids::IO_EVENT : {
            R::invoke(this, args, R::is(
                    +[](T *p, const IoSyscallEvent &event, const std::vector<uint8_t> &payload) {
                        p->onIoEvent(event, payload);
                    }));
            return true;
        }
        case Ids::REMOTE_DEATH: {
            R::invoke(this, args, R::is(+[](T *p, int pid) { p->onRemoteDeath(pid); }));
            return true;
        }
        default:
            return false;
    }
}

void AppClientImpl::onIoEvent(const IoSyscallEvent &event, const std::vector<uint8_t> &payload) {
    AppClientImpl_forwardRemoteIoEvent(event, payload);
}

void AppClientImpl::onRemoteDeath(int pid) {
    AppClientImpl_forwardRemoteDeathEvent(pid);
}
