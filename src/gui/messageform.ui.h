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

#ifdef HAVE_KDE
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <krun.h>
#include <kuserprofile.h>
#else
#include "utils/mime_database.h"
#endif

using namespace utils;

/** Maximum width for an inline image */
#define MAX_WIDTH_IMG_INLINE 400

/** Maximum height for an inline image */
#define MAX_HEIGHT_IMG_INLINE 400

#define IMG_SCALE_FACTOR(width, height) (std::min<float>( float(MAX_WIDTH_IMG_INLINE) / (width), float(MAX_HEIGHT_IMG_INLINE) / (height) ) )

void MessageForm::init()
{
	setWFlags(getWFlags() | Qt::WDestructiveClose);
	
	_getAddressForm = 0;
	_remotePartyComplete = false;
	
	// Add label to display size of typed message
	_msgSizeLabel = new QLabel(this);
	statusBar()->addWidget(_msgSizeLabel);
	showMessageSize();
	
	// Set toolbutton icons for disabled options.
	setDisabledIcon(addressToolButton, "kontact_contacts-disabled.png");
	
	attachmentPopupMenu = new QPopupMenu(this);
	MEMMAN_NEW(attachmentPopupMenu);
	
	connect(attachmentPopupMenu, SIGNAL(activated(int)), 
		this, SLOT(attachmentPopupActivated(int)));
	
	_serviceMap = NULL;
	_saveAsDialog = NULL;
	
	_isComposingLabel = NULL;
	
	// When the user edits the message, the composition indication
	// will be set to active.
	connect(msgLineEdit, SIGNAL(textChanged(const QString &)),
		this, SLOT(setLocalComposingIndicationActive()));
}

void MessageForm::destroy()
{
	if (_getAddressForm) {
		MEMMAN_DELETE(_getAddressForm);
		delete _getAddressForm;
	}
	
	MEMMAN_DELETE(attachmentPopupMenu);
	delete attachmentPopupMenu;
	
#ifdef HAVE_KDE
	vector<KService::Ptr> *serviceMap = (vector<KService::Ptr> *)_serviceMap;
	if (serviceMap) {
		MEMMAN_DELETE(serviceMap);
		delete serviceMap;
	}
#endif
	
	if (_saveAsDialog) {
		MEMMAN_DELETE(_saveAsDialog);
		delete _saveAsDialog;
	}
	
	if (_isComposingLabel) {
		MEMMAN_DELETE(_isComposingLabel);
		delete _isComposingLabel;
	}
}

void MessageForm::closeEvent(QCloseEvent *e)
{
	MEMMAN_DELETE(this); // destructive close
	QMainWindow::closeEvent(e);
}


void MessageForm::show()
{
	if (toLineEdit->text().isEmpty()) {
		sendFileAction->setEnabled(false);
		toLineEdit->setFocus();
	} else {
		// Once a message session has been created, the
		// source and destination cannot be changed anymore.
		fromComboBox->setEnabled(false);
		toLineEdit->setEnabled(false);
		addressToolButton->setEnabled(false);
		sendFileAction->setEnabled(true);
		msgLineEdit->setFocus();
	}
	
	QMainWindow::show();
}

void MessageForm::selectUserConfig(t_user *user_config)
{
	for (int i = 0; i < fromComboBox->count(); i++) {
		if (fromComboBox->text(i) == 
		    user_config->get_profile_name().c_str())
		{
			fromComboBox->setCurrentItem(i);
			break;
		}
	}
}

void MessageForm::showAddressBook()
{
	if (!_getAddressForm) {
		_getAddressForm = new GetAddressForm(
				this, "select address", true);
		MEMMAN_NEW(_getAddressForm);
	}
	
	connect(_getAddressForm, 
		SIGNAL(address(const QString &)),
		this, SLOT(selectedAddress(const QString &)));
	
	_getAddressForm->show();
}

void MessageForm::selectedAddress(const QString &address)
{
	toLineEdit->setText(address);
}

/**
  * Check if there is a valid sender and receiver. If so, then set the sender
  * and receiver addresses in the message session object.
  */
