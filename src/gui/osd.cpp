#include "osd.h"
#include <QtDebug>

#include <QDesktopWidget>
#include <QSettings>
#include <QApplication>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>

extern QSettings* g_gui_state;

OSD::OSD(QObject* parent)
	: QObject(parent)
{
	m_view = new QQuickView;
	m_view->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);

	m_view->rootContext()->setContextProperty("viewerWidget", m_view);
	m_view->setSource(QUrl("qrc:/qml/osd.qml"));

	if (!m_view->rootObject()) {
		throw std::runtime_error("Could not load osd.qml");
	}

	positionWindow();

	QObject* buttonHangup;
	buttonHangup = m_view->rootObject()->findChild<QObject*>("hangup");

	connect(buttonHangup, SIGNAL(clicked()), this, SLOT(onHangupClicked()));

	m_caller = m_view->rootObject()->findChild<QQuickItem*>("callerName");
	m_time = m_view->rootObject()->findChild<QQuickItem*>("callTime");
	m_mute = m_view->rootObject()->findChild<QQuickItem*>("mute");

	connect(m_mute, SIGNAL(clicked()), this, SLOT(onMuteClicked()));

	connect(m_view->rootObject(), SIGNAL(moved()), this, SLOT(saveState()));
}

OSD::~OSD()
{
	delete m_view;
}

void OSD::positionWindow()
{
	QDesktopWidget* desktop = QApplication::desktop();
	int x, y;
	int defaultX, defaultY;

	defaultX = desktop->width() - this->width() - 10;
	defaultY = 10;

	x = g_gui_state->value("osd/x", defaultX).toInt();
	y = g_gui_state->value("osd/y", defaultY).toInt();

	// Reset position if off screen
	if (x > desktop->width() || x < 0)
		x = defaultX;
	if (y > desktop->height() || y < 0)
		y = defaultY;

	m_view->setPosition(x, y);
}

void OSD::saveState()
{
	QPoint pos = m_view->position();
	g_gui_state->setValue("osd/x", pos.x());
	g_gui_state->setValue("osd/y", pos.y());
}

void OSD::onHangupClicked()
{
	emit hangupClicked();
}

void OSD::onMuteClicked()
{
	emit muteClicked();
}

void OSD::setMuted(bool muted)
{
	QString path;

	if (muted)
		path = "qrc:/icons/images/osd_mic_off.png";
	else
		path = "qrc:/icons/images/osd_mic_on.png";

	m_mute->setProperty("image", path);
}

void OSD::setCaller(const QString& text)
{
	m_caller->setProperty("text", text);
}

void OSD::setTime(const QString& timeText)
{
	m_time->setProperty("text", timeText);
}

void OSD::move(int x, int y)
{
	m_view->setPosition(x, y);
}

void OSD::show()
{
	m_view->show();
}

void OSD::hide()
{
	m_view->hide();
}

int OSD::width() const
{
	return m_view->width();
}

int OSD::height() const
{
	return m_view->height();
}
