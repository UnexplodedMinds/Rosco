/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __MENUDIALOG_H__
#define __MENUDIALOG_H__

#include <QDialog>

#include "ui_MenuDialog.h"
#include "AppDefs.h"


class MenuDialog : public QDialog, public Ui::MenuDialogBase
{
    Q_OBJECT

public:
    explicit MenuDialog( QWidget *pParent );

protected:
    void keyReleaseEvent( QKeyEvent *pEvent );

private:
    void updateTrafficButton();
    void updateStayOnButton();

    AHRS::TrafficDisp m_eTrafficDisp;
    bool              m_bKeepScreenOn;

private slots:
    void traffic();
    void stayOn();
    void settings();
    void exit();
};

#endif // __MENUDIALOG_H__
