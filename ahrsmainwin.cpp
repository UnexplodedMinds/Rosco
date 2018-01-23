/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include <QTimer>

#include "AHRSMainWin.h"
#include "StreamReader.h"


AHRSMainWin::AHRSMainWin( QWidget *parent )
    : QMainWindow( parent ),
      m_pStratuxStream( new StreamReader( this ) ),
      m_bTrafficOn( true )
{
    setupUi( this );

    connect( m_pConnectButton, SIGNAL( clicked() ), m_pStratuxStream, SLOT( connectStreams() ) );
    connect( m_pExitButton, SIGNAL( clicked() ), qApp, SLOT( closeAllWindows() ) );
    connect( m_pTrafficOnOffButton, SIGNAL( clicked()), this, SLOT( trafficToggle() ) );

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
    QPalette pal( m_pTrafficOnOffButton->palette() );

    m_bTrafficOn = (!m_bTrafficOn);

    if( m_bTrafficOn )
        pal.setColor( QPalette::Window, Qt::green );
    else
        pal.setColor( QPalette::Window, Qt::white );

    m_pTrafficOnOffButton->setPalette( pal );
    emit trafficToggled( m_bTrafficOn );
}
