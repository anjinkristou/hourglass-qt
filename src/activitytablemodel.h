#ifndef __ACTIVITYTABLEMODEL_H
#define __ACTIVITYTABLEMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QList>
#include <QSharedPointer>
#include "recordmanager.h"
#include "activity.h"

class ActivityTableModel : public QAbstractTableModel
{
  Q_OBJECT

  public:
    ActivityTableModel(RecordManager<Activity> *recordManager, QObject *parent = 0);
    ActivityTableModel(QDate date, RecordManager<Activity> *recordManager, QObject *parent = 0);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QSharedPointer<Activity> activityAt(int index) const;
    void stopActivity(int index);

  public slots:
    void refreshActivities();
    void activityCreated(QSharedPointer<Activity> activity);
    void activityDestroyed();
    void activitySaved();

  signals:
    void activityStarted();

  protected:
    //virtual bool activityChangesSince(const QDateTime &dateTime) const;
    virtual QList<int> fetchActivityIds() const;
    virtual bool containsActivity(QSharedPointer<Activity> ptr) const;

  private slots:
    void activityDurationChanged();

  private:
    static const QString s_timeSeparator;
    RecordManager<Activity> *m_recordManager;
    QDate m_date;
    QDateTime m_lastFetchedAt;
    QList<int> m_activityIds;
    QHash<int, QSharedPointer<Activity> > m_activities;
};

#endif
