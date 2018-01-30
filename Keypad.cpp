/*
Stratux AHRS Display
(c) 2018 Allen K. Lair, Unexploded Minds
*/

#include <QKeyEvent>

#include "Keypad.h"


extern bool g_bEmulated;


Keypad::Keypad( QWidget *pParent )
    : QDialog( pParent, Qt::Dialog | Qt::FramelessWindowHint )
{
    setupUi( this );
    setGeometry( (g_bEmulated ? 2000 : 0) + (pParent->width() / 2) - 400, (g_bEmulated ? 50 : 0) + (pParent->height() / 2) - 320, 800, 640 );

    QObjectList  kids = children();
    QObject     *pKid;
    QPushButton *pButton;
    QFont        big( "Roboto", 30, QFont::Bold );
    QFont        med( "Roboto", 20, QFont::Bold );

    foreach( pKid, kids )
    {
        pButton = qobject_cast<QPushButton *>( pKid );
        if( pButton != 0 )
        {
            if( pButton != m_pCancel )
                pButton->setFont( big );
            else
                pButton->setFont( med );
            connect( pButton, SIGNAL( clicked() ), this, SLOT( keypadClick() ) );
        }
    }
}


void Keypad::keypadClick()
{
    QObject *pObj = sender();

    if( pObj == 0 )
        return;

    QString qsObjName = pObj->objectName();

    if( qsObjName.isEmpty() )
        return;
    if( (qsObjName == "m_pCancel") || (qsObjName == "m_pSet") )
        return;

    QString qsValue( m_pValueLabel->text() );

    if( qsObjName == "m_p0" )
        qsValue.append( "0" );
    else if( qsObjName == "m_p1" )
        qsValue.append( "1" );
    else if( qsObjName == "m_p2" )
        qsValue.append( "2" );
    else if( qsObjName == "m_p3" )
        qsValue.append( "3" );
    else if( qsObjName == "m_p4" )
        qsValue.append( "4" );
    else if( qsObjName == "m_p5" )
        qsValue.append( "5" );
    else if( qsObjName == "m_p6" )
        qsValue.append( "6" );
    else if( qsObjName == "m_p7" )
        qsValue.append( "7" );
    else if( qsObjName == "m_p8" )
        qsValue.append( "8" );
    else if( qsObjName == "m_p9" )
        qsValue.append( "9" );
    else if( qsObjName == "m_pBack" )
    {
        if( !qsValue.isEmpty() )
            qsValue.chop( 1 );
    }

    m_pValueLabel->setText( qsValue );
}


int Keypad::value()
{
    return m_pValueLabel->text().toInt();
}


// Android back key accepts the dialog (B Key on emulator)
void Keypad::keyReleaseEvent( QKeyEvent *pEvent )
{
    if( (pEvent->key() == Qt::Key_Back) || (pEvent->key() == Qt::Key_B) )
    {
        pEvent->accept();
        reject();
    }
}

