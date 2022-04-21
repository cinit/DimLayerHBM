// SPDX-License-Identifier: MIT
//
// Created by kinit on 2021-10-30.
//

#ifndef DISP_TUNER_NATIVES_SESSIONLOG_H
#define DISP_TUNER_NATIVES_SESSIONLOG_H

#include <string_view>

class SessionLog {
public:
    SessionLog() = default;

    virtual ~SessionLog() = default;

    virtual void error(std::string_view msg) = 0;

    virtual void warn(std::string_view msg) = 0;

    virtual void info(std::string_view msg) = 0;

    virtual void verbose(std::string_view msg) = 0;
};

#endif //DISP_TUNER_NATIVES_SESSIONLOG_H
