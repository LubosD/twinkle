#ifndef LOGVIEWFORM_H
#define LOGVIEWFORM_H

#include <QDialog>
#include <QCloseEvent>
#include <Q3TextStream>

#include "ui_logviewform.h"

class LogViewForm : public QDialog, protected Ui::LogViewForm
{
    Q_OBJECT

private:
    QFile* logfile;
    Q3TextStream* logstream;

public:
    LogViewForm(QWidget* parent = 0);
    ~LogViewForm();

public slots:
    void show();
    void closeEvent(QCloseEvent* ev);
    void update(bool log_zapped);
    void clear();

};

#endif // LOGVIEWFORM_H
