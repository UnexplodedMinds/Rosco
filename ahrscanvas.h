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


class QDial;


class AHRSCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit AHRSCanvas( QWidget *parent = nullptr );
    ~AHRSCanvas();

public slots:
    void init();
    void situation( StratuxSituation s );
    void traffic( int iICAO, StratuxTraffic t );

protected:
    void resizeEvent( QResizeEvent *pEvent );
    void paintEvent( QPaintEvent *pEvent );
    void mousePressEvent( QMouseEvent *pEvent );

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
    bool                      m_bTrafficOn;
    bool                      m_bHideGPSLocation;

private slots:
    void trafficToggled( bool bOn );
};

#endif // __AHRSCANVAS_H__
