/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#ifndef __STREAMREADER_H__
#define __STREAMREADER_H__

#include <QObject>
#include <QWebSocket>

#include "StratuxStreams.h"


class QCoreApplication;


class StreamReader : public QObject
{
    Q_OBJECT

public:
    explicit StreamReader( QObject *parent );
    ~StreamReader();

public slots:
    void connectStreams();

private:
    void initTraffic( StratuxTraffic &traffic );
    void initSituation( StratuxSituation &situation );
    void initStatus( StratuxStatus &status );

    bool          m_bHaveMyPos;
    bool          m_bAHRSStatus;
    bool          m_bStratuxStatus;
    bool          m_bGPSStatus;
    bool          m_bWeatherStatus;
    bool          m_bTrafficStatus;
    QWebSocket    m_stratuxSituation;
    QWebSocket    m_stratuxTraffic;
    QWebSocket    m_stratuxStatus;
    double        m_dMyLat;
    double        m_dMyLong;

private slots:
    void situationUpdate( const QString &qsMessage );
    void trafficUpdate( const QString &qsMessage );
    void statusUpdate( const QString &qsMessage );

signals:
    void newSituation( StratuxSituation );
    void newTraffic( int, StratuxTraffic );     // ICAO, Rest of traffic struct
    void newStatus( bool, bool, bool, bool, bool );   // Stratux available, AHRS available, GPS available, Traffic available, Weather available
};

#endif // __STREAMREADER_H__
