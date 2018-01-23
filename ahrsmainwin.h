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

private:
    StreamReader     *m_pStratuxStream;
    AHRS::TrafficDisp m_eTrafficDisp;

private slots:
    void status( const QColor &statusColor, const QString &qsStatusText );
    void init();
    void stratuxConnected( bool bConnected );
    void trafficToggle();

signals:
    void trafficToggled( AHRS::TrafficDisp );
};

#endif // __AHRSMAINWIN_H__
