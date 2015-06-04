#ifndef ADDRESSCARDFORM_H
#define ADDRESSCARDFORM_H
#include "address_book.h"
#include "ui_addresscardform.h"

class AddressCardForm : public QDialog, public Ui::AddressCardForm
{
	Q_OBJECT

public:
    AddressCardForm(QWidget* parent = 0);
	~AddressCardForm();

	virtual int exec( t_address_card & card );

public slots:
	virtual void validate();

protected slots:
	virtual void languageChange();

};

#endif
