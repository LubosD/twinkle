#ifndef OSD_H
#define OSD_H
#include <QObject>
#include <QString>
#if 0 //QT_VERSION >= 0x050000
#	define OSD_VIEWCLASS QQuickView
#	define QML_ITEMTYPE QQuickItem
#else
#	define OSD_VIEWCLASS QDeclarativeView
#	define QML_ITEMTYPE QDeclarativeItem
#endif

// Must use forward declaration, otherwise build fails
// due to double QMetaTypeID<QAction*> definition (wtf).
// Hence I also cannot inherit from OSD_VIEWCLASS...
class OSD_VIEWCLASS;
class QML_ITEMTYPE;

class OSD : public QObject
{
	Q_OBJECT
public:
	OSD(QObject* parent = 0);
	~OSD();

	void setCaller(const QString& text);
	void setTime(const QString& timeText);
	void setMuted(bool muted);

	void move(int x, int y);
	void show();
	void hide();
	int width() const;
	int height() const;
	void setVisible(bool v) { if (v) show(); else hide(); }

private:
	void positionWindow();
public slots:
	void onHangupClicked();
	void onMuteClicked();
	void saveState();

signals:
	void hangupClicked();
	void muteClicked();

private:
	OSD_VIEWCLASS* m_view;
	QML_ITEMTYPE* m_caller;
	QML_ITEMTYPE* m_time;
	QML_ITEMTYPE* m_mute;
};

#endif // OSD_H
