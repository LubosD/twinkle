#include "tlswarningform.h"

TLSWarningForm::TLSWarningForm(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	connect(pushAccept, SIGNAL(clicked()), this, SLOT(accept()));
	connect(pushReject, SIGNAL(clicked()), this, SLOT(reject()));
	connect(pushAcceptAlways, SIGNAL(clicked()), this, SLOT(acceptAlwaysPressed()));
}

void TLSWarningForm::acceptAlwaysPressed()
{
	setResult(AcceptAlways);
	hide();
}
