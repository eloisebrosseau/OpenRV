//
// Copyright (C) 2023  Autodesk, Inc. All Rights Reserved. 
// 
// SPDX-License-Identifier: Apache-2.0 
//

#include <NDI/NDIVideoDevice.h>
#include <NDI/NDIModule.h>

#include <TwkExc/Exception.h>
#include <TwkFB/FastMemcpy.h>
#include <TwkFB/FastConversion.h>
#include <TwkGLF/GLFBO.h>
#include <TwkUtil/EnvVar.h>
#include <TwkUtil/TwkRegEx.h>
#include <TwkUtil/sgcHop.h>
#include <TwkUtil/sgcHopTools.h>
#include <boost/program_options.hpp> // This has to come before the ByteSwap.h
#include <TwkUtil/ByteSwap.h>
#include <TwkGLF/GL.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <stl_ext/replace_alloc.h>
#include <TwkGLF/GL.h>

#define DEFAULT_RINGBUFFER_SIZE 5
namespace NDI {
using namespace TwkApp;
using namespace TwkGLF;
using namespace TwkUtil;
using namespace boost::program_options;

static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;

bool NDIVideoDevice::m_infoFeedback = false;

struct NDISyncMode
{
    const char* desc;
    size_t      value;
};

struct NDISyncSource
{
    const char* desc;
    size_t      value;
};

static NDIDataFormat dataFormats[] = {
    //!!!{"8 Bit YUV", VideoDevice::CbY0CrY1_8_422, NDIlib_FourCC_type_UYVY, false},
    {"8 Bit RGBA", VideoDevice::BGRA8, NDIlib_FourCC_type_BGRX, true},
    {nullptr, VideoDevice::BGRA8, NDIlib_FourCC_type_BGRX, true}
};

static NDIVideoFormat videoFormats[] = {
    {1920, 1080, 1.0, 24.00, 24000, 1000, "1080p 24Hz"},
    {0,    0,    1.0, 00.00, 0, 0, nullptr},
};
        
static NDIAudioFormat audioFormats[] = {
    {48000.0, TwkAudio::Int16Format, 2, TwkAudio::Stereo_2, "16 bit 48kHz Stereo"},
    {48000.0, TwkAudio::Int32Format, 2, TwkAudio::Stereo_2, "24 bit 48kHz Stereo"},
    {48000.0, TwkAudio::Int32Format, 8, TwkAudio::Surround_7_1, "24 bit 48kHz 7.1 Surround"},
    {48000.0, TwkAudio::Int32Format, 8, TwkAudio::SDDS_7_1, "24 bit 48kHz 7.1 Surround SDDS"},
    {48000.0, TwkAudio::Int32Format, 16, TwkAudio::Generic_16, "24 bit 48kHz 16 channel"}
};

static NDISyncMode syncModes[] = {
    {"Free Running", 0}
};

NDIVideoDevice::PBOData::PBOData(GLuint g): globject(g), state(PBOData::Ready) 
{    
    pthread_mutex_init(&mutex, nullptr);
    pthread_mutex_init(&stateMutex, nullptr);
}

NDIVideoDevice::PBOData::~PBOData() 
{ 
    pthread_mutex_destroy(&mutex); 
    pthread_mutex_destroy(&stateMutex);
}

void NDIVideoDevice::PBOData::lockData() 
{ 
    Timer timer;
    timer.start();
    pthread_mutex_lock(&mutex); 
    Time t = timer.elapsed();
    if (t > 0.001)
    {
        std::cout << "lockData for " << t << std::endl;
    }
}

void NDIVideoDevice::PBOData::unlockData()
{
    pthread_mutex_unlock(&mutex);
}

void NDIVideoDevice::PBOData::lockState() 
{
    Timer timer;
    timer.start();
    pthread_mutex_lock(&stateMutex); 
    Time t = timer.elapsed();
    if (t > 0.001)
    {
        std::cout << "lockState for " << t << std::endl;
    }
}

void NDIVideoDevice::PBOData::unlockState() 
{ 
    pthread_mutex_unlock(&stateMutex); 
}

// Returns pixel size in bytes from NDI Video Type
static int pixelSizeInBytesFromNDIVideoType(const NDIlib_FourCC_video_type_e ndiVideoType)
{
    switch (ndiVideoType)
    {
        case NDIlib_FourCC_type_UYVY:
            return 2;
        case NDIlib_FourCC_type_BGRX:
            return 4;
        case NDIlib_FourCC_type_UYVA:
            break;
        case NDIlib_FourCC_type_P216:
            break;
        case NDIlib_FourCC_type_PA16:
            break;
        case NDIlib_FourCC_type_YV12:
            break;
        case NDIlib_FourCC_type_I420:
            break;
        case NDIlib_FourCC_type_NV12:
            break;
        case NDIlib_FourCC_type_BGRA:
            break;
        case NDIlib_FourCC_type_RGBA:
            break;
        case NDIlib_FourCC_type_RGBX:
            break;
        case NDIlib_FourCC_video_type_max:
            break;
        default:
            std::cout << "ERROR: Unknown data format." << std::endl;
#if defined (_DEBUG)
            assert(0);
#endif
            break;
    }

    return 4;
}

//
//  NDIVideoDevice
//
NDIVideoDevice::NDIVideoDevice(NDIModule* m,
                               const std::string& name)
    : GLBindableVideoDevice(m, name, 
                            BlockingTransfer |
                            ImageOutput | 
                            ProvidesSync | 
                            FixedResolution | 
                            Clock | AudioOutput |
                            NormalizedCoordinates),
      m_readyFrame(nullptr),
      m_readyStereoFrame(nullptr),
      m_needsFrameConverter(false),
      m_hasAudio(false),
      m_initialized(false),
      m_pbos(true),
      m_pboSize(DEFAULT_RINGBUFFER_SIZE),
      m_videoFrameBufferSize(DEFAULT_RINGBUFFER_SIZE),
      m_open(false),
      m_stereo(false),
      m_totalPlayoutFrames(0),
      m_transferTextureID(-1),
      m_internalAudioFormat(0),
      m_internalVideoFormat(0),
      m_internalDataFormat(0),
      m_internalSyncMode(0),
      m_frameCompleted(true)
{
    m_audioFrameSizes.resize(5);
    
    m_audioData[0] = nullptr;
    m_audioData[1] = nullptr;
    
    //
    //  Add in all supported video formats 
    //
    //  NOTE: according to blackmagic sdk, if pixel format + video
    //  format combo are supported then so are the same combo in 3D
    //  mode (stereo)
    //
    for (const NDIVideoFormat* p = videoFormats; p->desc; p++) 
    {	
        bool videoFormatSupported = false;

        for (const NDIDataFormat* q = dataFormats; q->desc; q++) 
        {
            videoFormatSupported = true;
            m_ndiDataFormats.push_back(*q);
        }

        if (videoFormatSupported)
        {
            m_ndiVideoFormats.push_back(*p);
        }
    }
}

NDIVideoDevice::~NDIVideoDevice()
{
    if (m_open)
    {
        close();
    }
}

size_t NDIVideoDevice::asyncMaxMappedBuffers() const
{
    return m_pboSize;
}

VideoDevice::Time NDIVideoDevice::deviceLatency() const
{
    return Time(0);
}

size_t NDIVideoDevice::numDataFormats() const
{
    return m_ndiDataFormats.size();
}

NDIVideoDevice::DataFormat NDIVideoDevice::dataFormatAtIndex(size_t i) const
{
    return DataFormat(m_ndiDataFormats[i].iformat, m_ndiDataFormats[i].desc);
}

void NDIVideoDevice::setDataFormat(size_t i)
{
    const NDIDataFormat& f = m_ndiDataFormats[i];
    m_internalDataFormat = i;
}

size_t NDIVideoDevice::currentDataFormat() const
{
    return m_internalDataFormat;
}

size_t NDIVideoDevice::numAudioFormats() const
{
    return 5;
}

NDIVideoDevice::AudioFormat NDIVideoDevice::audioFormatAtIndex(size_t index) const
{
    const NDIAudioFormat& f = audioFormats[index];
    return AudioFormat(f.hz, f.prec, f.numChannels, f.layout, f.desc);
}

size_t NDIVideoDevice::currentAudioFormat() const
{
    return m_internalAudioFormat;
}

void NDIVideoDevice::setAudioFormat(size_t i)
{
    if (i > numAudioFormats())
    {
        i = numAudioFormats() - 1;
    }
    const NDIAudioFormat& f = audioFormats[i];
    m_internalAudioFormat = i;
    
    m_audioFrameSizes.clear();
}

size_t NDIVideoDevice::numVideoFormats() const
{
    return m_ndiVideoFormats.size();
}

NDIVideoDevice::VideoFormat NDIVideoDevice::videoFormatAtIndex(size_t index) const
{
    const NDIVideoFormat& f = m_ndiVideoFormats[index];
    return VideoFormat(static_cast<size_t>(f.width), static_cast<size_t>(f.height), f.pa, 1.0f, f.hz, f.desc);
}

size_t NDIVideoDevice::currentVideoFormat() const
{
    return m_internalVideoFormat;
}

void NDIVideoDevice::setVideoFormat(size_t i)
{
    const size_t n = numVideoFormats();
    if (i >= n)
    {
        i = n - 1;
    }
    const NDIVideoFormat& f = m_ndiVideoFormats[i];
    m_internalVideoFormat = i;

    //
    //  Update the data formats based on the video format
    //

    m_ndiDataFormats.clear();
    const NDIVideoFormat* p = &f;

    for (const NDIDataFormat* q = dataFormats; q->desc; q++) 
    {
        m_ndiDataFormats.push_back(*q);
    }

    m_audioFrameSizes.clear();
}

NDIVideoDevice::Timing NDIVideoDevice::timing() const
{
    const NDIVideoFormat& f = m_ndiVideoFormats[m_internalVideoFormat];
    return NDIVideoDevice::Timing(f.hz);
}

NDIVideoDevice::VideoFormat NDIVideoDevice::format() const
{
    const NDIVideoFormat& f = m_ndiVideoFormats[m_internalVideoFormat];
    return NDIVideoDevice::VideoFormat(static_cast<size_t>(f.width), static_cast<size_t>(f.height), f.pa, 1.0f, f.hz, f.desc);
}

size_t NDIVideoDevice::numSyncModes() const
{
    return 1;
}

NDIVideoDevice::SyncMode NDIVideoDevice::syncModeAtIndex(size_t i) const
{
    const NDISyncMode& m = syncModes[i];
    return SyncMode(m.desc);
}

void NDIVideoDevice::setSyncMode(size_t i)
{
    const NDISyncMode& m = syncModes[i];
    m_internalSyncMode = i;
}

size_t NDIVideoDevice::currentSyncMode() const
{
    return m_internalSyncMode;
}

size_t NDIVideoDevice::numSyncSources() const
{
    return 0;
}

NDIVideoDevice::SyncSource NDIVideoDevice::syncSourceAtIndex(size_t) const
{
    return SyncSource();
}

size_t NDIVideoDevice::currentSyncSource() const
{
    return 0;
}

void NDIVideoDevice::audioFrameSizeSequence(AudioFrameSizeVector& fsizes) const
{
    // same as AJA
    const NDIVideoFormat& f = m_ndiVideoFormats[m_internalVideoFormat];
    
    m_audioFrameSizes.resize(5);
    fsizes.resize(5);
    for (size_t i = 0; i < 5; i++)
    {
        m_audioFrameSizes[i] = m_audioSamplesPerFrame;
        fsizes[i] = m_audioFrameSizes[i];
    }
}

void NDIVideoDevice::initialize()
{
    if (m_initialized) return;

    m_initialized = true;
}

namespace {
    std::string mapToEnvVar(std::string name)
    {
        if (name == "RV_NDI_HELP")
        {
            return "help";
        }
        if (name == "RV_NDI_VERBOSE")
        {
            return "verbose";
        }
        if (name == "RV_NDI_METHOD")
        {
            return "method";
        }
        if (name == "RV_NDI_RING_BUFFER_SIZE")
        {
            return "ring-buffer-size";
        }
        return "";
    }
}

void NDIVideoDevice::open(const StringVector& args)
{   
    if (!m_initialized) initialize();

    options_description desc("NDI Device Options");

    desc.add_options()
        ("help,h", "Usage Message")
        ("verbose,v", "Verbose")
        ("method,m", value<std::string>(), "Method (ipbo, basic)")
        ("ring-buffer-size,s", value<int>()->default_value(DEFAULT_RINGBUFFER_SIZE), "Ring Buffer Size");
        
    variables_map vm;

    try
    {
        store(command_line_parser(args).options(desc).run(), vm);
        store(parse_environment(desc, mapToEnvVar), vm);
        notify(vm);
    }
    catch (std::exception& e)
    {
        std::cout << "ERROR: DNI_ARGS: " << e.what() << std::endl;
    }
    catch (...) 
    {
        std::cout << "ERROR: DNI_ARGS: exception" << std::endl;
    }

    if (vm.count("help") > 0)
    {
        std::cout << std::endl << desc << std::endl;
    }

    m_infoFeedback = vm.count("verbose") > 0;
    
    if (vm.count("ring-buffer-size"))
    {
        int rc = vm["ring-buffer-size"].as<int>();
        m_pboSize = rc;
        std::cout << "INFO: ringbuffer size " << rc << std::endl;
    }

    bool PBOsOK = true;
    if (vm.count("method"))
    {
        std::string s = vm["method"].as<std::string>();

        if (s == "ipbo")
        {
            PBOsOK = true; 
        }
        else if (s == "basic") 
        {
            PBOsOK = false; 
        }
    }

    m_pbos = PBOsOK;

    if (m_pbos)
    {
        std::cout << "INFO: using PBOs with immediate copy" << std::endl;
    }
    else
    {
        std::cout << "INFO: using basic readback" << std::endl;
    }

    m_audioDataIndex     = 0;
    m_frameCount         = 0;
    m_totalPlayoutFrames = 0;
    m_lastPboData        = nullptr;
    m_secondLastPboData  = nullptr;
    
    // dynamically determine what pixel formats are supported based on the 
    // desired video format
    m_ndiDataFormats.clear();

    for (const NDIDataFormat* q = dataFormats; q->desc; q++)
    {
        m_ndiDataFormats.push_back(*q);
    }

    const NDIVideoFormat& v = m_ndiVideoFormats[m_internalVideoFormat];
    m_frameWidth = static_cast<size_t>(v.width);
    m_frameHeight = static_cast<size_t>(v.height);

    const NDIDataFormat& d= m_ndiDataFormats[m_internalDataFormat];
    const std::string& dname = d.desc;

    const NDIAudioFormat& a = audioFormats[m_internalAudioFormat];
    m_audioChannelCount          = a.numChannels;
    m_audioFormat                = a.prec;

    GLenumPair epair    = TwkGLF::textureFormatFromDataFormat(d.iformat);
    m_textureFormat     = epair.first;
    m_textureType       = epair.second;

    m_stereo = dname.find("Stereo") != std::string::npos;
    m_videoFrameBufferSize = m_stereo ? m_pboSize * 2 : m_pboSize; 

	m_ndiVideoFrame.xres = v.width;
	m_ndiVideoFrame.yres = v.height;
	m_ndiVideoFrame.FourCC = d.ndiFormat;
    m_ndiVideoFrame.frame_rate_N = v.frame_rate_N;
    m_ndiVideoFrame.frame_rate_D = v.frame_rate_D;

    // enable video
    // TODO

    // Allocater audio buffers
    // TODO
    // m_audioSamplesPerFrame = (unsigned long)((m_audioSampleRate * m_frameDuration) / m_frameTimescale);
        
    // if (m_audioFormat == TwkAudio::Int16Format)
    // {
    //     m_audioData[0] = new short[m_audioSamplesPerFrame * m_audioChannelCount];
    //     m_audioData[1] = new short[m_audioSamplesPerFrame * m_audioChannelCount];

    // }
    // else // 32
    // {
    //     m_audioData[0] = new int[m_audioSamplesPerFrame * m_audioChannelCount];
    //     m_audioData[1] = new int[m_audioSamplesPerFrame * m_audioChannelCount];
    // }

    //
    // Create a queue of IDeckLinkMutableVideoFrame objects to use for
    // scheduling output video frames.
    //

    for (size_t i = 0; i < m_videoFrameBufferSize; i++)
    {
        NDIVideoFrame* outputFrame = nullptr;
        const NDIDataFormat& d = m_ndiDataFormats[m_internalDataFormat];
        size_t bps = 4 * m_frameWidth;
        if (d.iformat > VideoDevice::Y0CbY1Cr_8_422)
        {
            bps = 128 * ((m_frameWidth + 47) / 48);
        } else if (d.iformat == VideoDevice::Y0CbY1Cr_8_422)
        {
            bps = 2 * m_frameWidth;
        }
        // TODO: Allocate output frame
        outputFrame = new NDIVideoFrame[m_frameHeight*bps];
        memset(outputFrame, 128, m_frameHeight*bps);
        m_DLOutputVideoFrameQueue.push_back(outputFrame);
        
        if (!m_needsFrameConverter)
        {
            continue;
        }

        NDIVideoFrame* readbackFrame = nullptr;

        // TODO: Allocate readbackFrame for conversion
        m_DLReadbackVideoFrameQueue.push_back(readbackFrame);
    } 

    m_readyFrame = m_DLOutputVideoFrameQueue.at(0);

    // Create an NDI sender 
    m_ndiSender = NDIlib_send_create();
	if (!m_ndiSender)
    {
        std::cout << "ERROR: Could not create NDI sender." << std::endl;
    }

    m_open = true;
}

void NDIVideoDevice::close()
{
    if (m_open)
    {    
        m_open = false;
        
        unbind();

        int rc = pthread_mutex_unlock(&audioMutex);
        m_hasAudio = false;
        if (m_audioData[0]) 
        {
            if (m_audioFormat == TwkAudio::Int16Format)
            {
                delete static_cast<short*>(m_audioData[0]);
            }
            else
            {
                delete[] static_cast<int*>(m_audioData[0]);
            } 
            m_audioData[0] = nullptr;
        }
        if (m_audioData[1]) 
        {
            if (m_audioFormat == TwkAudio::Int16Format)
            {
                delete static_cast<short*>(m_audioData[1]);
            }
            else
            {
                delete[] static_cast<int*>(m_audioData[1]);
            }
            m_audioData[1] = nullptr;
        }
        rc = pthread_mutex_unlock(&audioMutex);

        for (size_t i = 0; i < m_DLOutputVideoFrameQueue.size(); i++)
        {
            NDIVideoFrame* ndiVideoFrame = static_cast<NDIVideoFrame*>(m_DLOutputVideoFrameQueue.at(i));
            delete[] ndiVideoFrame;
        }

        m_DLOutputVideoFrameQueue.clear();
    
        for (size_t i = 0; i < m_DLReadbackVideoFrameQueue.size(); i++)
        {
            NDIVideoFrame* ndiVideoFrame = static_cast<NDIVideoFrame*>(m_DLReadbackVideoFrameQueue.at(i));
            delete[] ndiVideoFrame;
        }
      
        m_DLReadbackVideoFrameQueue.clear();

        if (m_ndiSender)
        {
            NDIlib_send_destroy(m_ndiSender);
            m_ndiSender = nullptr;
        }
    }

    TwkGLF::GLBindableVideoDevice::close();
}

bool NDIVideoDevice::isStereo() const
{
    return m_stereo;
}

bool NDIVideoDevice::isOpen() const
{
    return m_open;
}

void NDIVideoDevice::makeCurrent() const { }
void NDIVideoDevice::clearCaches() const { }
void NDIVideoDevice::syncBuffers() const { }

bool NDIVideoDevice::isDualStereo() const
{
    return isStereo();
}

void NDIVideoDevice::transferAudio(void* data, size_t) const
{
    int rc = pthread_mutex_lock(&audioMutex);
    if (!data) 
    {
        m_hasAudio = false; 
        rc = pthread_mutex_unlock(&audioMutex);  
        return; 
    }
    m_hasAudio = true;
    rc = pthread_mutex_unlock(&audioMutex);  
    
    size_t b = 2;
    if (m_audioFormat == TwkAudio::Int32Format)
    {
        b = 4;
    }
    memcpy(m_audioData[m_audioDataIndex], data, b * m_audioSamplesPerFrame * m_audioChannelCount);
    m_audioDataIndex = (m_audioDataIndex + 1) % 2;
}

bool NDIVideoDevice::transferChannel(size_t n, const GLFBO* fbo) const
{
    HOP_PROF_FUNC();

    fbo->bind();

    NDIVideoFrame* readbackVideoFrame = nullptr;
    NDIVideoFrame* outputVideoFrame = m_DLOutputVideoFrameQueue.front();
    m_DLOutputVideoFrameQueue.push_back(outputVideoFrame);
    m_DLOutputVideoFrameQueue.pop_front();

    if (m_pbos)
    {
        transferChannelPBO(n, fbo, outputVideoFrame, readbackVideoFrame);
    }
    else
    {
        transferChannelReadPixels(n, fbo, outputVideoFrame, readbackVideoFrame);
    }

    if (n == 0 && !m_stereo)
    { 
        // non stereo, then n is always 0
        m_readyFrame = outputVideoFrame;
    }
    else if (n == 1) // in case of stereo, ready frame is updated when right eye is transfered
    {
        // TODO: Add stereo support
        m_readyStereoFrame = outputVideoFrame;
    }

    // Set the new data pointer and calculate line stride
    // Note that the NDI SDK is expecting a top-down image whereas OpenGL is 
    // giving a bottom up image so we need to point the data pointer to the
    // last line of the image and program a negative line stride.
    int pixelSizeInBytes = pixelSizeInBytesFromNDIVideoType(m_ndiVideoFrame.FourCC);
    int lineStrideInBytes = m_ndiVideoFrame.xres*pixelSizeInBytes;
    m_ndiVideoFrame.p_data = m_readyFrame + (m_ndiVideoFrame.yres-1) * lineStrideInBytes;
    m_ndiVideoFrame.line_stride_in_bytes = -1 * lineStrideInBytes;

    // Send the frame
    NDIlib_send_send_video_v2(m_ndiSender, &m_ndiVideoFrame);

    return true;
}

void NDIVideoDevice::transferChannelPBO(size_t n,
                                   const GLFBO* fbo,
                                   NDIVideoFrame* outputVideoFrame,
                                   NDIVideoFrame*) const
{
    HOP_CALL( glFinish(); )
    HOP_PROF_FUNC();
    
    PBOData* lastPboData = m_lastPboData; 
    if (m_stereo && n == 0)
    {
        lastPboData = m_secondLastPboData;
    }

    const NDIDataFormat& d = m_ndiDataFormats[m_internalDataFormat];

    // GL_UNSIGNED_INT_10_10_10_2 is a non native GPU format which leads to costly 
    // GPU readbacks. To work around this performance issue we will use the GPU native
    // GL_UNSIGNED_INT_2_10_10_10_REV format instead and perform a CPU format 
    // conversion after readback. This is much more optimal than letting the GPU
    // do the conversion during readback.
    // For reference: SG-14524
    GLenum textureTypeToUse = m_textureType;
    bool perform_ABGR10_to_RGBA10_conversion = false;
    if (m_textureType == GL_UNSIGNED_INT_10_10_10_2)
    {
        textureTypeToUse = GL_UNSIGNED_INT_2_10_10_10_REV;
        perform_ABGR10_to_RGBA10_conversion = true;
    } 

    if (lastPboData != nullptr)
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, lastPboData->globject); TWK_GLDEBUG;
        
