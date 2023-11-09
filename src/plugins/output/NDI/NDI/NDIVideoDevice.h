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
#endif

#include <TwkGLF/GLFBO.h>
#include <TwkGLF/GLFence.h>

#include <Processing.NDI.Lib.h>

#include <iostream>
#include <stl_ext/thread_group.h>
#include <deque>

namespace NDI {
    class NDIModule;

    using NDIVideoFrame = unsigned char;

    struct NDIDataFormat
    {
        TwkApp::VideoDevice::InternalDataFormat iformat;
        NDIlib_FourCC_video_type_e              ndiFormat;
        bool                                    isRGB;
        const char*                             description;
    };

    struct NDIVideoFormat
    {
        int         width;
        int         height;
        float       pixelAspect;
        double      hertz;
        float       frame_rate_N;
        float       frame_rate_D;
        const char* description;
    };

    struct NDIAudioFormat
    {
        int              hertz;
        TwkAudio::Format precision;
        size_t           numChannels;
        TwkAudio::Layout layout;
        const char*      description;
    };

    using NDIVideoFormatVector = std::vector<NDIVideoFormat>;
    using NDIDataFormatVector = std::vector<NDIDataFormat>;

    class NDIVideoDevice : public TwkGLF::GLBindableVideoDevice {
        public:
            using Timer = TwkUtil::Timer;
            using GLFence = TwkGLF::GLFence;
            using GLFBO = TwkGLF::GLFBO;
            using DLVideoFrameDeque = std::deque<NDIVideoFrame *>;

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

            using PBOQueue = std::deque<PBOData *>;

            NDIVideoDevice(NDIModule* ndiModule, const std::string& name);
            ~NDIVideoDevice() override;

            size_t asyncMaxMappedBuffers() const override;
            Time deviceLatency() const override;

            size_t numVideoFormats() const override;
            VideoFormat videoFormatAtIndex(size_t index) const override;
            void setVideoFormat(size_t index) override;
            size_t currentVideoFormat() const override;

            size_t numAudioFormats() const override;
            AudioFormat audioFormatAtIndex(size_t index) const override;
            void setAudioFormat(size_t index) override;
            size_t currentAudioFormat() const override;

            size_t numDataFormats() const override;
            DataFormat dataFormatAtIndex(size_t index) const override;
            void setDataFormat(size_t index) override;
            size_t currentDataFormat() const override;

            size_t numSyncSources() const override;
            SyncSource syncSourceAtIndex(size_t index) const override;
            size_t currentSyncSource() const override;

            size_t numSyncModes() const override;
            SyncMode syncModeAtIndex(size_t index) const override;
            void setSyncMode(size_t index) override;
            size_t currentSyncMode() const override;

            bool readyForTransfer() const override;
            void transfer(const TwkGLF::GLFBO* glfbo) const override;
            void transfer2(const TwkGLF::GLFBO* glfbo1, const TwkGLF::GLFBO* glfbo2) const override;
            void transferAudio(void* interleavedData, size_t n) const override;
            bool willBlockOnTransfer() const override;

            size_t width() const override { return m_frameWidth; }
            size_t height() const override { return m_frameHeight; }
            void open(const StringVector& args) override;
            void close() override;
            bool isOpen() const override;
            void clearCaches() const override;
            VideoFormat format() const override;
            Timing timing() const override;

            void unbind() const override;
            void bind(const TwkGLF::GLVideoDevice* device) const override;
            void bind2(const TwkGLF::GLVideoDevice* device1, const TwkGLF::GLVideoDevice* device2) const override;
            void audioFrameSizeSequence(AudioFrameSizeVector& fsizes) const override;

        private:
            void initialize();
            bool transferChannel(size_t index, const TwkGLF::GLFBO*) const;
            void transferChannelPBO(size_t index, const TwkGLF::GLFBO*, NDIVideoFrame*, NDIVideoFrame*) const;
            void transferChannelReadPixels(size_t index, const TwkGLF::GLFBO*, NDIVideoFrame*, NDIVideoFrame*) const;

            NDIVideoFormatVector                             m_ndiVideoFormats;
            NDIDataFormatVector                              m_ndiDataFormats;
            NDIlib_send_instance_t                           m_ndiSender{nullptr};
            mutable NDIlib_video_frame_v2_t                  m_ndiVideoFrame;
            mutable NDIlib_audio_frame_v2_t                  m_ndiAudioFrame;
            mutable NDIlib_audio_frame_interleaved_16s_t     m_ndiInterleaved16AudioFrame;
            mutable NDIlib_audio_frame_interleaved_32s_t     m_ndiInterleaved32AudioFrame;
            mutable NDIlib_audio_frame_interleaved_32f_t     m_ndiInterleaved32fAudioFrame;
            mutable NDIVideoFrame*                           m_readyFrame;
            mutable DLVideoFrameDeque                        m_DLOutputVideoFrameQueue;
            mutable DLVideoFrameDeque                        m_DLReadbackVideoFrameQueue; // only rgb formats
            mutable bool                                     m_needsFrameConverter;
            mutable bool                                     m_hasAudio;
            mutable PBOQueue                                 m_pboQueue;
            mutable PBOData*                                 m_lastPboData;
            void*                                            m_audioData[2];
            mutable int                                      m_audioDataIndex;
            bool                                             m_isInitialized;
            bool                                             m_isPbos;
            size_t                                           m_pboSize;
            size_t                                           m_videoFrameBufferSize;
            bool                                             m_isOpen;  
            size_t                                           m_frameWidth;
            size_t                                           m_frameHeight;
            mutable size_t                                   m_totalPlayoutFrames;
            size_t                                           m_internalAudioFormat;
            size_t                                           m_internalVideoFormat;
            size_t                                           m_internalDataFormat;
            size_t                                           m_internalSyncMode;
            double                                           m_audioSamplesPerFrame;
            float                                            m_audioChannelCount;
            float                                            m_audioSampleRate;
            TwkAudio::Format                                 m_audioFormat;
            GLenum                                           m_textureFormat;
            GLenum                                           m_textureType;
            static bool                                      m_isInfoFeedback;
    };

} // NDI

#endif  // __NDI__NDIVideoDevice__h__
