// SPDX-License-Identifier: MIT
//
// Created by kinit on 2021-10-30.
//

#ifndef DISP_TUNER_NATIVES_DEFAULTSESSIONLOGIMPL_H
#define DISP_TUNER_NATIVES_DEFAULTSESSIONLOGIMPL_H

#include <string>

#include "SessionLog.h"

class DefaultSessionLogImpl : public SessionLog {
private:
    std::string mLogTag;

public:
    DefaultSessionLogImpl() = default;

    explicit DefaultSessionLogImpl(const std::string_view &tag);

    ~DefaultSessionLogImpl() override = default;

    void setLogTag(const std::string_view &tag);

    void error(std::string_view msg) override;

    void warn(std::string_view msg) override;

    void info(std::string_view msg) override;

    void verbose(std::string_view msg) override;
};

#endif //DISP_TUNER_NATIVES_DEFAULTSESSIONLOGIMPL_H
