//
// Copyright (C) 2023  Autodesk, Inc. All Rights Reserved. 
// 
// SPDX-License-Identifier: Apache-2.0 
//

#ifndef __NDI__NDIModule__h__
#define __NDI__NDIModule__h__

#include "TwkApp/VideoModule.h"

#include <string>

namespace NDI {

class NDIModule : public TwkApp::VideoModule
{
    public:
        NDIModule();
        ~NDIModule() override;

        [[nodiscard]] std::string name() const override;
        [[nodiscard]] std::string SDKIdentifier() const override;
        [[nodiscard]] std::string SDKInfo() const override;
        void open() override;
        void close() override;
        [[nodiscard]] bool isOpen() const override;
};

} // NDI

#endif  // __NDI__NDIModule__h__
