//#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include "./Managers/MultimediaManager.h"
#include "./Managers/MultimediaStream.h"
#include "./libdashframework/Adaptation/AdaptationLogicFactory.h"
#include "./libdashframework/Input/DASHManager.h"
#include "./libdashframework/Buffer/MediaObjectBuffer.h"
#include "./libdashframework/MPD/AdaptationSetHelper.h"
#define SEGMENTBUFFER_SIZE 2
using namespace dash;
using namespace std;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::mpd;
using namespace libdash::framework::buffer;
using namespace sampleplayer::managers;
using namespace dash::mpd;

int main(int argc ,char* argv[])
{
  int ret = 0;
  string url = "http://www-itec.aau.at/~cmueller/libdashtest/showcases/big_buck_bunny_480.mpd";
  //IDASHManager *manager = CreateDashManager();
  //IMPD *mpd = manager->Open((char *)url.c_str());


  MultimediaManager *multimediaManager = new MultimediaManager();
  multimediaManager->SetFrameRate(24);
  multimediaManager->Init(url);

  int period = 0;

  int videoRepresentation = 0;
  int videoAdaptationSet = 0;
  int audioRepresentation = -1;
  int audioAdaptationSet = -1;
  IPeriod                         *currentPeriod = multimediaManager->GetMPD()->GetPeriods().at(period);
  std::vector<IAdaptationSet *>   videoAdaptationSets = AdaptationSetHelper::GetVideoAdaptationSets(currentPeriod);
  std::vector<IAdaptationSet *>   audioAdaptationSets = AdaptationSetHelper::GetAudioAdaptationSets(currentPeriod);

  if (videoAdaptationSet >= 0 && videoRepresentation >= 0)
  {
    multimediaManager->SetVideoQuality(currentPeriod,
      videoAdaptationSets.at(videoAdaptationSet),
      videoAdaptationSets.at(videoAdaptationSet)->GetRepresentation().at(videoRepresentation));
  }
  else
  {
    multimediaManager->SetVideoQuality(currentPeriod, NULL, NULL);
  }

  if (audioAdaptationSet >= 0 && audioRepresentation >= 0)
  {
    multimediaManager->SetAudioQuality(currentPeriod,
      audioAdaptationSets.at(audioAdaptationSet),
      audioAdaptationSets.at(audioAdaptationSet)->GetRepresentation().at(audioRepresentation));
  }
  else
  {
    multimediaManager->SetAudioQuality(currentPeriod, NULL, NULL);
  }
 
  multimediaManager->InitVideoRendering(0);
  multimediaManager->videoStream->Start();//start decoding thread
  //multimediaManager->Init(url);
  // initial video rendering

//  multimediaManager->StartVideoRenderingThread();//start rendering thread
//  ret = manager->read(p_data, 32768);

  return 0;
}

