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

    QWebSocket m_stratuxSituation;
    QWebSocket m_stratuxTraffic;
    bool       m_bConnected;
//  double     m_dHeadingKludge;
    bool       m_bHaveMyPos;
    double     m_dMyLat;
    double     m_dMyLong;

private slots:
    void situationConnected();
    void situationDisconnected();
    void situationUpdate( const QString &qsMessage );
    void trafficUpdate( const QString &qsMessage );

signals:
    void status( const QColor&, const QString& );
    void newSituation( StratuxSituation );
    void newTraffic( int, StratuxTraffic ); // ICAO, Rest of traffic struct
    void stratuxConnected( bool );
};

#endif // __STREAMREADER_H__
