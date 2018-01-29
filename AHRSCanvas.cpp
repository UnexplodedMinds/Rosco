/*
Stratux AHRS Display
(c) 2018 Unexploded Minds

The AHRSCanvas is just a giant QWidget used as a painting surface.
This class also receives the structs that are filled in by StreamReader
and uses a subset of those to paint the display.
*/

#include <QPainter>
#include <QtDebug>
#include <QMouseEvent>
#include <QTimer>
#include <QFont>
#include <QLinearGradient>
#include <QLineF>

#include <math.h>

#include "AHRSCanvas.h"
#include "BugSelector.h"
#include "Keypad.h"
#include "AHRSMainWin.h"
#include "StreamReader.h"


extern bool g_bEmulated;


AHRSCanvas::AHRSCanvas( QWidget *parent )
    : QWidget( parent ),
      m_pCanvas( 0 ),
      m_bInitialized( false ),
      m_iHeadBugAngle( -1 ),
      m_iWindBugAngle( -1 ),
      m_pRollIndicator( 0 ),
      m_pHeadIndicator( 0 ),
      m_pAltTape( 0 ),
      m_pSpeedTape( 0 ),
      m_pVertSpeedTape( 0 ),
      m_eTrafficDisp( AHRS::AllTraffic ),
      m_bHideGPSLocation( false ),
      m_bUpdated( false ),
      m_bShowWeather( false ),
      m_bShowGPSDetails( false )
{
    // Initialize weather and AHRS settings
    // No need to init the traffic because it starts out as an empty QMap.
    StreamReader::initWeather( m_weather );
    StreamReader::initSituation( m_situation );

    // Preload the fancier icons that are impractical to paint programmatically
    m_planeIcon.load( ":/graphics/resources/Plane.png" );
    m_headIcon.load( ":/icons/resources/HeadingIcon.png" );
    m_windIcon.load( ":/icons/resources/WindIcon.png" );
    m_trafficAltKey.load( ":/graphics/resources/AltitudeKey.png" );
    m_headIcon = m_headIcon.scaled( 80, 80, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    m_windIcon = m_windIcon.scaled( 80, 80, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );

    m_dDPIMult = static_cast<double>( physicalDpiX() ) / 144.0; // Android DPI vs Desktop DPI

    QTimer::singleShot( 500, this, SLOT( init() ) );
}


// Delete everything that needs deleting
AHRSCanvas::~AHRSCanvas()
{
    if( m_pRollIndicator != 0 )
    {
        delete m_pRollIndicator;
        m_pRollIndicator = 0;
    }
    if( m_pHeadIndicator != 0 )
    {
        delete m_pHeadIndicator;
        m_pHeadIndicator = 0;
    }
    if( m_pAltTape != 0 )
    {
        delete m_pAltTape;
        m_pAltTape = 0;
    }
    if( m_pSpeedTape != 0 )
    {
        delete m_pSpeedTape;
        m_pSpeedTape = 0;
    }
    if( m_pVertSpeedTape != 0 )
    {
        delete m_pVertSpeedTape;
        m_pVertSpeedTape = 0;
    }
    if( m_pCanvas != 0 )
    {
        delete m_pCanvas;
        m_pCanvas = 0;
    }
}


// Create the canvas utility instance, create the various pixmaps that are built up for fast painting
// and start the update timer.
void AHRSCanvas::init()
{
    m_pCanvas = new Canvas( width(), height() );

    CanvasConstants c = m_pCanvas->contants();

    m_iAltSpeedOffset = static_cast<int>( static_cast<double>( c.iTinyFontHeight ) * 0.37 );

    m_pRollIndicator = new QPixmap( static_cast<int>( c.dW2 ), static_cast<int>( c.dH2 / c.dAspectP ) );
    m_pHeadIndicator = new QPixmap( static_cast<int>( c.dW2 / (c.dW2 / (c.dH2 - c.dH7)) ), static_cast<int>( c.dH2 - c.dH7 ) );
    m_pAltTape = new QPixmap( static_cast<int>( c.dW5 ) - 50, c.iTinyFontHeight * 400 );    // 20000 ft / 100 x double the font height
    m_pVertSpeedTape = new QPixmap( 50, c.dH2 );
    m_pSpeedTape = new QPixmap( static_cast<int>( c.dW5 ), c.iTinyFontHeight * 60 );        // 300 Knots x double the font height
    m_pRollIndicator->fill( Qt::transparent );
    m_pHeadIndicator->fill( Qt::transparent );
    m_pAltTape->fill( Qt::transparent );
    m_pSpeedTape->fill( Qt::transparent );
    m_pVertSpeedTape->fill( Qt::transparent );
    buildRollIndicator();
    buildHeadingIndicator();
    buildAltTape();
    buildSpeedTape();
    buildVertSpeedTape();
    m_iDispTimer = startTimer( 1000 );     // Just drives updating the canvas if we're not currently receiving anything new.
    m_bInitialized = true;
}


// Android suspend
void AHRSCanvas::suspend( bool bSuspend )
{
    if( bSuspend )
    {
        if( m_iDispTimer != 0 )
            killTimer( m_iDispTimer );
        m_iDispTimer = 0;
    }
    else
    {
        if( m_iDispTimer != 0 )
            killTimer( m_iDispTimer );
        m_bInitialized = false;
        delete m_pRollIndicator;
        delete m_pHeadIndicator;
        delete m_pAltTape;
        delete m_pSpeedTape;
        delete m_pVertSpeedTape;
        init();
    }
}


// Resize event - on Android typically happens once at init
// Current android manifest locks the display to portait so it won't fire on rotating the device.
// This needs some thought though since I'm not sure it's really necessary to lock it into portrait mode.
void AHRSCanvas::resizeEvent( QResizeEvent *pEvent )
{
    if( pEvent == 0 )
        return;

    if( m_bInitialized )
    {
        if( m_iDispTimer != 0 )
            killTimer( m_iDispTimer );
        m_bInitialized = false;
        delete m_pRollIndicator;
        delete m_pHeadIndicator;
        delete m_pAltTape;
        delete m_pSpeedTape;
        delete m_pVertSpeedTape;
        init();
    }
}


// Just a utility timer that periodically updates the display when it's not being driven by the streams
// coming from the Stratux.
void AHRSCanvas::timerEvent( QTimerEvent *pEvent )
{
    if( pEvent == 0 )
        return;

    if( !m_bUpdated )
        update();
    m_bUpdated = false;
}


// Where all the magic happens
void AHRSCanvas::paintEvent( QPaintEvent *pEvent )
{
    if( (!m_bInitialized) || (pEvent == 0) )
        return;

    QPainter        ahrs( this );
    CanvasConstants c = m_pCanvas->contants();
    double          dPitchH = c.dH2 + (m_situation.dAHRSpitch / 22.5 * c.dH2);     // The visible portion is only 1/4 of the 90 deg range
    double          dArrowOffset = g_bEmulated ? 20 : 30;
    QString         qsHead( QString::number( static_cast<int>( m_situation.dAHRSGyroHeading ) ) );
    QPolygon        shape;
    QPen            linePen( Qt::black );
    double          dSlipSkid = c.dW2 - ((m_situation.dAHRSSlipSkid / 100.0) * c.dW2);
    QFont           tiny( "Roboto", 12, QFont::Normal );
    QFont           small( "Roboto", 16, QFont::Bold );
    QFont           med( "Roboto", 18, QFont::Bold );
    QFont           large( "Roboto", 24, QFont::Bold );

    if( dSlipSkid < (c.dW4 + 25.0) )
        dSlipSkid = c.dW4 + 25.0;
    else if( dSlipSkid > (c.dW2 + c.dW4 - 25.0) )
        dSlipSkid = c.dW2 + c.dW4 - 25.0;

    linePen.setWidth( 3 );

    ahrs.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing, true );

    // Translate to dead center and rotate by stratux roll then translate back
    ahrs.translate( c.dW2, c.dH2 );
    ahrs.rotate( -m_situation.dAHRSroll );
    ahrs.translate( -c.dW2, -c.dH2 );

    // Top half sky blue gradient offset by stratux pitch
    QLinearGradient skyGradient( 0.0, -c.dH2, 0.0, dPitchH );
    skyGradient.setColorAt( 0, Qt::blue );
    skyGradient.setColorAt( 1, QColor( 85, 170, 255 ) );
    ahrs.fillRect( -400.0, -c.dH2, c.dW + 800.0, dPitchH + c.dH2, skyGradient );

    // Draw brown gradient horizon half offset by stratux pitch
    QLinearGradient groundGradient( 0.0, dPitchH, 0, c.dH + c.dH2 );
    groundGradient.setColorAt( 0, QColor( 170, 85, 0  ) );
    groundGradient.setColorAt( 1, Qt::black );
    ahrs.fillRect( -400.0, dPitchH, c.dW + 800.0, c.dH + c.dH2, groundGradient );
    ahrs.setPen( linePen );
    ahrs.drawLine( -400, dPitchH, c.dW + 400.0, dPitchH );

    ahrs.setClipRect( 0, (m_pRollIndicator->height() / 3) + c.iLargeFontHeight + 50.0, c.dW, c.dH );
    for( int i = 0; i < 50; i += 10 )
    {
        linePen.setColor( Qt::cyan );
        ahrs.setPen( linePen );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH - ((i + 2.5) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH - ((i + 2.5) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH - ((i + 5.0) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH - ((i + 5.0) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH - ((i + 7.5) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH - ((i + 7.5) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW5, dPitchH - ((i + 10.0) / 22.5 * c.dH2), c.dW2 + c.dW5, dPitchH - (( i + 10.0) / 22.5 * c.dH2) );
        linePen.setColor( QColor( 67, 33, 9 ) );
        ahrs.setPen( linePen );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH + ((i + 2.5) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH + ((i + 2.5) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH + ((i + 5.0) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH + ((i + 5.0) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW20, dPitchH + ((i + 7.5) / 22.5 * c.dH2), c.dW2 + c.dW20, dPitchH + ((i + 7.5) / 22.5 * c.dH2) );
        ahrs.drawLine( c.dW2 - c.dW5, dPitchH + ((i + 10.0) / 22.5 * c.dH2), c.dW2 + c.dW5, dPitchH + (( i + 10.0) / 22.5 * c.dH2) );
    }
    ahrs.setClipping( false );

    // Reset rotation
    ahrs.resetTransform();

    // Slip/Skid indicator
    ahrs.setPen( QPen( Qt::white, 5 ) );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( c.dW2 - c.dW4, 1, c.dW2, c.iLargeFontHeight );
    ahrs.drawRect( c.dW2 - 30.0, 1.0, 60.0, c.iLargeFontHeight );
    ahrs.setPen( Qt::NoPen );
    ahrs.setBrush( Qt::white );
    ahrs.drawEllipse( dSlipSkid - 25.0,
                      1.0,
                      50.0,
                      c.iLargeFontHeight );

    // Draw the top roll indicator
    ahrs.translate( c.dW2, c.iLargeFontHeight + c.dH4 + 20.0 );
    ahrs.rotate( -m_situation.dAHRSroll );
    ahrs.translate( -c.dW2, -(c.iLargeFontHeight + c.dH4 + 20.0) );
    ahrs.drawPixmap( c.dW2 - c.dW4, (c.iLargeFontHeight * 2) + 20.0, *m_pRollIndicator );
    ahrs.resetTransform();

    QPolygonF arrow;

    arrow.append( QPointF( c.dW2, (c.iLargeFontHeight * 2) + (g_bEmulated ? 70.0 : 130.0) ) );
    arrow.append( QPointF( c.dW2 + dArrowOffset, (c.iLargeFontHeight * 2) + (g_bEmulated ? 70.0 : 130.0) + dArrowOffset ) );
    arrow.append( QPointF( c.dW2 - dArrowOffset, (c.iLargeFontHeight * 2) + (g_bEmulated ? 70.0 : 130.0) + dArrowOffset ) );
    ahrs.setBrush( Qt::white );
    ahrs.setPen( Qt::black );
    ahrs.drawPolygon( arrow );

    // Draw the yellow pitch indicators
    ahrs.setBrush( Qt::yellow );
    shape.append( QPoint( c.dW5 + c.dW20, c.dH2 - c.dH160 ) );
    shape.append( QPoint( c.dW2 - c.dW10, c.dH2 - c.dH160 ) );
    shape.append( QPoint( c.dW2 - c.dW10 + (g_bEmulated ? 10 : 20) , c.dH2 ) );
    shape.append( QPoint( c.dW2 - c.dW10, c.dH2 + c.dH160 ) );
    shape.append( QPoint( c.dW5 + c.dW20, c.dH2 + c.dH160 ) );
    ahrs.drawPolygon( shape );
    shape.clear();
    shape.append( QPoint( c.dW - c.dW5 - c.dW20, c.dH2 - c.dH160 ) );
    shape.append( QPoint( c.dW2 + c.dW10, c.dH2 - c.dH160 ) );
    shape.append( QPoint( c.dW2 + c.dW10 - (g_bEmulated ? 10 : 20), c.dH2 ) );
    shape.append( QPoint( c.dW2 + c.dW10, c.dH2 + c.dH160 ) );
    shape.append( QPoint( c.dW - c.dW5 - c.dW20, c.dH2 + c.dH160 ) );
    ahrs.drawPolygon( shape );
    shape.clear();
    shape.append( QPoint( c.dW2, c.dH2 ) );
    shape.append( QPoint( c.dW2 - c.dW10, c.dH2 + (g_bEmulated ? 20 : 40) ) );
    shape.append( QPoint( c.dW2 + c.dW10, c.dH2 + (g_bEmulated ? 20 : 40) ) );
    ahrs.drawPolygon( shape );

    // Draw the heading value over the indicator
    ahrs.setPen( QPen( Qt::white, 5 ) );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( c.dW2 - c.dW10, c.dH - m_pHeadIndicator->height() - 45.0 - c.iLargeFontHeight, c.dW5, c.iLargeFontHeight );
    ahrs.setPen( Qt::white );
    ahrs.setFont( large );
    ahrs.drawText( c.dW2 - (m_pCanvas->largeWidth( qsHead ) / 2), c.dH - m_pHeadIndicator->height() - 45.0 - m_iAltSpeedOffset, qsHead );

    // Arrow for heading position above heading dial
    arrow.clear();
    arrow.append( QPointF( c.dW2, c.dH - m_pHeadIndicator->height() - 15.0 ) );
    arrow.append( QPointF( c.dW2 + dArrowOffset, c.dH - m_pHeadIndicator->height() - 35.0 ) );
    arrow.append( QPointF( c.dW2 - dArrowOffset, c.dH - m_pHeadIndicator->height() - 35.0 ) );
    ahrs.setBrush( Qt::white );
    ahrs.setPen( Qt::black );
    ahrs.drawPolygon( arrow );

    // Draw the heading pixmap and rotate it to the current heading
    ahrs.translate( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 );
    ahrs.rotate( -m_situation.dAHRSGyroHeading );
    ahrs.translate( -c.dW2, -(c.dH - (m_pHeadIndicator->height() / 2) - 10.0) );
    ahrs.drawPixmap( c.dW2 - (m_pHeadIndicator->width() / 2), c.dH - m_pHeadIndicator->height() - 10.0, *m_pHeadIndicator );
    ahrs.resetTransform();

    // Draw the central airplane
    ahrs.drawPixmap( QRect( c.dW2 - c.dW20, c.dH - 10 - (m_pHeadIndicator->height() / 2) - c.dH20, c.dW10, c.dH10 ), m_planeIcon );

    // Draw the heading bug
    if( m_iHeadBugAngle >= 0 )
    {
        ahrs.translate( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 );
        ahrs.rotate( m_iHeadBugAngle + m_situation.dAHRSGyroHeading );
        ahrs.translate( -c.dW2, -(c.dH - (m_pHeadIndicator->height() / 2) - 10.0) );
        ahrs.drawPixmap( c.dW2 - 50, c.dH - m_pHeadIndicator->height() - 50.0, m_headIcon );
        ahrs.resetTransform();
    }

    // Draw the wind bug
    if( m_iWindBugAngle >= 0 )
    {
        ahrs.translate( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 );
        ahrs.rotate( m_iWindBugAngle + m_situation.dAHRSGyroHeading );
        ahrs.translate( -c.dW2, -(c.dH - (m_pHeadIndicator->height() / 2) - 10.0) );
        ahrs.drawPixmap( c.dW2 - 50, c.dH - m_pHeadIndicator->height() - 50.0, m_windIcon );
        ahrs.resetTransform();
    }

    // Draw the Altitude tape
    linePen.setColor( Qt::white );
    linePen.setWidth( 5 );
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::NoBrush );
    ahrs.drawRect( c.dW - c.dW5, 1.0, c.dW5, c.dH2 - 1.0 );
    ahrs.setClipRect( c.dW - c.dW5 + 1.0, 2.0, c.dW5 - 4.0, c.dH2 - 4.0 );
    ahrs.drawPixmap( c.dW - c.dW5 + 5.0, c.dH4 - (c.iTinyFontHeight * 2) - (((20000.0 - m_situation.dBaroPressAlt) / 20000.0) * m_pAltTape->height()), *m_pAltTape );
    ahrs.setClipping( false );

    // Draw the dividing line and vertical speed static pixmap
    ahrs.drawLine( c.dW - 50.0, 1.0, c.dW - 50.0, c.dH2 - 1.0 );
    ahrs.drawPixmap( c.dW - 50.0, 0.0, *m_pVertSpeedTape );

    // Draw the vertical speed indicator
    ahrs.translate( 0.0, m_situation.dGPSVertSpeed / 1000.0 * c.dH4 );
    arrow.clear();
    arrow.append( QPoint( c.dW - dArrowOffset, c.dH4 ) );
    arrow.append( QPoint( c.dW, c.dH4 - dArrowOffset ) );
    arrow.append( QPoint( c.dW, c.dH4 + dArrowOffset ) );
    ahrs.setPen( Qt::black );
    ahrs.setBrush( Qt::white );
    ahrs.drawPolygon( arrow );
    ahrs.resetTransform();

    // Draw the current altitude
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( c.dW - c.dW5, c.dH4 - (c.iLargeFontHeight / 2), c.dW5 - 50.0, c.iLargeFontHeight );
    ahrs.setPen( Qt::white );
    ahrs.setFont( small );
    ahrs.drawText( c.dW - c.dW5 + 5, c.dH4 + (c.iLargeFontHeight / 2) - m_iAltSpeedOffset, QString::number( static_cast<int>( m_situation.dBaroPressAlt ) ) );

    // Draw the Speed tape
    linePen.setColor( Qt::white );
    linePen.setWidth( 5 );
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::NoBrush );
    ahrs.drawRect( 0, 1.0, c.dW5, c.dH2 - 1.0 );
    ahrs.setClipRect( 2.0, 2.0, c.dW5 - 4.0, c.dH2 - 4.0 );
    ahrs.drawPixmap( 3, c.dH4 - c.iSmallFontHeight - (((300.0 - m_situation.dGPSGroundSpeed) / 300.0) * m_pSpeedTape->height()), *m_pSpeedTape );
    ahrs.setClipping( false );

    // Draw the current speed
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( 0, c.dH4 - (c.iLargeFontHeight / 2), c.dW5, c.iLargeFontHeight );
    ahrs.setPen( Qt::white );
    ahrs.setFont( large );
    ahrs.drawText( 5, c.dH4 + (c.iLargeFontHeight / 2) - m_iAltSpeedOffset, QString::number( static_cast<int>( m_situation.dGPSGroundSpeed ) ) );

    // Draw the G-Force indicator box and scale
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::NoBrush );
    ahrs.drawRect( 0, c.dH2, c.dW5, c.iLargeFontHeight );
    ahrs.setFont( tiny );
    ahrs.setPen( Qt::white );
    ahrs.drawText( 5, c.dH2 + c.iTinyFontHeight, "2" );
    ahrs.drawText( (c.dW5 / 2) - (c.iTinyFontWidth / 2), c.dH2 + c.iTinyFontHeight, "0" );
    ahrs.drawText( c.dW5 - c.iTinyFontWidth - 5, c.dH2 + c.iTinyFontHeight, "2" );

    // Arrow for G-Force indicator
    arrow.clear();
    arrow.append( QPoint( c.dW10, c.dH2 + c.iLargeFontHeight - dArrowOffset ) );
    arrow.append( QPoint( c.dW10 - dArrowOffset, c.dH2 + c.iLargeFontHeight ) );
    arrow.append( QPoint( c.dW10 + dArrowOffset, c.dH2 + c.iLargeFontHeight ) );
    ahrs.setPen( Qt::black );
    ahrs.setBrush( Qt::white );
    ahrs.translate( (m_situation.dAHRSGLoad - 1.0) * c.dW5, 0.0 );
    ahrs.drawPolygon( arrow );
    ahrs.resetTransform();

    // GPS Lat/Long
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( c.dW - c.dW5, c.dH2, c.dW5, c.iLargeFontHeight );
    ahrs.drawRect( c.dW - c.dW5, c.dH2 + c.iLargeFontHeight, c.dW5, c.iLargeFontHeight );
    ahrs.setPen( Qt::green );
    ahrs.setFont( small );
    QString qsLat = QString( "%1 %2" )
                        .arg( m_bHideGPSLocation ? 12.3456 : fabs( m_situation.dGPSlat ) )
                        .arg( (m_situation.dGPSlat < 0.0) ? "E" : "W" );
    QString qsLong = QString( "%1 %2" )
                        .arg( m_bHideGPSLocation ? 34.5678 : fabs( m_situation.dGPSlong ) )
                        .arg( (m_situation.dGPSlong < 0.0) ? "N" : "S" );
    ahrs.drawText( c.dW - c.dW5 + 8.0, c.dH2 + c.iLargeFontHeight - m_iAltSpeedOffset - 4, qsLat );
    ahrs.drawText( c.dW - c.dW5 + 8.0, c.dH2 + (c.iLargeFontHeight * 2) - m_iAltSpeedOffset - 4, qsLong );

    // Traffic altitude key
    ahrs.setPen( linePen );
    ahrs.setBrush( Qt::black );
    ahrs.drawRect( 0, c.dH2 + c.iLargeFontHeight, c.dW5, c.iLargeFontHeight );
    ahrs.setPen( Qt::NoPen );
    ahrs.drawPixmap( 3.0, c.dH2 + c.iLargeFontHeight + 2.0, c.dW5 - 5.0, c.iLargeFontHeight - 4.0, m_trafficAltKey );

    if( m_eTrafficDisp != AHRS::NoTraffic )
        updateTraffic( &ahrs, c.dH2 + (c.iLargeFontHeight * 2.0) + 30.0 );

    QLinearGradient cloudyGradient( 0.0, 50.0, 0.0, c.dH - 50.0 );
    cloudyGradient.setColorAt( 0, QColor( 255, 255, 255, 225 ) );
    cloudyGradient.setColorAt( 1, QColor( 175, 175, 255, 225 ) );

    if( m_bShowWeather )
    {
        linePen.setColor( Qt::black );
        linePen.setWidth( 3 );
        ahrs.setPen( linePen );
        ahrs.setBrush( cloudyGradient );
        ahrs.drawRect( 50, 50, c.dW - 100, c.dH - 100 );
        ahrs.setFont( med );
        if( m_weather.prodTime.date() == QDate( 2000, 1, 1 ) )
            ahrs.drawText( 100, 100, "No Weather Data Available" );
        else
        {
            ahrs.drawText( 100, 100, m_weather.prodTime.toString() );
            ahrs.drawText( 100, 100 + (c.iMedFontHeight * 3), m_weather.qsType );
            ahrs.drawText( 100, 100 + (c.iMedFontHeight * 5), m_weather.qsLocation );
            ahrs.drawText( 100, 100 + (c.iMedFontHeight * 7), m_weather.qsData );
            ahrs.drawText( 100, 100 + (c.iMedFontHeight * 9), m_weather.qsLastMessage );
        }
    }

    if( m_bShowGPSDetails )
    {
        linePen.setColor( Qt::black );
        linePen.setWidth( 3 );
        ahrs.setPen( linePen );
        ahrs.setBrush( cloudyGradient );
        ahrs.drawRect( 50, 50, c.dW - 100, c.dH - 100 );
        ahrs.setFont( med );
        ahrs.drawText( 100, 100, "GPS Status" );
        ahrs.drawText( 100, 100 + (c.iMedFontHeight * 3),  QString( "GPS Satellites Seen: %1" ).arg( m_situation.iGPSSatsSeen ) );
        ahrs.drawText( 100, 100 + (c.iMedFontHeight * 5),  QString( "GPS Satellites Tracked: %1" ).arg( m_situation.iGPSSatsTracked ) );
        ahrs.drawText( 100, 100 + (c.iMedFontHeight * 7),  QString( "GPS Satellites Locked: %1" ).arg( m_situation.iGPSSats ) );
        ahrs.drawText( 100, 100 + (c.iMedFontHeight * 9),  QString( "GPS Fix Quality: %1" ).arg( m_situation.iGPSFixQuality ) );
    }
}


// Draw the traffic onto the heading indicator and the tail numbers on the side
void AHRSCanvas::updateTraffic( QPainter *pAhrs, double dListPos )
{
    QList<StratuxTraffic> trafficList = m_trafficMap.values();
    StratuxTraffic        traffic;
    double                dDistInc = m_pHeadIndicator->height() / 80.0 * 1.75;   // The heading indicator outer diameter = 20NM
    QPen                  planePen( Qt::black, g_bEmulated ? 15 : 30, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin );
    CanvasConstants       c = m_pCanvas->contants();
    QFont                 trafficFont( "Roboto", 12, QFont::Bold );
    QFontMetrics          trafficMetrics( trafficFont );
    QRect                 trafficRect( trafficMetrics.boundingRect( "N0000000" ) );
    int                   iTrafficCount = trafficList.count();

    foreach( traffic, trafficList )
    {
        if( (m_eTrafficDisp == AHRS::ADSBOnlyTraffic) && (!traffic.bHasADSB) )
            iTrafficCount--;
    }
    if( iTrafficCount > 0 )
    {
        QLinearGradient trafficGradient( 0.0, dListPos - 10.0, 0.0, dListPos - 10.0 + (c.iTinyFontHeight * (trafficList.count() + 1)) );

        trafficGradient.setColorAt( 0, Qt::lightGray );
        trafficGradient.setColorAt( 1, Qt::darkGray );
        pAhrs->setPen( Qt::black );
        pAhrs->setBrush( trafficGradient );
        pAhrs->drawRect( c.dW - trafficRect.width() - 40.0, dListPos - 10.0, trafficRect.width() + 20, c.iTinyFontHeight * (trafficList.count() + 1) );
    }

    pAhrs->setFont( trafficFont );

    // Draw a large dot for each aircraft; the outer edge of the heading indicator is calibrated to be 20 NM out from your position
    foreach( traffic, trafficList )
    {
        if( (m_eTrafficDisp == AHRS::ADSBOnlyTraffic) && (!traffic.bHasADSB) )
            continue;

        planePen.setColor( Qt::black );                   // Gray is hard to see on the gray gradient so the gray text is black
        if( traffic.dAlt >= 2000.0 )
            planePen.setColor( QColor( 0, 128, 128 ) );   // teal
        if( traffic.dAlt >= 5000.0 )
            planePen.setColor( QColor( 128, 0, 128 ) );   // purple
        if( traffic.dAlt >= 10000.0 )
            planePen.setColor( Qt::red );
        if( traffic.dAlt >= 12000.0 )
            planePen.setColor( Qt::magenta );
        if( traffic.dAlt >= 15000.0 )
            planePen.setColor( Qt::green );
        if( traffic.dAlt >= 18000.0 )
            planePen.setColor( Qt::yellow );
        if( traffic.dAlt >= 20000.0 )
            planePen.setColor( Qt::blue );
        if( traffic.dAlt >= 25000.0 )
            planePen.setColor( Qt::cyan );
        if( traffic.dAlt >= 30000.0 )
            planePen.setColor( QColor( 173, 255, 47 ) );  // snot green
        if( traffic.dAlt >= 40000.0 )
            planePen.setColor( QColor( 214, 153, 255 ) ); // lavender

        // If bearing and distance were able to be calculated then show relative position
        if( traffic.bHasADSB )
        {
            pAhrs->translate( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 );
            pAhrs->rotate( traffic.dBearing + m_situation.dAHRSMagHeading );
            pAhrs->translate( -c.dW2, -(c.dH - (m_pHeadIndicator->height() / 2) - 10.0) );
            planePen.setWidth( g_bEmulated ? 15 : 30 );
            pAhrs->setPen( planePen );
            pAhrs->drawPoint( c.dW2, c.dH - (m_pHeadIndicator->height() / 2) - 10.0 - (traffic.dDist * dDistInc) );
            pAhrs->resetTransform();
        }

        // List the tail numbers along the right side
        planePen.setWidth( 1 );
        pAhrs->setPen( planePen );
        dListPos += c.iTinyFontHeight;
        pAhrs->drawText( c.dW - trafficRect.width() - 20.0, dListPos, traffic.qsReg );
        if( traffic.bHasADSB )
        {
            planePen.setWidth( 7 );
            pAhrs->setPen( planePen );
            pAhrs->drawPoint( c.dW - trafficRect.width() - 30.0, dListPos - (c.iTinyFontHeight / 2) + 3 );
        }
    }
}


// Situation (mostly AHRS data) update
void AHRSCanvas::situation( StratuxSituation s )
{
    m_situation = s;
    m_bUpdated = true;
    update();
}


// Traffic update
void AHRSCanvas::traffic( int iICAO, StratuxTraffic t )
{
    QMapIterator<int, StratuxTraffic> it( m_trafficMap );
    bool                              bTrafficRemoved = true;

    // Each time this is updated, remove an old entry
    while( bTrafficRemoved )
    {
        bTrafficRemoved = false;
        while( it.hasNext() )
        {
            it.next();
            // Anything older than 60 seconds discard
            if( it.value().dAge > 60.0 )
            {
                m_trafficMap.remove( it.key() );
                bTrafficRemoved = true;
                break;
            }
        }
    }

    m_trafficMap.insert( iICAO, t );
    m_bUpdated = true;
    update();
}


// Weather update - almost no testing! The display of this is mostly guesswork
// and even the raw websocket message is displayed so we can easily look at the guts
// Once I have some logs of various weather outputs the raw message shown on the
// display will be removed.
void AHRSCanvas::weather( StratuxWeather w )
{
    m_weather = w;
    update();
}


// Handle various screen presses (pressing the screen is handled the same as a mouse click here)
void AHRSCanvas::mousePressEvent( QMouseEvent *pEvent )
{
    if( pEvent == 0 )
        return;

    // Tapping anywhere on the screen when the weather is active will turn it off
    if( m_bShowWeather )
    {
        m_bShowWeather = false;
        update();
        return;
    }
    else if( m_bShowGPSDetails )
    {
        m_bShowGPSDetails = false;
        update();
        return;
    }

    // Otherwise we're looking for specific spots
    CanvasConstants c = m_pCanvas->contants();
    QPoint          pressPt( pEvent->pos() );
    QRect           headRect( c.dW2 - (m_pHeadIndicator->width() / 2), c.dH - m_pHeadIndicator->height() - 10.0, m_pHeadIndicator->width(), m_pHeadIndicator->height() );
    QRect           gpsRect( c.dW - c.dW5, c.dH2, c.dW5, c.iLargeFontHeight * 2.0 );

    // User pressed on the heading indicator
    if( headRect.contains( pressPt ) )
    {
        int         iButton = -1;
        BugSelector bugSel( this );

        bugSel.setGeometry( c.dW2 - 250.0 + (g_bEmulated ? 2000 : 0), c.dH - (m_pHeadIndicator->height() / 2) - 150.0, 500.0, 300.0 );

        iButton = bugSel.exec();

        // Back was pressed
        if( iButton == -1 )
            return;

        Keypad keypad( this );

        if( keypad.exec() == QDialog::Accepted )
        {
            int iAngle = keypad.value();

            // Automatically wrap around
            while( iAngle > 360 )
                iAngle -= 360;
            // Heading bug
            if( iButton == QDialog::Accepted )
                m_iHeadBugAngle = iAngle;
            // Wind bug
            else if( iButton == QDialog::Rejected )
                m_iWindBugAngle = iAngle;
        }
        else
        {   // Heading bug
            if( iButton == QDialog::Accepted )
                m_iHeadBugAngle = -1;
            // Wind bug
            else if( iButton == QDialog::Rejected )
                m_iWindBugAngle = -1;
        }
    }
    else if( gpsRect.contains( pressPt ) )
    {
        m_bShowGPSDetails = (!m_bShowGPSDetails);
    }

    m_bUpdated = true;
    update();
}


// A small easter-egg for displaying bogus GPS location for when you're
// making videos or taking pictures and you don't want the good people of the internet
// to find you and kill you in your sleep.
void AHRSCanvas::mouseDoubleClickEvent( QMouseEvent *pEvent )
{
    CanvasConstants c = m_pCanvas->contants();
    QRect           gpsRect( c.dW - c.dW5, c.dH2, c.dW5, c.iLargeFontHeight * 2.0 );
    QPoint          pressPt = pEvent->pos();

    if( gpsRect.contains( pressPt ) )
        m_bHideGPSLocation = (!m_bHideGPSLocation);
}


// Build the altitude tape pixmap
// Note that there are pixmap size limits on Android that appear to be smaller than Windows and X11
// so there are font size limitations in order not to exceed them.
void AHRSCanvas::buildAltTape()
{
    QPainter        ahrs( m_pAltTape );
    QFont           altFont( "Roboto", 12 );
    int             iAlt, iV = 1, iY;
    CanvasConstants c = m_pCanvas->contants();

    ahrs.setFont( altFont );
    for( iAlt = 20000; iAlt >= 0; iAlt -= 100 )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * c.iTinyFontHeight * 2;
        ahrs.scale( (c.dW <= 1440) ? 1.5 : 2.0, 1.0 );
        ahrs.drawText( 0, iY, QString::number( iAlt ) );
        ahrs.resetTransform();
        iY = iY - (c.iTinyFontHeight / 2) + m_iAltSpeedOffset - 2;
        ahrs.drawLine( m_pAltTape->width() - 24, iY, m_pAltTape->width() - 4, iY );
        ahrs.setPen( QPen( Qt::blue, 2 ) );
        for( int i = 0; i < 3; i++ )
        {
            iY += (c.iTinyFontHeight / 2 );
            ahrs.drawLine( m_pAltTape->width() - 19, iY, m_pAltTape->width() - 4, iY );
        }
        iV++;
    }
}


// Build the vertical speed tape pixmap
void AHRSCanvas::buildVertSpeedTape()
{
    QPainter ahrs( m_pVertSpeedTape );
    QFont    vertFont( "Roboto", 10 );
    int      iVert, iV = 1, iY;
    int      iLineHeight = height() / 80;

    ahrs.setFont( vertFont );
    for( iVert = 10; iVert >= -10; iVert-- )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * iLineHeight * 2;
        if( (iVert % 2) == 0 )
            ahrs.drawText( 5, iY, QString::number( abs( iVert ) ) );
        iY -= 3;
        ahrs.drawLine( m_pVertSpeedTape->width() - 14, iY - (g_bEmulated ? 2 : 7), m_pVertSpeedTape->width() - 4, iY - (g_bEmulated ? 2 : 7) );
        ahrs.setPen( QPen( Qt::blue, 2 ) );
        if( iVert != -10 )
        {
            iY += iLineHeight;
            ahrs.drawLine( m_pVertSpeedTape->width() - 9, iY - (g_bEmulated ? 2 : 7), m_pVertSpeedTape->width() - 4, iY - (g_bEmulated ? 2 : 7) );
        }
        iV++;
    }
}


// Build the air speed (ground speed actually since this is GPS based without any pitot-static system)
void AHRSCanvas::buildSpeedTape()
{
    QPainter        ahrs( m_pSpeedTape );
    QFont           speedFont( "Roboto", 24, QFont::Bold );
    int             iSpeed, iV = 1, iY;
    CanvasConstants c = m_pCanvas->contants();

    ahrs.setFont( speedFont );
    for( iSpeed = 300; iSpeed >= 0; iSpeed -= 10 )
    {
        ahrs.setPen( QPen( Qt::white, 2 ) );
        iY = iV * c.iTinyFontHeight * 2;
        ahrs.drawText( 30, iY + (c.iLargeFontHeight / 6), QString::number( iSpeed ) );
        iY = iY - (c.iTinyFontHeight / 2) + m_iAltSpeedOffset - 2;
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
void AHRSCanvas::buildRollIndicator()
{
    QPainter     ahrs( m_pRollIndicator );
    double       dW = m_pRollIndicator->width();
    double       dH = m_pRollIndicator->height();
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
void AHRSCanvas::buildHeadingIndicator()
{
    QPainter ahrs( m_pHeadIndicator );
    double   dW = m_pHeadIndicator->width();
    double   dH = m_pHeadIndicator->height();
    double   dW2 = dW / 2.0;
    double   dH2 = dH / 2.0;
    QPen     linePen( Qt::white, 3 );
    bool     bThis = true;
    QColor   orange( 255, 165, 0 );
    int      iFontSize = 20;

    CanvasConstants c = m_pCanvas->contants();

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


// Traffic setting changed (All, ADSB-only, None)
void AHRSCanvas::trafficToggled( AHRS::TrafficDisp eDispType )
{
    m_eTrafficDisp = eDispType;
    m_bUpdated = true;
    update();
}


// Weather turned on/off
void AHRSCanvas::weatherToggled()
{
    m_bShowWeather = (!m_bShowWeather);
    update();
}

