/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <QDialog>

#include "ui_Keypad.h"


class Keypad : public QDialog, public Ui::KeypadBase
{
    Q_OBJECT

public:
    explicit Keypad( QWidget *pParent );

    int value();

protected:
    void keyReleaseEvent( QKeyEvent *pEvent );

private slots:
    void keypadClick();
};

#endif // __KEYPAD_H__
