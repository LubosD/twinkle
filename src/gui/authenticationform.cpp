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

#include "authenticationform.h"
#include <QtDebug>

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

    qDebug()  << "profile parameter: " << QString::fromStdString(user_config->get_profile_name());
    qDebug()  << "username parameter: " << QString::fromStdString(username.toStdString());
    qDebug()  << "pw parameter: (suppressed)" /*<< password.toStdString()*/;

    userValueTextLabel->setText(user_config->get_display_uri().c_str());
    realmTextLabel->setText(realm);
    profileValueTextLabel->setText(user_config->get_profile_name().c_str());
    if (username.isEmpty()) {
        qDebug()  << "retrieving credentials from profile:";
        string userFromProfile = user_config->get_auth_name();
        string pwFromProfile = user_config->get_auth_pass();

        qDebug() << "user from profile: " << QString::fromStdString(userFromProfile);
        qDebug()  << "pw from profile: (suppressed)" /*<< QString::fromStdString(pwFromProfile)*/;

        passwordLineEdit->setText(pwFromProfile.c_str());
        usernameLineEdit->setText(userFromProfile.c_str());
    } else {
        usernameLineEdit->setText(username);
        passwordLineEdit->setText(password);
        passwordLineEdit->setFocus();
    }
	retval = QDialog::exec();
	username = usernameLineEdit->text();
	password = passwordLineEdit->text();
	
	return retval;
}
