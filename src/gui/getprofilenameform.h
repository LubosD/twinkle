#ifndef GETPROFILENAMEFORM_H
#define GETPROFILENAMEFORM_H

#include <QDialog>
#include "ui_getprofilenameform.h"

class GetProfileNameForm : public QDialog, private Ui::GetProfileNameForm
{
    Q_OBJECT

public:
    GetProfileNameForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~GetProfileNameForm();

    QString getProfileName();
    int execNewName();
    int execRename( const QString & oldName );

public slots:
    void validate();

private:
    void init();
};

#endif // GETPROFILENAMEFORM_H
