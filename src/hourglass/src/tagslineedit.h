#ifndef __TAGSLINEEDIT_H
#define __TAGSLINEEDIT_H

#include "abstractlineedit.h"

class TagsLineEdit : public AbstractLineEdit
{
  Q_OBJECT

  public:
    TagsLineEdit(QWidget *parent = 0);
    TagsLineEdit(const QString &contents, QWidget *parent = 0);

  protected:
    virtual QRect currentWordBoundaries() const;
};

#endif
