//
// Copyright (C) 2023  Autodesk, Inc. All Rights Reserved. 
// 
// SPDX-License-Identifier: Apache-2.0 
//

#pragma once

#include <TwkGLF/GLVideoDevice.h>
#include <TwkGLF/GL.h>
#include <TwkUtil/Timer.h>
#include <boost/thread.hpp>

#ifdef PLATFORM_WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <pthread.h>
#include <windows.h>
#include <process.h>
#endif
#if defined(PLATFORM_LINUX)
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/glu.h>
#endif
#if defined(PLATFORM_DARWIN)
#include <TwkGLF/GL.h>
#endif

#include <TwkGLF/GLFBO.h>
#include <TwkGLF/GLFence.h>

#include <Processing.NDI.Lib.h>

#include <iostream>
#include <stl_ext/thread_group.h>
#include <deque>

namespace NDI {
class NDIModule;

typedef boost::mutex::scoped_lock ScopedLock;
typedef boost::mutex              Mutex;
typedef boost::condition_variable Condition;
typedef unsigned char             NDIVideoFrame;

struct NDIDataFormat
{
    const char*                     desc;
    TwkApp::VideoDevice::InternalDataFormat iformat;
    NDIlib_FourCC_video_type_e      ndiFormat;
    bool                            rgb;
};

struct NDIVideoFormat
{
    int             width;
    int             height;
    float           pa;
    float           hz;
    int             frame_rate_N;
    int             frame_rate_D;
    const char*     desc;
};

struct NDIAudioFormat
{
    double             hz;
    TwkAudio::Format   prec;
    size_t             numChannels;
    TwkAudio::Layout   layout;
    const char*        desc;
};

typedef std::vector<NDIVideoFormat> NDIVideoFormatVector;
typedef std::vector<NDIDataFormat>  NDIDataFormatVector;

class NDIVideoDevice : public TwkGLF::GLBindableVideoDevice {
    friend class PinnedMemoryAllocator;
    public:

        typedef TwkUtil::Timer                          Timer;
        typedef TwkGLF::GLFence                         GLFence;
        typedef TwkGLF::GLFBO                           GLFBO;
        typedef std::vector<unsigned char*>             BufferVector;
        typedef stl_ext::thread_group                   ThreadGroup;
        typedef std::vector<int>                        AudioBuffer;
        typedef std::deque<NDIVideoFrame*>              DLVideoFrameDeque;
        struct PBOData
        {
            enum State { Mapped, Transferring, NeedsUnmap, Ready };
            PBOData(GLuint g);
            ~PBOData();
            void lockData();
            void unlockData();
            void lockState();
            void unlockState();
            GLuint          globject;
            void*           mappedPointer;
            State           state;
            GLFence*        fence;
            const GLFBO*    fbo;
        private:
            pthread_mutex_t mutex;
            pthread_mutex_t stateMutex;
        };
        struct FrameData
        {
            FrameData(): audioData(NULL), videoFrame(NULL) {}
            ~FrameData() {}
            void*                   audioData;
            void*                   videoFrame;
        };
        typedef std::deque<PBOData*>                            PBOQueue;
        NDIVideoDevice(NDIModule* ndiModule, const std::string& name);
        virtual ~NDIVideoDevice();

        virtual size_t asyncMaxMappedBuffers() const;
        virtual Time deviceLatency() const;

        virtual size_t numVideoFormats() const;
        virtual VideoFormat videoFormatAtIndex(size_t) const;
        virtual void setVideoFormat(size_t);
        virtual size_t currentVideoFormat() const;

        virtual size_t numAudioFormats() const;
        virtual AudioFormat audioFormatAtIndex(size_t) const;
        virtual void setAudioFormat(size_t);
        virtual size_t currentAudioFormat() const;

        virtual size_t numDataFormats() const;
        virtual DataFormat dataFormatAtIndex(size_t) const;
        virtual void setDataFormat(size_t);
        virtual size_t currentDataFormat() const;

