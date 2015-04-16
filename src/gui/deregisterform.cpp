#include "deregisterform.h"

/*
 *  Constructs a DeregisterForm which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
DeregisterForm::DeregisterForm(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
DeregisterForm::~DeregisterForm()
{
	// no need to delete child widgets, Qt does it all for us
}
