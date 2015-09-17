#include "PresencePlayer.h"

struct AVDictionary {
  int count;
  AVDictionaryEntry *elems;
};

PresencePlayer:: PresencePlayer(AVFormatContext *m_avFormatContext):
                 m_avFormatContext(m_avFormatContext)
{
  
}

bool PresencePlayer::loadStreaming()
{
  //Video Info
  cout << "Number of streams: " << m_avFormatContext->nb_streams << "\n";
  for (unsigned int i = 0; i < m_avFormatContext->nb_streams; ++i) {
    AVStream *stream = m_avFormatContext->streams[i];
    AVMediaType codecType = stream->codec->codec_type;
    int codecID = stream->codec->codec_id;
    cout << "stream index: " << i;
    cout << "codec type: " << codecType;
    cout << "codecID: " << codecID;
    cout << "duration: " << stream->duration;
    cout << "first dts: " << stream->first_dts;
    cout << "start time: " << stream->start_time;
    cout << "time base: " << stream->time_base.num << " / " << stream->time_base.den;
    cout << "frames: " << stream->nb_frames;
    cout << "r_frame_rate " << stream->r_frame_rate.num << "/" << stream->r_frame_rate.den << endl;
  }
  AVDictionary *dict = m_avFormatContext->metadata;
  cout << "metadata count: " << dict->count;

  //parsing metadata
  for (int i = 0; i < dict->count; ++i) {
    AVDictionaryEntry entry = dict->elems[i];

    if (QString(entry.key) == QStringLiteral("network")) {
      QString metadata(entry.value);

      m_metaDataParser.setMetadataString(metadata);

      if (m_metaDataParser.versionString() == QStringLiteral("0.2")) {
        // Read the first package as that's always the metadata stream
        AVPacket avpkt;
        if (av_read_frame(m_avFormatContext, &avpkt) != 0) {
          qWarning() << "Error! Couldn't read the mp4 file";
          return false;
        }
        cout << "Reading metadata from first package, stream id:" << avpkt.stream_index << ", size:" << avpkt.size;
        m_metaDataParser.parseMetadataPackage(avpkt.data, avpkt.size);
      }

      cout << "Extracted metadata:";
      cout << m_metaDataParser.humanReadableString().toStdString();

      m_duration = 0;
      m_imageSourceMetadata = m_metaDataParser.imageSourceMetadata();

      m_imageSourceTextures.clear();

      foreach(ImageSourceMetadata0 metadata, m_imageSourceMetadata) {
        cout << "s" << metadata.streamIndex << ", nb:" << m_avFormatContext->nb_streams;
        u_int8_t streamIndex = metadata.streamIndex;
        if (m_videoStreams.contains(streamIndex)) {
          m_videoStreams[streamIndex].sources.insert(metadata.sourceId);
          continue;
        }

        VideoStreamContext videoStreamContext;
        videoStreamContext.sources.insert(metadata.sourceId);
        videoStreamContext.selected = true;

        AVStream *avStream = m_avFormatContext->streams[streamIndex];
        cout << avStream << avStream->time_base.den << avStream->time_base.num << avStream->r_frame_rate.den << avStream->r_frame_rate.num;
        videoStreamContext.timeBaseMultiplier =
          (avStream->time_base.den * avStream->r_frame_rate.den) /
          (avStream->time_base.num * avStream->r_frame_rate.num);
        cout << "Time base multiplier:" << videoStreamContext.timeBaseMultiplier;

        double fps = 0.;
        if (avStream->nb_frames > 1) {
          fps = avStream->r_frame_rate.num / avStream->r_frame_rate.den;
          cout << "fps: " << fps;
          if (m_duration == 0)
            m_duration = avStream->nb_frames / fps;
        }
        else {
          // single frame, ffmpeg can't guess fps
          fps = 30.;

        videoStreamContext.decoder = new VideoDecoderLibAV();

        videoStreamContext.decoder->init(avStream);
        m_videoStreams[streamIndex] = videoStreamContext;
        double frameInterval = 1000. / fps;
        if (frameInterval > 0)
          m_frameInterval = frameInterval;

        ImageSourceTexture texture;
        texture.textureId = 0;
        texture.yTextureId = 0;
        texture.uTextureId = 0;
        texture.vTextureId = 0;
        m_imageSourceTextures[streamIndex] = texture;
      }
     }
    }
  }
  // Seek back to beginning
  av_seek_frame(m_avFormatContext, m_videoStreams.firstKey(), -1, AVSEEK_FLAG_BACKWARD);

  m_elapsedTimer.restart();
  m_eof = false;
  m_ended = false;
  m_loaded = true;

  return true;
}

