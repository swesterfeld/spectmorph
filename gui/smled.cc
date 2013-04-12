// KDE4 Led widget:
//
// this is a copy of the kdelibs4 Led widget; SpectMorph should not
// depend on KDE4 libs, which is why this is included here

/* This file is part of the KDE libraries
    Copyright (C) 1998 Jörg Habenicht (j.habenicht@europemail.com)
    Copyright (C) 2010 Christoph Feck <christoph@maxiom.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "smled.hh"

#include <QPainter>
#include <QImage>
#include <QStyle>
#include <QStyleOption>

using namespace SpectMorph;

class Led::Private
{
  public:
    Private()
      : darkFactor( 300 ),
        state( On ), look( Raised ), shape( Circular )
    {
    }

    int darkFactor;
    QColor color;
    State state;
    Look look;
    Shape shape;

    QPixmap cachedPixmap[2]; // for both states
    QStyle::ControlElement ce_indicatorLedCircular;
    QStyle::ControlElement ce_indicatorLedRectangular;
};



Led::Led( QWidget *parent )
  : QWidget( parent ),
    d( new Private )
{
  setColor( Qt::green );
}


Led::Led( const QColor& color, QWidget *parent )
  : QWidget( parent ),
    d( new Private )
{
  setColor( color );
}

Led::Led( const QColor& color, State state, Look look, Shape shape,
            QWidget *parent )
  : QWidget( parent ),
    d( new Private )
{
  d->state = (state == Off ? Off : On);
  d->look = look;
  d->shape = shape;

  setColor( color );
}

Led::~Led()
{
  delete d;
}

void Led::paintEvent( QPaintEvent* )
{
    switch( d->shape ) {
      case Rectangular:
        switch ( d->look ) {
          case Sunken:
            paintRectFrame( false );
            break;
          case Raised:
            paintRectFrame( true );
            break;
          case Flat:
            paintRect();
            break;
        }
        break;
      case Circular:
        switch ( d->look ) {
          case Flat:
            paintFlat();
            break;
          case Raised:
            paintRaised();
            break;
          case Sunken:
            paintSunken();
            break;
        }
        break;
    }
}

int Led::ledWidth() const
{
  // Make sure the LED is round!
  int size = qMin(width(), height());

  // leave one pixel border
  size -= 2;

  return qMax(0, size);
}

bool Led::paintCachedPixmap()
{
    if (d->cachedPixmap[d->state].isNull()) {
        return false;
    }
    QPainter painter(this);
    painter.drawPixmap(1, 1, d->cachedPixmap[d->state]);
    return true;
}

void Led::paintFlat()
{
    paintLed(Circular, Flat);
}

void Led::paintRaised()
{
    paintLed(Circular, Raised);
}

void Led::paintSunken()
{
    paintLed(Circular, Sunken);
}

void Led::paintRect()
{
    paintLed(Rectangular, Flat);
}

void Led::paintRectFrame( bool raised )
{
    paintLed(Rectangular, raised ? Raised : Sunken);
}

Led::State Led::state() const
{
  return d->state;
}

Led::Shape Led::shape() const
{
  return d->shape;
}

QColor Led::color() const
{
  return d->color;
}

Led::Look Led::look() const
{
  return d->look;
}

void Led::setState( State state )
{
  if ( d->state == state)
    return;

  d->state = (state == Off ? Off : On);
  updateCachedPixmap();
}

void Led::setShape( Shape shape )
{
  if ( d->shape == shape )
    return;

  d->shape = shape;
  updateCachedPixmap();
}

void Led::setColor( const QColor &color )
{
  if ( d->color == color )
    return;

  d->color = color;
  updateCachedPixmap();
}

void Led::setDarkFactor( int darkFactor )
{
  if ( d->darkFactor == darkFactor )
    return;

  d->darkFactor = darkFactor;
  updateCachedPixmap();
}

int Led::darkFactor() const
{
  return d->darkFactor;
}

void Led::setLook( Look look )
{
  if ( d->look == look)
    return;

  d->look = look;
  updateCachedPixmap();
}

void Led::toggle()
{
  d->state = (d->state == On ? Off : On);
  updateCachedPixmap();
}

void Led::on()
{
  setState( On );
}

void Led::off()
{
  setState( Off );
}

void Led::resizeEvent( QResizeEvent * )
{
    updateCachedPixmap();
}

QSize Led::sizeHint() const
{
    QStyleOption option;
    option.initFrom(this);
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, &option, this);
    return QSize( iconSize,  iconSize );
}

QSize Led::minimumSizeHint() const
{
  return QSize( 16, 16 );
}

void Led::updateCachedPixmap()
{
    d->cachedPixmap[Off] = QPixmap();
    d->cachedPixmap[On] = QPixmap();
    update();
}

static QColor
overlayColors (const QColor &base, const QColor &paint,
               QPainter::CompositionMode comp = QPainter::CompositionMode_SourceOver)
{
    // This isn't the fastest way, but should be "fast enough".
    // It's also the only safe way to use QPainter::CompositionMode
    QImage img(1, 1, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    QColor start = base;
    start.setAlpha(255); // opaque
    p.fillRect(0, 0, 1, 1, start);
    p.setCompositionMode(comp);
    p.fillRect(0, 0, 1, 1, paint);
    p.end();
    return img.pixel(0, 0);
}

void Led::paintLed(Shape shape, Look look)
{
    if (paintCachedPixmap()) {
        return;
    }

    QSize size(width() - 2, height() - 2);
    if (shape == Circular) {
        const int width = ledWidth();
        size = QSize(width, width);
    }
    QPointF center(size.width() / 2.0, size.height() / 2.0);
    const int smallestSize = qMin(size.width(), size.height());
    QPainter painter;

    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(0);

    QRadialGradient fillGradient(center, smallestSize / 2.0, QPointF(center.x(), size.height() / 3.0));
    const QColor fillColor = d->state != Off ? d->color : d->color.dark(d->darkFactor);
    fillGradient.setColorAt(0.0, fillColor.light(250));
    fillGradient.setColorAt(0.5, fillColor.light(130));
    fillGradient.setColorAt(1.0, fillColor);

    QConicalGradient borderGradient(center, look == Sunken ? 90 : -90);
    QColor borderColor = palette().color(QPalette::Dark);
    if (d->state == On) {
        QColor glowOverlay = fillColor;
        glowOverlay.setAlpha(80);
        borderColor = overlayColors(borderColor, glowOverlay);
    }
    borderGradient.setColorAt(0.2, borderColor);
    borderGradient.setColorAt(0.5, palette().color(QPalette::Light));
    borderGradient.setColorAt(0.8, borderColor);

    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(look == Flat ? QBrush(fillColor) : QBrush(fillGradient));
    const QBrush penBrush = (look == Flat) ? QBrush(borderColor) : QBrush(borderGradient);
    const qreal penWidth = smallestSize / 8.0;
    painter.setPen(QPen(penBrush, penWidth));
    QRectF r(penWidth / 2.0, penWidth / 2.0, size.width() - penWidth, size.height() - penWidth);
    if (shape == Rectangular) {
        painter.drawRect(r);
    } else {
        painter.drawEllipse(r);
    }
    painter.end();

    d->cachedPixmap[d->state] = QPixmap::fromImage(image);
    painter.begin(this);
    painter.drawPixmap(1, 1, d->cachedPixmap[d->state]);
    painter.end();
}
