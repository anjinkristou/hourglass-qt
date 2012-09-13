#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "activitytablemodel.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

  private slots:
    void on_btnAddEarlierActivity_clicked();

  private:
    Ui::MainWindow m_ui;
};

#endif