#ifndef GETADDRESSFORM_UI_H
#define GETADDRESSFORM_UI_H
#include "ui_getaddressform.h"
#include "user.h"

class AddressTableModel;

class GetAddressForm : public QDialog, public Ui::GetAddressForm
{
Q_OBJECT
public:
	GetAddressForm(QWidget *parent, const char *name, bool modal);
	~GetAddressForm();
private:
	void init();
	// void destroy();
public slots:
	void reload();
	void show();
	void loadAddresses();
	void selectAddress();
	void selectKABCAddress();
	void selectLocalAddress();
	void toggleSipOnly( bool on );
	void addLocalAddress();
	void deleteLocalAddress();
	void editLocalAddress();

signals:
	void address(const QString &, const QString &);
	void address(const QString &);
private:
	void *addrBook;
	AddressTableModel* m_model;
};

#endif
