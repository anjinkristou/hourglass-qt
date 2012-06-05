#include <QWebSettings>
#include <QUrl>
#include "launcher.h"
#include "browser.h"

Launcher::Launcher(QWidget *parent)
  : QWidget(parent)
{
  view = new Browser(this);
  view->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
  connect(view, SIGNAL(titleChanged(QString)), this, SLOT(viewTitleChanged(QString)));
}

void Launcher::start()
{
  view->load(QUrl("http://127.0.0.1:5678/"));
  view->show();
  show();
}

void Launcher::viewTitleChanged(const QString &title)
{
  setWindowTitle(title);
}
