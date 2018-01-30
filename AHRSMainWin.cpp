/*
Stratux AHRS Display
(c) 2018 Allen K. Lair, Unexploded Minds
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


// Setup minimal UI elements and make the connections
AHRSMainWin::AHRSMainWin( QWidget *parent )
    : QMainWindow( parent ),
      m_pStratuxStream( new StreamReader( this ) ),
      m_bStartup( true )
{
    setupUi( this );

    connect( m_pMenuButton, SIGNAL( clicked() ), this, SLOT( menu() ) );
    connect( m_pWeatherButton, SIGNAL( clicked() ), this, SLOT( weather() ) );
    connect( qApp, SIGNAL( applicationStateChanged( Qt::ApplicationState ) ), this, SLOT( appStateChanged( Qt::ApplicationState ) ) );
}


// Delete the stream reader
AHRSMainWin::~AHRSMainWin()
{
    delete m_pStratuxStream;
    m_pStratuxStream = 0;
}


// Android only - handle android application state changes
void AHRSMainWin::appStateChanged( Qt::ApplicationState eState )
{
    switch( eState )
    {
        case Qt::ApplicationSuspended:
        case Qt::ApplicationHidden:
        case Qt::ApplicationInactive:
        {
#if defined( Q_OS_ANDROID )
            m_pStratuxStream->disconnectStreams();
            m_pAHRSDisp->suspend( true );
#endif
            break;
        }
        default:
        {
            if( m_bStartup )
            {
                connect( m_pStratuxStream, SIGNAL( newSituation( StratuxSituation ) ), m_pAHRSDisp, SLOT( situation( StratuxSituation ) ) );
                connect( m_pStratuxStream, SIGNAL( newTraffic( int, StratuxTraffic ) ), m_pAHRSDisp, SLOT( traffic( int, StratuxTraffic ) ) );
                connect( m_pStratuxStream, SIGNAL( newWeather( StratuxWeather ) ), m_pAHRSDisp, SLOT( weather( StratuxWeather ) ) );
                connect( m_pStratuxStream, SIGNAL( newStatus( bool, bool, bool, bool, bool ) ), this, SLOT( statusUpdate( bool, bool, bool, bool, bool ) ) );
            }
            m_pStratuxStream->connectStreams();
            m_pAHRSDisp->suspend( false );
            m_bStartup = false;
            break;
        }
    }
}


// Status stream is received here instead of the canvas since here is where the indicators are
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


// Display the menu dialog and handle specific returns
void::AHRSMainWin::menu()
{
    MenuDialog dlg( this );
    QSettings  config;
    int        iW = width();
    int        iH = height();
    int        iRet = 0;

    dlg.setGeometry( (iW / 2) - 250 + (g_bEmulated ? 2000 : 0), (iH / 2) - 400, 500, 800 );
    iRet = dlg.exec();
    if( iRet == QDialog::Rejected )
        qApp->closeAllWindows();
    else if( iRet == QDialog::Accepted )
    {
        config.beginGroup( "Global" );
        m_pAHRSDisp->trafficToggled( static_cast<AHRS::TrafficDisp>( config.value( "TrafficDisp", static_cast<int>( AHRS::AllTraffic ) ).toInt() ) );
        config.endGroup();
        // Call the Android function for locking the screen through JNI if so configured
#if defined( Q_OS_ANDROID )
        androidToggleScreenLock();
#endif
    }
}


#if defined( Q_OS_ANDROID )
void AHRSMainWin::androidToggleScreenLock()
{
/* This is currently causing application crashes when the method is called
    QSettings         config;
    QAndroidJniObject androidAct = QtAndroid::androidActivity();

    if( androidAct.isValid() )
    {
        QAndroidJniObject androidWin = androidAct.callObjectMethod( "getWindow", "()Landroid/view/Window;" );

        if( androidWin.isValid() )
        {
            config.beginGroup( "Global" );
            bool bKeepScreenOn = config.value( "KeepScreenOn", false ).toBool();

            if( bKeepScreenOn )
                androidWin.callMethod<void>( "addFlags", "(I)V", ANDROID_KEEP_SCREEN_ON );
            else
                androidWin.callMethod<void>( "clearFlags", "(I)V", ANDROID_KEEP_SCREEN_ON );
            config.endGroup();
        }
    }
*/
}
#endif


// Toggle the weather display on/off
void AHRSMainWin::weather()
{
    m_pAHRSDisp->weatherToggled();
}


// Android back key accepts the dialog (B Key on emulator)
void AHRSMainWin::keyReleaseEvent( QKeyEvent *pEvent )
{
    if( (pEvent->key() == Qt::Key_Back) || (pEvent->key() == Qt::Key_B) )
        qApp->closeAllWindows();
    pEvent->accept();
}

