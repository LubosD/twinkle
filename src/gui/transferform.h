#ifndef TRANSFERFORM_H
#define TRANSFERFORM_H
class t_phone;
extern t_phone *phone;

#include "getaddressform.h"
#include "phone.h"
#include "protocol.h"
#include <QtCore/QStringRef>
#include "sockets/url.h"
#include "user.h"
#include "ui_transferform.h"

class TransferForm : public QDialog, public Ui::TransferForm
{
	Q_OBJECT

public:
	TransferForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	~TransferForm();

public slots:
	virtual void initTransferOptions();
	virtual void show( t_user * user );
	virtual void show( t_user * user, const string & dest, t_transfer_type transfer_type );
	virtual void hide();
	virtual void reject();
	virtual void validate();
	virtual void closeEvent( QCloseEvent * );
	virtual void showAddressBook();
	virtual void selectedAddress( const QString & address );
	virtual void setOtherLineAddress( bool on );

signals:
	void destination(const t_display_url&, t_transfer_type);

protected slots:
	virtual void languageChange();

private:
	int consult_line;
	t_user *user_config;
	GetAddressForm *getAddressForm;
	QString previousAddress;

	void init();
	void destroy();

};


#endif
