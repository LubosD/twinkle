#include "numberconversionform.h"

#include "gui.h"

/*
 *  Constructs a NumberConversionForm which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
NumberConversionForm::NumberConversionForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
NumberConversionForm::~NumberConversionForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void NumberConversionForm::init()
{
	QRegExp rxNoAtSign("[^@]*");

	exprLineEdit->setValidator(new QRegExpValidator(rxNoAtSign, this));
	replaceLineEdit->setValidator(new QRegExpValidator(rxNoAtSign, this));
}

int NumberConversionForm::exec(QString &expr, QString &replace)
{
	exprLineEdit->setText(expr);
	replaceLineEdit->setText(replace);
	int retval = QDialog::exec();

	if (retval == QDialog::Accepted) {
		expr = exprLineEdit->text();
		replace = replaceLineEdit->text();
	}

	return retval;
}

void NumberConversionForm::validate()
{
	QString expr = exprLineEdit->text();
	QString replace = replaceLineEdit->text();

	if (expr.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,
			tr("Match expression may not be empty.").ascii(), MSG_CRITICAL);
		exprLineEdit->setFocus();
		exprLineEdit->selectAll();
		return;
	}

	if (replace.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,
			tr("Replace value may not be empty.").ascii(), MSG_CRITICAL);
		replaceLineEdit->setFocus();
		replaceLineEdit->selectAll();
		return;
	}

	try {
		boost::regex re(expr.ascii());
	} catch (boost::bad_expression) {
		((t_gui *)ui)->cb_show_msg(this,
			tr("Invalid regular expression.").ascii(), MSG_CRITICAL);
		exprLineEdit->setFocus();
		return;
	}

	accept();
}
