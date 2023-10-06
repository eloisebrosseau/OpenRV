//
// Copyright (C) 2023  Autodesk, Inc. All Rights Reserved. 
// 
// SPDX-License-Identifier: Apache-2.0 
//

#include <NDI/NDIModule.h>
#include <NDI/NDIVideoDevice.h>

#include <TwkExc/Exception.h>

#include <Processing.NDI.Lib.h>

namespace NDI {

NDIModule::NDIModule(NativeDisplayPtr p) 
  : VideoModule()
{
    open();

    if (!isOpen()) 
    {
        TWK_THROW_EXC_STREAM("Cannot run NDI");
    }
}

NDIModule::~NDIModule()
{
    close();
}

std::string 
NDIModule::name() const
{
    return "NDI";
}

std::string 
NDIModule::SDKIdentifier() const
{
    std::ostringstream str;
    str << "NDI SDK Version " << NDIlib_version();
    return str.str();
}

std::string 
NDIModule::SDKInfo() const
{
    return "";
}

void 
NDIModule::open()
{
    if (isOpen()) return;

    if (!NDIlib_initialize())
    {   
        return;
    }

    NDIVideoDevice* device = new NDIVideoDevice(this, "NDI");
    if (device->numVideoFormats()!=0)
    {
        m_devices.push_back(device); 
    }
    else
    {
        delete device;
        device = nullptr;
    }
}

void 
NDIModule::close()
{
    for (size_t i = 0; i < m_devices.size(); i++)
    {
        delete m_devices[i];
    }
    m_devices.clear();

    NDIlib_destroy();
}

bool 
NDIModule::isOpen() const
{
    return !m_devices.empty();
}

} // NDI