bool MessageForm::updateMessageSession()
{
	string display, dest_str;
	t_user *from_user = phone->ref_user_profile(
				fromComboBox->currentText().ascii());
	if (!from_user) {
		// The user profile is not active anymore
		fromComboBox->setFocus();
		return false;
	}
	
	ui->expand_destination(from_user, toLineEdit->text().stripWhiteSpace().ascii(), 
			       display, dest_str);
	t_url dest(dest_str);
	
	if (!dest.is_valid()) {
		toLineEdit->setFocus();
		toLineEdit->selectAll();
		return false;
	}
	
	_msgSession->set_user(from_user);
	_msgSession->set_remote_party(t_display_url(dest, display));
	
	setRemotePartyCaption();

	return true;
}

/**
  * Determine if a message can be sent, i.e. there is a valid sender and
  * receiver. If a message can be sent, then lock the sender and receiver.
  * @return True if message can be sent, false otherwise.
  */
bool MessageForm::prepareSendMessage() {
	if (toLineEdit->text().isEmpty()) {
		// No recipient selected.
		return false;
	}
	
	if (toLineEdit->isEnabled()) {
		if (!updateMessageSession()) {
			// No valid sender and receiver.
			return false;
		}

		// Once a message session has been created, the
		// source and destination cannot be changed anymore.
		fromComboBox->setEnabled(false);
		toLineEdit->setEnabled(false);
		addressToolButton->setEnabled(false);	
		
	}
	
	return true;
}

/** Send a text message */
void MessageForm::sendMessage() {
	if (msgLineEdit->text().isEmpty()) {
		// Nothing to send
		return;
	}
	
	if (!prepareSendMessage()) {
		return;
	}
	
	_msgSession->send_msg(msgLineEdit->text().ascii(), im::TXT_PLAIN);	
}

/**
  * Send a file.
  * @param filename [in] Name of file to send.
  * @param subject [in] Subject to set in the message.
  */
void MessageForm::sendFile(const QString &filename, const QString &subject) {
	if (!prepareSendMessage()) {
		return;
	}
	
	t_media media("application/octet-stream");
	
#ifdef HAVE_KDE
	// Get mime type for the attachment
	KMimeType::Ptr pMime = KMimeType::findByURL(filename);
	media = t_media(pMime->name().ascii());
#else
	string mime_type = mime_database->get_mimetype(filename.ascii());
	if (!mime_type.empty()) {
		media = t_media(mime_type);
	}
#endif

	_msgSession->send_file(filename.ascii(), media, subject.ascii());
}

/**
  * Add a message to the converstation broswer.
  * @param msg [in] The message to add.
  * @param name [in] The name of the sender of the message.
  */
void MessageForm::addMessage(const im::t_msg &msg, const QString &name)
{
	QString s = "<b>";
	
	// Timestamp and name of sender
	if (msg.direction == im::MSG_DIR_IN) s += "<font color=\"blue\">";
	s += time2str(msg.timestamp, "%H:%M:%S ").c_str();
	s += QStyleSheet::escape(name);
	if (msg.direction == im::MSG_DIR_IN) s += "</font>";
	s += "</b>";
	
	// Subject
	if (!msg.subject.empty()) {
		s += "<br>";
		s += "<b>";
		s += "Subject: ";
		s += "</b>";
		s += msg.subject.c_str();
	}
	
	// Text message
	if (!msg.message.empty()) {
		s += "<br>";
		if (msg.format == im::TXT_HTML) {
			s += msg.message.c_str();
		} else {
			s += QStyleSheet::escape(msg.message.c_str());
		}
	}
	
	// Attachment
	if (msg.has_attachment) {
		s += "<br>";
		s += "<a href=\"";
		s += msg.attachment_filename.c_str();
		s += "\">";
		
		bool show_attachment_inline = false;
		bool scale_image = false;
		int scaled_width = 0, scaled_height = 0;
		
		if (msg.attachment_media.type == "image") {
			// Show image inline if possible
			QPixmap image (msg.attachment_filename.c_str());
			if (!image.isNull()) {
				show_attachment_inline = true;
				if (image.width() > MAX_WIDTH_IMG_INLINE &&
				    image.height() > MAX_HEIGHT_IMG_INLINE)
				{
					// Shrink image
					scaled_width = int(image.width() * IMG_SCALE_FACTOR(image.width(), image.height()));
					scaled_height = int(image.height() * IMG_SCALE_FACTOR(image.width(), image.height()));
					scale_image = true;
				}
			}
		}
		
		s += "<img ";
		
		if (scale_image) {
			s += "width=";
			s += QString().setNum(scaled_width);
			s += " height=";
			s += QString().setNum(scaled_height);
			s += " ";
		}
		     
		s += "src=\"";
		
		if (show_attachment_inline) {
			s += msg.attachment_filename.c_str();
		} else {
			// Show an icon representing the attachment
#ifdef HAVE_KDE		
			KIconLoader iconLoader;
			QString iconName = KMimeType::iconForURL(
					msg.attachment_filename.c_str());
			s += iconLoader.iconPath(iconName, KIcon::Desktop);
#else
			// Set icon based on main mime type
			s += "mime_";
			s += msg.attachment_media.type.c_str();
			s += ".png";
#endif
		}
		
		s += "\">";
		s+= "<br>";
		
		s += msg.attachment_save_as_name.c_str();
		
		s += "</a> ";
		
		if (scale_image) {
			s += "<br>";
			s += "<i>";
			s += tr("image size is scaled down in preview");
			s += "</i>";
		}
		
		// Store the association between the tempory file name of the
		// save attachment and the suggested save-as file name. When
		// the user clicks on the attachment, only the temporary file name
		// is available. Through this association the suggested file name
		// can be retrieved.
		_filenameMap[msg.attachment_filename] = msg.attachment_save_as_name;
	}

	conversationBrowser->append(s);
}

