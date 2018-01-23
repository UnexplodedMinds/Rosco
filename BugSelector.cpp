/*
Stratux AHRS Display
(c) 2018 Unexploded Minds
*/

#include "BugSelector.h"


extern bool g_bEmulated;


BugSelector::BugSelector( QWidget *pParent )
    : QDialog( pParent, Qt::Dialog | Qt::FramelessWindowHint )
{
    setupUi( this );
}

