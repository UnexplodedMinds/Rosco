/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include <QSettings>
#include <QKeyEvent>

#include "MenuDialog.h"


MenuDialog::MenuDialog( QWidget *pParent )
    : QDialog( pParent, Qt::Dialog | Qt::FramelessWindowHint )
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
    connect( m_pSettingsButton, SIGNAL( clicked() ), this, SLOT( settings() ) );
}


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


void MenuDialog::settings()
{
    // TODO: Various UI settings like font sizes will go here
}


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
        accept();
    }
}


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


void MenuDialog::updateStayOnButton()
{
    if( m_bKeepScreenOn )
        m_pStayOnButton->setStyleSheet( "QPushButton { background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 green ); }" );
    else
        m_pStayOnButton->setStyleSheet( "QPushButton { background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 CornflowerBlue ); }" );
}

