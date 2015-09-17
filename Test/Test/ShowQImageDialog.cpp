#include "ShowQImageDialog.h"
ShowQImageDialog::ShowQImageDialog(QWidget *parent) : QDialog(parent)
{
  gridLayout = new QGridLayout();
  inputImg = new QImage("Libraries\Pictures\Capture.png");
  imgDisplayLabel = new QLabel("");
  imgDisplayLabel->setPixmap(QPixmap::fromImage(*inputImg));
  imgDisplayLabel->adjustSize();
  scrollArea = new QScrollArea();
  scrollArea->setWidget(imgDisplayLabel);
  scrollArea->setMinimumSize(256, 256);
  scrollArea->setMaximumSize(512, 512);
  gridLayout->addWidget(scrollArea, 0, 0);
  setLayout(gridLayout);
}

ShowQImageDialog::~ShowQImageDialog()
{
  delete inputImg;
  delete imgDisplayLabel;
  delete scrollArea;
  delete gridLayout;
}