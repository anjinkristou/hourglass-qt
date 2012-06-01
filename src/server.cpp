#include <QtDebug>
#include <QByteArray>
#include <QUrl>
#include <QDir>
#include <QStringList>
#include "server.h"
#include "project.h"
#include "tag.h"
#include "view.h"
#include "variables.h"

#include <iostream>
using namespace std;

const QRegExp Server::editActivityPath = QRegExp("^/activities/(\\d+)/edit$");
const QRegExp Server::activityPath = QRegExp("^/activities/(\\d+)$");
const QRegExp Server::deleteActivityPath = QRegExp("^/activities/(\\d+)/delete$");
const QRegExp Server::restartActivityPath = QRegExp("^/activities/(\\d+)/restart$");

Server::Server(QString port)
{
  this->port = port;
  ctx = NULL;
}

void *Server::route(enum mg_event event, struct mg_connection *conn, const struct mg_request_info *request_info)
{
  if (event != MG_NEW_REQUEST) {
    return NULL;
  }

  Server *s = (Server *) request_info->user_data;
  QUrl url(request_info->uri);
  QString path = url.path();
  QString method(request_info->request_method);

  QString result;
  bool isJSON = false;
  if (method == "GET") {
    if (path == "/") {
      result = s->index();
    }
    else if (path == "/activities/new") {
    }
    else if (path.contains(editActivityPath)) {
    }
    else if (path.contains(deleteActivityPath)) {
    }
    else if (path == "/activities") {
      result = s->partialActivityNames();
      isJSON = true;
    }
    else if (path == "/activities/current/stop") {
      result = s->stopCurrentActivities();
      isJSON = true;
    }
    else if (path == "/tags") {
      result = s->partialTagNames();
      isJSON = true;
    }
    else if (path == "/projects") {
      result = s->partialProjectNames();
      isJSON = true;
    }
    else if (path == "/totals") {
      result = s->partialTotals();
      isJSON = true;
    }
  }
  else if (method == "POST") {
    const char *tmp = mg_get_header(conn, "Content-Length");
    if (tmp != NULL) {
      QString contentLengthHeader = QString(tmp);
      bool ok;
      int contentLength = contentLengthHeader.toInt(&ok);
      if (ok) {
        //qDebug() << "POST Content-Length:" << contentLength;
        char *buffer = new char[contentLength + 1];
        if (mg_read(conn, buffer, contentLength) == contentLength) {
          // Valid POST
          buffer[contentLength] = 0;
          //qDebug() << "POST Content:" << buffer;
          QList<QPair<QString, QString> > params = decodePost(buffer);

          if (path == "/activities") {
            result = s->createActivity(params);
            isJSON = true;
          }
          else if (path.contains(activityPath)) {
          }
          else if (path.contains(restartActivityPath)) {
          }
        }
        delete[] buffer;
      }
    }
  }

  if (!result.isNull()) {
    QByteArray ba = result.toUtf8();
    mg_printf(conn,
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "\r\n",
        isJSON ? "application/json" : "text/html",
        ba.size());
    mg_write(conn, ba.data(), ba.size());

    return (void *) "";  // Mark as processed
  }

  // Let mongoose handle the request from the document root
  return NULL;
}

QList<QPair<QString, QString> > Server::decodePost(const char *data)
{
  QList<QPair<QString, QString> > result;

  QStringList parts = QString(data).split('&');
  for (int i = 0; i < parts.size(); i++) {
    QString variable = parts[i];
    QStringList variableParts = variable.split('=');
    if (variableParts.size() == 2) {
      QString &key = variableParts[0].replace("+", " ");
      QString &value = variableParts[1].replace("+", " ");
      QByteArray keyBA = key.toUtf8();
      QByteArray valueBA = value.toUtf8();
      QPair<QString, QString> pair(
          QUrl::fromPercentEncoding(keyBA),
          QUrl::fromPercentEncoding(valueBA));
      //qDebug() << pair;
      result << pair;
    }
  }
  return result;
}

QString &Server::toJSON(QString &str)
{
  str.replace("\\", "\\\\");
  str.replace("\"", "\\\"");
  str.replace("\n", "\\n");
  str.replace("\f", "\\f");
  str.replace("\b", "\\b");
  str.replace("\r", "\\r");
  str.replace("\t", "\\t");
  return str;
}

bool Server::start()
{
  QByteArray ba = port.toLocal8Bit();
  const char *options[] = {
    "listening_ports", ba.data(),
    "document_root", PUBLIC_PATH,
    NULL};

  ctx = mg_start(&route, this, options);
  if (ctx == NULL) {
    return false;
  }
  return true;
}

