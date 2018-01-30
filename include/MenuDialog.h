/*
Stratux AHRS Display
(c) 2018 Allen K. Lair, Unexploded Minds
*/

#ifndef __MENUDIALOG_H__
#define __MENUDIALOG_H__

#include <QDialog>
#include <QNetworkAccessManager>

#include "ui_MenuDialog.h"
#include "AppDefs.h"


class MenuDialog : public QDialog, public Ui::MenuDialogBase
{
    Q_OBJECT

public:
    explicit MenuDialog( QWidget *pParent );
    ~MenuDialog();

protected:
    void keyReleaseEvent( QKeyEvent *pEvent );

private:
    void updateTrafficButton();
    void updateStayOnButton();

    AHRS::TrafficDisp      m_eTrafficDisp;
    bool                   m_bKeepScreenOn;
    QNetworkAccessManager *m_pNetMan;

private slots:
    void traffic();
    void stayOn();
    void resetLevel();
    void exit();
};

#endif // __MENUDIALOG_H__
