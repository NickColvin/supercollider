/************************************************************************
*
* Copyright 2010 Jakob Leben (jakob.leben@gmail.com)
*
* This file is part of SuperCollider Qt GUI.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
************************************************************************/

#include "primitives.h"
#include "../Painting.h"
#include "../Slot.h"

#include <QPainter>

namespace QtCollider
{

template <int FN( PyrSlot*, PyrSlot*, VMGlobals* )>
class QPenPrimitive
{
public:
  QPenPrimitive ( const char *name, int argc ) {
    LangPrimitiveData d;
    d.mediator = &mediate;
    d.name = strdup(name);
    d.argc = argc;
    langPrimitives().append( d );
  }
private:
  static int mediate( VMGlobals *g, int i ) {
    if( !globalPainter() ) {
      qcErrorMsg( QString("Usage of QPen is not allowed at this point!") );
      return errFailed;
    }
    PyrSlot *stack = g->sp - i + 1;
    int ret = (*FN)( stack, i > 1 ? stack+1 : 0, g );
    return ret;
  }
};

}

#define QC_QPEN_PRIMITIVE( name, argc, receiver, args, global ) \
  int name ( receiver, args, global ); \
  static QtCollider::QPenPrimitive<&name> p_##name( "_" #name, argc ); \
  int name ( receiver, args, global )

static QPainter *painter = 0;
static QPainterPath path;

namespace QtCollider {
  void beginPainting( QPainter *p )
  {
    if( painter )
      qcErrorMsg( QString("Painting already in progress!") );

    painter = p;
    painter->setRenderHint( QPainter::Antialiasing, true );
    QColor black( 0,0,0 );
    painter->setPen( black );
    painter->setBrush( black );

    path = QPainterPath();
  }

  void endPainting()
  {
    painter = 0;
  }

  QPainter *globalPainter()
  {
    return painter;
  }
}

