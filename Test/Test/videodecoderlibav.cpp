#include "videodecoderlibav.h"

#include <QDebug>

VideoDecoderLibAV::VideoDecoderLibAV()
    : m_avCodec(0)
    , m_avCodecContext(0)
    , m_frameSize(QSize())
    , m_pboSize(0)
    , m_firstFrame(0)
    , m_lastDecodedFrame(0)
    , m_lastUploadedFrame(0)
    , m_timeBaseMultiplier(0)
{
    m_yuv[0] = -1;
    m_linesize[0] = 0;
}

VideoDecoderLibAV::~VideoDecoderLibAV()
{
    avcodec_close(m_avCodecContext);
}

bool VideoDecoderLibAV::init(AVStream *stream)
{
    qDebug() << "Initializing LibAV video decoder for stream " << stream->index;
    m_pboSize = 0;
    m_avCodec = avcodec_find_decoder(CODEC_ID_H264);
    m_avCodecContext = avcodec_alloc_context3(m_avCodec);
    m_avCodecContext->extradata = stream->codec->extradata;
    m_avCodecContext->extradata_size = stream->codec->extradata_size;
    m_avCodecContext->thread_count = 3; // What's good?
    m_avCodecContext->thread_type = FF_THREAD_FRAME;

    if (avcodec_open2(m_avCodecContext, m_avCodec, NULL) < 0) {
        qWarning() << "could not open H264 codec!";
        return false;
    }

    m_timeBaseMultiplier = (stream->time_base.den * stream->r_frame_rate.den) /
                           (stream->time_base.num * stream->r_frame_rate.num);

    if (m_timeBaseMultiplier <= 0) {
        qWarning() << "Invalid time base!";
        return false;
    }

    return true;
}

void VideoDecoderLibAV::reset()
{
    m_bufferMutex.lock();
    m_lastDecodedFrame = 0;
    m_lastUploadedFrame = 0;
    m_pboSize = 0;
    for (int i = 0; i < m_pictures.size(); ++i) {
        PBOPicture tmp = m_pictures.takeAt(i);
        free(tmp.picture);
        m_pboPool.append(tmp);
        --i;
    }
    avcodec_flush_buffers(m_avCodecContext);
    m_bufferMutex.unlock();
}

