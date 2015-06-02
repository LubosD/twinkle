#ifndef HISTORYFORM_H
#define HISTORYFORM_H
#include "phone.h"
#include <Qt3Support/Q3PopupMenu>
#include "user.h"
#include "ui_historyform.h"

class HistoryForm : public QDialog, public Ui::HistoryForm
{
	Q_OBJECT

public:
	HistoryForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	~HistoryForm();

public slots:
	virtual void loadHistory();
	virtual void update();
	virtual void show();
	virtual void closeEvent( QCloseEvent * e );
	virtual void showCallDetails( Q3ListViewItem * item );
	virtual void popupMenu( Q3ListViewItem * item, const QPoint & pos );
	virtual void call( Q3ListViewItem * item );
	virtual void call( void );
	virtual void deleteEntry( void );
	virtual void clearHistory();

signals:
	void call(t_user *, const QString &, const QString &, bool);

protected slots:
	virtual void languageChange();

private:
	time_t timeLastViewed;
	Q3PopupMenu *histPopupMenu;
	int itemCall;

	void init();
	void destroy();

};


#endif
