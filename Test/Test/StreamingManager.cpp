#include "StreamingManager.h"

enum   pixelFormat  {
  yuv420p = 0,
  yuv422p = 4
};


StreamingManager::StreamingManager(AVFormatContext *m_avFormatContext) :
avFormatContextPtr(m_avFormatContext)
{

}

void     StreamingManager::InitStreams()
{
  AVStream      *tempStream = NULL;
  AVCodec       *tempCodec = NULL;
  StreamConfig  tempConfig;

  for (size_t streamcnt = 0; streamcnt < avFormatContextPtr->nb_streams; ++streamcnt)
  {
    tempStream = avFormatContextPtr->streams[streamcnt];

    if ((tempStream->codec->codec_type == AVMEDIA_TYPE_VIDEO) ||
      (tempStream->codec->codec_type == AVMEDIA_TYPE_AUDIO))
    {
      tempConfig.stream = tempStream;
      tempCodec = avcodec_find_decoder(tempStream->codec->codec_id);
      tempConfig.codecContext = tempStream->codec;
      tempConfig.frameCnt = 0;

      avcodec_open2(tempConfig.codecContext, tempCodec, NULL);
      streamconfigs.push_back(tempConfig);
    }
  }
}

StreamConfig*    StreamingManager::FindStreamConfig(int streamIndex)
{
  size_t configsCount = streamconfigs.size();

  for (size_t i = 0; i < configsCount; i++)
  {
    if (streamconfigs.at(i).stream->index == streamIndex)
      return &streamconfigs.at(i);
  }

  return NULL;
}

StreamConfig*    StreamingManager::GetNextPacket(AVFormatContext* avFormatContextPtr, AVPacket* avpkt)
{
  while (true)
  {
    int err = av_read_frame(avFormatContextPtr, avpkt);

    if ((size_t)err == AVERROR_EOF)
      return NULL;

    if (err == AVERROR_RESSOURCE_NOT_AVAILABLE)
    {
      av_free_packet(avpkt);
      continue;
    }

    if (err < 0)
    {
      printf("Error while av_read_frame %s", err);
      return NULL;
    }

    StreamConfig *tempConfig = FindStreamConfig(avpkt->stream_index);
    if (tempConfig)
      return tempConfig;

    av_free_packet(avpkt);
  }
}

int   StreamingManager::DecodeMedia(AVFrame *frame, AVPacket *avpkt, StreamConfig *decConfig, int *got_frame)
{
  switch (decConfig->stream->codec->codec_type)
  {
  case AVMEDIA_TYPE_VIDEO:
    return avcodec_decode_video2(decConfig->codecContext, frame, got_frame, avpkt);
  case AVMEDIA_TYPE_AUDIO:
    return avcodec_decode_audio4(decConfig->codecContext, frame, got_frame, avpkt);
  case AVMEDIA_TYPE_SUBTITLE:
    break;
  case AVMEDIA_TYPE_UNKNOWN:
    break;
  default:
    break;
  }

  return -1;
}

void   StreamingManager::OnVideoFrameDecoded(const uint8_t **data, StreamConfig* decoConf, AVFrame * avFrame)
{
  /* TODO: some error handling here */
  if (data == NULL || false)
    return;

  int w = decoConf->stream->codec->width;
  int h = decoConf->stream->codec->height;

  AVFrame *rgbframe = avcodec_alloc_frame();
  int     numBytes = avpicture_get_size(PIX_FMT_RGB24, w, h);
  uint8_t *buffer = (uint8_t*)av_malloc(numBytes);
  pixelFormat pxlFmt;

  if (decoConf->stream->codec->pix_fmt == PIX_FMT_YUV420P)
    pxlFmt = yuv420p;
  if (decoConf->stream->codec->pix_fmt == PIX_FMT_YUV422P)
    pxlFmt = yuv422p;

  avpicture_fill((AVPicture*)rgbframe, buffer, PIX_FMT_RGB24, w, h);

  SwsContext *imgConvertCtx = sws_getContext(w, h, (PixelFormat)pxlFmt, w, h, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

  sws_scale(imgConvertCtx, data, avFrame->linesize, 0, h, rgbframe->data, rgbframe->linesize);
  QImage *image = new QImage(w, h, QImage::Format_RGB32);
  uint8_t *src = (uint8_t *)rgbframe->data[0];

  for (size_t y = 0; y < h; y++)
  {
    QRgb *scanLine = (QRgb *)image->scanLine(y);

    for (size_t x = 0; x < w; x++)
      scanLine[x] = qRgb(src[3 * x], src[3 * x + 1], src[3 * x + 2]);

    src += rgbframe->linesize[0];
  }

  frameBuffer->PushBack(image);

  av_free(rgbframe);
  av_free(buffer);
}

int  StreamingManager::DecodeFrame(AVFrame *frame, AVPacket* avpkt, StreamConfig* decConfig)
{
  int len = 0;
  int got_frame = 1;

  while (avpkt->size > 0)
  {
    /* TODO handle multi frame packets */
    len = DecodeMedia(frame, avpkt, decConfig, &got_frame);

    if (len < 0)
    {
      printf("Error while decoding frame %s", len);
      return -1;
    }

    if (got_frame)
    {
      OnVideoFrameDecoded((const uint8_t **)frame->data, decConfig, frame);
      decConfig->frameCnt++;
    }

    av_free_packet(avpkt);
  }
  return 0;
}


Buffer<QImage>* StreamingManager::Start()
{
  AVPacket  avpkt;
  AVFrame  *frame = avcodec_alloc_frame();
  av_init_packet(&avpkt);
  InitStreams();
  while (frameBuffer->Length() < FRAMEBUFFER_SIZE)
  {
    StreamConfig *decConfig = GetNextPacket(avFormatContextPtr, &avpkt);
    DecodeFrame(frame, &avpkt, decConfig);
  }

  return frameBuffer;
}