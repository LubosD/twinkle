#ifndef NUMBERCONVERSIONFORM_H
#define NUMBERCONVERSIONFORM_H

#include <QDialog>
#include "ui_numberconversionform.h"

class NumberConversionForm : public QDialog, private Ui::NumberConversionForm
{
    Q_OBJECT

public:
    NumberConversionForm(QWidget* parent = 0);
    ~NumberConversionForm();

    int exec(QString& expr, QString& replace);

public slots:
    void validate();

private:
    void init();
};

#endif // NUMBERCONVERSIONFORM_H