bool Server::stop()
{
  if (ctx != NULL) {
    mg_stop(ctx);
    ctx = NULL;
  }
}

// Partials
QString Server::partialActivities(QList<Activity> activities)
{
  View view("_activities.html", false);
  VariableMap variables(&view);
  variables.addActivities(activities);
  return view.render(variables);
}

QString Server::partialCurrent()
{
  QList<Activity> activities = Activity::findCurrent();
  View view("_current.html", false);
  VariableMap variables(&view);
  variables.addActivities(activities);
  return view.render(variables);
}

QString Server::partialToday()
{
  QList<Activity> activities = Activity::findToday();
  View view("_today.html", false);
  VariableMap variables(&view);
  variables.addVariable("activities", partialActivities(activities));
  return view.render(variables);
}

QString Server::partialWeek()
{
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

  View view("_week.html", false);
  VariableMap variables(&view);
  VariableMapList &days = variables.addMapList("days");
  for (int i = 0; i < 7; i++) {
    QList<Activity> activities = Activity::findDay(day);
    VariableMap &map = days.addMap();

    dayOfWeek = day.dayOfWeek();
    map.addVariable("dayName", QDate::longDayName(dayOfWeek));
    map.addVariable("activities", partialActivities(activities));
    day = day.addDays(1);
  }
  return view.render(variables);
}

QString Server::partialTotals()
{
  QList<Activity> activities = Activity::findToday();
  QMap<QString, int> projectTotals = Activity::projectTotals(activities);

  View view("_totals.html", false);
  VariableMap variables(&view);
  VariableMapList &totals = variables.addMapList("totals");
  QMapIterator<QString, int> i(projectTotals);
  while (i.hasNext()) {
    i.next();
    VariableMap &map = totals.addMap();
    map.addVariable("projectName", i.key());
    map.addVariable("duration", i.value());
    map.addVariable("durationInWords",
        QString("%1").arg(((double) i.value()) / 3600.0, 4, 'f', 2, '0'));
  }
  return view.render(variables);
}

QString Server::partialUpdates()
{
  QString current = partialCurrent();
  QString today = partialToday();
  QString week = partialWeek();
  QString totals = partialTotals();

  View view("_updates.js", false);
  VariableMap variables(&view);
  variables.addVariable("current", toJSON(current));
  variables.addVariable("today", toJSON(today));
  variables.addVariable("week", toJSON(week));
  variables.addVariable("totals", toJSON(totals));
  return view.render(variables);
}

QString Server::partialActivityNames()
{
  QList<QString> distinctNames = Activity::distinctNames();
  View view("_names.js", false);
  VariableMap variables(&view);
  VariableMapList &names = variables.addMapList("names");
  for (int i = 0; i < distinctNames.size(); i++) {
    VariableMap &map = names.addMap();
    map.addVariable("name", distinctNames.at(i));
  }
  return view.render(variables);
}

QString Server::partialProjectNames()
{
  QList<QString> distinctNames = Project::distinctNames();
  View view("_names.js", false);
  VariableMap variables(&view);
  VariableMapList &names = variables.addMapList("names");
  for (int i = 0; i < distinctNames.size(); i++) {
    VariableMap &map = names.addMap();
    map.addVariable("name", distinctNames.at(i));
  }
  return view.render(variables);
}

QString Server::partialTagNames()
{
  QList<QString> distinctNames = Tag::distinctNames();
  View view("_names.js", false);
  VariableMap variables(&view);
  VariableMapList &names = variables.addMapList("names");
  for (int i = 0; i < distinctNames.size(); i++) {
    VariableMap &map = names.addMap();
    map.addVariable("name", distinctNames.at(i));
  }
  return view.render(variables);
}

// GET /
QString Server::index()
{
  QString current = partialCurrent();
  QString today = partialToday();
  QString week = partialWeek();
  QString totals = partialTotals();

  View view("index.html");
  VariableMap variables(&view);
  variables.addVariable("current", current);
  variables.addVariable("today", today);
  variables.addVariable("week", week);
  variables.addVariable("totals", totals);
  return view.render(variables);
}

// POST /activities
QString Server::createActivity(const QList<QPair<QString, QString> > &params)
{
  if (Activity::createFromParams(params)) {
    return partialUpdates();
  }
  return QString("{\"errors\": \"There were errors!\"}");
}

// GET /activities/current/stop
QString Server::stopCurrentActivities()
{
  Activity::stopCurrent();
  return partialUpdates();
}
