//
// Created by kinit on 2021-12-10.
//
#include "protocol/ArgList.h"
#include "utils/SharedBuffer.h"

#include "IDispTunerDaemon.h"

using namespace std;
using namespace ipcprotocol;

bool IDispTunerDaemon::HistoryIoOperationEventList::deserializeFromByteVector(const vector<uint8_t> &src) {
    if (src.empty()) {
        return false;
    }
    ArgList args(src.data(), src.size());
    size_t count = 0;
    if (!(args.get(&totalStartIndex, 0) && args.get(&totalCount, 1)
          && args.get(&count, 2))) {
        return false;
    }
    // pull out the events
    events.resize(count);
    for (size_t i = 0; i < count; i++) {
        if (!args.get(&events[i], int(i + 3))) {
            return false;
        }
    }
    // pull out the event payloads
    payloads.resize(count);
    for (size_t i = 0; i < count; i++) {
        if (!args.get(&payloads[i], int(i + count + 3))) {
            return false;
        }
    }
    return true;
}

std::vector<uint8_t> IDispTunerDaemon::HistoryIoOperationEventList::serializeToByteVector() const {
    ArgList::Builder builder;
    builder.push(totalStartIndex);
    builder.push(totalCount);
    builder.push(events.size());
    for (const auto &event: events) {
        builder.push(event);
    }
    for (const auto &payload: payloads) {
        builder.push(payload);
    }
    return builder.build().toVector();
}

bool IDispTunerDaemon::DaemonStatus::deserializeFromByteVector(const vector<uint8_t> &src) {
    if (src.empty()) {
        return false;
    }
    ArgList args(src.data(), src.size());
    return args.get(&processId, 0) && args.get(&versionName, 1)
           && args.get(&abiArch, 2) && args.get(&daemonProcessSecurityContext, 3)
           && args.get(&opDispFeatureHidlServiceStatus.isHalServiceAttached, 4)
           && args.get(&opDispFeatureHidlServiceStatus.halServicePid, 5)
           && args.get(&opDispFeatureHidlServiceStatus.halServiceUid, 6)
           && args.get(&opDispFeatureHidlServiceStatus.halServiceExePath, 7)
           && args.get(&opDispFeatureHidlServiceStatus.halServiceArch, 8)
           && args.get(&opDispFeatureHidlServiceStatus.halServiceProcessSecurityContext, 9)
           && args.get(&opDispFeatureHidlServiceStatus.halServiceExecutableSecurityLabel, 10);
}

std::vector<uint8_t> IDispTunerDaemon::DaemonStatus::serializeToByteVector() const {
    ArgList::Builder builder;
    builder.push(processId);
    builder.push(versionName);
    builder.push(abiArch);
    builder.push(daemonProcessSecurityContext);
    builder.push(opDispFeatureHidlServiceStatus.isHalServiceAttached);
    builder.push(opDispFeatureHidlServiceStatus.halServicePid);
    builder.push(opDispFeatureHidlServiceStatus.halServiceUid);
    builder.push(opDispFeatureHidlServiceStatus.halServiceExePath);
    builder.push(opDispFeatureHidlServiceStatus.halServiceArch);
    builder.push(opDispFeatureHidlServiceStatus.halServiceProcessSecurityContext);
    builder.push(opDispFeatureHidlServiceStatus.halServiceExecutableSecurityLabel);
    return builder.build().toVector();
}