        lastPboData->lockState();
        lastPboData->state = PBOData::Transferring; 
        lastPboData->unlockState(); 
        
        void* p = static_cast<void*>(glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB)); TWK_GLDEBUG;       
        if (p != nullptr) 
        {        
            void* pFrame = nullptr;
            pFrame = outputVideoFrame;

            if (d.iformat >= VideoDevice::CbY0CrY1_8_422) 
            {
                if (d.iformat == VideoDevice::CbY0CrY1_8_422) 
                {
                    subsample422_8bit_UYVY_MP(m_frameWidth, 
                                               m_frameHeight, 
                                               reinterpret_cast<uint8_t*>(p), 
                                               reinterpret_cast<uint8_t*>(pFrame));
                }
                else
                {
                    subsample422_10bit_MP(m_frameWidth, 
                                           m_frameHeight, 
                                           reinterpret_cast<uint32_t*>(p), 
                                           reinterpret_cast<uint32_t*>(pFrame),
                                           m_frameWidth * sizeof(uint32_t),
                                           //TODO: consider pitch: static_cast<size_t>(outputVideoFrame->GetRowBytes())
                                           m_frameWidth * sizeof(uint32_t));
                }
            }
            else 
            {
                if (perform_ABGR10_to_RGBA10_conversion)
                {
                    convert_ABGR10_to_RGBA10_MP(m_frameWidth, 
                                                 m_frameHeight, 
                                                 reinterpret_cast<uint32_t*>(p), 
                                                 reinterpret_cast<uint32_t*>(pFrame));
                }
                else
                {
                    FastMemcpy_MP(pFrame, p, m_frameHeight * m_frameWidth * 4); 
                }
            }
        }

        glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB); TWK_GLDEBUG;
        lastPboData->lockState();
        lastPboData->state = PBOData::Ready;  
        lastPboData->fbo->endExternalReadback();
        lastPboData->fbo = nullptr;
        lastPboData->unlockState();
    }
        
    {
        #if defined(HOP_ENABLED)
                std::string hopMsg = std::string("glReadPixels() next PBO");
                const size_t pixelSize = TwkGLF::pixelSizeFromTextureFormat(m_textureFormat, m_textureType);
                hopMsg+=std::string(" - width=") +
                std::to_string(m_frameWidth) +
                std::string(", height=") +
                std::to_string(m_frameHeight) +
                std::string(", pixelSize=") +
                std::to_string(pixelSize) +
                std::string(", textureFormat=") +
                std::to_string(m_textureFormat) +
                std::string(", textureType=") +
                std::to_string(m_textureType);
                HOP_PROF_DYN_NAME(hopMsg.c_str());          
        #endif   

        // next pbo read
        PBOData* pboData = m_pboQueue.front();
        if (!pboData) 
        {
            std::cerr << "ERROR: pboData is NULL!" << std::endl;
        }
        else 
        {
            m_pboQueue.push_back(pboData);
            m_pboQueue.pop_front();
        }   
        glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pboData->globject); TWK_GLDEBUG;
        pboData->lockState();
        pboData->fbo = fbo;
        bool unmapit = pboData->state == PBOData::Mapped;
        pboData->unlockState();

        if (unmapit)
        {
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB); TWK_GLDEBUG;
        }

        glReadPixels(0,
                    0,
                    static_cast<int>(m_frameWidth),
                    static_cast<int>(m_frameHeight),
                    m_textureFormat,
                    textureTypeToUse,
                    nullptr); TWK_GLDEBUG;

        if (m_stereo && n == 0)
        {
            m_secondLastPboData = pboData;
        }
        else
        {
            m_lastPboData = pboData;
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
        HOP_CALL( glFinish(); )  
    }
}

