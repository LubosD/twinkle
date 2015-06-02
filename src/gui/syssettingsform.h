#ifndef SYSSETTINGSFORM_H
#define SYSSETTINGSFORM_H

#include "sys_settings.h"
#include <QDialog>
#include "ui_syssettingsform.h"

class SysSettingsForm : public QDialog, public Ui::SysSettingsForm
{
	Q_OBJECT

public:
	SysSettingsForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	~SysSettingsForm();

	virtual string comboItem2audio_dev( QString item, QLineEdit * qleOther, bool playback );

public slots:
	virtual void showCategory( int index );
	virtual void populateComboBox( QComboBox * cb, const QString & s );
	virtual void populate();
	virtual void validate();
	virtual void show();
	virtual int exec();
	virtual void chooseRingtone();
	virtual void chooseRingback();
	virtual void devRingtoneSelected( int idx );
	virtual void devSpeakerSelected( int idx );
	virtual void devMicSelected( int idx );
	virtual void playRingToneCheckBoxToggles( bool on );
	virtual void playRingBackToneCheckBoxToggles( bool on );

signals:
	void sipUdpPortChanged();
	void rtpPortChanged();

protected slots:
	virtual void languageChange();

private:
	int idxOtherCaptureDevOss;
	int idxOtherCaptureDevAlsa;
	int idxOtherPlaybackDevOss;
	int idxOtherPlaybackDevAlsa;
	list<t_audio_device> list_audio_playback_dev;
	list<t_audio_device> list_audio_capture_dev;

	void init();

};


#endif
