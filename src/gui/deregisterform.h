#ifndef DEREGISTERFORM_H
#define DEREGISTERFORM_H

#include <QDialog>

#include "ui_deregisterform.h"

class DeregisterForm : public QDialog, private Ui::DeregisterForm
{
    Q_OBJECT

public:
    DeregisterForm(QWidget* parent = 0);
    ~DeregisterForm();
};

#endif // DEREGISTERFORM_H
