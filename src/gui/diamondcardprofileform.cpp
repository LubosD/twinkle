//Added by qt3to4:
#include <QEvent>
#include <QMouseEvent>

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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QRegExp>
#include <QValidator>
#include <QRegExpValidator>
#include "gui.h"
#include "diamondcard.h"
#include "getprofilenameform.h"
#include "audits/memman.h"
#include "diamondcardprofileform.h"
/*
 *  Constructs a DiamondcardProfileForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
DiamondcardProfileForm::DiamondcardProfileForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
DiamondcardProfileForm::~DiamondcardProfileForm()
{
	destroy();
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void DiamondcardProfileForm::languageChange()
{
	retranslateUi(this);
}


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
        ((t_gui *)ui)->cb_show_msg(this, tr("Fill in your account ID.").toStdString(),
					   MSG_CRITICAL);
		accountIdLineEdit->setFocus();
		return;
	}
	
	if (pinCodeLineEdit->text().isEmpty()) {
        ((t_gui *)ui)->cb_show_msg(this, tr("Fill in your PIN code.").toStdString(),
					   MSG_CRITICAL);
		pinCodeLineEdit->setFocus();
		return;
	}
	
	QString profileName("Diamondcard-");
	profileName.append(accountIdLineEdit->text());
	QString filename(profileName);
	filename.append(USER_FILE_EXT);
	
	// Create a new user config
    while (!user_config->set_config(filename.toStdString())) {
		((t_gui *)ui)->cb_show_msg(this, 
            tr("A user profile with name %1 already exists.").arg(profileName).toStdString(),
			MSG_WARNING);
		
		// Ask user for a profile name
        GetProfileNameForm getProfileNameForm(this);
        getProfileNameForm.setModal(true);
		if (!getProfileNameForm.execNewName()) return;
		
		profileName = getProfileNameForm.getProfileName();
		filename = profileName;
		filename.append(USER_FILE_EXT);
	}
	
	diamondcard_set_user_config(*user_config, 
                    nameLineEdit->text().toStdString(),
                    accountIdLineEdit->text().toStdString(),
                    pinCodeLineEdit->text().toStdString());
	
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

void DiamondcardProfileForm::signUpLinkActivated()
{
	string url = diamondcard_url(DC_ACT_SIGNUP, "", "");
	((t_gui *)ui)->open_url_in_browser(url.c_str());
}
