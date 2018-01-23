/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __AHRSMAINWIN_H__
#define __AHRSMAINWIN_H__

#include <QMainWindow>

#include "ui_AHRSMainWin.h"


class StreamReader;


class AHRSMainWin : public QMainWindow, public Ui::AHRSMainWin
{
    Q_OBJECT

public:
    explicit AHRSMainWin(QWidget *parent = 0);
    ~AHRSMainWin();

private:
    StreamReader *m_pStratuxStream;
    bool          m_bTrafficOn;

private slots:
    void status( const QColor &statusColor, const QString &qsStatusText );
    void init();
    void stratuxConnected( bool bConnected );
    void trafficToggle();

signals:
    void trafficToggled( bool );
};

#endif // __AHRSMAINWIN_H__
