/*
Stratux AHRS Display
(c) 2018 Allen K. Lair, Unexploded Minds
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

    void connectStreams();
    void disconnectStreams();

    static void initTraffic( StratuxTraffic &traffic );
    static void initSituation( StratuxSituation &situation );
    static void initStatus( StratuxStatus &status );
    static void initWeather( StratuxWeather &weather );

private:
    bool          m_bHaveMyPos;
    bool          m_bAHRSStatus;
    bool          m_bStratuxStatus;
    bool          m_bGPSStatus;
    bool          m_bWeatherStatus;
    bool          m_bTrafficStatus;
    QWebSocket    m_stratuxSituation;
    QWebSocket    m_stratuxTraffic;
    QWebSocket    m_stratuxStatus;
    QWebSocket    m_stratuxWeather;
    double        m_dMyLat;
    double        m_dMyLong;

private slots:
    void situationUpdate( const QString &qsMessage );
    void trafficUpdate( const QString &qsMessage );
    void statusUpdate( const QString &qsMessage );
    void weatherUpdate( const QString &qsMessage );

signals:
    void newSituation( StratuxSituation );
    void newTraffic( int, StratuxTraffic );             // ICAO, Rest of traffic struct
    void newStatus( bool, bool, bool, bool, bool );     // Stratux available, AHRS available, GPS available, Traffic available, Weather available
    void newWeather( StratuxWeather );
};

#endif // __STREAMREADER_H__
