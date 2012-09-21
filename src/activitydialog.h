#ifndef __ACTIVITYDIALOG_H
#define __ACTIVITYDIALOG_H

#include <QDialog>
#include <QSharedPointer>
#include "ui_activitydialog.h"
#include "activity.h"

class ActivityDialog : public QDialog
{
  Q_OBJECT

  public:
    ActivityDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ActivityDialog(QSharedPointer<Activity> activity, QWidget *parent = 0, Qt::WindowFlags f = 0);

    QSharedPointer<Activity> activity() const;
    void setActivityCompleter(QCompleter *completer);
    void setTagCompleter(QCompleter *completer);

  public slots:
    void accept();

  private slots:
    void on_cbInProgress_clicked();

  private:
    Ui::ActivityDialog m_ui;
    QSharedPointer<Activity> m_activity;
};

#endif
