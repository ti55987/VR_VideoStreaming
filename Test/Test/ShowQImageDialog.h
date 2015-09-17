#ifndef SHOWQIMAGEDIALOG_H
#define SHOWQIMAGEDIALOG_H
#include <QDialog>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QImage>

class ShowQImageDialog : public QDialog
{
  Q_OBJECT

public:
  ShowQImageDialog(QWidget *parent = 0);
  ~ShowQImageDialog();
private:
  QGridLayout* gridLayout;
  QImage* inputImg;
  QLabel* imgDisplayLabel;
  QScrollArea* scrollArea;
};

#endif // SHOWQIMAGEDIALOG_H