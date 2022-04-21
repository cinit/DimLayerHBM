//
// Created by kinit on 2021-11-25.
//

#include "AppClientProxy.h"

using namespace ipcprotocol;

using Ids = IAppClient::EventIds;

uint32_t AppClientProxy::getProxyId() const {
    return PROXY_ID;
}

void AppClientProxy::onIoEvent(const halpatch::IoSyscallEvent &event, const std::vector<uint8_t> &payload) {
    (void) onProxyEvent(false, Ids::IO_EVENT, event, payload);
}

void AppClientProxy::onRemoteDeath(int pid) {
    (void) onProxyEvent(true, Ids::REMOTE_DEATH, pid);
}
