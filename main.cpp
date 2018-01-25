/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include <QApplication>
#include <QFontDatabase>
#include <QDesktopWidget>

#include "AHRSMainWin.h"


bool g_bEmulated = false;


int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );
    QStringList  args( a.arguments() );
    AHRSMainWin w;

    QCoreApplication::setOrganizationName( "Unexploded Minds" );
    QCoreApplication::setOrganizationDomain( "unexplodedminds.com" );
    QCoreApplication::setApplicationName( "Rosco" );

    a.setAttribute( Qt::AA_EnableHighDpiScaling );

    // Passed if running as an emulated display
    if( args.count() > 1 )
    {
        if( args.at( 1 ) == "emulated" )
        {
            w.setGeometry( 2000, 50, 733, 1100 );
            g_bEmulated = true;
        }
    }

    w.show();

    return a.exec();
}
