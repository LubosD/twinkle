#include "dtmfform.h"

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
	switch (e->key()) {
	case Qt::Key_1:
		dtmf1();
		break;
	case Qt::Key_2:
	case Qt::Key_A:
	case Qt::Key_B:
	case Qt::Key_C:
		dtmf2();
		break;
	case Qt::Key_3:
	case Qt::Key_D:
	case Qt::Key_E:
	case Qt::Key_F:
		dtmf3();
		break;
	case Qt::Key_4:
	case Qt::Key_G:
	case Qt::Key_H:
	case Qt::Key_I:
		dtmf4();
		break;
	case Qt::Key_5:
	case Qt::Key_J:
	case Qt::Key_K:
	case Qt::Key_L:
		dtmf5();
		break;
	case Qt::Key_6:
	case Qt::Key_M:
	case Qt::Key_N:
	case Qt::Key_O:
		dtmf6();
		break;
	case Qt::Key_7:
	case Qt::Key_P:
	case Qt::Key_Q:
	case Qt::Key_R:
	case Qt::Key_S:
		dtmf7();
		break;
	case Qt::Key_8:
	case Qt::Key_T:
	case Qt::Key_U:
	case Qt::Key_V:
		dtmf8();
		break;
	case Qt::Key_9:
	case Qt::Key_W:
	case Qt::Key_X:
	case Qt::Key_Y:
	case Qt::Key_Z:
		dtmf9();
		break;
	case Qt::Key_0:
	case Qt::Key_Space:
		dtmf0();
		break;
	case Qt::Key_Asterisk:
		dtmfStar();
		break;
	case Qt::Key_NumberSign:
		dtmfPound();
		break;
	default:
		e->ignore();
	}
}
