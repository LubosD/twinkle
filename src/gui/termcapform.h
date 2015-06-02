#ifndef TERMCAPFORM_H
#define TERMCAPFORM_H
#include "ui_termcapform.h"
#include "getaddressform.h"
#include "phone.h"
#include "sockets/url.h"
#include "user.h"

class t_phone;
extern t_phone *phone;

class TermCapForm : public QDialog, public Ui::TermCapForm
{
	Q_OBJECT

public:
	TermCapForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	~TermCapForm();

public slots:
	virtual void show( t_user * user_config, const QString & dest );
	virtual void validate();
	virtual void showAddressBook();
	virtual void selectedAddress( const QString & address );

signals:
	void destination(t_user *, const t_url &);

protected slots:
	virtual void languageChange();

private:
	GetAddressForm *getAddressForm;

	void init();
	void destroy();

};

#endif
