#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QSqlDatabase>
#include <QString>
#include <QDateTime>
#include <QDate>
#include <QList>

class Activity : public QObject
{
  Q_OBJECT

  public:
    static QList<Activity> find(QString conditions);
    static QList<Activity> findCurrent();
    static QList<Activity> findToday();
    static QList<Activity> findDay(QDate date);
    static QSqlDatabase getDatabase();

    Activity();
    Activity(QString name, int projectId, QDateTime startedAt, QDateTime endedAt, QString projectName);
    Activity(const Activity &other);
    Activity &operator=(const Activity &other);

    QString startedAtMDY();
    QString startedAtHM();
    QString startedAtISO8601();
    QString endedAtMDY();
    QString endedAtHM();
    QString endedAtISO8601();
    int duration();
    QString durationInWords();
    bool isRunning();

    int id;
    QString name;
    int projectId;
    QDateTime startedAt;
    QDateTime endedAt;
    QString projectName;

  private:
    static const QString findQuery;
};

#endif
