#include "osd.h"
#include <QtDebug>

#if 0 //QT_VERSION >= 0x050000
#	include <QQuickView>
#	include <QQmlContext>
#	include <QQuickItem>
#else
#	include <QDeclarativeView>
#	include <QDeclarativeContext>
#	include <QDeclarativeItem>
#endif

OSD::OSD(QObject* parent)
	: QObject(parent)
{
	m_view = new OSD_VIEWCLASS;
	// Qt5 QQuickView: setFlags()
	m_view->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);

	m_view->rootContext()->setContextProperty("viewerWidget", m_view);
	m_view->setSource(QUrl("qrc:/qml/osd.qml"));

	QObject* buttonHangup;
	buttonHangup = m_view->rootObject()->findChild<QObject*>("hangup");

	connect(buttonHangup, SIGNAL(clicked()), this, SLOT(onHangupClicked()));

	m_caller = m_view->rootObject()->findChild<QML_ITEMTYPE*>("callerName");
	m_time = m_view->rootObject()->findChild<QML_ITEMTYPE*>("callTime");
	m_mute = m_view->rootObject()->findChild<QML_ITEMTYPE*>("mute");

	connect(m_mute, SIGNAL(clicked()), this, SLOT(onMuteClicked()));
}

OSD::~OSD()
{
	delete m_view;
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
	// Qt5 QQuickView: setPosition
	m_view->move(x, y);
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
