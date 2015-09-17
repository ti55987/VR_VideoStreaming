//#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>
//#include <libavutil/avutil.h>
//#include <libavutil/dict.h>
#include "../../Decoder/LibavDecoder.h"
#include <iostream>

#include <QtGlobal>
#include <QThread>
#include <QMutex>
#include <QString>
#include <QVector2D>
#include <QVector3D>
#include <QMap>
#include <QSet>
#include <QTime>
#include <QDebug>
#include <QQuickItem>
#include <QLibrary>

#include "videodecoderlibav.h"
#include "presenceprovider.h"
#include "videodecoder.h"
#include "metadataparser.h"

using namespace std;

class PresencePlayer
{
public:
  PresencePlayer(AVFormatContext *m_avFormatContext);
  bool loadStreaming();
  bool run();
  bool parseFrame();
  void setVideoSyncTime(double ms);
  void resetVideoContexts(bool clearContexts);

private:
  
  struct VideoStreamContext {
    VideoDecoder *decoder;
    QList<AVPacket*> avPackets;
    double timeBaseMultiplier;
    QSet<u_int8_t> sources;
    bool selected;
  };

  bool m_sourceChanged;
  bool m_loaded;
  bool m_stopped;
  u_int64_t m_currentFrame;
  AVFormatContext *m_avFormatContext;

  MetadataParser m_metaDataParser;


  QMap<u_int8_t, VideoStreamContext> m_videoStreams;
  QMap<u_int8_t, ImageSourceMetadata0> m_imageSourceMetadata;
  QMap<u_int8_t, ImageSourceTexture> m_imageSourceTextures;
  QSet<u_int8_t> m_selectedImageSources;
  QSet<u_int8_t> m_availableImageSources;
  QMutex m_controlMutex;
  QMutex m_videoStreamsLoadingMutex;
  QMutex m_avPacketMutex;
  QTime m_videoSyncTime;
  QMutex m_videoSyncMutex;
  u_int64_t m_lastUploadedFramePTS;
  double m_audioOffsetMs;

  bool m_eof;
  bool m_ended;
  QTime m_elapsedTimer;
  double m_frameInterval;
  bool m_paused;
  double m_position;
  int64_t m_seekTarget;
  double m_duration;

};