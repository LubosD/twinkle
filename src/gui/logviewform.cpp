#include "logviewform.h"

#include <QScrollBar>
#include <QTimer>
#include "audits/memman.h"
#include "log.h"

/*
 *  Constructs a LogViewForm which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
LogViewForm::LogViewForm(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
LogViewForm::~LogViewForm()
{
	// no need to delete child widgets, Qt does it all for us
}

bool LogViewForm::isOnBottom() const
{
	const QScrollBar* vsb = logTextEdit->verticalScrollBar();
	return (vsb->value() == vsb->maximum());
}

void LogViewForm::scrollToBottom()
{
	QScrollBar* vsb = logTextEdit->verticalScrollBar();
	vsb->setValue(vsb->maximum());
	logTextEdit->update();
}

void LogViewForm::show()
{
    if (isVisible()) {
		raise();
		return;
	}

	QString fname = log_file->get_filename().c_str();
	logfile = new QFile(fname);
	MEMMAN_NEW(logfile);
	logstream = NULL;
	if (logfile->open(QIODevice::ReadOnly)) {
		logstream = new QTextStream(logfile);
		MEMMAN_NEW(logstream);
        logTextEdit->setPlainText(logstream->readAll());
	}
	log_file->enable_inform_user(true);

	QDialog::show();

	// Couldn't get it to scroll AND show contents(!) without this hack
	QTimer::singleShot(50, this, SLOT(scrollToBottom()));
	raise();
}

void LogViewForm::closeEvent(QCloseEvent* ev)
{
	log_file->enable_inform_user(false);
	// logTextEdit->clear(); // causes crashes with Qt5

	if (logstream) {
		MEMMAN_DELETE(logstream);
		delete logstream;
		logstream = NULL;
	}

	logfile->close();
	MEMMAN_DELETE(logfile);
	delete logfile;
	logfile = NULL;

	QDialog::closeEvent(ev);
}

void LogViewForm::update(bool log_zapped)
{
    if (!isVisible()) return;

	if (log_zapped) {
		close();
		show();
		return;
	}

	if (logstream) {
        QString s = logstream->readAll();
		if (!s.isNull() && !s.isEmpty()) {
			bool bottom = isOnBottom();
			logTextEdit->appendPlainText(s);
			if (bottom)
				scrollToBottom();
		}
	}
}

void LogViewForm::clear()
{
	logTextEdit->clear();
}