void NDIVideoDevice::transferChannelReadPixels(size_t,
                                               const GLFBO* fbo,
                                               NDIVideoFrame* outputVideoFrame,
                                               NDIVideoFrame* readbackVideoFrame) const
{
    HOP_PROF_FUNC();

    void* pFrame;
    if (m_needsFrameConverter)
    {
        // TODO: readbackVideoFrame->GetBytes(&pFrame);
        pFrame = readbackVideoFrame;
    }
    else 
    {
        // TODO: outputVideoFrame->GetBytes(&pFrame);
        pFrame = outputVideoFrame;
    }

    const NDIDataFormat& d  = m_ndiDataFormats[m_internalDataFormat];

    TwkUtil::Timer timer;
    timer.start();

    glReadPixels(0, 0, static_cast<int>(m_frameWidth), static_cast<int>(m_frameHeight), m_textureFormat, m_textureType, pFrame);

    timer.stop();

    fbo->endExternalReadback();

    if (m_needsFrameConverter)
    {
        TwkUtil::Timer converterTimer;
        converterTimer.start();
    
        void* outData;
        // TODO: outputVideoFrame->GetBytes(&outData);
        outData = outputVideoFrame;
        if (d.iformat == VideoDevice::CbY0CrY1_8_422)
        {
            subsample422_8bit_UYVY_MP( m_frameWidth, 
                                       m_frameHeight, 
                                       reinterpret_cast<uint8_t*>(pFrame), 
                                       reinterpret_cast<uint8_t*>(outData) );
        }
        else
        {
            subsample422_10bit_MP( m_frameWidth, 
                                   m_frameHeight, 
                                   reinterpret_cast<uint32_t*>(pFrame), 
                                   reinterpret_cast<uint32_t*>(outData),
                                   m_frameWidth * sizeof(uint32_t),
                                   // TODO: static_cast<size_t>(outputVideoFrame->GetRowBytes()) 
                                   m_frameWidth * sizeof(uint32_t)
                                   );
        }

        converterTimer.stop();
    }
}

