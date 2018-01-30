/*
Stratux AHRS Display
(c) 2018 Allen K. Lair, Unexploded Minds
*/

#include <QPixmap>
#include <QPainter>

#include "Builder.h"
#include "Canvas.h"


extern bool g_bEmulated;


Builder::Builder()
{
}


// Build the altitude tape pixmap
// Note that there are pixmap size limits on Android that appear to be smaller than Windows and X11
// so there are font size limitations in order not to exceed them.
void Builder::buildAltTape( QPixmap *pAltTape, Canvas *pCanvas )
{
    QPainter        ahrs( pAltTape );
    QFont           altFont( "Roboto", 12 );
    int             iAlt, iV = 1, iY;
    CanvasConstants c = pCanvas->contants();

    ahrs.setFont( altFont );
    for( iAlt = 20000; iAlt >= 0; iAlt -= 100 )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * c.iTinyFontHeight * 2;
        ahrs.scale( (c.dW <= 1200) ? 1.5 : 3.0, 1.0 );
        ahrs.drawText( 0, iY, QString::number( iAlt ) );
        ahrs.resetTransform();
        iY = iY - (c.iTinyFontHeight / 2) + c.iAltSpeedOffset - 2;
        ahrs.drawLine( pAltTape->width() - 24, iY, pAltTape->width() - 4, iY );
        ahrs.setPen( QPen( Qt::blue, 2 ) );
        for( int i = 0; i < 3; i++ )
        {
            iY += (c.iTinyFontHeight / 2 );
            ahrs.drawLine( pAltTape->width() - 19, iY, pAltTape->width() - 4, iY );
        }
        iV++;
    }
}


// Build the vertical speed tape pixmap
void Builder::buildVertSpeedTape( QPixmap *pVertTape, Canvas *pCanvas )
{
    QPainter        ahrs( pVertTape );
    QFont           vertFont( "Roboto", 10 );
    int             iVert, iV = 1, iY;
    CanvasConstants c = pCanvas->contants();
    int             iLineHeight = c.dH / 90;

    ahrs.setFont( vertFont );
    for( iVert = 10; iVert >= -10; iVert-- )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * iLineHeight * 2;
        if( (iVert % 2) == 0 )
            ahrs.drawText( 5, iY, QString::number( abs( iVert ) ) );
        iY -= 3;
        ahrs.drawLine( pVertTape->width() - 14, iY - (g_bEmulated ? 2 : 7), pVertTape->width() - 4, iY - (g_bEmulated ? 2 : 7) );
        ahrs.setPen( QPen( Qt::blue, 2 ) );
        if( iVert != -10 )
        {
            iY += iLineHeight;
            ahrs.drawLine( pVertTape->width() - 9, iY - (g_bEmulated ? 2 : 7), pVertTape->width() - 4, iY - (g_bEmulated ? 2 : 7) );
        }
        iV++;
    }
}


// Build the air speed (ground speed actually since this is GPS based without any pitot-static system)
void Builder::buildSpeedTape( QPixmap *pSpeedTape, Canvas *pCanvas )
{
    QPainter        ahrs( pSpeedTape );
    QFont           speedFont( "Roboto", 24, QFont::Bold );
    int             iSpeed, iV = 1, iY;
    CanvasConstants c = pCanvas->contants();

    ahrs.setFont( speedFont );
    for( iSpeed = 300; iSpeed >= 0; iSpeed -= 10 )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * c.iTinyFontHeight * 2;
        ahrs.scale( (c.dW <= 1200) ? 1.0 : 2.0, 1.0 );
        ahrs.drawText( 30, iY + (c.iLargeFontHeight / 6), QString::number( iSpeed ) );
        ahrs.resetTransform();
        iY = iY - (c.iTinyFontHeight / 2) + c.iAltSpeedOffset - 2;
        ahrs.drawLine( 0, iY, 25, iY );
        ahrs.setPen( QPen( Qt::blue, 2 ) );
        for( int i = 0; i < 3; i++ )
        {
            iY += (c.iTinyFontHeight / 2 );
            ahrs.drawLine( 0, iY, 15, iY );
        }
        iV++;
    }
}


