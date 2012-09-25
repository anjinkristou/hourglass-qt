include(../hourglass.pri)
include(ui/ui.pri)
include(icons/icons.pri)
#include(public/public.pri)
#include(views/views.pri)

TARGET = hourglass

TEMPLATE = app
DESTDIR = ../bin

QT += sql
CONFIG += release
DEFINES += QT_NO_DEBUG_OUTPUT

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc
UI_DIR = .ui

SOURCES += mainwindow.cpp hourglass.cpp abstractactivitymodel.cpp activitytablemodel.cpp activitydelegate.cpp activitytableview.cpp abstractlineedit.cpp activitylineedit.cpp activitydialog.cpp record.cpp project.cpp tag.cpp activity.cpp currentactivitytablemodel.cpp currentactivitydelegate.cpp currentactivitytableview.cpp recordmanager.cpp activitynameslistmodel.cpp namesdelegate.cpp tagnameslistmodel.cpp tagslineedit.cpp aboutdialog.cpp preferencesdialog.cpp projecttotalslistmodel.cpp projecttotaldelegate.cpp projecttotalslistview.cpp main.cpp
HEADERS += mainwindow.h hourglass.h abstractactivitymodel.h activitytablemodel.h activitydelegate.h activitytableview.h abstractlineedit.h activitylineedit.h activitydialog.h record.h project.h tag.h activity.h currentactivitytablemodel.h currentactivitydelegate.h currentactivitytableview.h recordmanager.h activitynameslistmodel.h namesdelegate.h tagnameslistmodel.h tagslineedit.h aboutdialog.h preferencesdialog.h projecttotalslistmodel.h projecttotaldelegate.h projecttotalslistview.h

target.path = $${PREFIX}/bin
INSTALLS += target
