#include "dtmfform.h"
#include <QClipboard>
#include <QTimer>

/*
 *  Constructs a DtmfForm which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
DtmfForm::DtmfForm(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	// Speed 40/40 (i.e. one tone every 80ms)
	m_insertTimer.setInterval(80);
	m_insertTimer.setSingleShot(false);

	connect(&m_insertTimer, SIGNAL(timeout()), this, SLOT(insertNextKey()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
DtmfForm::~DtmfForm()
{
    // no need to delete child widgets, Qt does it all for us
}


void DtmfForm::dtmf1()
{
	emit digits("1");
}

void DtmfForm::dtmf2()
{
	emit digits("2");
}

void DtmfForm::dtmf3()
{
	emit digits("3");
}

void DtmfForm::dtmf4()
{
	emit digits("4");
}

void DtmfForm::dtmf5()
{
	emit digits("5");
}

void DtmfForm::dtmf6()
{
	emit digits("6");
}

void DtmfForm::dtmf7()
{
	emit digits("7");
}

void DtmfForm::dtmf8()
{
	emit digits("8");
}

void DtmfForm::dtmf9()
{
	emit digits("9");
}

void DtmfForm::dtmf0()
{
	emit digits("0");
}

void DtmfForm::dtmfStar()
{
	emit digits("*");
}

void DtmfForm::dtmfPound()
{
	emit digits("#");
}

void DtmfForm::dtmfA()
{
	emit digits("A");
}

void DtmfForm::dtmfB()
{
	emit digits("B");
}

void DtmfForm::dtmfC()
{
	emit digits("C");
}

void DtmfForm::dtmfD()
{
	emit digits("D");
}

void DtmfForm::keyPressEvent(QKeyEvent *e)
{
	// DTMF keys
	if ((e->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))
			== Qt::NoModifier)
	{
		switch (e->key()) {
		case Qt::Key_1:
			onePushButton->animateClick();
			break;
		case Qt::Key_2:
		case Qt::Key_A:
		case Qt::Key_B:
		case Qt::Key_C:
			twoPushButton->animateClick();
			break;
		case Qt::Key_3:
		case Qt::Key_D:
		case Qt::Key_E:
		case Qt::Key_F:
			threePushButton->animateClick();
			break;
		case Qt::Key_4:
		case Qt::Key_G:
		case Qt::Key_H:
		case Qt::Key_I:
			fourPushButton->animateClick();
			break;
		case Qt::Key_5:
		case Qt::Key_J:
		case Qt::Key_K:
		case Qt::Key_L:
			fivePushButton->animateClick();
			break;
		case Qt::Key_6:
		case Qt::Key_M:
		case Qt::Key_N:
		case Qt::Key_O:
			sixPushButton->animateClick();
			break;
		case Qt::Key_7:
		case Qt::Key_P:
		case Qt::Key_Q:
		case Qt::Key_R:
		case Qt::Key_S:
			sevenPushButton->animateClick();
			break;
		case Qt::Key_8:
		case Qt::Key_T:
		case Qt::Key_U:
		case Qt::Key_V:
			eightPushButton->animateClick();
			break;
		case Qt::Key_9:
		case Qt::Key_W:
		case Qt::Key_X:
		case Qt::Key_Y:
		case Qt::Key_Z:
			ninePushButton->animateClick();
			break;
		case Qt::Key_0:
		case Qt::Key_Space:
			zeroPushButton->animateClick();
			break;
		case Qt::Key_Asterisk:
			starPushButton->animateClick();
			break;
		case Qt::Key_NumberSign:
			poundPushButton->animateClick();
			break;
		default:
			e->ignore();
		}
	}
	else if ((e->modifiers() == Qt::ShiftModifier && e->key() == Qt::Key_Insert)
			 || (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_V))
	{
		// Insert from clipboard
		QClipboard *clipboard = QApplication::clipboard();
		QString text = clipboard->text();

		if (!text.isEmpty())
		{
			m_remainingKeys = text;
			insertNextKey();
			m_insertTimer.start();
		}
	}
}

void DtmfForm::insertNextKey()
{
	QChar key;
	bool keyValid;

	do
	{
		keyValid = true;
		key = m_remainingKeys[0].toLower();
		m_remainingKeys = m_remainingKeys.mid(1);

		switch (key.toLatin1()) {
		case '0':
			zeroPushButton->animateClick();
			break;
		case '1':
			onePushButton->animateClick();
			break;
		case '2':
			twoPushButton->animateClick();
			break;
		case '3':
			threePushButton->animateClick();
			break;
		case '4':
			fourPushButton->animateClick();
			break;
		case '5':
			fivePushButton->animateClick();
			break;
		case '6' :
			sixPushButton->animateClick();
			break;
		case '7':
			sevenPushButton->animateClick();
			break;
		case '8':
			eightPushButton->animateClick();
			break;
		case '9':
			ninePushButton->animateClick();
			break;
		case '#':
			poundPushButton->animateClick();
			break;
		case '*':
			starPushButton->animateClick();
			break;
		default:
			keyValid = false;
			break;
		}
	}
	while (!keyValid);

	if (m_remainingKeys.isEmpty())
		m_insertTimer.stop();
}
