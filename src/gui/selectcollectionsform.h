#ifndef SELECTCOLLECTIONSFORM_H
#define SELECTCOLLECTIONSFORM_H

#include <AkonadiCore/Collection>
#include <QList>
#include "ui_selectcollectionsform.h"

class SelectCollectionsForm : public QDialog, public Ui::SelectCollectionsForm
{
	Q_OBJECT

public:
    SelectCollectionsForm(QWidget* parent = 0);
	~SelectCollectionsForm();
	QList<int> selected_list;
	unsigned int interval();

public slots:
	virtual void exec(const Akonadi::Collection::List& collections);
	virtual void validate();
	virtual void selectAll();
	virtual void clearAll();
	virtual void toggle( QModelIndex item );

	/*
signals:
	void selection(QList<int>);
	*/

protected slots:
	virtual void languageChange();

private:
	void init();

};


#endif
