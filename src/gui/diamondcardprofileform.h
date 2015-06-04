#ifndef DIAMONDCARDPROFILEFORM_H
#define DIAMONDCARDPROFILEFORM_H
#include <QLabel>
#include <QLineEdit>
#include "user.h"
#include "ui_diamondcardprofileform.h"

class DiamondcardProfileForm : public QDialog, public Ui::DiamondcardProfileForm
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
	virtual void mouseReleaseEvent( QMouseEvent * e );
	virtual void processLeftMouseButtonRelease( QMouseEvent * e );

signals:
	void success();
	void newDiamondcardProfile(const QString&);

protected slots:
	virtual void languageChange();

private:
	t_user *user_config;
	bool destroy_user_config;
	QLineEdit *accountIdLineEdit;
	QLineEdit *pinCodeLineEdit;
	QLineEdit *nameLineEdit;
	QLabel *signUpTextLabel;

	void init();
	void destroy();

};


#endif
