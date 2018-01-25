/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __AHRSCANVAS_H__
#define __AHRSCANVAS_H__

#include <QWidget>
#include <QPixmap>
#include <QMap>

#include "StratuxStreams.h"
#include "Canvas.h"
#include "AppDefs.h"


class QDial;


class AHRSCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit AHRSCanvas( QWidget *parent = nullptr );
    ~AHRSCanvas();

    void trafficToggled( AHRS::TrafficDisp eDispType );

public slots:
    void init();
    void situation( StratuxSituation s );
    void traffic( int iICAO, StratuxTraffic t );

protected:
    void resizeEvent( QResizeEvent *pEvent );
    void paintEvent( QPaintEvent *pEvent );
    void mousePressEvent( QMouseEvent *pEvent );
    void timerEvent( QTimerEvent *pEvent );

private:
    void   buildRollIndicator();
    void   buildHeadingIndicator();
    void   buildAltTape();
    void   buildSpeedTape();
    void   buildVertSpeedTape();
    void   updateTraffic( QPainter *pAhrs, double dListPos );

    Canvas *m_pCanvas;

    bool                      m_bInitialized;
    StratuxSituation          m_situation;
    QPixmap                   m_planeIcon;
    QPixmap                   m_headIcon;
    QPixmap                   m_windIcon;
    int                       m_iHeadBugAngle;
    int                       m_iWindBugAngle;
    QPixmap                  *m_pRollIndicator;
    QPixmap                  *m_pHeadIndicator;
    QPixmap                  *m_pAltTape;
    QPixmap                  *m_pSpeedTape;
    QPixmap                  *m_pVertSpeedTape;
    QPixmap                   m_trafficAltKey;
    double                    m_dDPIMult;
    int                       m_iAltSpeedOffset;
    QMap<int, StratuxTraffic> m_trafficMap;
    AHRS::TrafficDisp         m_eTrafficDisp;
    bool                      m_bHideGPSLocation;
    bool                      m_iDispTimer;
    bool                      m_bUpdated;

signals:
    void simpleStatus( bool, bool, bool, bool ); // Stratux connected, Weather available, AHRS situation available, Traffic available, GPS position available
};

#endif // __AHRSCANVAS_H__