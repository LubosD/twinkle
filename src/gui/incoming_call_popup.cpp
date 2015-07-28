#include "incoming_call_popup.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QDeclarativeContext>
#include <QSettings>

extern QSettings* g_gui_state;

IncomingCallPopup::IncomingCallPopup(QObject *parent) : QObject(parent)
{
	m_view = new QDeclarativeView;

	// Qt5 QQuickView: setFlags()
	m_view->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);

	m_view->rootContext()->setContextProperty("viewerWidget", m_view);
	m_view->setSource(QUrl("qrc:/qml/incoming_call.qml"));

    // Place into the middle of the screen
	positionWindow();

	QObject* button;

	button = m_view->rootObject()->findChild<QObject*>("buttonAnswer");
	connect(button, SIGNAL(clicked()), this, SLOT(onAnswerClicked()));

	button = m_view->rootObject()->findChild<QObject*>("buttonReject");
	connect(button, SIGNAL(clicked()), this, SLOT(onRejectClicked()));

	m_callerText = m_view->rootObject()->findChild<QDeclarativeItem*>("callerText");
	connect(m_view->rootObject(), SIGNAL(moved()), this, SLOT(saveState()));
}

IncomingCallPopup::~IncomingCallPopup()
{
	delete m_view;
}

void IncomingCallPopup::positionWindow()
{
	QDesktopWidget* desktop = qApp->desktop();
	int x, y;
	int defaultX, defaultY;

	defaultX = desktop->width()/2 - m_view->width()/2;
	defaultY = desktop->height()/2 - m_view->height()/2;

	x = g_gui_state->value("incoming_popup/x", defaultX).toInt();
	y = g_gui_state->value("incoming_popup/y", defaultY).toInt();

	// Reset position if off screen
	if (x > desktop->width() || x < 0)
		x = defaultX;
	if (y > desktop->height() || y < 0)
		y = defaultY;

	m_view->move(x, y);
}

void IncomingCallPopup::saveState()
{
	QPoint pos = m_view->pos();
	g_gui_state->setValue("incoming_popup/x", pos.x());
	g_gui_state->setValue("incoming_popup/y", pos.y());
}

void IncomingCallPopup::move(int x, int y)
{
    m_view->move(QPoint(x, y));
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