QC_QPEN_PRIMITIVE( QPen_Save, 0, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  painter->save();
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Restore, 0, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  painter->restore();
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Clear, 0, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  path = QPainterPath();
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_FillColor, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QColor color = Slot::toColor( a );
  painter->setBrush( color );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_StrokeColor, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QColor color = Slot::toColor( a );
  painter->setPen( color );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Width, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  float width;
  if( slotFloatVal( a, &width ) ) return errWrongType;
  QPen pen = painter->pen();
  pen.setWidthF( width );
  painter->setPen( pen );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Clip, 0, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  painter->setClipPath( path );
  path = QPainterPath();
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_AntiAliasing, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  bool b = IsTrue( a );
  if( !b && !IsFalse( a ) ) return errWrongType;
  painter->setRenderHint( QPainter::Antialiasing, b );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_SetFont, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  painter->setFont( Slot::toFont( a ) );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Translate, 2, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  float x, y;
  if( slotFloatVal( a, &x ) ) return errWrongType;
  if( slotFloatVal( a+1, &y ) ) return errWrongType;
  painter->translate( x, y );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Scale, 2, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  float x, y;
  if( slotFloatVal( a, &x ) ) return errWrongType;
  if( slotFloatVal( a+1, &y ) ) return errWrongType;
  painter->scale( x, y );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Shear, 2, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  float x, y;
  if( slotFloatVal( a, &x ) ) return errWrongType;
  if( slotFloatVal( a+1, &y ) ) return errWrongType;
  painter->shear( x, y );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Rotate, 3, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  float angle, x, y;
  if( slotFloatVal( a, &angle ) ) return errWrongType;
  if( slotFloatVal( a+1, &x ) ) return errWrongType;
  if( slotFloatVal( a+2, &y ) ) return errWrongType;

  painter->translate( x, y );
  painter->rotate( angle );
  painter->translate( -x, -y );

  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Transform, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  VariantList list = Slot::toVariantList( a );
  if( list.data.count() < 6 ) return errWrongType;
  float f[6];
  int i = 6;
  while( i ) {
    --i;
    QVariant var = list.data[i];
    if( !var.canConvert( QVariant::Double ) ) return errWrongType;
    f[i] = list.data[i].value<float>();
  }
  QTransform transform( f[0], f[1], f[2], f[3], f[4], f[5] );
  painter->setWorldTransform( transform );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_MoveTo, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QPointF point = Slot::toPoint( a );
  path.moveTo( point );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_LineTo, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QPointF point = Slot::toPoint( a );
  path.lineTo( point );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_CubicTo, 3, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QPointF endPoint = Slot::toPoint( a );
  QPointF cPoint1 = Slot::toPoint( a+1 );
  QPointF cPoint2 = Slot::toPoint( a+2 );
  path.cubicTo( cPoint1, cPoint2, endPoint );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_QuadTo, 2, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QPointF endPoint = Slot::toPoint( a );
  QPointF cPoint = Slot::toPoint( a+1 );
  path.quadTo( cPoint, endPoint );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_AddRect, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QRectF rect = Slot::toRect( a );
  path.addRect( rect );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_AddRoundedRect, 3, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  float radiusX, radiusY;
  QRectF rect;

  rect = Slot::toRect( a );
  if( slotFloatVal( a+1, &radiusX ) ) return errWrongType;
  if( slotFloatVal( a+2, &radiusY ) ) return errWrongType;

  path.addRoundedRect( rect, radiusX, radiusY );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_AddEllipse, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QRectF rect = Slot::toRect( a );
  path.addEllipse( rect );
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_AddWedge, 4, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QPointF center = Slot::toPoint( a );
  float radius, startAngle, sweepLength;
  if( slotFloatVal( a+1, &radius ) ) return errWrongType;
  if( slotFloatVal( a+2, &startAngle ) ) return errWrongType;
  if( slotFloatVal( a+3, &sweepLength ) ) return errWrongType;
  path.moveTo( center );
  QRectF rect;
  rect.setSize( QSizeF( 2*radius, 2*radius ) );
  rect.moveCenter( center );
  path.arcTo( rect, startAngle, sweepLength );
  path.closeSubpath();
  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_AddAnnularWedge, 5, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QPointF center = Slot::toPoint( a );
  float innerRadius, outerRadius, startAngle, sweepLength;
  if( slotFloatVal( a+1, &innerRadius ) ) return errWrongType;
  if( slotFloatVal( a+2, &outerRadius ) ) return errWrongType;
  if( slotFloatVal( a+3, &startAngle ) ) return errWrongType;
  if( slotFloatVal( a+4, &sweepLength ) ) return errWrongType;

  QPainterPath annularWedge;
  annularWedge.moveTo( center );
  QRectF rect;
  rect.setSize( QSizeF( 2*outerRadius, 2*outerRadius ) );
  rect.moveCenter( center );
  annularWedge.arcTo( rect, startAngle, sweepLength );
  annularWedge.closeSubpath();

  QPainterPath circle;
  QRectF r2( 0,0,2*innerRadius, 2*innerRadius );
  r2.moveCenter( center );
  circle.addEllipse( r2 );

  //FIXME this is VERY slow!!
  path.addPath( annularWedge - circle );

  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_Draw, 1, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  if( path.isEmpty() ) return errNone;

  int style = Slot::toInt( a );
  QPen pen = painter->pen();
  QBrush brush = painter->brush();

  switch ( style ) {
    case 0:
    case 1:
      painter->setPen( Qt::NoPen ); break;
    case 2:
      painter->setBrush( Qt::NoBrush ); break;
    case 3:
    case 4:
    default: ;
  }

  painter->drawPath( path );

  path = QPainterPath();

  painter->setPen( pen );
  painter->setBrush( brush );

  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_StringAtPoint, 2, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QString str = Slot::toString( a );
  if( str.isEmpty() ) return errNone;
  QPointF pt = Slot::toPoint( a+1 );

  QFont f( painter->font() );
  QFontMetrics fm( f );
  QRect rect = fm.boundingRect( str );
  painter->drawText( pt - rect.topLeft(), str );

  return errNone;
}

QC_QPEN_PRIMITIVE( QPen_StringInRect, 2, PyrSlot *r, PyrSlot *a, VMGlobals *g )
{
  QString str = Slot::toString( a );
  if( str.isEmpty() ) return errNone;
  QRectF rect = Slot::toRect( a+1 );

  painter->drawText( rect, Qt::AlignTop | Qt::AlignLeft, str );

  return errNone;
}