        virtual size_t numSyncSources() const;
        virtual SyncSource syncSourceAtIndex(size_t) const;
        virtual void setSyncSource(size_t);
        virtual size_t currentSyncSource() const;

        virtual size_t numSyncModes() const;
        virtual SyncMode syncModeAtIndex(size_t) const;
        virtual void setSyncMode(size_t);
        virtual size_t currentSyncMode() const;

        virtual bool isStereo() const;
        virtual bool isDualStereo() const;

        virtual bool readyForTransfer() const;
        virtual void transfer(const TwkGLF::GLFBO*) const;
        virtual void transfer2(const TwkGLF::GLFBO*, const TwkGLF::GLFBO*) const;
        virtual void transferAudio(void* interleavedData, size_t n) const;
        virtual bool willBlockOnTransfer() const;

        virtual size_t width() const { return m_frameWidth; }
        virtual size_t height() const { return m_frameHeight; }
        virtual void open(const StringVector& args);
        virtual void close();
        virtual bool isOpen() const;
        virtual void makeCurrent() const;
        virtual void clearCaches() const;
        virtual void syncBuffers() const;
        virtual VideoFormat format() const;
        virtual Timing timing() const;

        virtual void unbind() const;
        virtual void bind(const TwkGLF::GLVideoDevice*) const;
        virtual void bind2(const TwkGLF::GLVideoDevice*, const TwkGLF::GLVideoDevice*) const;
        virtual void audioFrameSizeSequence(AudioFrameSizeVector&) const;

    private:
        void initialize();
        bool transferChannel(size_t i, const TwkGLF::GLFBO*) const;
        void transferChannelPBO(size_t i, const TwkGLF::GLFBO*, NDIVideoFrame*, NDIVideoFrame*) const;
        void transferChannelReadPixels(size_t i, const TwkGLF::GLFBO*, NDIVideoFrame*, NDIVideoFrame*) const;

    private:

        NDIVideoFormatVector                m_ndiVideoFormats;
        NDIDataFormatVector                 m_ndiDataFormats;
        NDIlib_send_instance_t              m_ndiSender{nullptr};
        mutable NDIlib_video_frame_v2_t     m_ndiVideoFrame;
        mutable NDIVideoFrame*              m_readyFrame;
        mutable NDIVideoFrame*              m_readyStereoFrame;
        mutable DLVideoFrameDeque           m_DLOutputVideoFrameQueue;
        mutable DLVideoFrameDeque           m_DLReadbackVideoFrameQueue; // only rgb formats
        mutable bool                        m_needsFrameConverter;
        mutable bool                        m_hasAudio;
        mutable PBOQueue                    m_pboQueue;
        mutable PBOData*                    m_lastPboData;
        mutable PBOData*                    m_secondLastPboData;  // use of stereo formats; stores left eye.
        void*                               m_audioData[2];
        mutable int                         m_audioDataIndex;
        bool                                m_initialized;
        mutable bool                        m_bound;
        bool                                m_asyncSDISend; 
        bool                                m_pbos;
        size_t                              m_pboSize;
        size_t                              m_videoFrameBufferSize;
        bool                                m_open;  
        bool                                m_stereo;
        size_t                              m_frameWidth;
        size_t                              m_frameHeight;
        mutable size_t                      m_totalPlayoutFrames;
        mutable int                         m_transferTextureID;        
        int                                 m_internalAudioFormat;
        int                                 m_internalVideoFormat;
        int                                 m_internalDataFormat;
        int                                 m_internalSyncMode;
        unsigned long                       m_framesPerSecond;
        unsigned long                       m_audioBufferSampleLength;
        unsigned long                       m_audioSamplesPerFrame;
        mutable bool                        m_frameCompleted;
        unsigned long                       m_audioChannelCount;
        TwkAudio::Format                    m_audioFormat;
        GLenum                              m_textureFormat;
        GLenum                              m_textureType;
        static bool                         m_infoFeedback;
    };

} // NDI
