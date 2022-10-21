#ifndef MESSAGEFORM_H
#define MESSAGEFORM_H
#include "getaddressform.h"
#include "im/msg_session.h"
#include "phone.h"
#include <QLabel>
#include <QtCore/QStringRef>
#include "user.h"
#include "ui_messageform.h"
#include <QMainWindow>
#include <QMenu>

class t_phone;
extern t_phone *phone;

class MessageForm : public QMainWindow, public Ui::MessageForm
{
	Q_OBJECT

public:
    MessageForm(QWidget* parent = 0);
	~MessageForm();

	virtual bool updateMessageSession();
	virtual bool prepareSendMessage();

public slots:
	virtual void closeEvent( QCloseEvent * e );
	virtual void show();
	virtual void selectUserConfig( t_user * user_config );
	virtual void showAddressBook();
	virtual void selectedAddress( const QString & address );
	virtual void sendMessage();
	virtual void sendFile( const QString & filename, const QString & subject );
	virtual void addMessage( const im::t_msg & msg, const QString & name );
	virtual void displayError( const QString & errorMsg );
	virtual void displayDeliveryNotification( const QString & notification );
	virtual void setRemotePartyCaption( void );
	virtual void showAttachmentPopupMenu( const QUrl & attachment );
    virtual void attachmentPopupActivated( QAction* action );
	virtual void saveAttachment();
	virtual void chooseFileToSend();
	virtual void setComposingIndication( const QString & name );
	virtual void clearComposingIndication();
	virtual void setLocalComposingIndicationActive();
	virtual void keyPressEvent( QKeyEvent * e );
	virtual void toAddressChanged( const QString & address );
	virtual void showMessageSize();

protected:
	im::t_msg_session *_msgSession;

protected slots:
	virtual void languageChange();

private:
	QDialog *_saveAsDialog;
	map<string, string> _filenameMap;
	bool _remotePartyComplete;
	GetAddressForm *_getAddressForm;
	QMenu *attachmentPopupMenu;
	QString clickedAttachment;
	void *_serviceMap;
	QLabel *_isComposingLabel;
	QLabel *_msgSizeLabel;

	void init();
	void destroy();

};


#endif
