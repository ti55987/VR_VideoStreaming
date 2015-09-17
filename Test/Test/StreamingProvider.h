#include <deque>

//Classes in libdash 
#include "IMPD.h"
#include "IDASHManager.h"
#include "IRepresentation.h"
#include "libdash.h"

//Classes in libdash sample player
#include "libdashframework/Buffer/Buffer.h"
#include "libdashframework/MPD/BaseUrlResolver.h"
#include "libdashframework/MPD/AdaptationSetHelper.h"
#include "libdashframework/Input/MediaObject.h"

//libdash's own decoder
#include "../../Decoder/LibavDecoder.h"

#define SEGMENTBUFFER_SIZE 2

using namespace dash;
using namespace std;
using namespace dash::mpd;
using namespace libdash::framework::mpd;
using namespace libdash::framework::buffer;
using namespace libdash::framework::input;

struct MediaObjectSet
{
  MediaObject *initSegment;
  MediaObject *mediaSegment;
  int initSegmentOffset;
};

class StreamingProvider
{
public:
  StreamingProvider(int period, int bufferSize);
 /*
  * Initalize the video representation object and download the initial segment. 
  *  return false if fail to parse the downloaded MPD
  */
  bool Init(string url);
 /*
  * Download the following segments
  * return fasle if fail to download them or all segments have been downloaded
  */
  AVFormatContext* StartDownloading();

private:

  IDASHManager *manager;
  IMPD *mpd;
  IPeriod  *currentPeriod;
  vector<IAdaptationSet *>   videoAdaptationSets;
  vector<IAdaptationSet *>   audioAdaptationSets;
  IAdaptationSet *videoAdaptationSet;
  IRepresentation *videoRepresentation;
  IAdaptationSet *audioAdaptationSet;
  IRepresentation *audioRepresentation;

  vector<IBaseUrl *> sbaseUrls;
  ISegmentTemplate  *segmentTemp;
  ISegmentList  *segmentList;
  MediaObjectSet mediaSet;
  AVFormatContext *avFormatContextPtr;
  MediaObject *initSeg;
  string RepresentationId;
  uint8_t start_idx;
  uint8_t seg_number;
  int period;
  int bufferSize;

};