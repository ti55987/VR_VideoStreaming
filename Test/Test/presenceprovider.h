#ifndef PRESENCEPROVIDER
#define PRESENCEPROVIDER

#include <QQuickItem>
#include <QQuickWindow>
#include <QSGNode>
#include <QMap>
#include <QSet>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include <QRunnable>
#include <qopengl.h>

#include "metadataparser.h"
//#include "audiochannelcircularbuffer.h"
#include "../LibHVR/cpp/playbackmetadata.h"

struct ImageSourceTexture {
    QSize textureSize;

    // Texture id for RGB texture if applicaple
    GLuint textureId;

    // Texture ids for Y, U and V textures if applicaple
    GLuint yTextureId;
    GLuint uTextureId;
    GLuint vTextureId;
};

class PresenceProvider : public QQuickItem
{
    Q_OBJECT

public:
    enum ImageSourceTextureType {
        RGBTexture,
        YUVTexture,
        NV12Texture
    };

    struct SelectedSources {
        // Provider should make sure that all required sources are provided
        // always at once and in sync
        QSet<u_int8_t> required;
        // If the provider is capable of providing additional sources real time, this
        // list contains the prioritized list of the sources to be provided
        QList<u_int8_t> additional;

        bool operator==(const SelectedSources &o) const {
            return required == o.required &&
                   additional == o.additional;
        }

        bool operator!=(const SelectedSources &o) const {
            return required != o.required ||
                   additional != o.additional;
        }
    };

    PresenceProvider(QQuickItem *parent = 0)
        : QQuickItem(parent)
        , m_audioChannels(0)
    {
        setFlag(ItemHasContents, true);
        update();
    }

    ~PresenceProvider()
    {
        //for (int i = 0; i < m_audioBuffers.size(); ++i)
        //    delete m_audioBuffers[i];
    }

    // Returns all image source calibration data as a map where the key is the source id
    virtual QMap<u_int8_t, ImageSourceMetadata0> imageSourceMetadata() = 0;

    // Returns image source texture ids and sizes
    // NOTE: View needs to check if these are valid or not
    virtual QMap<u_int8_t, ImageSourceTexture> textures() = 0;

    // Returns the type of texture data this provider provides
    virtual ImageSourceTextureType textureType() { return RGBTexture; }

    // Returns the set of source ids that are available for rendering
    virtual QSet<u_int8_t> latestSyncedImageSources() = 0;

    // Called from GL thread before rendering. Valid GL context is current
    virtual void prepareTextures() {}

    // Number of audio channels, return 0 if no audio
    u_int8_t audioChannels() { QMutexLocker lock(&m_mutex); return m_audioChannels; }

    // Audio signal types
    QMap<u_int8_t, AudioSourceMetadata0::SignalType> audioSignalTypes() { return m_signalTypes; }

    virtual void lockMutex() { m_mutex.lock(); }
    virtual void unlockMutex() { m_mutex.unlock(); }

    // Audio player calls this to read an audio frame from the buffer. It will own the frame until
    // calling markAudioFramesRead()
    //virtual float *readAudioFrame(u_int8_t channel, int timeout = -1) { return m_audioBuffers[channel]->readFrame(timeout); }

    //virtual void lockAudioBufferReading() {
    //    AudioChannelCircularBuffer::lockReading();
    //}
    //virtual void unlockAudioBufferReading() {
    //    AudioChannelCircularBuffer::unlockReading();
    //}

    //// After getting the frame with readAudioFrame(), the audio player marks it read with markFramesRead()
    //// This causes the frame to be recycled
    //virtual void markAudioFramesRead() {
    //    foreach(AudioChannelCircularBuffer *buffer, m_audioBuffers)
    //        buffer->markFrameRead();
    //}

    virtual bool audioPaused() { return true; }

public slots:
    // This is called by the view to set the picked source ids
    virtual void setSelectedSources(SelectedSources selectedSources) = 0;

    virtual void frameSwapped() { }

    virtual void releaseOpenGLResources() { }

signals:
    // Emited when lens properties, image sources or audio channel count have changed
    void metadataChanged();

    // Emitted when the provider has new content ready to be prepared as textures and rendered.
    // It can be emitted at any point, any number of times.
    // Once it has been emitted (one or more times), one render round will follow.
    // Emitting it during a render inside prepareTextures() will request another render round to be done.
    void requestUpdate();

    // Emitted when audio paused state changes. When audio is paused, audio player won't read frames from the buffer
    void audioPausedStateChanged();

protected:
    // These make sure we always release GL resource in the GL thread before
    // the QML item is destroyed
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) {
        if (!node)
            node = new DummyNode(this);
        return node;
    }

    class DummyNode : public QSGNode
    {
    public:
        DummyNode(PresenceProvider *provider)
            : QSGNode()
            , m_provider(provider)
        { }
        ~DummyNode() { m_provider->releaseOpenGLResources(); }

        private:
        PresenceProvider *m_provider;
    };

    //void setAudioChannels(u_int8_t channels,u_int32_t frameSize = 512, u_int32_t frameCount = 1024) {
    //    QMutexLocker lock(&m_mutex);
    //    m_audioChannels = channels;
    //    for (int i = 0; i < channels; ++i) {
    //        m_audioBuffers.append(new AudioChannelCircularBuffer(frameSize, frameCount));
    //        m_signalTypes[i] = AudioSourceMetadata0::SignalSpeaker;
    //    }
    //}

    //void setAudioSignalTypes(QMap<u_int8_t, AudioSourceMetadata0::SignalType> types) {
    //    m_signalTypes = types;
    //}

    //void flushAudioBuffers() {
    //    lockAudioBufferReading();
    //    QMutexLocker lock(&m_mutex);
    //    foreach (AudioChannelCircularBuffer *buffer, m_audioBuffers)
    //        buffer->flush();
    //    unlockAudioBufferReading();
    //}

    //// Returns how many samples are left in the audio buffer
    //virtual u_int64_t samplesInBuffer(u_int8_t channel) { return m_audioBuffers[channel]->used(); }

    //QList<AudioChannelCircularBuffer*> m_audioBuffers;

private:
    u_int8_t m_audioChannels;
    QMutex m_mutex;
    QMap<u_int8_t, AudioSourceMetadata0::SignalType> m_signalTypes;
};

#endif // PRESENCEPROVIDER