void MessageForm::displayError(const QString &errorMsg)
{
	QString s = "<font color =\"red\">";
	s += "<b>";
	s += tr("Delivery failure").ascii();
	s += ": </b>";
	s += QStyleSheet::escape(errorMsg);
	s += "</font>";
	
	conversationBrowser->append(s);
}

void MessageForm::displayDeliveryNotification(const QString &notification)
{
	QString s = "<font color =\"darkgreen\">";
	s += "<b>";
	s += tr("Delivery notification").ascii();
	s += ": </b>";
	s += QStyleSheet::escape(notification);
	s += "</font>";
	
	conversationBrowser->append(s);
}

void MessageForm::setRemotePartyCaption(void) {
	if (!_msgSession) return;
	t_user *user = _msgSession->get_user();
	t_display_url remote_party = _msgSession->get_remote_party();
	setCaption(ui->format_sip_address(user, 
			remote_party.display, remote_party.url).c_str());
}

void MessageForm::showAttachmentPopupMenu(const QString &attachment) {
#ifdef HAVE_KDE
	vector<KService::Ptr> *serviceMap = (vector<KService::Ptr> *)_serviceMap;
	
	if (serviceMap) {
		MEMMAN_DELETE(serviceMap);
		delete serviceMap;
	}
	serviceMap = new vector<KService::Ptr>;
	MEMMAN_NEW(serviceMap);
	_serviceMap = (void *)serviceMap;
#endif
	
	int id = 0; // Identity of popup menu item
	
	// Store attachment. When the user selects an attachment we still
	// know which attachment was clicked.
	clickedAttachment = attachment;
	
	attachmentPopupMenu->clear();
	
	QIconSet saveIcon(QPixmap::fromMimeSource("save_as.png"));
	attachmentPopupMenu->insertItem(saveIcon, "Save as...", id++);
	
#ifdef HAVE_KDE
	// Get mime type for the attachment
	KMimeType::Ptr pMime = KMimeType::findByURL(attachment);
	
	// Get applications that can open the mime type
	KServiceTypeProfile::OfferList services = KServiceTypeProfile::offers(
			pMime->name(), "Application");
	
	KServiceTypeProfile::OfferList::ConstIterator it;
	for (it = services.begin(); it != services.end(); ++it) {
		KService::Ptr service = (*it).service();
		serviceMap->push_back(service);
		QString menuText = tr("Open with %1...").arg(service->name());
		QPixmap pixmap = service->pixmap(KIcon::Small);
		QIconSet iconSet;
		iconSet.setPixmap(pixmap, QIconSet::Small);
		attachmentPopupMenu->insertItem(iconSet, menuText, id++);
	}
	
	QIconSet openIcon(QPixmap::fromMimeSource("fileopen.png"));
	attachmentPopupMenu->insertItem(openIcon, tr("Open with..."), id++);
#endif
	
	attachmentPopupMenu->popup(QCursor::pos());
}

