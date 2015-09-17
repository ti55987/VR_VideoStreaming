#ifndef VIDEODECODERMAC_H
#define VIDEODECODERMAC_H

#include "videodecoder.h"

#include <QOpenGLFunctions_2_0>

class VideoDecoderMac : public VideoDecoder
{
public:
    VideoDecoderMac();
    virtual ~VideoDecoderMac();

    virtual bool init(AVStream *avStream);
    virtual bool decodeFrame(AVPacket *packet, bool selected = true);
    virtual bool uploadTextures(u_int64_t frameNumber);
    virtual GLuint* yuvTextureIds();
    virtual QSize frameSize();
    virtual u_int16_t stride();
    virtual u_int64_t lastDecodedFrame();
    virtual void reset();

private:
    void releaseBuffers();

    class Private;
    Private *p;
};

#endif // VIDEODECODERMAC_H
