#ifndef SELECTNICFORM_H
#define SELECTNICFORM_H
#include <QDialog>
#include "ui_selectnicform.h"

class SelectNicForm : public QDialog, public Ui::SelectNicForm
{
	Q_OBJECT

public:
    SelectNicForm(QWidget* parent = 0);
	~SelectNicForm();

public slots:
	virtual void setAsDefault( bool setIp );
	virtual void setAsDefaultIp();
	virtual void setAsDefaultNic();

protected slots:
	virtual void languageChange();

private:
	int idxDefault;

	void init();

};


#endif
