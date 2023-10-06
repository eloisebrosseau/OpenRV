//
// Copyright (C) 2023  Autodesk, Inc. All Rights Reserved. 
// 
// SPDX-License-Identifier: Apache-2.0 
//

#pragma once

#include <TwkApp/VideoModule.h>

#include <string>

namespace NDI {

class NDIModule : public TwkApp::VideoModule
{
  public:
    NDIModule(NativeDisplayPtr);
    virtual ~NDIModule();

    virtual std::string name() const;
    virtual std::string SDKIdentifier() const;
    virtual std::string SDKInfo() const;
    virtual void open();
    virtual void close();
    virtual bool isOpen() const;
};

} // NDI
