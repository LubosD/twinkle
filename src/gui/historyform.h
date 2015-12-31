#ifndef HISTORYFORM_H
#define HISTORYFORM_H
#include "phone.h"
#include <QMenu>
#include <QStandardItemModel>
#include <QPixmap>
#include "user.h"
#include "ui_historyform.h"

class HistoryForm : public QDialog, public Ui::HistoryForm
{
	Q_OBJECT

public:
    HistoryForm(QWidget* parent = 0);
	~HistoryForm();

public slots:
	virtual void loadHistory();
	virtual void update();
	virtual void show();
	virtual void closeEvent( QCloseEvent * e );
    virtual void popupMenu( QPoint pos );
    virtual void call( QModelIndex index );
	virtual void call( void );
	virtual void deleteEntry( void );
	virtual void clearHistory();

signals:
	void call(t_user *, const QString &, const QString &, bool);

protected slots:
	virtual void languageChange();

private:
	time_t timeLastViewed;
    QMenu *histPopupMenu;
    QStandardItemModel *m_model;
    QAction* itemCall;
    QPixmap m_pixmapIn, m_pixmapOut;
    QPixmap m_pixmapOk, m_pixmapCancel;
    QList<t_call_record> m_history;

	void init();
	void destroy();

private slots:
	void showCallDetails(const QModelIndex &);
};


#endif
