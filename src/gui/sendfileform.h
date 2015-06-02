#ifndef SENDFILEFORM_H
#define SENDFILEFORM_H
#include "ui_sendfileform.h"

class SendFileForm : public QDialog, public Ui::SendFileForm
{
	Q_OBJECT

public:
	SendFileForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	~SendFileForm();

public slots:
	virtual void signalSelectedInfo();
	virtual void chooseFile();
	virtual void setFilename();

signals:
	void selected(const QString &filename, const QString &subject);

protected slots:
	virtual void languageChange();

private:
	QDialog *_chooseFileDialog;

	void init();
	void destroy();

};


#endif
