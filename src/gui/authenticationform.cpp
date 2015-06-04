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

#include "authenticationform.h"
/*
 *  Constructs a AuthenticationForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
AuthenticationForm::AuthenticationForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
AuthenticationForm::~AuthenticationForm()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AuthenticationForm::languageChange()
{
	retranslateUi(this);
}


int AuthenticationForm::exec(t_user *user_config, const QString &realm, QString &username,
			     QString &password)
{
	int retval;
	
	profileValueTextLabel->setText(user_config->get_profile_name().c_str());
	userValueTextLabel->setText(user_config->get_display_uri().c_str());
	realmTextLabel->setText(realm);
	usernameLineEdit->setText(username);
	passwordLineEdit->setText(password);
	if (!username.isEmpty()) passwordLineEdit->setFocus();
	retval = QDialog::exec();
	username = usernameLineEdit->text();
	password = passwordLineEdit->text();
	
	return retval;
}