void MessageForm::attachmentPopupActivated(int id) {
#ifdef HAVE_KDE
	vector<KService::Ptr> *serviceMap = (vector<KService::Ptr> *)_serviceMap;
	assert(serviceMap);
#endif
	
	if (id == 0) {
#ifdef HAVE_KDE		
		KFileDialog *d = new KFileDialog(QString::null, QString::null, this, 0, true);
		MEMMAN_NEW(d);
		d->setOperationMode(KFileDialog::Saving);
		
		connect(d, SIGNAL(okClicked()), this,
			SLOT(saveAttachment()));
#else
		QFileDialog *d = new QFileDialog(QString::null, QString::null, this, 0, true);
		MEMMAN_NEW(d);
		d->setMode(QFileDialog::AnyFile);
		
		connect(d, SIGNAL(fileSelected(const QString &)), this,
			SLOT(saveAttachment()));
#endif
		d->setSelection(_filenameMap[clickedAttachment.ascii()].c_str());
		d->setCaption(tr("Save attachment as..."));
		
		if (_saveAsDialog) {
			MEMMAN_DELETE(_saveAsDialog);
			delete _saveAsDialog;
		}
		_saveAsDialog = d;
		
		d->show();
#ifdef HAVE_KDE
	} else if (id > serviceMap->size()) {
		KURL::List urls;
		urls << clickedAttachment;
		KRun::displayOpenWithDialog(urls, false);
	} else {
		KURL::List urls;
		urls << clickedAttachment;
		KRun::run(*serviceMap->at(id-1), urls);
#endif
	}
}

void MessageForm::saveAttachment() {
#ifdef HAVE_KDE
	KFileDialog *d = dynamic_cast<KFileDialog *>(_saveAsDialog);
#else
	QFileDialog *d = dynamic_cast<QFileDialog *>(_saveAsDialog);
#endif
	QString filename = d->selectedFile();
	
	if (QFile::exists(filename)) {
		bool overwrite = ((t_gui *)ui)->cb_ask_msg(this, 
				  tr("File already exists. Do you want to overwrite this file?").ascii(),
				  MSG_WARNING);
		
		if (!overwrite) return;
	}
	
	if (!filecopy(clickedAttachment.ascii(), filename.ascii())) {
		((t_gui *)ui)->cb_show_msg(this, tr("Failed to save attachment.").ascii(), 
					   MSG_CRITICAL);
	}
}

/** Choose a file to send */
void MessageForm::chooseFileToSend()
{
	// Indicate that a message is being composed.
	setLocalComposingIndicationActive();
	
	SendFileForm *form = new SendFileForm();
	MEMMAN_NEW(form);
	// Form will auto destruct itself on close.
	
	connect(form, SIGNAL(selected(const QString &, const QString &)),
		this, SLOT(sendFile(const QString &, const QString &)));
	
	form->show();
}

/** 
 * Show an is-composing indication in the status bar.
 * @param name [in] The name of the sender of the message.
 */
void MessageForm::setComposingIndication(const QString &name)
{

	if (!_isComposingLabel) {
		_isComposingLabel = new QLabel(NULL);
		MEMMAN_NEW(_isComposingLabel);
		
		_isComposingLabel->setText(tr("%1 is typing a message.").arg(name));
		_isComposingLabel->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
		statusBar()->addWidget(_isComposingLabel);
	}
}

/** Clear an is-composing indication from the status bar. */
void MessageForm::clearComposingIndication()
{
	if (_isComposingLabel) {
		statusBar()->removeWidget(_isComposingLabel);
		statusBar()->clear();
	
		MEMMAN_DELETE(_isComposingLabel);
		delete _isComposingLabel;
		_isComposingLabel = NULL;
	}
}

/** Set the local composition indication to active. */
void MessageForm::setLocalComposingIndicationActive()
{
	_msgSession->set_local_composing_state(im::COMPOSING_STATE_ACTIVE);
}

void MessageForm::keyPressEvent(QKeyEvent *e)
{
	switch (e->key()) {
	case Qt::Key_Return:
	case Qt::Key_Enter:
		if (sendPushButton->isEnabled()) {
			sendPushButton->animateClick();
		}
		break;
	default:
		e->ignore();
	}
}

void MessageForm::toAddressChanged(const QString &address)
{
	sendFileAction->setEnabled(!address.isEmpty());
}

/** Show the size of the typed message */
void MessageForm::showMessageSize()
{
	uint len = msgLineEdit->text().length();
	
	QString s(tr("Size"));
	s += ": ";
	s += QString().setNum(len);
	
	_msgSizeLabel->setText(s);
}