// Returns false if the parser should next time give the same frame again
bool VideoDecoderLibAV::decodeFrame(AVPacket *packet, bool selected)
{
    int got = 0;

    m_bufferMutex.lock();

    QString type("");

    u_int64_t p = 0;
    int fragment_type = 0;
    int nal_type = 0;
    for(int i = 0; i < 10; ++i) {
        // Big endian to little endian
        u_int32_t *d32 = (u_int32_t*) &packet->data[p];
        u_int32_t num = *d32;
        u_int32_t b0 = (num & 0x000000ff) << 24u;
        u_int32_t b1 = (num & 0x0000ff00) << 8u;
        u_int32_t b2 = (num & 0x00ff0000) >> 8u;
        u_int32_t b3 = (num & 0xff000000) >> 24u;
        u_int32_t size = b0 | b1 | b2 | b3;

        p += 4;
        fragment_type = packet->data[p] & 0x1F;
        nal_type = packet->data[p + 1] & 0x1F;
        //qDebug() << "nalu size" << size << "type" << (packet->data[p] & 0x1F) << ":" << (packet->data[p+1] & 0x1F);
        if ((fragment_type == 5 && nal_type == 8) || (fragment_type == 6 && nal_type == 5)) {
            type = "I";
            break;
        } else if (nal_type == 26 || nal_type == 27) {
            type = "P";
            break;
        } else if (nal_type == 30 || nal_type == 31) {
            type = "B";
            break;
        }
        p += size;
        if (p > packet->size)
            break;
    }

    if (type.isEmpty()) {
        qWarning() << "WARNING: Unknown packet received! frag: " << fragment_type << ", nal:" << nal_type;
        Q_ASSERT(false);
        return true;
    }

    //qDebug() << "type:" << type;

    if (!m_pboSize) {
        // Decode first frame to know the frame size and stride
        m_firstFrame = avcodec_alloc_frame();
        avcodec_decode_video2(m_avCodecContext, m_firstFrame, &got, packet);

        if (!got) {
            av_free(m_firstFrame);
            m_firstFrame = 0;
            m_bufferMutex.unlock();
            return true;
        }

        qDebug() << "First frame size: " << m_firstFrame->width << ":" << m_firstFrame->height;
        qDebug() << "linesize: " << m_firstFrame->linesize[0] << "," << m_firstFrame->linesize[1] << "," << m_firstFrame->linesize[2];

        m_linesize[0] = m_firstFrame->linesize[0];
        m_linesize[1] = m_firstFrame->linesize[1];
        m_linesize[2] = m_firstFrame->linesize[2];
        m_frameSize = QSize(m_firstFrame->width, m_firstFrame->height);

        qreal u = qreal(m_linesize[1]) / qreal(m_linesize[0]);
        qreal v = qreal(m_linesize[2]) / qreal(m_linesize[0]);
        m_pboSize = m_linesize[0] * m_firstFrame->height +
                      m_linesize[1] * m_firstFrame->height * u +
                      m_linesize[2] * m_firstFrame->height * v;

        qDebug() << "PBO size is: " << m_pboSize;

        m_bufferMutex.unlock();

        return true;
    }

    if (m_pboPool.isEmpty()) {
        // Not ready to decode yet
        m_bufferMutex.unlock();
        return false;
    }

    if (!selected && type == QStringLiteral("B")) {
        m_bufferMutex.unlock();
        return true;
    }

    PBOPicture pboPicture = m_pboPool.takeFirst();
    m_bufferMutex.unlock();

    AVFrame *avFrame = avcodec_alloc_frame();

    m_decodeTimer.restart();
    avcodec_decode_video2(m_avCodecContext, avFrame, &got, packet);
/*
    m_log.append(QString::number(avFrame->pkt_dts));
    m_log.append(":");
    m_log.append(QString::number(packet->flags));
    m_log.append(QStringLiteral(":"));
    m_log.append(QString::number(avFrame->pict_type) + ":");
    m_log.append(QString::number(m_decodeTimer.nsecsElapsed() * 0.000001));
    m_log.append("\n");

    //qDebug() << avFrame->pkt_dts;

    if (!(avFrame->pkt_dts % (40 * 512))) {
        qDebug() << "LOG: " << this;
        qDebug() << m_log;
        m_log.clear();
    }
*/
    //qDebug() << "type: " << type << "dts:" << packet->dts << "pts:" << packet->pts;
    if (!got) {
        qWarning("Not able to decompress a frame!");
        return true;
    }

    AVPicture *copy = (AVPicture*)malloc(sizeof(AVPicture));
    qreal u = qreal(avFrame->linesize[1]) / qreal(avFrame->linesize[0]);
    qreal v = qreal(avFrame->linesize[2]) / qreal(avFrame->linesize[0]);
    copy->linesize[0] = avFrame->linesize[0];
    copy->linesize[1] = avFrame->linesize[1];
    copy->linesize[2] = avFrame->linesize[2];

    if (pboPicture.pboDataPtr) {
        memcpy(pboPicture.pboDataPtr, avFrame->data[0], avFrame->linesize[0] * avFrame->height);

        memcpy(pboPicture.pboDataPtr + avFrame->linesize[0] * avFrame->height,
                avFrame->data[1], avFrame->linesize[1] * avFrame->height * u);

        memcpy(pboPicture.pboDataPtr + avFrame->linesize[0] * avFrame->height + int(avFrame->linesize[1] * avFrame->height * u),
                avFrame->data[2], avFrame->linesize[2] * avFrame->height * v);
    }

    pboPicture.picture = copy;
    pboPicture.frameNumber = avFrame->pkt_pts / m_timeBaseMultiplier;

    m_bufferMutex.lock();
    m_pictures.append(pboPicture);
    //qDebug() << "decoded " << avFrame->pkt_pts / m_timeBaseMultiplier << "pics : " << m_pictures.size();
    m_lastDecodedFrame = avFrame->pkt_pts / m_timeBaseMultiplier; // !!!
    //qDebug() << "Last decoded frame" << m_lastDecodedFrame << type;
    //qDebug() << "Gave dts: " << packet->dts/m_timeBaseMultiplier << ", pts: " << packet->pts/m_timeBaseMultiplier << ", type: " << type << ", got: " << avFrame->pkt_pts / m_timeBaseMultiplier;
    m_bufferMutex.unlock();

    av_free(avFrame);

    return true;
}

