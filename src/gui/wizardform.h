#ifndef WIZARDFORM_H
#define WIZARDFORM_H

struct t_provider;

#include <map>
#include "user.h"
#include "ui_wizardform.h"

class WizardForm : public QDialog, public Ui::WizardForm
{
	Q_OBJECT

public:
    WizardForm(QWidget* parent = 0);
	~WizardForm();

	virtual void show( t_user * user );

public slots:
	virtual void initProviders();
	virtual int exec( t_user * user );
	virtual void update( const QString & item );
	virtual void updateAuthName( const QString & s );
	virtual void disableSuggestAuthName();
	virtual void validate();

signals:
	void success();

protected slots:
	virtual void languageChange();

private:
	bool suggestAuthName;
	std::map<QString, t_provider> mapProviders;
	t_user *user_config;

	void init();

};


#endif
