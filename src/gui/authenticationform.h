#ifndef AUTHENTICATIONFORM_H
#define AUTHENTICATIONFORM_H
#include "user.h"
#include "ui_authenticationform.h"

class AuthenticationForm : public QDialog, public Ui::AuthenticationForm
{
	Q_OBJECT

public:
	AuthenticationForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	~AuthenticationForm();

	virtual int exec( t_user * user_config, const QString & realm, QString & username, QString & password );

protected slots:
	virtual void languageChange();

};

#endif
