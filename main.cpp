/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include <QGuiApplication>
#include <QFontDatabase>
#include <QDesktopWidget>

#include "AHRSMainWin.h"

bool g_bEmulated = false;


int main( int argc, char *argv[] )
{
    QApplication guiApp( argc, argv );
    QStringList  args( guiApp.arguments() );
    AHRSMainWin  mainWin;

    QCoreApplication::setOrganizationName( "Unexploded Minds" );
    QCoreApplication::setOrganizationDomain( "unexplodedminds.com" );
    QCoreApplication::setApplicationName( "Rosco" );
    QGuiApplication::setApplicationDisplayName( "Rosco" );

//  guiApp.setAttribute( Qt::AA_EnableHighDpiScaling );

    // Passed if running as an "emulated" display useful for very fast compile and run
    // but not always accurate for display on android.
    if( args.count() > 1 )
    {
        if( args.at( 1 ) == "emulated" )
        {
            mainWin.setGeometry( 2000, 50, 733, 1100 );
            g_bEmulated = true;
        }
    }

    mainWin.show();

    return guiApp.exec();
}
