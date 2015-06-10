#include "incoming_call_popup.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QDeclarativeContext>

IncomingCallPopup::IncomingCallPopup(QObject *parent) : QObject(parent)
{
	m_view = new QDeclarativeView;

	// Qt5 QQuickView: setFlags()
	m_view->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);

	m_view->setSource(QUrl("qrc:/qml/incoming_call.qml"));

	// Place into the middle of the screen
	QDesktopWidget* desktop = qApp->desktop();
	QPoint pos;

	pos.setX(desktop->width()/2 - m_view->width()/2);
	pos.setY(desktop->height()/2 - m_view->height()/2);

	m_view->move(pos);

	QObject* button;

	button = m_view->rootObject()->findChild<QObject*>("buttonAnswer");
	connect(button, SIGNAL(clicked()), this, SLOT(onAnswerClicked()));

	button = m_view->rootObject()->findChild<QObject*>("buttonReject");
	connect(button, SIGNAL(clicked()), this, SLOT(onRejectClicked()));

	m_callerText = m_view->rootObject()->findChild<QDeclarativeItem*>("callerText");
}

IncomingCallPopup::~IncomingCallPopup()
{
	delete m_view;
}

void IncomingCallPopup::setCallerName(const QString& name)
{
	QString text = tr("%1 calling").arg(name);
	m_callerText->setProperty("text", text);
}

void IncomingCallPopup::onAnswerClicked()
{
	emit answerClicked();
	m_view->hide();
}

void IncomingCallPopup::onRejectClicked()
{
	emit rejectClicked();
	m_view->hide();
}

void IncomingCallPopup::show()
{
	m_view->show();
}

void IncomingCallPopup::hide()
{
	m_view->hide();
}

