#include <QFile>
#include <QStringList>
#include "view.h"
#include "controller.h"
#include "project.h"
#include "tag.h"

const QString Controller::editActivityPattern =
  QString("^/activities/(\\d+)/edit$");
const QString Controller::activityPattern =
  QString("^/activities/(\\d+)$");
const QString Controller::deleteActivityPattern =
  QString("^/activities/(\\d+)/delete$");
const QString Controller::restartActivityPattern =
  QString("^/activities/(\\d+)/restart$");

QList<QPair<QString, QString> > Controller::decodePost(const char *data)
{
  QList<QPair<QString, QString> > result;

  QStringList parts = QString(data).split('&');
  for (int i = 0; i < parts.size(); i++) {
    QString variable = parts[i];
    QStringList variableParts = variable.split('=');
    if (variableParts.size() == 2) {
      QString &rawKey = variableParts[0].replace("+", " ");
      QString &rawValue = variableParts[1].replace("+", " ");
      QByteArray rawKeyBA = rawKey.toUtf8();
      QByteArray rawValueBA = rawValue.toUtf8();
      QPair<QString, QString> pair(
          QUrl::fromPercentEncoding(rawKeyBA),
          QUrl::fromPercentEncoding(rawValueBA));
      //qDebug() << pair;
      result << pair;
    }
  }
  return result;
}

Controller::Controller(QDir root, QHttpRequest *req, QHttpResponse *resp, QObject *parent)
  : QObject(parent), m_root(root), m_req(req), m_resp(resp)
{
  connect(m_req, SIGNAL(data(const QByteArray&)), this, SLOT(accumulate(const QByteArray&)));
  connect(m_req, SIGNAL(end()), this, SLOT(route()));
}

void Controller::accumulate(const QByteArray &data)
{
  this->data.append(data);
}

void Controller::route()
{
  QString path = m_req->path();
  QString method = m_req->method();

  QString result;
  QStringList matchData;
  bool isJSON = false;
  if (method == "GET") {
    if (path == "/") {
      result = index();
    }
    else if (path == "/activities/new") {
      result = newActivity();
    }
    else if (pathMatches(path, editActivityPattern, matchData)) {
      bool ok = false;
      int activityId = matchData[1].toInt(&ok);
      if (ok) {
        result = editActivity(activityId);
      }
    }
    else if (pathMatches(path, deleteActivityPattern, matchData)) {
      bool ok = false;
      int activityId = matchData[1].toInt(&ok);
      if (ok) {
        result = deleteActivity(activityId);
      }
    }
    else if (path == "/activities") {
      result = partialActivityNames();
      isJSON = true;
    }
    else if (path == "/activities/current/stop") {
      result = stopCurrentActivities();
      isJSON = true;
    }
    else if (path == "/tags") {
      result = partialTagNames();
      isJSON = true;
    }
    else if (path == "/projects") {
      result = partialProjectNames();
      isJSON = true;
    }
    else if (path == "/totals") {
      result = partialTotals();
      isJSON = true;
    }
  }
  else if (method == "POST") {
    QList<QPair<QString, QString> > params = decodePost(data);

    if (path == "/activities") {
      result = createActivity(params);
      isJSON = true;
    }
    else if (pathMatches(path, activityPattern, matchData)) {
      bool ok = false;
      int activityId = matchData[1].toInt(&ok);
      if (ok) {
        result = updateActivity(activityId, params);
        isJSON = true;
      }
      else {
      }
    }
    //else if (!(strings = restartActivityPattern.matches(path)).isEmpty()) {
    //}
  }

  if (result.isNull()) {
    serveFile(path);
  }
  else {
    m_resp->setHeader("Content-Type",
        isJSON ? "application/json" : "text/html");
    m_resp->setHeader("Content-Length", QString("%1").arg(result.size()));
    m_resp->writeHead(200);
    m_resp->write(result);
    m_resp->end();
  }
  emit done();
}

void Controller::includeActivities(Dictionary *dictionary, QList<Activity> &activities)
{
  Dictionary *d = dictionary->addIncludeDictionary("activities", "_activities.html");
  d->addActivitySection(activities);
}

