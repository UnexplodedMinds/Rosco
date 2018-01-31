/*
Stratux AHRS Display
(c) 2018 Allen K. Lair, Unexploded Minds
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
    void timerEvent( QTimerEvent *pEvent );

private:
#if defined( Q_OS_ANDROID )
    void androidToggleScreenLock();
#endif

    StreamReader *m_pStratuxStream;
    bool          m_bStartup;
    QDateTime     m_lastStatusUpdate;

private slots:
    void appStateChanged( Qt::ApplicationState eState );
    void statusUpdate( bool bStratux, bool bAHRS, bool bGPS, bool bTraffic, bool bWeather );
    void menu();
    void weather();
};

#endif // __AHRSMAINWIN_H__
