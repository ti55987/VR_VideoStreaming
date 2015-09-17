#include "QGLCanvas.h"

QGLCanvas::QGLCanvas(QWidget* parent)
  : QWidget(parent)
{
  this->setGeometry(9 + 10, 35 + 30, 582, 401);
}

void QGLCanvas::setImage(const QImage& image)
{
  img = image;
}

void QGLCanvas::paintEvent(QPaintEvent*)
{
  QPainter p;
  p.begin(this);
  //Set the painter to use a smooth scaling algorithm.
  //p.SetRenderHint(QPainter::SmoothPixmapTransform, 1);

  p.drawImage(this->rect(), img);
  p.end();
}