/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include <QTimer>

#include "AHRSMainWin.h"
#include "StreamReader.h"
#include "AppDefs.h"


AHRSMainWin::AHRSMainWin( QWidget *parent )
    : QMainWindow( parent ),
      m_pStratuxStream( new StreamReader( this ) ),
      m_eTrafficDisp( AHRS::AllTraffic )
{
    setupUi( this );

    connect( m_pConnectButton, SIGNAL( clicked() ), m_pStratuxStream, SLOT( connectStreams() ) );
    connect( m_pExitButton, SIGNAL( clicked() ), qApp, SLOT( closeAllWindows() ) );
    connect( m_pTrafficButton, SIGNAL( clicked()), this, SLOT( trafficToggle() ) );

    QTimer::singleShot( 100, this, SLOT( init() ) );
}


AHRSMainWin::~AHRSMainWin()
{
    delete m_pStratuxStream;
    m_pStratuxStream = 0;
}


void AHRSMainWin::status( const QColor &statusColor, const QString &qsStatusText )
{
    QPalette statusPal( m_pStatusLabel->palette() );

    statusPal.setColor( QPalette::WindowText, statusColor );
    m_pStatusLabel->setPalette( statusPal );
    m_pStatusLabel->setText( qsStatusText );
}


void AHRSMainWin::init()
{
    connect( m_pStratuxStream, SIGNAL( status( const QColor&, const QString& ) ), this, SLOT( status( const QColor&, const QString& ) ) );
    connect( m_pStratuxStream, SIGNAL( newSituation( StratuxSituation ) ), m_pAHRSDisp, SLOT( situation( StratuxSituation ) ) );
    connect( m_pStratuxStream, SIGNAL( newTraffic( int, StratuxTraffic ) ), m_pAHRSDisp, SLOT( traffic( int, StratuxTraffic ) ) );
    connect( m_pStratuxStream, SIGNAL( stratuxConnected( bool ) ), this, SLOT( stratuxConnected( bool ) ) );
    m_pStratuxStream->connectStreams();
}


void AHRSMainWin::stratuxConnected( bool bConnected )
{
    if( bConnected )
        m_pConnectButton->hide();
    else
        m_pConnectButton->show();
}


void AHRSMainWin::trafficToggle()
{
    int i = static_cast<int>( m_eTrafficDisp ) + 1;

    m_eTrafficDisp = static_cast<AHRS::TrafficDisp>( i );
    if( m_eTrafficDisp > AHRS::NoTraffic )
        m_eTrafficDisp = AHRS::AllTraffic;

    switch( m_eTrafficDisp )
    {
        case AHRS::AllTraffic:
            m_pTrafficButton->setText( "ALL" );
            break;
        case AHRS::ADSBOnlyTraffic:
            m_pTrafficButton->setText( "ADSB" );
            break;
        case AHRS::NoTraffic:
            m_pTrafficButton->setText( "OFF" );
            break;
    }

    emit trafficToggled( m_eTrafficDisp );
}
