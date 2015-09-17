#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QtGlobal>
#include <QtOpenGL/qgl.h>
#include "../../Decoder/LibavDecoder.h"
// To avoid UINT64 error in libav duh
#define __STDC_CONSTANT_MACROS
#ifdef HAVE_AV_CONFIG_H
#undef HAVE_AV_CONFIG_H
#endif

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#ifdef Q_OS_WIN
#include <stdint.h>
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;
#endif

//extern "C" {
//    #include <libavformat/avformat.h>
//    #include <libavcodec/avcodec.h>
//    #include <libavutil/avutil.h>
//    #include <libavutil/dict.h>
//}

class VideoDecoder
{
public:
    virtual ~VideoDecoder() { }
    virtual bool init(AVStream *stream) = 0;
    virtual bool decodeFrame(AVPacket *packet, bool selected = true) = 0;
    virtual bool uploadTextures(u_int64_t frameNumber) = 0;
    virtual void reset() { }
    virtual GLuint* yuvTextureIds() = 0;
    virtual QSize frameSize() = 0;
    virtual u_int16_t stride() = 0;
    virtual u_int64_t lastDecodedFrame() = 0;
};

#endif // VIDEODECODER_H
