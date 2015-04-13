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

#define PROV_NONE	QT_TRANSLATE_NOOP("WizardForm", "None (direct IP to IP calls)")
#define PROV_OTHER	QT_TRANSLATE_NOOP("WizardForm", "Other")

struct t_provider {
	QString domain;
	QString sip_proxy;
	QString stun_server;
};

void WizardForm::init()
{
	QRegExp rxNoSpace("\\S*");
	
	// Set validators
	usernameLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	domainLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	authNameLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	proxyLineEdit->setValidator(new QRegExpValidator(rxNoSpace, this));
	
	initProviders();
	serviceProviderComboBox->setCurrentItem(serviceProviderComboBox->count() - 1);
	update(tr(PROV_OTHER));
}

void WizardForm::initProviders()
{
	serviceProviderComboBox->clear();
	serviceProviderComboBox->insertItem(tr(PROV_NONE));
	
	QString fname = sys_config->get_dir_share().c_str();
	fname.append("/").append(FILE_PROVIDERS);
	QFile providersFile(fname);
	if (providersFile.open(IO_ReadOnly)) {
		QTextStream providersStream(&providersFile);
		QString entry;
		while ((entry = providersStream.readLine()) != QString::null) {
			// Skip comment
			if (entry[0] == '#') continue;
			
			QStringList l = QStringList::split(";", entry, true);
			
			// Skip invalid lines
			if (l.size() != 4) continue;
			
			t_provider p;
			p.domain = l[1];
			p.sip_proxy = l[2];
			p.stun_server = l[3];
			mapProviders[l[0]] = p;
			
			serviceProviderComboBox->insertItem(l[0]);
		}
		providersFile.close();
	}
	
	serviceProviderComboBox->insertItem(tr(PROV_OTHER));
}

int WizardForm::exec(t_user *user)
{
	user_config = user;
	
	// Set user profile name in the titlebar
	QString s = PRODUCT_NAME;
	s.append(" - ").append(tr("User profile wizard:")).append(" ");
	s.append(user_config->get_profile_name().c_str());
	setCaption(s);
	
	return QDialog::exec();
}

void WizardForm::show(t_user *user)
{
	user_config = user;
	
	// Set user profile name in the titlebar
	QString s = PRODUCT_NAME;
	s.append(" - ").append(tr("User profile wizard:")).append(" ");
	s.append(user_config->get_profile_name().c_str());
	setCaption(s);
	
	QDialog::show();
}

void WizardForm::update(const QString &item)
{
	// Disable/Enable controls
	if (item == tr(PROV_NONE)) {
		suggestAuthName = false;
		authNameTextLabel->setEnabled(false);
		authNameLineEdit->setEnabled(false);
		authPasswordTextLabel->setEnabled(false);
		authPasswordLineEdit->setEnabled(false);
		proxyTextLabel->setEnabled(false);
		proxyLineEdit->setEnabled(false);
		stunServerTextLabel->setEnabled(false);
		stunServerLineEdit->setEnabled(false);
	} else {
		if (usernameLineEdit->text() == authNameLineEdit->text()) {
			suggestAuthName = true;
		} else {
			suggestAuthName = false;
		}
		
		authNameTextLabel->setEnabled(true);
		authNameLineEdit->setEnabled(true);
		authPasswordTextLabel->setEnabled(true);
		authPasswordLineEdit->setEnabled(true);
		proxyTextLabel->setEnabled(true);
		proxyLineEdit->setEnabled(true);
		stunServerTextLabel->setEnabled(true);
		stunServerLineEdit->setEnabled(true);
	}
	
	// Set values
	if (item == tr(PROV_NONE)) {
		domainLineEdit->clear();
		authNameLineEdit->clear();
		authPasswordLineEdit->clear();
		proxyLineEdit->clear();
		stunServerLineEdit->clear();
	} else if (item == tr(PROV_OTHER)) {
		domainLineEdit->clear();
		stunServerLineEdit->clear();
		proxyLineEdit->clear();
	} else {
		t_provider p = mapProviders[item];
		domainLineEdit->setText(p.domain);
		proxyLineEdit->setText(p.sip_proxy);
		stunServerLineEdit->setText(p.stun_server);
	}
}

void WizardForm::updateAuthName(const QString &s)
{
	if (suggestAuthName) {
		authNameLineEdit->setText(s);
	}
}

void WizardForm::disableSuggestAuthName()
{
	suggestAuthName = false;
}

void WizardForm::validate()
{
	QString s;
	
	// Validity check user page
	// SIP username is mandatory
	if (usernameLineEdit->text().isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this, tr("You must fill in a user name for your SIP account.").ascii(),
				MSG_CRITICAL);
		usernameLineEdit->setFocus();
		return;
	}
	
	// SIP user domain is mandatory
	if (domainLineEdit->text().isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this, tr(
				"You must fill in a domain name for your SIP account.\n"
				"This could be the hostname or IP address of your PC "
				"if you want direct PC to PC dialing.").ascii(),
				MSG_CRITICAL);
		domainLineEdit->setFocus();
		return;
	}
	
	// SIP proxy
	if (proxyLineEdit->text() != "") {
		s = USER_SCHEME;
		s.append(':').append(proxyLineEdit->text());
		t_url u(s.ascii());
		if (!u.is_valid() || u.get_user() != "") {
			((t_gui *)ui)->cb_show_msg(this, tr("Invalid value for SIP proxy.").ascii(), 
					MSG_CRITICAL);
			proxyLineEdit->setFocus();
			proxyLineEdit->selectAll();
			return;
		}
	}
	
	// Register and publish presence at startup
	if (serviceProviderComboBox->currentText() == tr(PROV_NONE)) {
		user_config->set_register_at_startup(false);
		user_config->set_pres_publish_startup(false);
	}
	
	// STUN server
	if (stunServerLineEdit->text() != "") {
		s = "stun:";
		s.append(stunServerLineEdit->text());
		t_url u(s.ascii());
		if (!u.is_valid() || u.get_user() != "") {
			((t_gui *)ui)->cb_show_msg(this, tr("Invalid value for STUN server.").ascii(), 
					MSG_CRITICAL);
			stunServerLineEdit->setFocus();
			stunServerLineEdit->selectAll();
			return;
		}
	}
	
	// Set all values in the user_config object
	// USER
	user_config->set_display(displayLineEdit->text().ascii());
	user_config->set_name(usernameLineEdit->text().ascii());
	user_config->set_domain(domainLineEdit->text().ascii());
	user_config->set_auth_name(authNameLineEdit->text().ascii());
	user_config->set_auth_pass(authPasswordLineEdit->text().ascii());
	
	// SIP SERVER
	user_config->set_use_outbound_proxy(!proxyLineEdit->text().isEmpty());
	s = USER_SCHEME;
	s.append(':').append(proxyLineEdit->text());
	user_config->set_outbound_proxy(t_url(s.ascii()));
	
	// NAT
	user_config->set_use_stun(!stunServerLineEdit->text().isEmpty());
	s = "stun:";
	s.append(stunServerLineEdit->text());
	user_config->set_stun_server(t_url(s.ascii()));
	
	// Save user config
	string error_msg;
	if (!user_config->write_config(user_config->get_filename(), error_msg)) {
		// Failed to write config file
		((t_gui *)ui)->cb_show_msg(this, error_msg, MSG_CRITICAL);
		return;
	}
	
	emit success();
	accept();
}