void Controller::includeNames(Dictionary *dictionary, QList<QString> &names)
{
  Dictionary *d = dictionary->addIncludeDictionary("names", "_names.js");

  Dictionary *d2;
  QListIterator<QString> i(names);
  while (i.hasNext()) {
    d2 = d->addSectionDictionary("name");
    d2->setValue("value", i.next());
    d2->setValue("comma", i.hasNext() ? "," : "");
  }
}

void Controller::includeCurrent(Dictionary *dictionary)
{
  Dictionary *d = dictionary->addIncludeDictionary("current", "_current.html");
  QList<Activity> activities = Activity::findCurrent();
  if (activities.count() > 0) {
    d->addActivitySection(activities);
  }
  else {
    d->showSection("noCurrentActivity");
  }
}

void Controller::includeToday(Dictionary *dictionary)
{
  Dictionary *d = dictionary->addIncludeDictionary("today", "_today.html");
  QList<Activity> activities = Activity::findToday();
  includeActivities(d, activities);
}

void Controller::includeWeek(Dictionary *dictionary)
{
  Dictionary *d = dictionary->addIncludeDictionary("week", "_week.html");

  // Start day at Sunday
  QDate day;
  QDate today = QDate::currentDate();
  int dayOfWeek = today.dayOfWeek();
  if (dayOfWeek != Qt::Sunday) {
    day = today.addDays(-dayOfWeek);
  }
  else {
    day = today;
  }

  Dictionary *d2;
  for (int i = 0; i < 7; i++) {
    d2 = d->addSectionDictionary("day");
    QList<Activity> activities = Activity::findDay(day);

    dayOfWeek = day.dayOfWeek();
    d2->setValue("dayName", QDate::longDayName(dayOfWeek));
    d2->setValue("dayNumber", i);
    includeActivities(d2, activities);

    day = day.addDays(1);
  }
}

void Controller::includeTotals(Dictionary *dictionary, bool addIncludeDictionary)
{
  Dictionary *d;
  if (addIncludeDictionary) {
    d = dictionary->addIncludeDictionary("totals", "_totals.html");
  }
  else {
    d = dictionary;
  }

  QList<Activity> activities = Activity::findToday();
  QMap<QString, int> projectTotals = Activity::projectTotals(activities);

  Dictionary *d2;
  QMapIterator<QString, int> i(projectTotals);
  while (i.hasNext()) {
    i.next();
    d2 = d->addSectionDictionary("total");
    d2->setValue("projectName", i.key().isEmpty() ? "Unsorted" : i.key());
    d2->setValue("duration", i.value());
    d2->setValue("durationInWords",
        QString("%1").arg(((double) i.value()) / 3600.0, 4, 'f', 2, '0'));
    d2->setValue("comma", i.hasNext() ? "," : "");
  }
}

QString Controller::partialTotals()
{
  View view("_totals.html", false);
  Dictionary *dictionary = view.dictionary();
  includeTotals(dictionary, false);
  return view.render();
}

QString Controller::partialUpdates()
{
  View view("_updates.js", false);
  Dictionary *dictionary = view.dictionary();
  includeCurrent(dictionary);
  includeToday(dictionary);
  includeWeek(dictionary);
  includeTotals(dictionary);
  return view.render();
}

QString Controller::partialNames(QList<QString> &names)
{
  View view("_names.js", false);
  Dictionary *d = view.dictionary();

  Dictionary *d2;
  QListIterator<QString> i(names);
  while (i.hasNext()) {
    QString name = i.next();
    d2 = d->addSectionDictionary("name");
    d2->setValue("value", name);
    d2->setValue("comma", i.hasNext() ? "," : "");
  }

  return view.render();
}

QString Controller::partialActivityNames()
{
  QList<QString> distinctNames = Activity::distinctNames();
  return partialNames(distinctNames);
}

QString Controller::partialProjectNames()
{
  QList<QString> distinctNames = Project::distinctNames();
  return partialNames(distinctNames);
}

