/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
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

void SelectProfileForm::init()
{
	user_config = 0;
}

void SelectProfileForm::destroy()
{
	if (user_config) {
		MEMMAN_DELETE(user_config);
		delete user_config;
	}
}

// The exec() method is called at startup
int SelectProfileForm::execForm()
{
	mainWindow = 0;
	profileListView->clear();
	defaultSet = false; // no default has been set
	
	// Get list of all profiles
	QStringList profiles;
	QString error;
	if (!SelectProfileForm::getUserProfiles(profiles, error)) {
		QMessageBox::critical(this, PRODUCT_NAME, error);
		return QDialog::Rejected;
	}
	
	// If there are no profiles then the user has to create one
	if (profiles.isEmpty()) {
		QMessageBox::information(this, PRODUCT_NAME, tr(
			"<html>"\
			"Before you can use Twinkle, you must create a user "\
			"profile.<br>Click OK to create a profile.</html>"));
		
		int newProfileMethod = QMessageBox::question(this, PRODUCT_NAME, tr(
			"<html>"\
			"You can use the profile editor to create a profile. "\
			"With the profile editor you can change many settings "\
			"to tune the SIP protocol, RTP and many other things.<br><br>"\
			"Alternatively you can use the wizard to quickly setup a "\
			"user profile. The wizard asks you only a few essential "\
			"settings. If you create a user profile with the wizard you "\
			"can still edit the full profile with the profile editor at a later "\
			"time.<br><br>"\
			"You can create a Diamondcard account to make worldwide "\
			"calls to regular and cell phones and send SMS messages.<br><br>"\
			"Choose what method you wish to use.</html>"),
			tr("&Wizard"), tr("&Profile editor"), tr("&Diamondcard"));
		
		switch (newProfileMethod) {
		case 0:
			wizardProfile(true);
			break;
		case 1:
			newProfile(true);
			break;
		case 2:
			diamondcardProfile(true);
			break;
		default:
			return QDialog::Rejected;
		}
		
		if (profileListView->childCount() == 0) {
			// No profile has been created.
			return QDialog::Rejected;
		}
		
		// Select the created profile
		QCheckListItem *item = (QCheckListItem *)profileListView->currentItem();
		QString profile = item->text();
		profile.append(USER_FILE_EXT);
		selectedProfiles.clear();
		selectedProfiles.push_back(profile.ascii());
		
		QMessageBox::information(this, PRODUCT_NAME, tr(
			"<html>"\
			"Next you may adjust the system settings. "\
			"You can change these settings always at a later time."\
			"<br><br>"\
			"Click OK to view and adjust the system settings.</html>"));
		
		SysSettingsForm f(this, "system settings", true);
		f.exec();
		
		return QDialog::Accepted;
	}
	
	fillProfileListView(profiles);
	sysPushButton->show();
	runPushButton->setFocus();
	
	// Show the modal dialog
	return QDialog::exec();
}

// The showForm() method is called from File menu when Twinkle is running.
// The execForm() method cannot be used as it will block the Qt event loop.
// NOTE: the method show() is not re-implemented as Qt calls this method
//   from exec() internally.
void SelectProfileForm::showForm(QMainWindow *_mainWindow)
{
	mainWindow = _mainWindow;
	profileListView->clear();
	defaultSet = false;
	
	// Get list of all profiles
	QStringList profiles;
	QString error;
	if (!SelectProfileForm::getUserProfiles(profiles, error)) {
		QMessageBox::critical(this, PRODUCT_NAME, error);
		return;
	}
	
	// Initialize profile list view
	fillProfileListView(profiles);
	QListViewItemIterator j(profileListView);
	while (j.current()) {
		QCheckListItem *item = (QCheckListItem *)j.current();
		QString profile = item->text();
		
		// Set pixmap of default profile
		list<string> l = sys_config->get_start_user_profiles();
		if (std::find(l.begin(),  l.end(), profile.ascii()) != l.end())
		{
			item->setPixmap(0, QPixmap::fromMimeSource("twinkle16.png"));
			defaultSet = true;
		}
		
		// Tick check box of active profile
		if (phone->ref_user_profile(profile.ascii())) {
			item->setOn(true);
		}

		j++;
	}	
	
	sysPushButton->hide();
	runPushButton->setText("&OK");
	runPushButton->setFocus();
	QDialog::show();
}

void SelectProfileForm::runProfile()
{
	selectedProfiles.clear();
	QListViewItemIterator i(profileListView, QListViewItemIterator::Checked);
	while (i.current()) {
		QCheckListItem *item = (QCheckListItem *)i.current();
		QString profile =item->text();
		profile.append(USER_FILE_EXT);
		selectedProfiles.push_back(profile.ascii());
		i++;
	}
	
	if (selectedProfiles.empty()) {
		QMessageBox::warning(this, PRODUCT_NAME, tr(
				"You did not select any user profile to run.\n"\
				"Please select a profile."));
		return;
	}
	
	// This signal will be caught when Twinkle is running.
	// At startup the selectedProfiles attribute is read.
	emit selection(selectedProfiles);
	
	accept();
}

