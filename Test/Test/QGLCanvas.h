
#include <QDialog>
#include <QtWidgets>
#include <QPainter>
#include <QLabel>
#include <QImage>

class QGLCanvas : public QWidget
{
public:
  QGLCanvas(QWidget* parent = NULL);
  void setImage(const QImage& image);
protected:
  void paintEvent(QPaintEvent*);
private:
  QImage img;
};