QString Controller::partialTagNames()
{
  QList<QString> distinctNames = Tag::distinctNames();
  return partialNames(distinctNames);
}

// GET /
QString Controller::index()
{
  View view("index.html");
  view.addJavascript("/js/index.js");
  Dictionary *dictionary = view.dictionary();
  includeCurrent(dictionary);
  includeToday(dictionary);
  includeWeek(dictionary);
  includeTotals(dictionary);
  return view.render();
}

// POST /activities
QString Controller::createActivity(const QList<QPair<QString, QString> > &params)
{
  Activity activity;
  activity.setStartedAt(QDateTime::currentDateTime());
  activity.setFromParams(params);
  if (activity.save()) {
    return partialUpdates();
  }
  return QString("{\"errors\": \"There were errors!\"}");
}

// GET /activities/current/stop
QString Controller::stopCurrentActivities()
{
  Activity::stopCurrent();
  return partialUpdates();
}

// GET /activities/new
QString Controller::newActivity()
{
  View view("popup.html");
  view.setTitle("Add earlier activity");
  Dictionary *dictionary = view.dictionary();

  QDateTime now = QDateTime::currentDateTime();
  Activity activity = Activity();
  activity.setStartedAt(now);
  activity.setEndedAt(now);
  activity.setRunning(true);
  dictionary->setValue("submitUrl", "/activities");
  dictionary->addActivitySection(activity);
  return view.render();
}

// GET /activities/1/edit
QString Controller::editActivity(int activityId)
{
  View view("popup.html");
  view.setTitle("Edit activity");
  Dictionary *dictionary = view.dictionary();

  QDateTime now = QDateTime::currentDateTime();
  Activity activity = Activity::findById(activityId);
  if (activity.isNew()) {
    return QString();
  }
  dictionary->setValue("submitUrl", QString("/activities/%1").arg(activityId));
  dictionary->addActivitySection(activity);
  return view.render();
}

// POST /activities/1
QString Controller::updateActivity(int activityId, const QList<QPair<QString, QString> > &params)
{
  Activity activity = Activity::findById(activityId);
  if (activity.isNew()) {
    return QString();
  }
  activity.setFromParams(params);
  if (activity.save()) {
    return partialUpdates();
  }
  return QString("{\"errors\": \"There were errors!\"}");
}

// GET /activities/1/delete
QString Controller::deleteActivity(int activityId)
{
  Activity activity = Activity::findById(activityId);
  if (activity.isNew()) {
    return QString();
  }
  if (activity.destroy()) {
    return partialUpdates();
  }
  return QString("{\"errors\": \"There were errors!\"}");
}

bool Controller::pathMatches(const QString &path, const QString &pattern, QStringList &matchData)
{
  QRegExp rx(pattern);
  if (path.contains(rx)) {
    //qDebug() << "Matched path:" << rx.cap(0);
    matchData << rx.capturedTexts();
    return true;
  }
  else {
    //qDebug() << "Path (" << path << ") didn't match pattern:" << pattern;
    return false;
  }
}

void Controller::serveFile(const QString &path)
{
  QFile file(m_root.absolutePath() + path);
  if (file.exists()) {
    //qDebug() << "Serving file:" << file.fileName();
    QFileInfo info(file);
    QString contentType("text/plain");
    if (info.suffix() == "css") {
      contentType = "text/css";
    }
    else if (info.suffix() == "js") {
      contentType = "text/javascript";
    }
    else if (info.suffix() == "png") {
      contentType = "image/png";
    }
    else if (info.suffix() == "gif") {
      contentType = "image/gif";
    }
    m_resp->setHeader("Content-Type", contentType);
    m_resp->setHeader("Content-Length", QString("%1").arg(file.size()));
    m_resp->writeHead(200);

    file.open(QIODevice::ReadOnly);
    m_resp->write(file.readAll());
    file.close();
  }
  else {
    qDebug() << "File doesn't exist:" << file.fileName();
    m_resp->writeHead(404);
  }
  m_resp->end();
}