void VideoDecoderLibAV::initPBOs()
{
    Q_ASSERT(QOpenGLContext::currentContext());
    initializeOpenGLFunctions();

    qDebug() << "Allocating 30 PBOs, size: " << m_pboSize;
    for (int i = 0; i < 30; ++i) {
        PBOPicture pboPicture;
        glGenBuffers(1, &pboPicture.pboId);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboPicture.pboId);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, m_pboSize, 0, GL_STREAM_DRAW);
        pboPicture.pboDataPtr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        m_pboPool.append(pboPicture);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

bool VideoDecoderLibAV::uploadTextures(u_int64_t frameNumber)
{
    if (m_lastUploadedFrame == frameNumber)
        return true;

    // Here we can issue gl commands

    m_bufferMutex.lock();
    if (m_pboPool.isEmpty() && m_pictures.isEmpty()) {
        if (m_pboSize) {
            initPBOs();
        }
        m_bufferMutex.unlock();
        return false;
    }

    if (m_pictures.isEmpty()) {
        //qWarning() << "No decoded frames! " << this;
        m_bufferMutex.unlock();
        return false;
    }

    PBOPicture pboPicture;
    bool found = false;
    for (int i = 0; i < m_pictures.size(); ++i) {
        //qDebug() << "exist: " << m_pictures.at(i).frameNumber << ", seeking: " << frameNumber;
        if (m_pictures.at(i).frameNumber < frameNumber) {
            PBOPicture tmp = m_pictures.takeAt(i);
            free(tmp.picture);
            m_pboPool.append(tmp);
            --i;
        } else if (m_pictures.at(i).frameNumber == frameNumber) {
            pboPicture = m_pictures.takeAt(i);
            found = true;
            break;
        }
    }

    const QSize size = m_frameSize;
    m_bufferMutex.unlock();

    if (!found) {
        //qDebug() << "!found" << frameNumber;
        return false; // We should distuingish this from the situation where pictures is empty
    }

    glBindBuffer (GL_PIXEL_UNPACK_BUFFER, pboPicture.pboId);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

    AVPicture *picture = pboPicture.picture;
    qreal u = qreal(picture->linesize[1]) / qreal(picture->linesize[0]);
    qreal v = qreal(picture->linesize[2]) / qreal(picture->linesize[0]);

    if (m_yuv[0]==GLuint(-1)) {
        qDebug() << "Creating yuv textures";
        glGenTextures(3, &m_yuv[0]);
        glBindTexture(GL_TEXTURE_2D, m_yuv[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, picture->linesize[0], size.height(), 0, GL_RED, GL_UNSIGNED_BYTE, 0);

        glBindTexture(GL_TEXTURE_2D, m_yuv[1]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, picture->linesize[1], size.height() * u, 0, GL_RED, GL_UNSIGNED_BYTE,
                (void*)(picture->linesize[0] * size.height()));

        glBindTexture(GL_TEXTURE_2D, m_yuv[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, picture->linesize[2], size.height() * v, 0, GL_RED, GL_UNSIGNED_BYTE,
                (void*)(int)(picture->linesize[0] * size.height() + picture->linesize[1] * size.height() * u));
    } else {

        glBindTexture(GL_TEXTURE_2D, m_yuv[0]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, picture->linesize[0], size.height(), GL_RED, GL_UNSIGNED_BYTE, 0);

        glBindTexture(GL_TEXTURE_2D, m_yuv[1]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, picture->linesize[1], size.height() * u, GL_RED, GL_UNSIGNED_BYTE,
                (void*)(picture->linesize[0] * size.height()));

        glBindTexture(GL_TEXTURE_2D, m_yuv[2]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, picture->linesize[2], size.height() * v, GL_RED, GL_UNSIGNED_BYTE,
                (void*)(int)(picture->linesize[0] * size.height() + picture->linesize[1] * size.height() * u));

    }

    m_bufferMutex.lock();
    glBufferData(GL_PIXEL_UNPACK_BUFFER, m_pboSize, 0, GL_STREAM_DRAW);
    pboPicture.pboDataPtr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    m_lastUploadedFrame = pboPicture.frameNumber;
    //qDebug() << "uploaded " << m_lastUploadedFrame;
    free(picture);
    m_pboPool.append(pboPicture);
    m_bufferMutex.unlock();

    glBindBuffer (GL_PIXEL_UNPACK_BUFFER, 0);

    return true;
}
