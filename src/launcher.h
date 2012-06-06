#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <QWidget>
class QWebView;

class Launcher : public QWidget
{
  Q_OBJECT

  public:
    Launcher(QWidget *parent = 0);

  public slots:
    void go();
    void loadFinished(bool ok);

  private slots:
    void viewTitleChanged(const QString &title);

  private:
    QWebView *view;
};

#endif
