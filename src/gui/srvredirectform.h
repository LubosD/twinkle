#ifndef SRVREDIRECTFORM_H
#define SRVREDIRECTFORM_H
#include <list>
#include "getaddressform.h"
#include "phone.h"
#include <QLineEdit>
#include "sockets/url.h"
#include "user.h"
#include "ui_srvredirectform.h"

class t_phone;
extern t_phone *phone;

class SrvRedirectForm : public QDialog, public Ui::SrvRedirectForm
{
	Q_OBJECT

public:
    SrvRedirectForm(QWidget* parent = 0);
	~SrvRedirectForm();

	virtual bool validateValues();
	virtual bool validate( bool cf_active, QLineEdit * dst1, QLineEdit * dst2, QLineEdit * dst3, list<t_display_url> & dest_list );

public slots:
	virtual void show();
	virtual void populate();
	virtual void validate();
	virtual void toggleAlways( bool on );
	virtual void toggleBusy( bool on );
	virtual void toggleNoanswer( bool on );
	virtual void changedUser( const QString & user_display_uri );
	virtual void showAddressBook();
	virtual void showAddressBook1();
	virtual void showAddressBook2();
	virtual void showAddressBook3();
	virtual void showAddressBook4();
	virtual void showAddressBook5();
	virtual void showAddressBook6();
	virtual void showAddressBook7();
	virtual void showAddressBook8();
	virtual void showAddressBook9();
	virtual void selectedAddress( const QString & address );

signals:
	void destinations(t_user *, const list<t_display_url> &always, const list<t_display_url> &busy, const list<t_display_url> &noanswer);

protected slots:
	virtual void languageChange();

private:
	int nrAddressBook;
	GetAddressForm *getAddressForm;
	t_user *current_user;
	int current_user_idx;

	void init();
	void destroy();

};


#endif
