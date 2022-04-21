//
// Created by kinit on 2021-11-26.
//
#include <vector>

#include "hw/opdispfeature/OpDispFeatureHandler.h"

#include "HwServiceStatus.h"

using namespace hwhal;

std::vector<HwServiceStatus> HwServiceStatus::enumerateHwServices() {
    std::vector<HwServiceStatus> serviceList;
    const std::vector<HwServiceStatus(*)()> statusDetectors = {
            &OpDispFeatureHandler::getHwServiceStatus
    };
    for (auto func: statusDetectors) {
        HwServiceStatus status = func();
        if (status.valid) {
            serviceList.push_back(status);
        }
    }
    return serviceList;
}
