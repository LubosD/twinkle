#ifndef REDIRECTFORM_UI_H
#define REDIRECTFORM_UI_H
#include "ui_redirectform.h"
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <list>
#include "getaddressform.h"
#include "sockets/url.h"
#include "user.h"


class RedirectForm : public QDialog, public Ui::RedirectForm
{
	Q_OBJECT

public:
	RedirectForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	~RedirectForm();

public slots:
	virtual void show( t_user * user, const list<string> & contacts );
	virtual void validate();
	virtual void showAddressBook();
	virtual void showAddressBook1();
	virtual void showAddressBook2();
	virtual void showAddressBook3();
	virtual void selectedAddress( const QString & address );

signals:
	void destinations(const list<t_display_url> &);

protected slots:
	virtual void languageChange();

private:
	GetAddressForm *getAddressForm;
	int nrAddressBook;
	t_user *user_config;

	void init();
	void destroy();

};


#endif
