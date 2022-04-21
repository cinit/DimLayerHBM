//
// Created by kinit on 2021-10-15.
//

#include "DispTunerDaemonProxy.h"

using namespace ipcprotocol;

using Ids = IDispTunerDaemon::TransactionIds;

uint32_t DispTunerDaemonProxy::getProxyId() const {
    return PROXY_ID;
}

TypedLpcResult<std::string> DispTunerDaemonProxy::getVersionName() {
    return invokeRemoteProcedure<std::string>(Ids::getVersionName);
}

TypedLpcResult<int> DispTunerDaemonProxy::getVersionCode() {
    return invokeRemoteProcedure<int>(Ids::getVersionCode);
}

TypedLpcResult<std::string> DispTunerDaemonProxy::getBuildUuid() {
    return invokeRemoteProcedure<std::string>(Ids::getBuildUuid);
}

TypedLpcResult<int> DispTunerDaemonProxy::getSelfPid() {
    return invokeRemoteProcedure<int>(Ids::getSelfPid);
}

TypedLpcResult<void> DispTunerDaemonProxy::exitProcess() {
    return invokeRemoteProcedure<void>(Ids::exitProcess);
}

TypedLpcResult<bool> DispTunerDaemonProxy::isDeviceSupported() {
    return invokeRemoteProcedure<bool>(Ids::isDeviceSupported);
}

TypedLpcResult<bool> DispTunerDaemonProxy::isHwServiceConnected() {
    return invokeRemoteProcedure<bool>(Ids::isHwServiceConnected);
}

TypedLpcResult<bool> DispTunerDaemonProxy::attachHwHidlService(const std::vector<std::string> &soPath) {
    return invokeRemoteProcedure<bool, const std::vector<std::string> &>(Ids::attachHwHidlService, soPath);
}

TypedLpcResult<IDispTunerDaemon::HistoryIoOperationEventList>
DispTunerDaemonProxy::getHistoryIoOperations(uint32_t start, uint32_t length) {
    return invokeRemoteProcedure<IDispTunerDaemon::HistoryIoOperationEventList, const uint32_t &, const uint32_t &>(
            Ids::getHistoryIoOperations, start, length);
}

TypedLpcResult<bool> DispTunerDaemonProxy::clearHistoryIoEvents() {
    return invokeRemoteProcedure<bool>(Ids::clearHistoryIoEvents);
}

TypedLpcResult<IDispTunerDaemon::DaemonStatus> DispTunerDaemonProxy::getDaemonStatus() {
    return invokeRemoteProcedure<IDispTunerDaemon::DaemonStatus>(Ids::getDaemonStatus);
}

TypedLpcResult<int> DispTunerDaemonProxy::deviceDriverIoctl0(uint32_t request, uint64_t arg) {
    return invokeRemoteProcedure<int, const uint32_t &, const uint64_t &>(Ids::deviceDriverIoctl0, request, arg);
}


TypedLpcResult<std::vector<LogEntryRecord>> DispTunerDaemonProxy::getLogsPartial(uint32_t startIndex, uint32_t count) {
    return invokeRemoteProcedure<std::vector<LogEntryRecord>, const uint32_t &, const uint32_t &>(
            Ids::getLogsPartial, startIndex, count);
}

TypedLpcResult<bool> DispTunerDaemonProxy::isOpDimLayerHbmEnabled() {
    return invokeRemoteProcedure<bool>(Ids::isOpDimLayerHbmEnabled);
}

TypedLpcResult<void> DispTunerDaemonProxy::setOpDimLayerHbmEnabled(bool enabled) {
    return invokeRemoteProcedure<void, const bool &>(Ids::setOpDimLayerHbmEnabled, enabled);
}

TypedLpcResult<bool> DispTunerDaemonProxy::isOpDimLayerHbmForceEnabled() {
    return invokeRemoteProcedure<bool>(Ids::isOpDimLayerHbmForceEnabled);
}

TypedLpcResult<void> DispTunerDaemonProxy::setOpDimLayerHbmForceEnabled(bool enabled) {
    return invokeRemoteProcedure<void, const bool &>(Ids::setOpDimLayerHbmForceEnabled, enabled);
}
