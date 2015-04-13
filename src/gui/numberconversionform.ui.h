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

void NumberConversionForm::init()
{
	QRegExp rxNoAtSign("[^@]*");
	
	exprLineEdit->setValidator(new QRegExpValidator(rxNoAtSign, this));
	replaceLineEdit->setValidator(new QRegExpValidator(rxNoAtSign, this));
}

int NumberConversionForm::exec(QString &expr, QString &replace)
{
	exprLineEdit->setText(expr);
	replaceLineEdit->setText(replace);
	int retval = QDialog::exec();
	
	if (retval == QDialog::Accepted) {
		expr = exprLineEdit->text();
		replace = replaceLineEdit->text();
	}
	
	return retval;
}

void NumberConversionForm::validate()
{
	QString expr = exprLineEdit->text();
	QString replace = replaceLineEdit->text();
	
	if (expr.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,  
			tr("Match expression may not be empty.").ascii(), MSG_CRITICAL);
		exprLineEdit->setFocus();
		exprLineEdit->selectAll();
		return;
	}
	
	if (replace.isEmpty()) {
		((t_gui *)ui)->cb_show_msg(this,  
			tr("Replace value may not be empty.").ascii(), MSG_CRITICAL);
		replaceLineEdit->setFocus();
		replaceLineEdit->selectAll();
		return;
	}
	
	try {
		boost::regex re(expr.ascii());
	} catch (boost::bad_expression) {
		((t_gui *)ui)->cb_show_msg(this,  
			tr("Invalid regular expression.").ascii(), MSG_CRITICAL);
		exprLineEdit->setFocus();
		return;
	}
	
	accept();
}
