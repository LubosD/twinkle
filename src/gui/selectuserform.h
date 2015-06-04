#ifndef SELECTUSERFORM_H
#define SELECTUSERFORM_H
class t_phone;
extern t_phone *phone;

#include "gui.h"
#include "phone.h"
#include "user.h"
#include "ui_selectuserform.h"

class SelectUserForm : public QDialog, public Ui::SelectUserForm
{
	Q_OBJECT

public:
    SelectUserForm(QWidget* parent = 0);
	~SelectUserForm();

public slots:
	virtual void show( t_select_purpose purpose );
	virtual void validate();
	virtual void selectAll();
	virtual void clearAll();
    virtual void toggle( QModelIndex item );

signals:
	void selection(list<t_user *>);
	void not_selected(list<t_user*>);

protected slots:
	virtual void languageChange();

private:
	void init();

};


#endif
