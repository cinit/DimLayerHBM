//
// Created by kinit on 2021-10-15.
//

#ifndef APPCLIENT_NCIHOSTDAEMONPROXY_H
#define APPCLIENT_NCIHOSTDAEMONPROXY_H

#include "rpcprotocol/protocol/BaseIpcProxy.h"
#include "rpcprotocol/protocol/IpcTransactor.h"
#include "rpcprotocol/IDispTunerDaemon.h"

namespace ipcprotocol {

class DispTunerDaemonProxy : public IDispTunerDaemon, public BaseIpcProxy {
public:
    DispTunerDaemonProxy() = default;

    ~DispTunerDaemonProxy() override = default;

    [[nodiscard]] uint32_t getProxyId() const override;

    TypedLpcResult<std::string> getVersionName() override;

    TypedLpcResult<int> getVersionCode() override;

    TypedLpcResult<std::string> getBuildUuid() override;

    TypedLpcResult<int> getSelfPid() override;

    TypedLpcResult<void> exitProcess() override;

    TypedLpcResult<bool> isDeviceSupported() override;

    TypedLpcResult<bool> isHwServiceConnected() override;

    TypedLpcResult<bool> attachHwHidlService(const std::vector<std::string> &soPath) override;

    TypedLpcResult<HistoryIoOperationEventList> getHistoryIoOperations(uint32_t start, uint32_t length) override;

    TypedLpcResult<bool> clearHistoryIoEvents() override;

    TypedLpcResult<DaemonStatus> getDaemonStatus() override;

    TypedLpcResult<int> deviceDriverIoctl0(uint32_t request, uint64_t arg) override;

    TypedLpcResult<bool> isOpDimLayerHbmEnabled() override;

    TypedLpcResult<void> setOpDimLayerHbmEnabled(bool enabled) override;

    TypedLpcResult<bool> isOpDimLayerHbmForceEnabled() override;

    TypedLpcResult<void> setOpDimLayerHbmForceEnabled(bool enabled) override;

    TypedLpcResult<std::vector<LogEntryRecord>> getLogsPartial(uint32_t startIndex, uint32_t count) override;
};

}

#endif //APPCLIENT_NCIHOSTDAEMONPROXY_H