// Build the roll indicator (the arc scale at the top)
void Builder::buildRollIndicator( QPixmap *pRollInd, Canvas *pCanvas )
{
    Q_UNUSED( pCanvas )

    QPainter     ahrs( pRollInd );
    double       dW = pRollInd->width();
    double       dH = pRollInd->height();
    double       dW2 = dW / 2.0;
    double       dH2 = dH / 2.0;
    QPen         linePen( Qt::white, 3 );
    QFont        rollFont( "Roboto Thin", 12 );
    QFontMetrics rollMetrics( rollFont );
    QString      qsRoll;
    QRect        rollRect;

    ahrs.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing, true );

    // -30 to 30 with black lines on -30, 0 and 30
    for( int i = 0; i < 7; i++ )
    {
        ahrs.translate( dW2, dH2 );
        ahrs.rotate( -30 + (i * 10) );
        ahrs.translate( -dW2, -dH2 );
        if( (i == 0) || (i == 3) || (i == 6) )
            linePen.setColor( QColor( 0xFF, 0xA5, 0x00, 0xFF ) );   // Orange
        else
            linePen.setColor( Qt::white );
        ahrs.setPen( linePen );
        ahrs.drawLine( dW2, 0, dW2, 20 );
        qsRoll = QString::number( abs( -30 + (i * 10) ) );
        qsRoll.chop( 1 );
        rollRect = rollMetrics.boundingRect( qsRoll );
        ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
        ahrs.resetTransform();
    }
    // Red lines on -60, -45, 45 and 60
    linePen.setColor( Qt::red );
    ahrs.setPen( linePen );
    // -45
    qsRoll = "45";
    ahrs.translate( dW2, dH2 );
    ahrs.rotate( -45 );
    ahrs.translate( -dW2, -dH2 );
    ahrs.drawLine( dW2, 0, dW2, 20 );
    rollRect = rollMetrics.boundingRect( qsRoll );
    ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
    ahrs.resetTransform();
    // 45
    ahrs.translate( dW2, dH2 );
    ahrs.rotate( 45 );
    ahrs.translate( -dW2, -dH2 );
    ahrs.drawLine( dW2, 0, dW2, 20 );
    ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
    ahrs.resetTransform();
    // -60
    ahrs.translate( dW2, dH2 );
    ahrs.rotate( -60 );
    ahrs.translate( -dW2, -dH2 );
    ahrs.drawLine( dW2, 0, dW2, 20 );
    qsRoll = "6";
    rollRect = rollMetrics.boundingRect( qsRoll );
    ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
    ahrs.resetTransform();
    // 60
    ahrs.translate( dW2, dH2 );
    ahrs.rotate( 60 );
    ahrs.translate( -dW2, -dH2 );
    ahrs.drawLine( dW2, 0, dW2, 20 );
    ahrs.drawText( dW2 - (rollRect.width() / 2.0), 22.0 + rollRect.height(), qsRoll );
    ahrs.resetTransform();

    linePen.setColor( Qt::white );
    ahrs.setPen( linePen );
    for( double d = 0; d <= 120.0; d += 0.1 )
    {
        ahrs.translate( dW2, dH2 );
        ahrs.rotate( -60.0 + d );
        ahrs.translate( -dW2, -dH2 );
        ahrs.drawLine( dW2, 20.0, dW2, 21.0 );
        ahrs.resetTransform();
    }
}


// Build the round heading indicator
void Builder::buildHeadingIndicator( QPixmap *pHeadInd, Canvas *pCanvas )
{
    QPainter        ahrs( pHeadInd );
    double          dW = pHeadInd->width();
    double          dH = pHeadInd->height();
    double          dW2 = dW / 2.0;
    double          dH2 = dH / 2.0;
    QPen            linePen( Qt::white, 3 );
    bool            bThis = true;
    QColor          orange( 255, 165, 0 );
    int             iFontSize = 20;
    CanvasConstants c = pCanvas->contants();

    if( c.dW > 1080 )
        iFontSize = 30;

    QFont        headFont( "Roboto", iFontSize );
    QFontMetrics headMetrics( headFont );
    QString      qsHead;
    QRect        headRect;

    ahrs.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing, true );
    // Heading background
    ahrs.setPen( Qt::NoPen );
    ahrs.setBrush( Qt::black );
    ahrs.drawEllipse( 0.0, 0.0, dW, dH );
    ahrs.setPen( linePen );
    // Tens of degrees
    for( int i = 0; i < 360; i += 10 )
    {
        ahrs.translate( dW2, dH2 );
        ahrs.rotate( i );
        ahrs.translate( -dW2, -dH2 );
        ahrs.drawLine( dW2, 0, dW2, 20 );
        ahrs.resetTransform();
    }
    // Every five degrees
    linePen.setWidth( 2 );
    linePen.setColor( Qt::gray );
    ahrs.setPen( linePen );
    for( int i = 5; i < 360; i += 5 )
    {
        if( bThis )
        {
            ahrs.translate( dW2, dH2 );
            ahrs.rotate( i );
            ahrs.translate( -dW2, -dH2 );
            ahrs.drawLine( dW2, 0, dW2, 10 );
            ahrs.resetTransform();
            bThis = false;
        }
        else
            bThis = true;
    }
    ahrs.resetTransform();
    // Number labels
    ahrs.setFont( headFont );
    for( int i = 0; i < 36; i += 3 )
    {
        ahrs.translate( dW2, dH2 );
        ahrs.rotate( i * 10 );
        ahrs.translate( -dW2, -dH2 );
        if( i == 0 )
        {
            ahrs.setPen( orange );
            qsHead = "N";
        }
        else if( i == 9 )
        {
            ahrs.setPen( orange );
            qsHead = "E";
        }
        else if( i == 18 )
        {
            ahrs.setPen( orange );
            qsHead = "S";
        }
        else if( i == 27 )
        {
            ahrs.setPen( orange );
            qsHead = "W";
        }
        else
        {
            ahrs.setPen( Qt::white );
            qsHead = QString::number( abs( i ) );
        }
        headRect = headMetrics.boundingRect( qsHead );
        ahrs.drawText( dW2 - (headRect.width() / 2.0), 22.0 + headRect.height(), qsHead );
        ahrs.resetTransform();
    }
}

