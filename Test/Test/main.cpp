#include <QtGui>
#include <QLabel>
#include <QApplication>

//Ti-Fen's defined class
#include "StreamingProvider.h"
#include "StreamingManager.h"
#include "PresencePlayer.h"
#include "QGLCanvas.h"

int main(int argc, char* argv[])
{
  int ret = 0;
  string url = "http://10.51.12.213/~hoseok/dash/myshelter_960tile/myshelter_dash.mpd";

  //Downloading the segments
  StreamingProvider *provider = new StreamingProvider(0, 82947633);
  provider->Init(url);
  AVFormatContext* avFormatContextPtr = provider->StartDownloading();

  //Get into the PresencePlayer loading method --> parse the megadata
  PresencePlayer *player = new PresencePlayer(avFormatContextPtr);
  //player->run();
  
  //Decode the frames by using libdash sample player's methods
  StreamingManager *streaming = new StreamingManager(avFormatContextPtr);
  Buffer<QImage>          *frameBuffer = streaming->Start();


  //Display a single frame
  QApplication app(argc, argv);
  QGLCanvas w;
  
  QImage  *myImage = NULL;
  while (frameBuffer->Length() > 1)
  {
    myImage = frameBuffer->GetFront();
    w.setImage(*myImage);
    w.show();
  }
  
  return app.exec();
}