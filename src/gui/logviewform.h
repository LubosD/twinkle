#ifndef LOGVIEWFORM_H
#define LOGVIEWFORM_H

#include <QDialog>
#include <QCloseEvent>
#include <QTextStream>

#include "ui_logviewform.h"

class LogViewForm : public QDialog, protected Ui::LogViewForm
{
    Q_OBJECT

private:
    QFile* logfile;
    QTextStream* logstream;

    void scrollToBottom();

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
