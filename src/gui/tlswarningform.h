#ifndef TLSWARNINGFORM_H
#define TLSWARNINGFORM_H
#include "ui_tlswarningform.h"

class TLSWarningForm : public QDialog, public Ui::TLSWarningForm
{
Q_OBJECT
public:
	TLSWarningForm(QWidget* parent = nullptr);

	static const int AcceptAlways = QDialog::Accepted + 1;

private slots:
	void acceptAlwaysPressed();
};

#endif // TLSWARNINGFORM_H