void SelectProfileForm::editProfile()
{
	QCheckListItem *item = (QCheckListItem *)profileListView->currentItem();
	QString profile = item->text();
	
	// If the profile to edit is currently active, then edit the in-memory
	// user profile owned by the t_phone_user object
	if (mainWindow) {
		t_user *active_user = phone->ref_user_profile(profile.ascii());
		if (active_user) {
			list<t_user *> user_list;
			user_list.push_back(active_user);
			UserProfileForm *f = new UserProfileForm(this, 
						"edit user profile", true, 
						 Qt::WDestructiveClose);
			
			connect(f, SIGNAL(authCredentialsChanged(t_user *, const string&)),
				mainWindow, 
				SLOT(updateAuthCache(t_user *, const string&)));
			
			connect(f, SIGNAL(stunServerChanged(t_user *)),
				mainWindow, SLOT(updateStunSettings(t_user *)));
		
			f->show(user_list, "");
			return;
		}
	}
	
	// Edit the user profile from disk.
	profile.append(USER_FILE_EXT);
	
	// Read selected config file
	string error_msg;
	
	if (user_config) {
		MEMMAN_DELETE(user_config);
		delete user_config;
	}
	user_config = new t_user();
	MEMMAN_NEW(user_config);
	
	if (!user_config->read_config(profile.ascii(), error_msg)) {
		((t_gui *)ui)->cb_show_msg(this, error_msg, MSG_WARNING);
		return;
	}
	
	// Show the edit user profile form (modal dialog)
	list<t_user *> user_list;
	user_list.push_back(user_config);
	UserProfileForm *f = new UserProfileForm(this, "edit user profile", true, 
						 Qt::WDestructiveClose);
	f->show(user_list, "");
}

void SelectProfileForm::newProfile()
{
	newProfile(false);
}

void SelectProfileForm::newProfile(bool exec_mode)
{
	// Ask user for a profile name
	GetProfileNameForm getProfileNameForm(this, "get profile name", true);
	if (!getProfileNameForm.execNewName()) return;
	
	// Create file name
	QString profile = getProfileNameForm.getProfileName();
	QString filename = profile;
	filename.append(USER_FILE_EXT);
	
	// Create a new user config
	if (user_config) {
		MEMMAN_DELETE(user_config);
		delete user_config;
	}
	user_config = new t_user();
	MEMMAN_NEW(user_config);
	user_config->set_config(filename.ascii());
	
	// Show the edit user profile form (modal dialog)
	list<t_user *> user_list;
	user_list.push_back(user_config);
	UserProfileForm *f = new UserProfileForm(this, "edit user profile", true, 
						 Qt::WDestructiveClose);
	connect(f, SIGNAL(success()), this, SLOT(newProfileCreated()));
	
	if (exec_mode) {
		f->exec(user_list, "");
	} else {
		f->show(user_list, "");
	}
}

void SelectProfileForm::newProfileCreated()
{
	// New profile created
	// Add the new profile to the profile list box
	QCheckListItem *item = new QCheckListItem(profileListView,
				user_config->get_profile_name().c_str(), 
				QCheckListItem::CheckBox);
	item->setPixmap(0, QPixmap::fromMimeSource("penguin-small.png"));
		
	// Make the new profile the selected profile
	// Do not change this without changing the exec method.
	// When there are no profiles, the exec methods relies on the
	// fact that afer creation of the profile it is selected.
	profileListView->setSelected(item, true);
		
	// Enable buttons that act on a profile
	editPushButton->setEnabled(true);
	deletePushButton->setEnabled(true);
	renamePushButton->setEnabled(true);
	defaultPushButton->setEnabled(true);
	runPushButton->setEnabled(true);
}

