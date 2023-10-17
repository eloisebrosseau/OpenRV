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
        virtual ~NDIModule() override;

        virtual std::string name() const override;
        virtual std::string SDKIdentifier() const override;
        virtual std::string SDKInfo() const override;
        virtual void open() override;
        virtual void close() override;
        virtual bool isOpen() const override;
};

} // NDI

#endif  // __NDI__NDIModule__h__
