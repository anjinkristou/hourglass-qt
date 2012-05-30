#include <QByteArray>
#include <QUrl>
#include <QDir>
#include "server.h"
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
    }
    else if (path == "/activities/current/stop") {
    }
    else if (path == "/tags") {
    }
    else if (path == "/projects") {
    }
  }
  else if (method == "POST") {
    if (path == "/activities") {
    }
    else if (path.contains(activityPath)) {
    }
    else if (path.contains(restartActivityPath)) {
    }
  }

  if (!result.isNull()) {
    QByteArray ba = result.toLocal8Bit();
    mg_printf(conn,
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s", ba.size(), ba.data());

    return (void *) "";  // Mark as processed
  }

  // Let mongoose handle the request from the document root
  return NULL;
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
  QDate sunday;
  QDate today = QDate::currentDate();
  int dayOfWeek = today.dayOfWeek();
  if (dayOfWeek != Qt::Sunday) {
    sunday = today.addDays(-dayOfWeek);
  }
  else {
    sunday = today;
  }

  // TODO
  View view("_week.html", false);
}

QString Server::partialActivities(QList<Activity> activities)
{
  View view("_activities.html", false);
  VariableMap variables(&view);
  variables.addActivities(activities);
  return view.render(variables);
}

// GET /
QString Server::index()
{
  QString current = partialCurrent();
  QString today = partialToday();

  View view("index.html");
  VariableMap variables(&view);
  variables.addVariable("current", current);
  variables.addVariable("today", today);
  return view.render(variables);
}
