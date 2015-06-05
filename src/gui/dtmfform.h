#ifndef DTMFFORM_H
#define DTMFFORM_H

#include <QDialog>
#include <QKeyEvent>
#include <QTimer>
#include "ui_dtmfform.h"

class DtmfForm : public QDialog, private Ui::DtmfForm
{
    Q_OBJECT

public:
    explicit DtmfForm(QWidget *parent = 0);
    ~DtmfForm();

public slots:
    void dtmf1();
    void dtmf2();
    void dtmf3();
    void dtmf4();
    void dtmf5();
    void dtmf6();
    void dtmf7();
    void dtmf8();
    void dtmf9();
    void dtmf0();
    void dtmfStar();
    void dtmfPound();
    void dtmfA();
    void dtmfB();
    void dtmfC();
    void dtmfD();

	void insertNextKey();

protected:
    void keyPressEvent(QKeyEvent* e);
signals:
    void digits(const QString&);
private:
	QTimer m_insertTimer;
	QString m_remainingKeys;
};

#endif // DTMFFORM_H
