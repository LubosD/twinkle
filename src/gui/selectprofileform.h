#ifndef SELECTPROFILEFORM_H
#define SELECTPROFILEFORM_H
class t_phone;
extern t_phone *phone;

#include <list>
#include <string>
#include "phone.h"
#include <QDialog>
#include "ui_selectprofileform.h"

class SelectProfileForm : public QDialog, public Ui::SelectProfileForm
{
	Q_OBJECT

public:
	SelectProfileForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	~SelectProfileForm();

	std::list<std::string> selectedProfiles;

	virtual int execForm();
	static bool getUserProfiles( QStringList & profiles, QString & error );

public slots:
	virtual void showForm( Q3MainWindow * _mainWindow );
	virtual void runProfile();
	virtual void editProfile();
	virtual void newProfile();
	virtual void newProfile( bool exec_mode );
	virtual void newProfileCreated();
	virtual void deleteProfile();
	virtual void renameProfile();
	virtual void setAsDefault();
	virtual void wizardProfile();
	virtual void wizardProfile( bool exec_mode );
	virtual void diamondcardProfile();
	virtual void diamondcardProfile( bool exec_mode );
	virtual void sysSettings();
	virtual void fillProfileListView( const QStringList & profiles );
    virtual void toggleItem( QModelIndex item );

signals:
	void selection(const list<string> &);
	void profileRenamed();

protected slots:
	virtual void languageChange();

private:
	bool defaultSet;
	t_user *user_config;
	Q3MainWindow *mainWindow;

	void init();
	void destroy();

};


#endif
