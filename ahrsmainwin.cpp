/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include <QTimer>
#include <QSettings>
#include <QKeyEvent>

#if defined( Q_OS_ANDROID )
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

#include "AHRSMainWin.h"
#include "StreamReader.h"
#include "AppDefs.h"
#include "MenuDialog.h"
#include "Canvas.h"


#define ANDROID_KEEP_SCREEN_ON 128


extern bool g_bEmulated;


AHRSMainWin::AHRSMainWin( QWidget *parent )
    : QMainWindow( parent ),
      m_pStratuxStream( new StreamReader( this ) )
{
    setupUi( this );

    connect( m_pMenuButton, SIGNAL( clicked() ), this, SLOT( menu() ) );

    QTimer::singleShot( 100, this, SLOT( init() ) );
}


AHRSMainWin::~AHRSMainWin()
{
    delete m_pStratuxStream;
    m_pStratuxStream = 0;
}


void AHRSMainWin::init()
{
    connect( m_pStratuxStream, SIGNAL( newSituation( StratuxSituation ) ), m_pAHRSDisp, SLOT( situation( StratuxSituation ) ) );
    connect( m_pStratuxStream, SIGNAL( newTraffic( int, StratuxTraffic ) ), m_pAHRSDisp, SLOT( traffic( int, StratuxTraffic ) ) );
    connect( m_pStratuxStream, SIGNAL( newStatus( bool, bool, bool, bool, bool ) ), this, SLOT( statusUpdate( bool, bool, bool, bool, bool ) ) );
    m_pStratuxStream->connectStreams();
}


void AHRSMainWin::statusUpdate( bool bStratux, bool bAHRS, bool bGPS, bool bTraffic, bool bWeather )
{
    QString qsOn( "QLabel { border: 5px solid black; background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 green ); }" );
    QString qsOff( "QLabel { border: 5px solid black; background-color: qlineargradient( x1:0, y1:0, x2:0, y2:1, stop: 0 white, stop:1 red ); }" );

    m_pStatusIndicator->setStyleSheet( bStratux ? qsOn : qsOff );
    m_pWeatherIndicator->setStyleSheet( bWeather ? qsOn : qsOff );
    m_pAHRSIndicator->setStyleSheet( bAHRS ? qsOn : qsOff );
    m_pTrafficIndicator->setStyleSheet( bTraffic ? qsOn : qsOff );
    m_pGPSIndicator->setStyleSheet( bGPS ? qsOn : qsOff );
}


void::AHRSMainWin::menu()
{
    MenuDialog dlg( this );
    QSettings  config;
    int        iW = width();
    int        iH = height();

    dlg.setGeometry( (iW / 2) - 250 + (g_bEmulated ? 2000 : 0), (iH / 2) - 300, 500, 600 );
    if( dlg.exec() == QDialog::Rejected )
        qApp->closeAllWindows();
    else
    {
        config.beginGroup( "Global" );
        m_pAHRSDisp->trafficToggled( static_cast<AHRS::TrafficDisp>( config.value( "TrafficDisp", static_cast<int>( AHRS::AllTraffic ) ).toInt() ) );
#if defined( Q_OS_ANDROID )
        if( config.value( "KeepScreenOn", false ).toBool() )
        {
            QAndroidJniObject androidAct = QtAndroid::androidActivity();

            if( androidAct.isValid() )
            {
                QAndroidJniObject androidWin = androidAct.callObjectMethod( "getWindow", "()Landroid/view/Window;" );

                if( androidWin.isValid() )
                    androidWin.callMethod<void>( "addFlags", "(I)V", ANDROID_KEEP_SCREEN_ON );
            }
        }
#endif
        config.endGroup();
    }
}


// Android back key accepts the dialog (B Key on emulator)
void AHRSMainWin::keyReleaseEvent( QKeyEvent *pEvent )
{
    if( (pEvent->key() == Qt::Key_Back) || (pEvent->key() == Qt::Key_B) )
        qApp->closeAllWindows();
}

