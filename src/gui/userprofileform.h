#ifndef USERPROFILEFORM_H
#define USERPROFILEFORM_H
#include <list>
#include <map>
#include <QList>
#include "user.h"
#include "ui_userprofileform.h"

class UserProfileForm : public QDialog, public Ui::UserProfileForm
{
	Q_OBJECT

public:
    UserProfileForm(QWidget* parent = 0);
	~UserProfileForm();

	virtual t_audio_codec label2codec( const QString & label );
	virtual QString codec2label( t_audio_codec & codec );
	virtual int ext_support2indexComboItem( t_ext_support ext );
	virtual t_ext_support indexComboItem2ext_support( int index );
	virtual int exec( list<t_user *> profiles, QString show_profile );
    virtual bool check_dynamic_payload( QSpinBox * spb, QList<int> & checked_list );
	virtual list<t_number_conversion> get_number_conversions();
	virtual bool validateValues();

public slots:
	virtual void showCategory( int index );
	virtual void populate();
	virtual void initProfileList( list<t_user *> profiles, QString show_profile_name );
	virtual void show( list<t_user *> profiles, QString show_profile );
	virtual void validate();
	virtual void changeProfile( const QString & profileName );
	virtual void chooseFile( QLineEdit * qle, const QString & filter, const QString & caption );
	virtual void chooseRingtone();
	virtual void chooseRingback();
	virtual void chooseIncomingCallScript();
	virtual void chooseInCallAnsweredScript();
	virtual void chooseInCallFailedScript();
	virtual void chooseOutgoingCallScript();
	virtual void chooseOutCallAnsweredScript();
	virtual void chooseOutCallFailedScript();
	virtual void chooseLocalReleaseScript();
	virtual void chooseRemoteReleaseScript();
	virtual void addCodec();
	virtual void removeCodec();
	virtual void upCodec();
	virtual void downCodec();
	virtual void upConversion();
	virtual void downConversion();
	virtual void addConversion();
	virtual void editConversion();
	virtual void removeConversion();
	virtual void testConversion();
	virtual void changeMWIType( int idxMWIType );
	virtual void changeSipTransportProtocol( int idx );

signals:
	void stunServerChanged(t_user *);
	void authCredentialsChanged(t_user *, const string &);
	void sipUserChanged(t_user *);
	void success();
	void mwiChangeUnsubscribe(t_user *);
	void mwiChangeSubscribe(t_user *);

protected slots:
	virtual void languageChange();

private:
	map<t_user *, int> map_last_cat;
	t_user *current_profile;
	int current_profile_idx;
	list<t_user *> profile_list;

	void init();

};


#endif
