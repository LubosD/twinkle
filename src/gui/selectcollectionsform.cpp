#include <AkonadiCore/Collection>
#include <QListWidget>
#include "selectcollectionsform.h"

SelectCollectionsForm::SelectCollectionsForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

	init();
}

SelectCollectionsForm::~SelectCollectionsForm()
{
}

void SelectCollectionsForm::languageChange()
{
	retranslateUi(this);
}


void SelectCollectionsForm::init()
{
}

void SelectCollectionsForm::exec(const Akonadi::Collection::List& collections)
{
	for (const Akonadi::Collection &collection : collections)
	{
		QListWidgetItem* item = new QListWidgetItem(collection.name(), collectionsListView);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Unchecked);
	}

	QDialog::exec();
}

void SelectCollectionsForm::validate()
{
	selected_list.clear();

	for (int i = 0; i < collectionsListView->count(); i++)
	{
		QListWidgetItem *item = collectionsListView->item(i);
		if (item->checkState() == Qt::Checked)
			selected_list.append(i);
	}

	//emit (selection(selected_list));
	accept();
}

void SelectCollectionsForm::selectAll()
{
    for (int i = 0; i < collectionsListView->count(); i++)
    {
        QListWidgetItem *item = collectionsListView->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void SelectCollectionsForm::clearAll()
{
    for (int i = 0; i < collectionsListView->count(); i++)
    {
        QListWidgetItem *item = collectionsListView->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}

void SelectCollectionsForm::toggle(QModelIndex index)
{
    QListWidgetItem *item = collectionsListView->item(index.row());
    Qt::CheckState state = item->checkState();
    item->setCheckState((state == Qt::Checked) ? Qt::Unchecked : Qt::Checked);
}

unsigned int SelectCollectionsForm::interval()
{
	return intervalSpinBox->value() * 1000;
}