void NDIVideoDevice::transfer(const GLFBO* fbo) const
{
    HOP_ZONE( HOP_ZONE_COLOR_4 );
    HOP_PROF_FUNC();

    if (!m_open)
    {
        return;
    }
    
    transferChannel(0, fbo);

    incrementClock();
}

bool NDIVideoDevice::willBlockOnTransfer() const
{
    return false;
}

bool NDIVideoDevice::readyForTransfer() const
{
    return true;
}

void NDIVideoDevice::transfer2(const GLFBO* fbo, const GLFBO* fbo2) const
{
    if (!m_open)
    {
        return;
    }

    if (m_pbos)
    {
        // Note transferChannelPBO() 'fbo' argument represents
        // the fbo to be read into the next pbo; hence we call transferChannel()
        // with the right eye fbo first.
        transferChannel(0, fbo2);
        transferChannel(1, fbo);
    }
    else
    {
        transferChannel(0, fbo);
        transferChannel(1, fbo2);
    }
    
    incrementClock();
}

void NDIVideoDevice::unbind() const
{
    if (m_pbos)
    {
        for (const auto& pbo : m_pboQueue) {
            PBOData* p = pbo;
            if (p->state == PBOData::NeedsUnmap)
            {
                glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, p->globject); TWK_GLDEBUG;
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB); TWK_GLDEBUG;
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            }
            glDeleteBuffers(1, &(p->globject)); TWK_GLDEBUG;
            delete pbo;
        }
        
        m_pboQueue.clear();
    }
}

void NDIVideoDevice::bind(const GLVideoDevice*) const
{
    if (m_pbos)
    {
        size_t num = m_stereo ? m_pboSize * 2 : m_pboSize;

        for (size_t q = 0; q < num; q++)
        {
            GLuint glObject;
            glGenBuffers(1, &glObject); TWK_GLDEBUG;
            glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, glObject); TWK_GLDEBUG;
            glBufferData(GL_PIXEL_PACK_BUFFER_ARB, static_cast<long>(m_frameWidth * 4 * m_frameHeight), nullptr, GL_STATIC_READ); TWK_GLDEBUG; // read back is always in 4 bytes
            glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0); TWK_GLDEBUG;
            
            m_pboQueue.push_back(new PBOData(glObject));
        }
    }

    resetClock();
}

void  NDIVideoDevice::bind2(const GLVideoDevice* d, const GLVideoDevice*) const
{
    bind(d);
}

} // NDI
