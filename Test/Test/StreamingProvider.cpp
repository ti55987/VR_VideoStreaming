#include "StreamingProvider.h"

static int          IORead(void* opaque, uint8_t *buf, int buf_size)
{
  MediaObjectSet* datrec = (MediaObjectSet*)opaque;
  int ret = 0;
  if (datrec->initSegment)
  {
    ret = datrec->initSegment->Peek(buf, buf_size, datrec->initSegmentOffset);
    datrec->initSegmentOffset += (size_t)ret;
  }

  if (ret == 0)
    ret = datrec->mediaSegment->Read(buf, buf_size);
  //int ret = datrec->Read(buf, buf_size);
  return ret;
}

StreamingProvider::StreamingProvider(int period, int bufferSize) :
            period(period), bufferSize(bufferSize), seg_number(0)
{

}

bool StreamingProvider::Init(string url)
{
    int ret = 0;
    manager = CreateDashManager();
    

    mpd = manager->Open((char *)url.c_str());
    currentPeriod = mpd->GetPeriods().at(period);

    videoAdaptationSets = AdaptationSetHelper::GetVideoAdaptationSets(currentPeriod);
    audioAdaptationSets = AdaptationSetHelper::GetAudioAdaptationSets(currentPeriod);

    videoAdaptationSet = videoAdaptationSets.at(0);
    videoRepresentation = videoAdaptationSets.at(0)->GetRepresentation().at(0);
    //Start downloading the segments
    sbaseUrls = BaseUrlResolver::ResolveBaseUrl(mpd, currentPeriod, videoAdaptationSet, 0, 0, 0);
    RepresentationId = videoRepresentation->GetId();
    start_idx = videoRepresentation->GetStartWithSAP();
    segmentTemp = videoRepresentation->GetSegmentTemplate();

    ISegment *seg = segmentTemp->ToInitializationSegment(sbaseUrls, RepresentationId, 82947633);
    //segmentList = videoRepresentation->GetSegmentList();
    //ISegment *seg = segmentList->GetInitialization()->ToSegment(sbaseUrls);

    initSeg = new MediaObject(seg, videoRepresentation);
    if (initSeg)
    {
      return initSeg->StartDownload();
    }
    else
    {
      return false;
    }

}

AVFormatContext* StreamingProvider::StartDownloading()
{

  av_register_all();
  //ISegment *seg2 = segmentList->GetSegmentURLs().at(10)->ToMediaSegment(sbaseUrls);
  ISegment *seg2 = segmentTemp->GetMediaSegmentFromNumber(sbaseUrls, RepresentationId, 82947633, start_idx + seg_number);
  seg_number++;

  MediaObject *mediaSeg = new MediaObject(seg2, videoRepresentation);
  if (mediaSeg)
  {
    mediaSeg->StartDownload();
  }

  //Packaging the initial segment and first media segment
  mediaSet.initSegment = initSeg;
  mediaSet.mediaSegment = mediaSeg;
  mediaSet.initSegmentOffset = 0;

  char *errbuf = (char*)av_malloc(2222);
  unsigned char  *iobuffer = (unsigned char*)av_malloc(bufferSize);
  avFormatContextPtr = avformat_alloc_context();
  avFormatContextPtr->pb = avio_alloc_context(iobuffer, bufferSize, 0, &mediaSet, IORead, NULL, NULL);
  avFormatContextPtr->pb->seekable = 0;

  int err = avformat_open_input(&avFormatContextPtr, "", NULL, NULL);
  if (err < 0)
  {
    av_strerror(err, errbuf, 2222);
    printf("Error:: %s", errbuf);
    return NULL;
  }
  err = avformat_find_stream_info(avFormatContextPtr, 0);
  if (err < 0)
  {
    av_strerror(err, errbuf, 2222);
    printf("Error:: %s", errbuf);
    return NULL;
  }

  return avFormatContextPtr;
}