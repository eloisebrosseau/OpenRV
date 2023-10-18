//
// Copyright (C) 2023  Autodesk, Inc. All Rights Reserved. 
// 
// SPDX-License-Identifier: Apache-2.0 
//

#ifndef __NDI__NDIVideoDevice__h__
#define __NDI__NDIVideoDevice__h__

#include <TwkGLF/GLVideoDevice.h>
#include <TwkGLF/GL.h>
#include <TwkUtil/Timer.h>
#include <boost/thread.hpp>
#include <cstddef>
#include <string>

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

    typedef unsigned char             NDIVideoFrame;

    struct NDIDataFormat
    {
        TwkApp::VideoDevice::InternalDataFormat iformat;
        NDIlib_FourCC_video_type_e              ndiFormat;
        bool                                    rgb;
        const char*                             description;
    };

    struct NDIVideoFormat
    {
        int         width;
        int         height;
        float       pa;
        float       hz;
        int         frame_rate_N;
        int         frame_rate_D;
        const char* description;
    };

    struct NDIAudioFormat
    {
        double           hz;
        TwkAudio::Format prec;
        size_t           numChannels;
        TwkAudio::Layout layout;
        const char*      description;
    };

    typedef std::vector<NDIVideoFormat> NDIVideoFormatVector;
    typedef std::vector<NDIDataFormat>  NDIDataFormatVector;

    class NDIVideoDevice : public TwkGLF::GLBindableVideoDevice {
        public:
            typedef TwkUtil::Timer                          Timer;
            typedef TwkGLF::GLFence                         GLFence;
            typedef TwkGLF::GLFBO                           GLFBO;
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
                State           state;
                const GLFBO*    fbo;

            private:
                pthread_mutex_t mutex;
                pthread_mutex_t stateMutex;
            };

            typedef std::deque<PBOData*> PBOQueue;

            NDIVideoDevice(NDIModule* ndiModule, const std::string& name);
            virtual ~NDIVideoDevice() override;

            virtual size_t asyncMaxMappedBuffers() const override;
            virtual Time deviceLatency() const override;

            virtual size_t numVideoFormats() const override;
            virtual VideoFormat videoFormatAtIndex(size_t) const override;
            virtual void setVideoFormat(size_t) override;
            virtual size_t currentVideoFormat() const override;

            virtual size_t numAudioFormats() const override;
            virtual AudioFormat audioFormatAtIndex(size_t) const override;
            virtual void setAudioFormat(size_t) override;
            virtual size_t currentAudioFormat() const override;

            virtual size_t numDataFormats() const override;
            virtual DataFormat dataFormatAtIndex(size_t) const override;
            virtual void setDataFormat(size_t) override;
            virtual size_t currentDataFormat() const override;

            virtual size_t numSyncSources() const override;
            virtual SyncSource syncSourceAtIndex(size_t) const override;
            virtual size_t currentSyncSource() const override;

            virtual size_t numSyncModes() const override;
            virtual SyncMode syncModeAtIndex(size_t) const override;
            virtual void setSyncMode(size_t) override;
            virtual size_t currentSyncMode() const override;

            virtual bool isStereo() const override;
            virtual bool isDualStereo() const override;

            virtual bool readyForTransfer() const override;
            virtual void transfer(const TwkGLF::GLFBO*) const override;
            virtual void transfer2(const TwkGLF::GLFBO*, const TwkGLF::GLFBO*) const override;
            virtual void transferAudio(void* interleavedData, size_t n) const override;
            virtual bool willBlockOnTransfer() const override;

            virtual size_t width() const override { return m_frameWidth; }
            virtual size_t height() const override { return m_frameHeight; }
            virtual void open(const StringVector& args) override;
            virtual void close() override;
            virtual bool isOpen() const override;
            virtual void clearCaches() const override;
            virtual VideoFormat format() const override;
            virtual Timing timing() const override;

            virtual void unbind() const override;
            virtual void bind(const TwkGLF::GLVideoDevice*) const override;
            virtual void bind2(const TwkGLF::GLVideoDevice*, const TwkGLF::GLVideoDevice*) const override;
            virtual void audioFrameSizeSequence(AudioFrameSizeVector&) const override;

        private:
            void initialize();
            bool transferChannel(size_t i, const TwkGLF::GLFBO*) const;
            void transferChannelPBO(size_t i, const TwkGLF::GLFBO*, NDIVideoFrame*, NDIVideoFrame*) const;
            void transferChannelReadPixels(size_t i, const TwkGLF::GLFBO*, NDIVideoFrame*, NDIVideoFrame*) const;

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
            bool                                m_isInitialized;
            bool                                m_isPbos;
            size_t                              m_pboSize;
            size_t                              m_videoFrameBufferSize;
            bool                                m_isOpen;  
            bool                                m_isStereo;
            size_t                              m_frameWidth;
            size_t                              m_frameHeight;
            mutable size_t                      m_totalPlayoutFrames;
            size_t                              m_internalAudioFormat;
            size_t                              m_internalVideoFormat;
            size_t                              m_internalDataFormat;
            size_t                              m_internalSyncMode;
            unsigned long                       m_audioSamplesPerFrame;
            unsigned long                       m_audioChannelCount;
            TwkAudio::Format                    m_audioFormat;
            GLenum                              m_textureFormat;
            GLenum                              m_textureType;
            static bool                         m_isInfoFeedback;
    };

} // NDI

#endif  // __NDI__NDIVideoDevice__h__