bool PresencePlayer::parseFrame()
{
  AVPacket *avPacket = new AVPacket();
  //av_init_packet(avPacket);
  bool ret = av_read_frame(m_avFormatContext, avPacket);
  if (ret == 0) {
    const u_int8_t streamIndex = avPacket->stream_index;

    //const double elapsedMs = getVideoSyncTime();

    //if (m_audioStreams.contains(streamIndex)) {
    //  // Audio packet
    //  const AudioStreamContext &stream = m_audioStreams[streamIndex];
    //  const double kHz = stream.frequency * 0.001;
    //  if ((avPacket->pts + stream.ptsStep) / kHz >= elapsedMs - m_frameInterval) {
    //    const float gainInt2Float = 1.0 / 32767.0;
    //    const int channels = stream.channels.size();
    //    const int bytesPerChannel = avPacket->size / channels;
    //    const int16_t *sdata = (int16_t*)(avPacket->data);
    //    const int samples = bytesPerChannel >> 1;
    //    float **buffers = new float *[channels];
    //    const double currentMs = (avPacket->pts - m_loader->samplesInBuffer(stream.channels.firstKey())) / kHz;

    //    // Filter the offset to avoid oscillation
    //    m_audioOffsetMs = 0.998 * m_audioOffsetMs + 0.002 * (currentMs - elapsedMs);
    //    double newOffset = m_audioOffsetMs;
    //    m_loader->lockMutex();

    //    if (m_audioOffsetMs > 10) { // Audio should never come more than 10 ms in advance
    //      // Video is behind
    //      qDebug() << "Video is advanced in time";
    //      m_controlMutex.lock();
    //      m_elapsedTimer.restart();
    //      m_elapsedTimer = m_elapsedTimer.addMSecs(-currentMs - 1 * m_frameInterval);
    //      m_controlMutex.unlock();
    //      newOffset = 0;
    //    }
    //    else if (m_audioOffsetMs < -40) { // Audio should never come more than 40 ms behind
    //      // Audio is behind
    //      qDebug() << "Video is delayed in time";
    //      m_controlMutex.lock();
    //      m_elapsedTimer.restart();
    //      m_elapsedTimer = m_elapsedTimer.addMSecs(-currentMs + 1 * m_frameInterval);
    //      m_controlMutex.unlock();
    //      newOffset = 0;
    //      /*
    //      m_loader->lockAudioBufferReading();
    //      foreach (u_int8_t ch, stream.channels.keys())
    //      m_loader->advanceAudioBuffer(ch, (elapsedMs - currentMs) * kHz);
    //      m_loader->unlockAudioBufferReading();
    //      newOffset = 0;
    //      */
    //    }
    //    m_audioOffsetMs = newOffset;

    //    u_int8_t offset = 0;
    //    foreach(u_int8_t ch, stream.channels.keys()) {
    //      // Feed the audio into the playback buffer
    //      buffers[ch] = m_loader->getAudioBuffer(ch, samples);
    //      for (int i = 0; i < samples; ++i) {
    //        buffers[ch][i] = (float)sdata[offset + i * channels] * gainInt2Float;
    //      }
    //      offset++;
    //      m_loader->markAudioBufferReady(ch);
    //    }
    //    m_loader->unlockMutex();
    //    delete[] buffers;
    //  }
    //  else {
    //    // Audio packet is old, clear the playback buffer so that correct
    //    // audio will be played immediately when gotten
    //    m_loader->flushAudioBuffers();
    //  }
    //  av_free_packet(avPacket);
    //  delete avPacket;
    //}
    //else 
    if (m_videoStreams.contains(streamIndex)) {
      // Video packet
      m_videoStreams[streamIndex].avPackets.append(avPacket);
    }
    else {
      // Unknown packet
      av_free_packet(avPacket);
      delete avPacket;
      return true;
    }
  }
  else {
    // EOF
    qDebug() << "End of file!";
    m_eof = true;
    av_free_packet(avPacket);
    delete avPacket;
    return false;
  }

  return true;
}

