#include "getprofilenameform.h"
#include <QDir>
#include <QMessageBox>
#include <QRegExpValidator>
#include "user.h"
#include "protocol.h"

/*
 *  Constructs a GetProfileNameForm which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
GetProfileNameForm::GetProfileNameForm(QWidget* parent)
    : QDialog(parent)
{
	setupUi(this);
	init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
GetProfileNameForm::~GetProfileNameForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void GetProfileNameForm::init()
{
	// Letters, digits, underscore, minus, "@" and period
	QRegExp rxFilenameChars("[\\w\\-@\\.]*");

	// Set validators
	// USER
	profileLineEdit->setValidator(new QRegExpValidator(rxFilenameChars, this));
}

void GetProfileNameForm::validate()
{
	QString profileName = profileLineEdit->text();

	if (profileName.isEmpty()) return;

	// Check for anything that was let through by the validator
	QRegExp rxFilename("^[\\w\\-][\\w\\-@\\.]*$");
	if (!rxFilename.exactMatch(profileName)) {
		QMessageBox::critical(this, PRODUCT_NAME,
			tr("Invalid profile name: %1").arg(profileName)
			// While this would be better suited as informativeText,
			// Qt wouldn't take it into account when sizing the box
			// (see http://apocalyptech.com/linux/qt/qmessagebox/)
			+ QString("\n\n") + tr(
				"Allowed characters are: letters, digits, '-', '_', '.' and '@'.\n"
				"Furthermore, the initial character cannot be '.' or '@'."
			));

		return;
	}

	// Find the .twinkle directory in HOME
	QDir d = QDir::home();
	if (!d.cd(USER_DIR)) {
		QMessageBox::critical(this, PRODUCT_NAME,
			tr("Cannot find .twinkle directory in your home directory."));
		reject();
	}

	QString filename = profileName;
	filename.append(USER_FILE_EXT);
	QString fullname = d.filePath(filename);
	if (QFile::exists(fullname)) {
		QMessageBox::warning(this, PRODUCT_NAME,
			tr("Profile already exists."));
		return;
	}

	accept();
}

QString GetProfileNameForm:: getProfileName()
{
	return profileLineEdit->text();
}

// Execute a dialog to get a name for a new profile
int GetProfileNameForm::execNewName()
{
	profileTextLabel->setText(tr("Enter a name for your profile:"));
	return exec();
}

// Execute this dialog to get a new name for an existing profile
int GetProfileNameForm::execRename(const QString &oldName)
{
	QString s = tr("Rename profile '%1' to:").arg(oldName);
	profileTextLabel->setText(s);
	return exec();
}
