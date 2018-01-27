/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __AHRSMAINWIN_H__
#define __AHRSMAINWIN_H__

#include <QMainWindow>

#include "ui_AHRSMainWin.h"

#include "AppDefs.h"


class StreamReader;


class AHRSMainWin : public QMainWindow, public Ui::AHRSMainWin
{
    Q_OBJECT

public:
    explicit AHRSMainWin(QWidget *parent = 0);
    ~AHRSMainWin();

protected:
    void keyReleaseEvent( QKeyEvent *pEvent );

private:
    StreamReader *m_pStratuxStream;
    bool          m_bStartup;

private slots:
    void appStateChanged( Qt::ApplicationState eState );
    void statusUpdate( bool bStratux, bool bAHRS, bool bGPS, bool bTraffic, bool bWeather );
    void menu();
    void weather();
};

#endif // __AHRSMAINWIN_H__
