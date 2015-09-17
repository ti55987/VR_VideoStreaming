#include <QtGui>
#include <QLabel>
#include <QApplication>

#include <deque>

#include "libdashframework/Buffer/Buffer.h"
#include "../../Decoder/LibavDecoder.h"

using namespace std;

#define AVERROR_RESSOURCE_NOT_AVAILABLE -11
#define FRAMEBUFFER_SIZE 5

struct StreamConfig
{
  AVStream       *stream;
  AVCodecContext *codecContext;
  int             frameCnt;
};


class StreamingManager
{
public: 


  StreamConfig*    GetNextPacket(AVFormatContext* avFormatContextPtr, AVPacket* avpkt);
  StreamingManager(AVFormatContext *avFormatContextPtr);
  void InitStreams();
  StreamConfig*    FindStreamConfig(int streamIndex);
  int  DecodeMedia(AVFrame *frame, AVPacket *avpkt, StreamConfig *decConfig, int *got_frame);
  void  OnVideoFrameDecoded(const uint8_t **data, StreamConfig* decoConf, AVFrame * avFrame);
  int  DecodeFrame(AVFrame *frame, AVPacket* avpkt, StreamConfig* decConfig);
  Buffer<QImage> *Start();

private:

  AVFormatContext *avFormatContextPtr;
  vector <StreamConfig>    streamconfigs;
  Buffer<QImage>          *frameBuffer = new Buffer<QImage>(FRAMEBUFFER_SIZE, VIDEO);
};