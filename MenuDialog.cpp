/*
Stratux AHRS Display
(c) 2018 Allen K. Lair, Unexploded Minds
*/

#include <QSettings>
#include <QKeyEvent>
#include <QByteArray>
#include <QUrl>
#include <QNetworkRequest>

#if defined( Q_OS_ANDROID )
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

#include "MenuDialog.h"


MenuDialog::MenuDialog( QWidget *pParent )
    : QDialog( pParent, Qt::Dialog | Qt::FramelessWindowHint ),
      m_pNetMan( 0 )
{
    QSettings config;

    setupUi( this );

    config.beginGroup( "Global" );
    m_eTrafficDisp = static_cast<AHRS::TrafficDisp>( config.value( "TrafficDisp", static_cast<int>( AHRS::AllTraffic ) ).toInt() );
    updateTrafficButton();
    m_bKeepScreenOn = config.value( "KeepScreenOn", false ).toBool();
    updateStayOnButton();
    config.endGroup();

    connect( m_pExitButton, SIGNAL( clicked() ), this, SLOT( exit() ) );
    connect( m_pTrafficButton, SIGNAL( clicked() ), this, SLOT( traffic() ) );
    connect( m_pStayOnButton, SIGNAL( clicked() ), this, SLOT( stayOn() ) );
    connect( m_pResetLevelButton, SIGNAL( clicked() ), this, SLOT( resetLevel() ) );
    connect( m_pDoneButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
}


MenuDialog::~MenuDialog()
{
    if( m_pNetMan != 0 )
    {
        delete m_pNetMan;
        m_pNetMan = 0;
    }
}


// Change which traffic is displayed
void MenuDialog::traffic()
{
    QSettings config;
    int       i = static_cast<int>( m_eTrafficDisp ) + 1;

    m_eTrafficDisp = static_cast<AHRS::TrafficDisp>( i );
    if( m_eTrafficDisp > AHRS::NoTraffic )
        m_eTrafficDisp = AHRS::AllTraffic;

    updateTrafficButton();

    config.beginGroup( "Global" );
    config.setValue( "TrafficDisp", static_cast<int>( m_eTrafficDisp ) );
    config.endGroup();
    config.sync();
}


// Set screen to stay on or not
void MenuDialog::stayOn()
{
    QSettings config;

    m_bKeepScreenOn = (!m_bKeepScreenOn);
    updateStayOnButton();
    config.beginGroup( "Global" );
    config.setValue( "KeepScreenOn", m_bKeepScreenOn );
    config.endGroup();
    config.sync();
}


// Bring up the settings dialog that just has an embedded QtWebEngineView
void MenuDialog::resetLevel()
{
    QUrl            url( "http://192.168.10.1/cageAHRS" );
    QNetworkRequest req( url );
    QByteArray      empty;

    if( m_pNetMan == 0 )
        m_pNetMan = new QNetworkAccessManager( this );
    m_pNetMan->post( req, empty );
}


// Sync the config and close the dialog
void MenuDialog::exit()
{
    QSettings config;

    config.sync();
    reject();
}


// Android back key accepts the dialog (B Key on emulator)
void MenuDialog::keyReleaseEvent( QKeyEvent *pEvent )
{
    if( (pEvent->key() == Qt::Key_Back) || (pEvent->key() == Qt::Key_B) )
    {
        QSettings config;

        config.sync();
        pEvent->accept();
        accept();
    }
}


// Apply the stylesheet to the traffic display type button depending on traffic display mode
void MenuDialog::updateTrafficButton()
{
    switch( m_eTrafficDisp )
    {
        case AHRS::AllTraffic:
            m_pTrafficButton->setStyleSheet( "QPushButton { background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 green ); }" );
            m_pTrafficButton->setText( " ALL TRAFFIC" );
            break;
        case AHRS::ADSBOnlyTraffic:
            m_pTrafficButton->setStyleSheet( "QPushButton { background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 #ffa500 ); }" );
            m_pTrafficButton->setText( " ADSB ONLY  " );
            break;
        case AHRS::NoTraffic:
            m_pTrafficButton->setStyleSheet( "QPushButton { background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 CornflowerBlue ); }" );
            m_pTrafficButton->setText( " NO TRAFFIC " );
            break;
    }
}


// Apply stylesheet to the stay on button depending on whether it's on or off
void MenuDialog::updateStayOnButton()
{
    if( m_bKeepScreenOn )
        m_pStayOnButton->setStyleSheet( "QPushButton { background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 green ); }" );
    else
        m_pStayOnButton->setStyleSheet( "QPushButton { background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 CornflowerBlue ); }" );
}

