#ifndef VIDEODECODERLIBAV_H
#define VIDEODECODERLIBAV_H

#include "videodecoder.h"

#include <QList>
#include <QMutexLocker>
#include <QElapsedTimer>
#include <QOpenGLFunctions_2_0>

class VideoDecoderLibAV : public VideoDecoder, protected QOpenGLFunctions_2_0
{
public:
    VideoDecoderLibAV();
    ~VideoDecoderLibAV();

    virtual bool init(AVStream *stream);
    virtual bool decodeFrame(AVPacket *packet, bool selected = true);
    virtual bool uploadTextures(u_int64_t frameNumber);
    virtual void reset();
    virtual GLuint* yuvTextureIds() { return m_yuv; }
    virtual QSize frameSize() { QMutexLocker lock(&m_bufferMutex); return m_frameSize; }
    virtual u_int16_t stride() { QMutexLocker lock(&m_bufferMutex); return m_linesize[0]; }
    virtual u_int64_t lastDecodedFrame() { QMutexLocker lock(&m_bufferMutex); return m_lastDecodedFrame; }

private:
    struct PBOPicture {
        GLuint pboId;
        GLubyte *pboDataPtr;
        AVPicture *picture;
        u_int64_t frameNumber;
    };

    void initPBOs();

    AVCodec *m_avCodec;
    AVCodecContext *m_avCodecContext;
    QList<PBOPicture> m_pboPool;
    QList<PBOPicture> m_pictures;
    QSize m_frameSize;
    size_t m_pboSize;
    int m_linesize[3];
    AVFrame *m_firstFrame;
    QMutex m_bufferMutex;
    GLuint m_yuv[6];

    QString m_log;
    QElapsedTimer m_decodeTimer;

    u_int64_t m_lastDecodedFrame;
    u_int64_t m_lastUploadedFrame;
    double m_timeBaseMultiplier;
};

#endif // VIDEODECODERLIBAV_H
