#ifndef GETADDRESSFORM_UI_H
#define GETADDRESSFORM_UI_H

#include "twinkle_config.h"

#include "ui_getaddressform.h"
#include "user.h"

class AddressTableModel;
#ifdef HAVE_AKONADI
class KContactsTableModel;
#endif

class GetAddressForm : public QDialog, public Ui::GetAddressForm
{
Q_OBJECT
public:
    GetAddressForm(QWidget *parent);
	~GetAddressForm();
private:
	void init();
	// void destroy();
public slots:
	void reload();
	void synchronize();
	void show();
	void loadAddresses();
	void selectAddress();
	void selectAkonadiAddress();
	void selectLocalAddress();
	void toggleSipOnly( bool on );
	void addLocalAddress();
	void deleteLocalAddress();
	void editLocalAddress();

signals:
	void address(const QString &, const QString &);
	void address(const QString &);
private:
	AddressTableModel* m_model;
#ifdef HAVE_AKONADI
	KContactsTableModel* k_model;
#endif
};

#endif
