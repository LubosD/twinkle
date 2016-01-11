#ifndef OSD_H
#define OSD_H
#include <QObject>
#include <QString>

// Must use forward declaration, otherwise build fails
// due to double QMetaTypeID<QAction*> definition (wtf).
// Hence I also cannot inherit from OSD_VIEWCLASS...
class QQuickView;
class QQuickItem;

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
	QQuickView* m_view;
	QQuickItem* m_caller;
	QQuickItem* m_time;
	QQuickItem* m_mute;
};

#endif // OSD_H