bool PresencePlayer::run()
{
  //Q_ASSERT(m_loader);
  qDebug() << "MP4Provider thread started";
  m_stopped = false;
  bool ok = false;
  while (1) {
    m_controlMutex.lock();
    if (m_stopped) {
      m_controlMutex.unlock();
      break;
    }

    if (m_sourceChanged) {
      // Wait until gl thread has destroyed video decoders
      //if (!m_videoStreams.isEmpty()) {
      //  m_controlMutex.unlock();
      //  emit m_loader->requestUpdate();
      //  QThread::msleep(10);
      //  continue;
      //}

      bool ok = loadStreaming();
      //emit loadingFinished(ok);
      //m_sourceChanged = false;
      //if (ok)
      //  m_elapsedTimer.start();
    }

    if (ok) {
      if (!m_paused || m_currentFrame < 1) {
        u_int64_t currentFrame = m_elapsedTimer.elapsed() / m_frameInterval;
        if (currentFrame != m_currentFrame) {
          m_currentFrame = currentFrame;
        }
      }

      m_controlMutex.unlock();
      m_avPacketMutex.lock();
      QMap<u_int8_t, VideoStreamContext>::iterator i = m_videoStreams.begin();
      while (i != m_videoStreams.end()) {
        VideoStreamContext &videoStreamContext = i.value();

        // Update seleceted status
        m_controlMutex.lock();
        // Decode all streams when paused
        bool selected = m_paused;

        // The stream is selected if it contains any of the selected sources
        QSet<u_int8_t> selectedSources = m_selectedImageSources;
        selectedSources.intersect(videoStreamContext.sources);
        selected |= !selectedSources.isEmpty();
        videoStreamContext.selected = selected;
        m_controlMutex.unlock();

        while (videoStreamContext.avPackets.size() < 1 && !m_eof) {
          if (!parseFrame())
            break;
        }

        if (videoStreamContext.avPackets.isEmpty()) {
          //qWarning() << "video stream context " << i.key() << " has no avpackets!";
          ++i;
          continue;
        }

        VideoDecoder *decoder = videoStreamContext.decoder;
        AVPacket *avPacket = videoStreamContext.avPackets.takeFirst();

        bool shouldDecode = false;
        if (selected && decoder->lastDecodedFrame() < m_currentFrame + 30)
          shouldDecode = true;
        if (!selected && (avPacket->pts / videoStreamContext.timeBaseMultiplier < m_currentFrame + 5))
          shouldDecode = true;

        if (avPacket->pts / videoStreamContext.timeBaseMultiplier < m_currentFrame) {
          // Drop B-frames even the source would be selected to catch up faster
          selected = false;
        }

        if (shouldDecode) {
          bool ok = decoder->decodeFrame(avPacket, selected);

          if (!ok)
            videoStreamContext.avPackets.prepend(avPacket);
          else {
            av_free_packet(avPacket);
            delete avPacket;
          }
        }
        else {
          videoStreamContext.avPackets.prepend(avPacket);
        }
        ++i;
      }
      m_avPacketMutex.unlock();

      if (m_paused)
        QThread::msleep(2);

   /*   emit m_loader->requestUpdate();*/
    }
    else {
      m_controlMutex.unlock();
      QThread::msleep(50);
    }

    // Seeking
    m_controlMutex.lock();
    if (m_seekTarget != -1) {
      u_int64_t frameTarget = round(m_seekTarget / m_frameInterval);
      qDebug() << "SEEKING TO " << m_seekTarget << "ms, frame:" << frameTarget;

      //if (!m_audioStreams.isEmpty()) {
      //  // We use the first audio stream for seeking
      //  const AudioStreamContext &stream = m_audioStreams.first();
      //  const int64_t targetAudioPTS = frameTarget * stream.frequency * 0.001 * m_frameInterval;
      //  av_seek_frame(m_avFormatContext, m_audioStreams.firstKey(), targetAudioPTS, AVSEEK_FLAG_BACKWARD);
      //}
      //else 
      if (!m_videoStreams.isEmpty()) {
        av_seek_frame(m_avFormatContext, m_videoStreams.firstKey(),
          (frameTarget + 1) * m_videoStreams.first().timeBaseMultiplier, AVSEEK_FLAG_BACKWARD);
      }
      // This is only needed if we mux audio packages more than 1 sec before video
      m_avFormatContext->pb->seekable = false;

      m_currentFrame = frameTarget + 1;
      m_elapsedTimer.restart();
      setVideoSyncTime(m_seekTarget);
      if (!m_videoStreams.isEmpty())
        m_lastUploadedFramePTS = m_seekTarget * m_videoStreams.first().timeBaseMultiplier / m_frameInterval;
      m_position = m_seekTarget;
      m_audioOffsetMs = 0;
      m_elapsedTimer = m_elapsedTimer.addMSecs(-m_position);
      m_controlMutex.unlock();

      // Flush video decoding pipe
      m_avPacketMutex.lock();
      m_controlMutex.lock();
      resetVideoContexts(true);
      //m_loader->flushAudioBuffers();

      m_eof = false;
      m_ended = false;
      m_seekTarget = -1;
      m_avPacketMutex.unlock();

      //emit positionChanged(m_position);
    }
    m_controlMutex.unlock();
  }

  qDebug() << "MP4Provider thread finished";

  return true;
}


void PresencePlayer::setVideoSyncTime(double ms) {
  QMutexLocker lock(&m_videoSyncMutex);
  m_videoSyncTime.restart();
  m_videoSyncTime = m_videoSyncTime.addMSecs(-ms);
}

void PresencePlayer::resetVideoContexts(bool clearContexts)
{
  QMap<u_int8_t, VideoStreamContext>::iterator i = m_videoStreams.begin();
  while (i != m_videoStreams.end()) {
    VideoStreamContext &videoStreamContext = i.value();
    while (!videoStreamContext.avPackets.isEmpty()) {
      AVPacket *avPacket = videoStreamContext.avPackets.takeFirst();
      av_free_packet(avPacket);
      delete avPacket;
    }

    if (clearContexts) {
      delete videoStreamContext.decoder;
      videoStreamContext.decoder = 0;
    }
    else
      videoStreamContext.decoder->reset();

    ++i;
  }

  if (clearContexts)
    m_videoStreams.clear();
}