#include "activitydialog.h"

ActivityDialog::ActivityDialog(QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f), m_errorStartedAt(true), m_errorEndedAt(true)
{
  m_ui.setupUi(this);
  setupFrames();

  QDateTime now = QDateTime::currentDateTime();
  m_ui.deStartedAt->setDate(now.date());
  m_ui.teStartedAt->setTime(now.time());
  m_ui.deEndedAt->setDate(now.date());
  m_ui.teEndedAt->setTime(now.time());
  m_ui.deUntimedDate->setDate(now.date());
}

ActivityDialog::ActivityDialog(QSharedPointer<Activity> activity, QWidget *parent, Qt::WindowFlags f)
  : QDialog(parent, f), m_activity(activity), m_errorStartedAt(true), m_errorEndedAt(true)
{
  m_ui.setupUi(this);
  setWindowTitle("Edit activity");
  setupFrames();

  if (activity->isUntimed()) {
    QDate date = activity->day();
    m_ui.deStartedAt->setDate(date);
    m_ui.deEndedAt->setDate(date);
    m_ui.deUntimedDate->setDate(date);
    m_ui.tabWidget->setCurrentIndex(1);
  }
  else {
    QDateTime startedAt = activity->startedAt();
    m_ui.deStartedAt->setDate(startedAt.date());
    m_ui.teStartedAt->setTime(startedAt.time());
    m_ui.deUntimedDate->setDate(startedAt.date());

    if (activity->isRunning()) {
      QDateTime now = QDateTime::currentDateTime();
      m_ui.deEndedAt->setDate(now.date());
      m_ui.deEndedAt->setEnabled(false);
      m_ui.teEndedAt->setTime(now.time());
      m_ui.teEndedAt->setEnabled(false);
      m_ui.cbInProgress->setChecked(true);
    }
    else {
      QDateTime endedAt = activity->endedAt();
      m_ui.deEndedAt->setDate(endedAt.date());
      m_ui.teEndedAt->setTime(endedAt.time());
    }
    m_ui.tabWidget->setCurrentIndex(0);
  }

  m_ui.leActivity->setText(activity->nameWithProject());
  m_ui.leUntimedActivity->setText(activity->nameWithProject());
  m_ui.leTags->setText(activity->tagNames());
  m_ui.leUntimedTags->setText(activity->tagNames());

  int minutes = activity->duration() / 60;
  m_ui.sbUntimedHours->setValue(minutes / 60);
  m_ui.sbUntimedMinutes->setValue(minutes % 60);
}

QSharedPointer<Activity> ActivityDialog::activity() const
{
  return m_activity;
}

void ActivityDialog::setActivityCompleter(QCompleter *completer)
{
  m_ui.leActivity->setCompleter(completer);
  m_ui.leUntimedActivity->setCompleter(completer);
}

void ActivityDialog::setTagCompleter(QCompleter *completer)
{
  m_ui.leTags->setCompleter(completer);
  m_ui.leUntimedTags->setCompleter(completer);
}

void ActivityDialog::accept()
{
  if (m_activity.isNull()) {
    Activity *activity = new Activity();
    m_activity = QSharedPointer<Activity>(activity, &QObject::deleteLater);
  }

  if (m_ui.tabWidget->currentIndex() == 0) {
    /* Timed activity */
    m_activity->setNameWithProject(m_ui.leActivity->text());
    m_activity->setTagNames(m_ui.leTags->text());

    m_activity->setStartedAt(QDateTime(m_ui.deStartedAt->date(),
          m_ui.teStartedAt->time()));

    if (m_ui.cbInProgress->isChecked()) {
      m_activity->setRunning(true);
      m_activity->setEndedAt(QDateTime());
    }
    else {
      m_activity->setRunning(false);
      m_activity->setEndedAt(QDateTime(m_ui.deEndedAt->date(),
            m_ui.teEndedAt->time()));
    }
  }
  else {
    /* Untimed activity */
    m_activity->setNameWithProject(m_ui.leUntimedActivity->text());
    m_activity->setTagNames(m_ui.leUntimedTags->text());
    m_activity->setUntimed(true);
    m_activity->setDay(m_ui.deUntimedDate->date());
    m_activity->setDuration(m_ui.sbUntimedHours->value() * 3600 +
        m_ui.sbUntimedMinutes->value() * 60);
  }

  if (m_activity->isValid()) {
    /* Let the parent widget save the activity */
    QDialog::accept();
  }
  else {
    qDebug() << "Activity was invalid!";
    m_activity->reset();
  }
}

void ActivityDialog::on_cbInProgress_clicked()
{
  bool checked = m_ui.cbInProgress->isChecked();
  m_ui.deEndedAt->setDisabled(checked);
  m_ui.teEndedAt->setDisabled(checked);
}

void ActivityDialog::on_deStartedAt_dateChanged(const QDate &date)
{
  if (m_errorStartedAt) {
    setErrorStartedAt(isTimeValid());
  }
  else {
    setErrorEndedAt(isTimeValid());
  }
}

void ActivityDialog::on_teStartedAt_timeChanged(const QTime &time)
{
  if (m_errorStartedAt) {
    setErrorStartedAt(isTimeValid());
  }
  else {
    setErrorEndedAt(isTimeValid());
  }
}

void ActivityDialog::on_deEndedAt_dateChanged(const QDate &date)
{
  if (m_errorEndedAt) {
    setErrorEndedAt(isTimeValid());
  }
  else {
    setErrorStartedAt(isTimeValid());
  }
}

void ActivityDialog::on_teEndedAt_timeChanged(const QTime &time)
{
  if (m_errorEndedAt) {
    setErrorEndedAt(isTimeValid());
  }
  else {
    setErrorStartedAt(isTimeValid());
  }
}

void ActivityDialog::setupFrames()
{
  m_frStartedAtPalette = m_ui.frStartedAt->palette();
  m_frEndedAtPalette = m_ui.frEndedAt->palette();
  setErrorStartedAt(false);
  setErrorEndedAt(false);
}

QDateTime ActivityDialog::startedAt()
{
  return QDateTime(m_ui.deStartedAt->date(), m_ui.teStartedAt->time());
}

QDateTime ActivityDialog::endedAt()
{
  return QDateTime(m_ui.deEndedAt->date(), m_ui.teEndedAt->time());
}

bool ActivityDialog::isTimeValid()
{
  return startedAt() > endedAt();
}

void ActivityDialog::setErrorStartedAt(bool toggle)
{
  if (toggle != m_errorStartedAt) {
    m_errorStartedAt = toggle;
    m_frStartedAtPalette.setColor(QPalette::Foreground, (toggle ? Qt::red : Qt::transparent));
    m_ui.frStartedAt->setPalette(m_frStartedAtPalette);
  }
}

void ActivityDialog::setErrorEndedAt(bool toggle)
{
  if (toggle != m_errorEndedAt) {
    m_errorEndedAt = toggle;
    m_frEndedAtPalette.setColor(QPalette::Foreground, (toggle ? Qt::red : Qt::transparent));
    m_ui.frEndedAt->setPalette(m_frEndedAtPalette);
  }
}
