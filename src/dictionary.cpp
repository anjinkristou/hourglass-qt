#include "dictionary.h"

Dictionary::Dictionary(QString name, QObject *parent)
  : QObject(parent)
{
  dictionary = new ctemplate::TemplateDictionary(name.toStdString());
  deleteDictionary = true;
}

Dictionary::Dictionary(ctemplate::TemplateDictionary *dictionary, QObject *parent)
  : QObject(parent), dictionary(dictionary)
{
  deleteDictionary = false;
}

Dictionary::~Dictionary()
{
  if (deleteDictionary) {
    delete dictionary;
  }
}

Dictionary *Dictionary::addSectionDictionary(QString name)
{
  ctemplate::TemplateDictionary *sectionDictionary =
    dictionary->AddSectionDictionary(name.toStdString());

  return new Dictionary(sectionDictionary, this);
}

Dictionary *Dictionary::addIncludeDictionary(QString name, QString templateName)
{
  ctemplate::TemplateDictionary *includeDictionary =
    dictionary->AddIncludeDictionary(name.toStdString());
  includeDictionary->SetFilename(templateName.toStdString());
  return new Dictionary(includeDictionary, this);
}

void Dictionary::setValue(QString name, QString value)
{
  dictionary->SetValue(name.toStdString(), value.toStdString());
}

void Dictionary::setValue(QString name, int value)
{
  dictionary->SetIntValue(name.toStdString(), value);
}

void Dictionary::showSection(QString name)
{
  dictionary->ShowSection(name.toStdString());
}

void Dictionary::addActivity(Activity &activity)
{
  setValue("id", activity.id());
  setValue("name", activity.name());
  setValue("startedAtISO8601", activity.startedAtISO8601());
  setValue("startedAtHM", activity.startedAtHM());
  setValue("startedAtMDY", activity.startedAtMDY());
  setValue("endedAtHM", activity.endedAtHM());
  setValue("endedAtMDY", activity.endedAtMDY());
  setValue("durationInWords", activity.durationInWords());
  setValue("nameWithProject", activity.nameWithProject());
  setValue("tagNames", activity.tagNames());

  QString projectName = activity.projectName();
  setValue("projectName", projectName.isEmpty() ? "unsorted" : projectName);

  if (activity.isRunning()) {
    showSection("running");
  }
  else {
    showSection("notRunning");
  }
}

void Dictionary::addActivitySection(Activity &activity)
{
  Dictionary *section = addSectionDictionary("activity");
  section->addActivity(activity);
}

void Dictionary::addActivitySection(QList<Activity> &activities)
{
  for (int i = 0; i < activities.size(); i++) {
    addActivitySection(activities[i]);
  }
}

ctemplate::TemplateDictionary *Dictionary::object()
{
  return dictionary;
}
