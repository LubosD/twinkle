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

void DiamondcardProfileForm::init()
{
	user_config = NULL;
	destroy_user_config = false;
	
	QRegExp rxNoSpace("\\S*");
	accountIdLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	pinCodeLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
}

void DiamondcardProfileForm::destroy()
{
	destroyOldUserConfig();
}

void DiamondcardProfileForm::destroyOldUserConfig()
{
	if (user_config && destroy_user_config) {
		MEMMAN_DELETE(user_config);
		delete user_config;
	}
	user_config = NULL;
}

// Show the form
void DiamondcardProfileForm::show(t_user *user)
{
	destroyOldUserConfig();
	
	if (user) {
		user_config = user;
		destroy_user_config = false;
	} else {
		user_config = new t_user();
		MEMMAN_NEW(user_config);
		destroy_user_config = true;
	}
	QDialog::show();
}

// Modal execution
int DiamondcardProfileForm::exec(t_user *user)
{
	destroyOldUserConfig();
	user_config = user;
	destroy_user_config = false;
	return QDialog::exec();
}

void DiamondcardProfileForm::validate()
{
	if (accountIdLineEdit->text().isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this, tr("Fill in your account ID.").ascii(), 
					   MSG_CRITICAL);
		accountIdLineEdit->setFocus();
		return;
	}
	
	if (pinCodeLineEdit->text().isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this, tr("Fill in your PIN code.").ascii(), 
					   MSG_CRITICAL);
		pinCodeLineEdit->setFocus();
		return;
	}
	
	QString profileName("Diamondcard-");
	profileName.append(accountIdLineEdit->text());
	QString filename(profileName);
	filename.append(USER_FILE_EXT);
	
	// Create a new user config
	while (!user_config->set_config(filename.ascii())) {
		((t_gui *)ui)->cb_show_msg(this, 
			tr("A user profile with name %1 already exists.").arg(profileName).ascii(), 
			MSG_WARNING);
		
		// Ask user for a profile name
		GetProfileNameForm getProfileNameForm(this, "get profile name", true);
		if (!getProfileNameForm.execNewName()) return;
		
		profileName = getProfileNameForm.getProfileName();
		filename = profileName;
		filename.append(USER_FILE_EXT);
	}
	
	diamondcard_set_user_config(*user_config, 
				    nameLineEdit->text().ascii(),
				    accountIdLineEdit->text().ascii(),
				    pinCodeLineEdit->text().ascii());
	
	string error_msg;
	if (!user_config->write_config(user_config->get_filename(), error_msg)) {
		// Failed to write config file
		((t_gui *)ui)->cb_show_msg(this, error_msg, MSG_CRITICAL);
		return;
	}
	
	emit newDiamondcardProfile(user_config->get_filename().c_str());
	emit success();
	accept();
}

// Handle mouse clicks on labels.
void DiamondcardProfileForm::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton && e->type() == QEvent::MouseButtonRelease) {
		processLeftMouseButtonRelease(e);
	} else {
		e->ignore();
	}
}

void DiamondcardProfileForm::processLeftMouseButtonRelease(QMouseEvent *e)
{
	if (signUpTextLabel->hasMouse()) {
		string url = diamondcard_url(DC_ACT_SIGNUP, "", "");
		((t_gui *)ui)->open_url_in_browser(url.c_str());
	} else {
		e->ignore();
	}
}
