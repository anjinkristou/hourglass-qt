#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QStringList>
#include "database.h"
#include "activity.h"

const QString Activity::findQuery = QString(
    "SELECT activities.id, activities.name, activities.project_id, "
    "activities.started_at, activities.ended_at, "
    "projects.name AS project_name "
    "FROM activities "
    "LEFT JOIN projects ON activities.project_id = projects.id");

const QString Activity::distinctNamesQuery = QString(
    "SELECT DISTINCT activities.name, projects.name AS project_name "
    "FROM activities "
    "LEFT JOIN projects ON activities.project_id = projects.id "
    "ORDER BY activities.name, projects.name");

const QString Activity::distinctProjectNamesQuery = QString(
    "SELECT DISTINCT name FROM projects ORDER BY name");

QList<Activity> Activity::find(QString conditions)
{
  QStringList queryStrings;
  queryStrings << findQuery;
  if (!conditions.isEmpty()) {
    queryStrings << conditions;
  }
  qDebug() << queryStrings.join(" ");

  QSqlDatabase &database = getDatabase();
  QSqlQuery query = database.exec(queryStrings.join(" "));

  QList<Activity> result;
  while (query.next()) {
    int id        = query.value(0).toInt();
    qDebug() << "id:" << id;
    QString name  = query.value(1).toString();
    qDebug() << "name:" << name;
    int projectId = query.value(2).toInt();
    qDebug() << "projectId:" << projectId;
    QDateTime startedAt = query.value(3).toDateTime();
    qDebug() << "startedAt:" << startedAt;
    QDateTime endedAt   = query.value(4).toDateTime();
    qDebug() << "endedAt:" << endedAt;
    QString projectName = query.value(5).toString();

    Activity activity(name, projectId, startedAt, endedAt, projectName);
    activity.id = id;
    result << activity;
  }
  return result;
}

QList<Activity> Activity::findCurrent()
{
  return find("WHERE activities.ended_at IS NULL");
}


QList<Activity> Activity::findToday()
{
  return findDay(QDate::currentDate());
}

QList<Activity> Activity::findDay(QDate date)
{
  return find(QString("WHERE date(activities.started_at) = date('%1')").arg(date.toString(Qt::ISODate)));
}

QMap<QString, int> Activity::projectTotals(QList<Activity> &activities)
{
  QMap<QString, int> totals;
  for (int i = 0; i < activities.size(); i++) {
    Activity &activity = activities[i];
    int total = (totals.contains(activity.projectName) ? totals[activity.projectName] : 0);
    total += activity.duration();
    totals.insert(activity.projectName, total);
  }
  return totals;
}

QList<QString> Activity::distinctNames()
{
  QSqlDatabase &database = getDatabase();
  QSqlQuery query = database.exec(distinctNamesQuery);

  QList<QString> names;
  while (query.next()) {
    QString name = query.value(0).toString();
    QVariant projectName = query.value(1);
    if (projectName.isNull()) {
      names << name;
    }
    else {
      names << QString("%1@%2").arg(name).arg(projectName.toString());
    }
  }
  return names;
}

QList<QString> Activity::distinctProjectNames()
{
  QSqlDatabase &database = getDatabase();
  QSqlQuery query = database.exec(distinctProjectNamesQuery);

  QList<QString> names;
  while (query.next()) {
    names << query.value(0).toString();
  }
  return names;
}

QSqlDatabase &Activity::getDatabase()
{
  return DatabaseManager::getInstance().getDatabase();
}

Activity::Activity()
{
  this->id = this->projectId = -1;
}

Activity::Activity(QString name, int projectId, QDateTime startedAt, QDateTime endedAt, QString projectName)
{
  this->name = name;
  this->projectId = projectId;
  this->startedAt = startedAt;
  this->endedAt = endedAt;
  this->projectName = projectName;
}

Activity::Activity(const Activity &other)
{
  this->id = other.id;
  this->name = other.name;
  this->projectId = other.projectId;
  this->startedAt = other.startedAt;
  this->endedAt = other.endedAt;
}

Activity &Activity::operator=(const Activity &other)
{
  this->id = other.id;
  this->name = other.name;
  this->projectId = other.projectId;
  this->startedAt = other.startedAt;
  this->endedAt = other.endedAt;
}

QString Activity::startedAtMDY()
{
  if (startedAt.isValid()) {
    return startedAt.toString("MM/dd/yyyy");
  }
  else {
    return QString();
  }
}

QString Activity::startedAtHM()
{
  if (startedAt.isValid()) {
    return startedAt.toString("hh:mm");
  }
  else {
    return QString();
  }
}

QString Activity::startedAtISO8601()
{
  if (startedAt.isValid()) {
    return startedAt.toString(Qt::ISODate);
  }
  else {
    return QString();
  }
}

QString Activity::endedAtMDY()
{
  if (endedAt.isValid()) {
    return endedAt.toString("MM/dd/yyyy");
  }
  else {
    return QString();
  }
}

QString Activity::endedAtHM()
{
  if (endedAt.isValid()) {
    return endedAt.toString("hh:mm");
  }
  else {
    return QString();
  }
}

QString Activity::endedAtISO8601()
{
  if (endedAt.isValid()) {
    return endedAt.toString(Qt::ISODate);
  }
  else {
    return QString();
  }
}

int Activity::duration()
{
  if (endedAt.isValid()) {
    return startedAt.secsTo(endedAt);
  }
  else {
    return startedAt.secsTo(QDateTime::currentDateTime());
  }
}

QString Activity::durationInWords()
{
  int d = duration();
  int totalMinutes = d / 60;

  if (totalMinutes == 0) {
    return QString("0min");
  }
  else {
    int minutes = totalMinutes % 60;
    int totalHours = totalMinutes / 60;
    int hours = totalHours % 24;
    int days = totalHours / 24;
    qDebug() << "minutes:" << minutes << "; hours:" << hours << "; days:" << days;

    QStringList strings;
    if (days > 0) {
      strings << QString("%1d").arg(days);
    }
    if (hours > 0) {
      strings << QString("%1h").arg(hours);
    }
    if (minutes > 0) {
      strings << QString("%1min").arg(minutes);
    }
    return strings.join(" ");
  }
}

bool Activity::isRunning()
{
  return endedAt.isValid();
}
