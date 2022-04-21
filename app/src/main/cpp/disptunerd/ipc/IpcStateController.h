//
// Created by kinit on 2021-10-12.
//

#ifndef COM_DAEMON_IPCSTATECONTROLLER_H
#define COM_DAEMON_IPCSTATECONTROLLER_H

#include "rpcprotocol/protocol/IpcTransactor.h"
#include "rpcprotocol/protocol/LpcArgListExtractor.h"
#include "../service/front/DispTunerDaemonImpl.h"
#include "../service/front/AppClientProxy.h"

namespace ipcprotocol {

class IpcStateController {
public:
    void attachIpcSeqPacketSocket(int fd);

    [[nodiscard]] static IpcStateController &getInstance();

    [[nodiscard]] DispTunerDaemonImpl &getNciHostDaemon();

    [[nodiscard]] AppClientProxy &getNciClientProxy();

    [[nodiscard]] IpcTransactor &getIpcProxy();

private:
    IpcTransactor mIpcTransactor;
    DispTunerDaemonImpl mDaemon;
    AppClientProxy mClientProxy;

    void init();
};

}

#endif //COM_DAEMON_IPCSTATECONTROLLER_H
