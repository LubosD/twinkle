#ifndef DIAMONDCARDPROFILEFORM_H
#define DIAMONDCARDPROFILEFORM_H
#include <QLabel>
#include <QLineEdit>
#include "user.h"
#include "ui_diamondcardprofileform.h"

class DiamondcardProfileForm : public QDialog, public Ui_DiamondcardProfileForm
{
	Q_OBJECT

public:
    DiamondcardProfileForm(QWidget* parent = 0);
	~DiamondcardProfileForm();

	virtual int exec( t_user * user );

public slots:
	virtual void destroyOldUserConfig();
	virtual void show( t_user * user );
	virtual void validate();
	virtual void signUpLinkActivated();

signals:
	void success();
	void newDiamondcardProfile(const QString&);

protected slots:
	virtual void languageChange();

private:
	t_user *user_config;
	bool destroy_user_config;

	void init();
	void destroy();

};


#endif
