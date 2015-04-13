/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
/*
    Copyright (C) 2005-2009  Michel de Boer <michel@twinklephone.com>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Indices of categories in the category list box
#define idxCatGeneral	0
#define idxCatAudio	1
#define idxCatRingtones	2
#define idxCatAddressBook	3
#define idxCatNetwork	4
#define idxCatLog		5

void SysSettingsForm::init()
{
	// Set toolbutton icons for disabled options.
	QIconSet i;
	i = openRingtoneToolButton->iconSet();
	i.setPixmap(QPixmap::fromMimeSource("fileopen-disabled.png"), 
		    QIconSet::Automatic, QIconSet::Disabled);
	openRingtoneToolButton->setIconSet(i);
	i = openRingbackToolButton->iconSet();
	i.setPixmap(QPixmap::fromMimeSource("fileopen-disabled.png"), 
		    QIconSet::Automatic, QIconSet::Disabled);
	openRingbackToolButton->setIconSet(i);
	
	QRegExp rxNumber("[0-9]+");
	maxUdpSizeLineEdit->setValidator(new QRegExpValidator(rxNumber, this));
	maxTcpSizeLineEdit->setValidator(new QRegExpValidator(rxNumber, this));
}

void SysSettingsForm::showCategory( int index )
{
	if (index == idxCatGeneral) {
		settingsWidgetStack->raiseWidget(pageGeneral);
	} else if (index == idxCatAudio) {
		settingsWidgetStack->raiseWidget(pageAudio);
	} else if (index == idxCatRingtones) {
		settingsWidgetStack->raiseWidget(pageRingtones);
	} else if (index == idxCatAddressBook) {
		settingsWidgetStack->raiseWidget(pageAddressBook);
	} else if (index == idxCatNetwork) {
		settingsWidgetStack->raiseWidget(pageNetwork);
	} else if (index == idxCatLog) {
		settingsWidgetStack->raiseWidget(pageLog);
	}
}

string SysSettingsForm::comboItem2audio_dev(QString item, QLineEdit *qleOther, bool playback)
{
	if (item == QString("ALSA: ") + DEV_OTHER) {
		if (qleOther->text().isEmpty()) return "";
		return (QString(PFX_ALSA) + qleOther->text()).ascii();
	}
	
	if (item == QString("OSS: ") + DEV_OTHER) {
		if (qleOther->text().isEmpty()) return "";
		return (QString(PFX_OSS) + qleOther->text()).ascii();
	}
	
	list<t_audio_device> &list_audio_dev = (playback ?
			list_audio_playback_dev : list_audio_capture_dev);
	
	for (list<t_audio_device>::iterator i = list_audio_dev.begin(); 
	i != list_audio_dev.end(); i++)
	{
		if (i->get_description() == item.ascii()) {
			return i->get_settings_value();
		}
	}
	
	return "";
}

void SysSettingsForm::populateComboBox(QComboBox *cb, const QString &s)
{
	for (int i = 0; i < cb->count(); i++) {
		if (cb->text(i) == s) {
			cb->setCurrentItem(i);
			return;
		}
	}
}

void SysSettingsForm::populate()
{
	QString msg;
	int idx;
	
	// Select the Audio category
	categoryListBox->setSelected(idxCatGeneral, true);
	settingsWidgetStack->raiseWidget(pageGeneral);
	
	// Set focus on first field
	categoryListBox->setFocus();
	
	// Audio settings
	list_audio_playback_dev = sys_config->get_audio_devices(true);
	list_audio_capture_dev = sys_config->get_audio_devices(false);
	ringtoneComboBox->clear();
	speakerComboBox->clear();
	micComboBox->clear();
	bool devRingtoneFound = false;
	bool devSpeakerFound = false;
	bool devMicFound = false;
	
	// Playback devices
	idx = 0;
	for (list<t_audio_device>::iterator i = list_audio_playback_dev.begin(); 
	i != list_audio_playback_dev.end(); i++, idx++) {
		string item = i->get_description();
		ringtoneComboBox->insertItem(QString(item.c_str()));
		speakerComboBox->insertItem(QString(item.c_str()));
		
		// Select audio device
		if (sys_config->get_dev_ringtone().device == i->device) {
			ringtoneComboBox->setCurrentItem(idx);
			otherRingtoneLineEdit->clear();
			devRingtoneFound = true;
		}
		if (sys_config->get_dev_speaker().device == i->device) {
			speakerComboBox->setCurrentItem(idx);
			otherSpeakerLineEdit->clear();
			devSpeakerFound = true;
		}
		
		// Determine index for other non-standard device
		if (i->device == DEV_OTHER) {
			if (i->type == t_audio_device::ALSA) {
				idxOtherPlaybackDevAlsa = idx;
			} else {
				idxOtherPlaybackDevOss = idx;
			}
		}
	}
	
	// Check for non-standard audio devices
	if (!devRingtoneFound) {
		t_audio_device dev = sys_config->get_dev_ringtone();
		otherRingtoneLineEdit->setText(dev.device.c_str());
		ringtoneComboBox->setCurrentItem(
			(dev.type == t_audio_device::ALSA ? idxOtherPlaybackDevAlsa : idxOtherPlaybackDevOss));
	}
	if (!devSpeakerFound) {
		t_audio_device dev = sys_config->get_dev_speaker();
		otherSpeakerLineEdit->setText(dev.device.c_str());
		speakerComboBox->setCurrentItem(
			(dev.type == t_audio_device::ALSA ? idxOtherPlaybackDevAlsa : idxOtherPlaybackDevOss));
	}
	
	// Capture device
	idx = 0;
	for (list<t_audio_device>::iterator i = list_audio_capture_dev.begin(); 
	i != list_audio_capture_dev.end(); i++, idx++) {
		string item = i->get_description();
		micComboBox->insertItem(QString(item.c_str()));
		
		// Select audio device
		if (sys_config->get_dev_mic().device == i->device) {
			micComboBox->setCurrentItem(idx);
			otherMicLineEdit->clear();
			devMicFound = true;
		}
		
		// Determine index for other non-standard device
		if (i->device == DEV_OTHER) {
			if (i->type == t_audio_device::ALSA) {
				idxOtherCaptureDevAlsa = idx;
			} else {
				idxOtherCaptureDevOss = idx;
			}
		}
	}
	
	// Check for non-standard audio devices
	if (!devMicFound) {
		t_audio_device dev = sys_config->get_dev_mic();
		otherMicLineEdit->setText(dev.device.c_str());
		micComboBox->setCurrentItem(
			(dev.type == t_audio_device::ALSA ? idxOtherCaptureDevAlsa : idxOtherCaptureDevOss));
	}
	
	// Enable/disable line edit for non-standard device
	devRingtoneSelected(ringtoneComboBox->currentItem());
	devSpeakerSelected(speakerComboBox->currentItem());
	devMicSelected(micComboBox->currentItem());
	
	validateAudioCheckBox->setChecked(sys_config->get_validate_audio_dev());
	
	populateComboBox(ossFragmentComboBox, 
			 QString::number(sys_config->get_oss_fragment_size()));
	populateComboBox(alsaPlayPeriodComboBox,
			 QString::number(sys_config->get_alsa_play_period_size()));
	populateComboBox(alsaCapturePeriodComboBox,
			QString::number(sys_config->get_alsa_capture_period_size()));
	
	// Log settings
	logMaxSizeSpinBox->setValue(sys_config->get_log_max_size());
	logDebugCheckBox->setChecked(sys_config->get_log_show_debug());
	logSipCheckBox->setChecked(sys_config->get_log_show_sip());
	logStunCheckBox->setChecked(sys_config->get_log_show_stun());
	logMemoryCheckBox->setChecked(sys_config->get_log_show_memory());
	
	// General settings
	guiUseSystrayCheckBox->setChecked(sys_config->get_gui_use_systray());
	guiHideCheckBox->setChecked(sys_config->get_gui_hide_on_close());
	guiHideCheckBox->setEnabled(sys_config->get_gui_use_systray());
	
	// Call history
	histSizeSpinBox->setValue(sys_config->get_ch_max_size());
	
	// Auto show on incoming call
	autoShowCheckBox->setChecked(sys_config->get_gui_auto_show_incoming());
	autoShowTimeoutSpinBox->setValue(sys_config->get_gui_auto_show_timeout());
	
	// Services
	callWaitingCheckBox->setChecked(sys_config->get_call_waiting());
	hangupBothCheckBox->setChecked(sys_config->get_hangup_both_3way());
	
	// Startup settings
	startHiddenCheckBox->setChecked(sys_config->get_start_hidden());
	
	QStringList profiles;
	if (!SelectProfileForm::getUserProfiles(profiles, msg)) {
		((t_gui *)ui)->cb_show_msg(this, msg.ascii(), MSG_CRITICAL);
	}
	profileListView->clear();
	for (QStringList::Iterator i = profiles.begin(); i != profiles.end(); i++) {
		// Strip off the .cfg suffix
		QString profile = *i;
		profile.truncate(profile.length() - 4);
		QCheckListItem *item = new QCheckListItem(profileListView, 
					profile, QCheckListItem::CheckBox);
		item->setPixmap(0, QPixmap::fromMimeSource("penguin-small.png"));
		
		list<string> l = sys_config->get_start_user_profiles();
		if (std::find(l.begin(), l.end(), profile.ascii()) != l.end())
		{
			item->setOn(true);
		}
	}
	
	// Web browser command
	browserLineEdit->setText(sys_config->get_gui_browser_cmd().c_str());
	
	// Network settings
	sipUdpPortSpinBox->setValue(sys_config->get_config_sip_port());
	rtpPortSpinBox->setValue(sys_config->get_rtp_port());
	
	maxUdpSizeLineEdit->setText(QString::number(sys_config->get_sip_max_udp_size()));
	maxTcpSizeLineEdit->setText(QString::number(sys_config->get_sip_max_tcp_size()));
	
	// Ring tone settings
	playRingtoneCheckBox->setChecked(sys_config->get_play_ringtone());
	defaultRingtoneRadioButton->setChecked(sys_config->get_ringtone_file().empty());
	customRingtoneRadioButton->setChecked(!sys_config->get_ringtone_file().empty());
	ringtoneLineEdit->setText(sys_config->get_ringtone_file().c_str());
	defaultRingtoneRadioButton->setEnabled(sys_config->get_play_ringtone());
	customRingtoneRadioButton->setEnabled(sys_config->get_play_ringtone());
	ringtoneLineEdit->setEnabled(!sys_config->get_ringtone_file().empty());
	openRingtoneToolButton->setEnabled(!sys_config->get_ringtone_file().empty());
	
	playRingbackCheckBox->setChecked(sys_config->get_play_ringback());
	defaultRingbackRadioButton->setChecked(sys_config->get_ringback_file().empty());
	customRingbackRadioButton->setChecked(!sys_config->get_ringback_file().empty());
	ringbackLineEdit->setText(sys_config->get_ringback_file().c_str());
	defaultRingbackRadioButton->setEnabled(sys_config->get_play_ringback());
	customRingbackRadioButton->setEnabled(sys_config->get_play_ringback());
	ringbackLineEdit->setEnabled(!sys_config->get_ringback_file().empty());
	openRingbackToolButton->setEnabled(!sys_config->get_ringback_file().empty());
	
	// Address book settings
	abLookupNameCheckBox->setChecked(sys_config->get_ab_lookup_name());
	abOverrideDisplayCheckBox->setChecked(sys_config->get_ab_override_display());
	abOverrideDisplayCheckBox->setEnabled(sys_config->get_ab_lookup_name());
	abLookupPhotoCheckBox->setChecked(sys_config->get_ab_lookup_photo());
}

void SysSettingsForm::validate()
{
	bool conversion_ok = false;
	unsigned short sip_max_udp_size = maxUdpSizeLineEdit->text().toUShort(&conversion_ok);
	if (!conversion_ok) sip_max_udp_size = sys_config->get_sip_max_udp_size();

	unsigned long sip_max_tcp_size = maxTcpSizeLineEdit->text().toULong(&conversion_ok);
	if (!conversion_ok) sip_max_tcp_size = sys_config->get_sip_max_tcp_size();
	
	// Audio
	string dev;
	dev = comboItem2audio_dev(ringtoneComboBox->currentText(), otherRingtoneLineEdit, true);
	if (dev != "") sys_config->set_dev_ringtone(sys_config->audio_device(dev));
	dev = comboItem2audio_dev(speakerComboBox->currentText(), otherSpeakerLineEdit, true);
	if (dev != "") sys_config->set_dev_speaker(sys_config->audio_device(dev));
	dev = comboItem2audio_dev(micComboBox->currentText(), otherMicLineEdit, false);
	if (dev != "") sys_config->set_dev_mic(sys_config->audio_device(dev));
	
	sys_config->set_validate_audio_dev(validateAudioCheckBox->isChecked());
	
	sys_config->set_oss_fragment_size(
			ossFragmentComboBox->currentText().toInt());
	sys_config->set_alsa_play_period_size(
			alsaPlayPeriodComboBox->currentText().toInt());
	sys_config->set_alsa_capture_period_size(
			alsaCapturePeriodComboBox->currentText().toInt());
	
	// Log
	sys_config->set_log_max_size(logMaxSizeSpinBox->value());
	sys_config->set_log_show_debug(logDebugCheckBox->isChecked());
	sys_config->set_log_show_sip(logSipCheckBox->isChecked());
	sys_config->set_log_show_stun(logStunCheckBox->isChecked());
	sys_config->set_log_show_memory(logMemoryCheckBox->isChecked());
	
	// General
	sys_config->set_gui_use_systray(guiUseSystrayCheckBox->isChecked());
	sys_config->set_gui_hide_on_close(guiHideCheckBox->isChecked());
	
	// Auto show on incoming call
	sys_config->set_gui_auto_show_incoming(autoShowCheckBox->isChecked());
	sys_config->set_gui_auto_show_timeout(autoShowTimeoutSpinBox->value());
	
	// Call history
	sys_config->set_ch_max_size(histSizeSpinBox->value());
	
	// Services
	sys_config->set_call_waiting(callWaitingCheckBox->isChecked());
	sys_config->set_hangup_both_3way(hangupBothCheckBox->isChecked());

	// Startup
	sys_config->set_start_hidden(startHiddenCheckBox->isChecked() &&
				   guiUseSystrayCheckBox->isChecked());
	
	list<string> start_user_profiles;
	QListViewItemIterator i(profileListView, QListViewItemIterator::Checked);
	while (i.current()) {
		QCheckListItem *item = (QCheckListItem *)i.current();
		start_user_profiles.push_back(item->text().ascii());
		i++;
	}
	sys_config->set_start_user_profiles(start_user_profiles);
	
	// Web browser command
	sys_config->set_gui_browser_cmd(browserLineEdit->text().stripWhiteSpace().ascii());
	
	// Network
	if (sys_config->get_config_sip_port() != sipUdpPortSpinBox->value()) {
		sys_config->set_config_sip_port(sipUdpPortSpinBox->value());
		emit sipUdpPortChanged();
	}
	if (sys_config->get_rtp_port() != rtpPortSpinBox->value()) {
		sys_config->set_rtp_port(rtpPortSpinBox->value());
		emit rtpPortChanged();
	}
	sys_config->set_sip_max_udp_size(sip_max_udp_size);
	sys_config->set_sip_max_tcp_size(sip_max_tcp_size);
	
	// Ring tones
	sys_config->set_play_ringtone(playRingtoneCheckBox->isChecked());
	if (sys_config->get_play_ringtone()) {
		if (defaultRingtoneRadioButton->isOn()) {
			sys_config->set_ringtone_file("");
		} else {
			sys_config->set_ringtone_file(ringtoneLineEdit->
					text().stripWhiteSpace().ascii());
		}
	} else {
		sys_config->set_ringtone_file("");
	}
	
	sys_config->set_play_ringback(playRingbackCheckBox->isChecked());
	if (sys_config->get_play_ringback()) {
		if (defaultRingbackRadioButton->isOn()) {
			sys_config->set_ringback_file("");
		} else {
			sys_config->set_ringback_file(ringbackLineEdit->
					text().stripWhiteSpace().ascii());
		}
	} else {
		sys_config->set_ringback_file("");
	}
	
	// Address book settings
	sys_config->set_ab_lookup_name(abLookupNameCheckBox->isChecked());
	sys_config->set_ab_override_display(abOverrideDisplayCheckBox->isChecked());
	sys_config->set_ab_lookup_photo(abLookupPhotoCheckBox->isChecked());
	
	// Save user config
	string error_msg;
	if (!sys_config->write_config(error_msg)) {
		// Failed to write config file
		((t_gui *)ui)->cb_show_msg(this, error_msg, MSG_CRITICAL);
		return;
	}
	
	accept();
}

void SysSettingsForm::show()
{
	populate();
	QDialog::show();
}

int SysSettingsForm::exec()
{
	populate();
	return QDialog::exec();
}

void SysSettingsForm::chooseRingtone()
{
	QString file = QFileDialog::getOpenFileName(
			((t_gui *)ui)->get_last_file_browse_path(),
			tr("Ring tones", "Description of .wav files in file dialog").append(" (*.wav)"), this, "ring tone file dialog",
			tr("Choose ring tone"));
	if (!file.isEmpty()) {
		ringtoneLineEdit->setText(file);
		((t_gui *)ui)->set_last_file_browse_path(QFileInfo(file).dirPath(true));
	}
}

void SysSettingsForm::chooseRingback()
{
	QString file = QFileDialog::getOpenFileName(
			((t_gui *)ui)->get_last_file_browse_path(),
			tr("Ring back tones", "Description of .wav files in file dialog").append(" (*.wav)"), this, "ring back file dialog",
			tr("Choose ring back tone"));
	if (!file.isEmpty()) {
		ringbackLineEdit->setText(file);
		((t_gui *)ui)->set_last_file_browse_path(QFileInfo(file).dirPath(true));
	}
}

void SysSettingsForm::devRingtoneSelected(int idx) {
	bool b = (idx == idxOtherPlaybackDevAlsa || idx == idxOtherPlaybackDevOss);
	otherRingtoneTextLabel->setEnabled(b);
	otherRingtoneLineEdit->setEnabled(b);
}

void SysSettingsForm::devSpeakerSelected(int idx) {
	bool b = (idx == idxOtherPlaybackDevAlsa || idx == idxOtherPlaybackDevOss);
	otherSpeakerTextLabel->setEnabled(b);
	otherSpeakerLineEdit->setEnabled(b);
}

void SysSettingsForm::devMicSelected(int idx) {
	bool b = (idx == idxOtherCaptureDevAlsa || idx == idxOtherCaptureDevOss);
	otherMicTextLabel->setEnabled(b);
	otherMicLineEdit->setEnabled(b);
}

void SysSettingsForm::playRingToneCheckBoxToggles(bool on) {
	if (on) {
		ringtoneLineEdit->setEnabled(customRingtoneRadioButton->isChecked());
	} else {
		ringtoneLineEdit->setEnabled(false);
	}
}

void SysSettingsForm::playRingBackToneCheckBoxToggles(bool on) {
	if (on) {
		ringbackLineEdit->setEnabled(customRingbackRadioButton->isChecked());
	} else {
		ringbackLineEdit->setEnabled(false);
	}
}
