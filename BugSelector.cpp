/*
Stratux AHRS Display
(c) 2018 Unexploded Minds

BugSelector is a simple dialog for allowing user to chose between a heading and a wind bug.
*/

#include <QKeyEvent>

#include "BugSelector.h"


extern bool g_bEmulated;


BugSelector::BugSelector( QWidget *pParent )
    : QDialog( pParent, Qt::Dialog | Qt::FramelessWindowHint )
{
    setupUi( this );
}


// Android back key accepts the dialog (B Key on emulator)
void BugSelector::keyReleaseEvent( QKeyEvent *pEvent )
{
    if( (pEvent->key() == Qt::Key_Back) || (pEvent->key() == Qt::Key_B) )
        done( -1 );
}

