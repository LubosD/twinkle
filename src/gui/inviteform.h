#ifndef INVITEFORM_UI_H
#define INVITEFORM_UI_H
#include "ui_inviteform.h"
#include "sockets/url.h"
#include "getaddressform.h"
#include "user.h"
#include "phone.h"
#include <QDialog>

class t_phone;
extern t_phone *phone;

class InviteForm : public QDialog, public Ui::InviteForm
{
	Q_OBJECT
public:
	InviteForm(QWidget *parent, const char *name, bool modal);
	~InviteForm();
public slots:
	void clear();
	void show( t_user * user_config, const QString & dest, const QString & subject, bool anonymous );
	void validate();
	void addToInviteComboBox( const QString & destination );
	void reject();
	void closeEvent( QCloseEvent * );
	void showAddressBook();
	void selectedAddress( const QString & address );
	void warnHideUser( void );
signals:
	void destination(t_user *, const QString &, const t_url &, const QString &, bool);
	void raw_destination(const QString &);
private:
	void init();
	void destroy();

	GetAddressForm *getAddressForm;
};

#endif