void SelectProfileForm::deleteProfile()
{
	QCheckListItem *item = (QCheckListItem *)profileListView->currentItem();
	QString profile = item->text();
	QString msg = tr("Are you sure you want to delete profile '%1'?").arg(profile);
	QMessageBox *mb = new QMessageBox(tr("Delete profile"), msg,
			QMessageBox::Warning,
			QMessageBox::Yes,
			QMessageBox::No,
			QMessageBox::NoButton,
			this);
	MEMMAN_NEW(mb);
	if (mb->exec() == QMessageBox::Yes) {
		// Delete file
		QDir d = QDir::home();
		d.cd(USER_DIR);
		QString filename = profile;
		filename.append(USER_FILE_EXT);
		QString fullname = d.filePath(filename);
		if (!QFile::remove(fullname)) {
			// Failed to delete file
			QMessageBox::critical(this, PRODUCT_NAME,
				tr("Failed to delete profile."));
		} else {
			// Delete possible backup of the profile
			QString backupname = fullname;
			backupname.append("~");
			(void)QFile::remove(backupname);
			
			// Delete service files
			filename = profile;
			filename.append(SVC_FILE_EXT);
			fullname = d.filePath(filename);
			(void)QFile::remove(fullname);
			fullname.append("~");
			(void)QFile::remove(fullname);
			
			// Delete profile from list of default profiles in
			// system settings
			list<string> l = sys_config->get_start_user_profiles();
			if (std::find(l.begin(), l.end(), profile.ascii()) != l.end()) {
				l.remove(profile.ascii());
				sys_config->set_start_user_profiles(l);
				
				string error_msg;
				if (!sys_config->write_config(error_msg)) {
					// Failed to write config file
					((t_gui *)ui)->cb_show_msg(this, 
						error_msg, MSG_CRITICAL);
				}
			}
			
			// Delete profile from profile list box
			QCheckListItem *item = (QCheckListItem *)profileListView->
					       currentItem();
			delete item;
			if (profileListView->childCount() == 0) {
				// There are no profiles anymore
				// Disable buttons that act on a profile
				editPushButton->setEnabled(false);
				deletePushButton->setEnabled(false);
				renamePushButton->setEnabled(false);
				defaultPushButton->setEnabled(false);
				runPushButton->setEnabled(false);
			} else {
				profileListView->setSelected(profileListView->
						firstChild(), true);
			}
		}
	}
	
	MEMMAN_DELETE(mb);
	delete mb;
}

void SelectProfileForm::renameProfile()
{
	QCheckListItem *item = (QCheckListItem *)profileListView->currentItem();
	QString oldProfile = item->text();
	
	// Ask user for a new profile name
	GetProfileNameForm getProfileNameForm(this, "get profile name", true);
	if (!getProfileNameForm.execRename(oldProfile)) return;
	
	// Create file name for the new profile
	QString newProfile = getProfileNameForm.getProfileName();
	QString newFilename = newProfile;
	newFilename.append(USER_FILE_EXT);
	
	// Create file name for the old profile
	QString oldFilename = oldProfile;
	oldFilename.append(USER_FILE_EXT);
	
	// Rename the file
	QDir d = QDir::home();
	d.cd(USER_DIR);
	if (!d.rename(oldFilename, newFilename)) {
		// Failed to delete file
		QMessageBox::critical(this, PRODUCT_NAME, 
				      tr("Failed to rename profile."));
	} else {
		// If there is a backup of the profile, rename it too.
		QString oldBackupFilename = oldFilename;
		oldBackupFilename.append("~");
		QString oldBackupFullname = d.filePath(oldBackupFilename);
		if (QFile::exists(oldBackupFullname)) {
			QString newBackupFilename = newFilename;
			newBackupFilename.append("~");
			d.rename(oldBackupFilename, newBackupFilename);
		}
		
		// Rename service files
		oldFilename = oldProfile;
		oldFilename.append(SVC_FILE_EXT);
		QString oldFullname = d.filePath(oldFilename);
		if (QFile::exists(oldFullname)) {
			newFilename = newProfile;
			newFilename.append(SVC_FILE_EXT);
			d.rename(oldFilename, newFilename);
		}
		
		// Rename service backup file
		oldFilename.append("~");
		oldFullname = d.filePath(oldFilename);
		if (QFile::exists(oldFullname)) {
			newFilename.append("~");
			d.rename(oldFilename, newFilename);
		}
		
		// Rename buddy list file
		oldFilename = oldProfile;
		oldFilename.append(BUDDY_FILE_EXT);
		oldFullname = d.filePath(oldFilename);
		if (QFile::exists(oldFullname)) {
			newFilename = newProfile;
			newFilename.append(BUDDY_FILE_EXT);
			d.rename(oldFilename, newFilename);
		}
		
		// Rename profile in list of default profiles in
		// system settings
		list<string> l = sys_config->get_start_user_profiles();
		if (std::find(l.begin(), l.end(), oldProfile.ascii()) != l.end())
		{
			std::replace(l.begin(), l.end(), oldProfile.ascii(), newProfile.ascii());
			sys_config->set_start_user_profiles(l);
				
			string error_msg;
			if (!sys_config->write_config(error_msg)) {
				// Failed to write config file
				((t_gui *)ui)->cb_show_msg(this, 
						error_msg, MSG_CRITICAL);
			}
		}
		
		emit profileRenamed();
		
		// Change profile name in the list box
		QCheckListItem *item = (QCheckListItem *)profileListView->currentItem();
		item->setText(0, newProfile);
	}
}

void SelectProfileForm::setAsDefault()
{
	// Only show the information when the default button is
	// pressed for the first time.
	if (!defaultSet) {
		QMessageBox::information(this, PRODUCT_NAME, tr(
			"<p>"
			"If you want to remove or "
			"change the default at a later time, you can do that "
			"via the system settings."
			"</p>"));
	}
	
	defaultSet = true;
	
	// Restore all pixmaps
	QListViewItemIterator i(profileListView);
	while (i.current()) {
		i.current()->setPixmap(0, QPixmap::fromMimeSource("penguin-small.png"));
		i++;
	}
	
	// Set pixmap of the default profiles.
	// Set default profiles in system settings.
	list<string> l;
	QListViewItemIterator j(profileListView, QListViewItemIterator::Checked);
	while (j.current()) {
		QCheckListItem *item = (QCheckListItem *)j.current();
		item->setPixmap(0, QPixmap::fromMimeSource("twinkle16.png"));
		l.push_back(item->text().ascii());
		j++;
	}
	sys_config->set_start_user_profiles(l);
	
	// Write default to system settings
	string error_msg;
	if (!sys_config->write_config(error_msg)) {
		// Failed to write config file
		((t_gui *)ui)->cb_show_msg(this, error_msg, MSG_CRITICAL);
	}
}

void SelectProfileForm::wizardProfile()
{
	wizardProfile(false);
}

void SelectProfileForm::wizardProfile(bool exec_mode)
{
	// Ask user for a profile name
	GetProfileNameForm getProfileNameForm(this, "get profile name", true);
	if (!getProfileNameForm.execNewName()) return;
	
	// Create file name
	QString profile = getProfileNameForm.getProfileName();
	QString filename = profile;
	filename.append(USER_FILE_EXT);
	
	// Create a new user config
	if (user_config) {
		MEMMAN_DELETE(user_config);
		delete user_config;
	}	
	user_config = new t_user();
	MEMMAN_NEW(user_config);
	user_config->set_config(filename.ascii());
	
	// Show the wizard form (modal dialog)
	WizardForm *f = new WizardForm(this, "wizard", true, Qt::WDestructiveClose);
	connect(f, SIGNAL(success()), this, SLOT(newProfileCreated()));
	
	if (exec_mode) {
		f->exec(user_config);
	} else {
		f->show(user_config);
	}
}

void SelectProfileForm::diamondcardProfile()
{
	diamondcardProfile(false);
}

void SelectProfileForm::diamondcardProfile(bool exec_mode)
{
	// Create a new user config
	if (user_config) {
		MEMMAN_DELETE(user_config);
		delete user_config;
	}	
	user_config = new t_user();
	MEMMAN_NEW(user_config);
	
	// Show the diamondcard profile form (modal dialog)
	DiamondcardProfileForm *f = new DiamondcardProfileForm(this, "diamondcard",
							       true, Qt::WDestructiveClose);
	connect(f, SIGNAL(success()), this, SLOT(newProfileCreated()));
	
	if (exec_mode) {
		f->exec(user_config);
	} else {
		f->show(user_config);
	}
}


void SelectProfileForm::sysSettings()
{
	SysSettingsForm *f = new SysSettingsForm(this, "system settings", true,
						 Qt::WDestructiveClose);
	f->show();
}

// Get a list of all profiles. Returns false if there is an error.
bool SelectProfileForm::getUserProfiles(QStringList &profiles, QString &error)
{
	// Find the .twinkle directory in HOME
	QDir d = QDir::home();
	if (!d.cd(USER_DIR)) {
		error = tr("Cannot find .twinkle directory in your home directory.");
		return false;
	}
	
	// Select all config files
	QString filterName = "*";
	filterName.append(USER_FILE_EXT);
	d.setFilter(QDir::Files);
	d.setNameFilter(filterName);
	d.setSorting(QDir::Name | QDir::IgnoreCase);
	profiles = d.entryList();
	
	return true;
}

void SelectProfileForm::fillProfileListView(const QStringList &profiles)
{
	// Put the profiles in the profile list view
	for (QStringList::ConstIterator i = profiles.begin(); i != profiles.end(); i++) {
		// Strip off the user file extension
		QString profile = *i;
		profile.truncate(profile.length() - strlen(USER_FILE_EXT));
		QCheckListItem *item = new QCheckListItem(
				profileListView, profile, QCheckListItem::CheckBox);
		item->setPixmap(0, QPixmap::fromMimeSource("penguin-small.png"));
	}
	
	// Highlight the first profile
	profileListView->setSelected(profileListView->firstChild(), true);
}

void SelectProfileForm::toggleItem(QListViewItem *item)
{
	QCheckListItem *checkItem = (QCheckListItem *)item;
	checkItem->setOn(!checkItem->isOn());
}

