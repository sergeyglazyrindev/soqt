/**************************************************************************\
 *
 *  Copyright (C) 1998-1999 by Systems in Motion.  All rights reserved.
 *
 *  This file is part of the Coin library.
 *
 *  This file may be distributed under the terms of the Q Public License
 *  as defined by Troll Tech AS of Norway and appearing in the file
 *  LICENSE.QPL included in the packaging of this file.
 *
 *  If you want to use Coin in applications not covered by licenses
 *  compatible with the QPL, you can contact SIM to aquire a
 *  Professional Edition license for Coin.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

static const char rcsid[] =
  "$Id$";

#include <Inventor/Qt/SoQtCursors.h>

// Graphic data for the mouse pointer bitmaps.

unsigned char so_qt_zoom_bitmap[] =
{
  ~0x0c, ~0x09, 0xff, 0xff, ~0x1e, ~0x00, 0xff, 0xff, ~0x3f, ~0x00, 0xff, 0xff,
  ~0x6d, 0x7f, 0xff, 0xff, 0x33, 0x3f, 0xff, 0xff, ~0x0c, ~0x00, 0xff, 0xff,
  ~0x0c, ~0x00, 0xff, 0xff, ~0x0c, ~0x00, 0xff, 0xff, ~0x0c, ~0x00, 0xff, 0xff,
  0x33, 0x3f, 0xff, 0xff, ~0x6d, 0x7f, 0xff, 0xff, ~0x3f, ~0x00, 0xff, 0xff,
  ~0x1e, ~0x00, 0xff, 0xff, ~0x0c, ~0x00, 0xff, 0xff, ~0x00, ~0x00, 0xff, 0xff,
  ~0x00, ~0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

unsigned char so_qt_zoom_mask_bitmap[] =
{
  0x0f, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00,
  0x7f, 0xe0, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00,
  0xff, 0xf0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0xff, 0xf0, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00,
  0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00,
  0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char so_qt_pan_bitmap[] =
{
  ~0x03, ~0x00, 0xff, 0xff, ~0x07, 0x7f, 0xff, 0xff, ~0x0f, 0x3f, 0xff, 0xff,
  ~0x03, ~0x00, 0xff, 0xff, ~0x23, ~0x10, 0xff, 0xff, ~0x63, ~0x18, 0xff, 0xff,
  0x00, 0x03, 0xff, 0xff, 0x00, 0x03, 0xff, 0xff, ~0x63, ~0x18, 0xff, 0xff,
  ~0x23, ~0x10, 0xff, 0xff, ~0x03, ~0x00, 0xff, 0xff, ~0x0f, 0x3f, 0xff, 0xff,
  ~0x07, 0x7f, 0xff, 0xff, ~0x03, ~0x00, 0xff, 0xff, ~0x00, ~0x00, 0xff, 0xff,
  ~0x00, ~0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

unsigned char so_qt_pan_mask_bitmap[] =
{
  0x03, 0xc0, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00,
  0x0f, 0xf0, 0x00, 0x00, 0x17, 0xe8, 0x00, 0x00, 0x3b, 0xdc, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0x3b, 0xdc, 0x00, 0x00, 0x17, 0xe8, 0x00, 0x00,
  0x0f, 0xf0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00,
  0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char so_qt_rotate_bitmap[] =
{
  0x70, 0x3f, 0xff, 0xff, 0x20, 0x1f, 0xff, 0xff, 0x07, ~0x30, 0xff, 0xff,
  0x0f, ~0x10, 0xff, 0xff, 0x07, ~0x00, 0xff, 0xff, 0x03, ~0x00, 0xff, 0xff,
  ~0x00, ~0x00, 0xff, 0xff, ~0x00, ~0x00, 0xff, 0xff, ~0x00, 0x03, 0xff, 0xff,
  ~0x00, ~0x7c, 0xff, 0xff, ~0x20, ~0x3c, 0xff, 0xff, ~0x30, ~0x7c, 0xff, 0xff,
  ~0x1f, 0x13, 0xff, 0xff, ~0x0f, 0x3b, 0xff, 0xff, ~0x00, ~0x00, 0xff, 0xff,
  ~0x00, ~0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

unsigned char so_qt_rotate_mask_bitmap[] =
{
  0xc7, 0xe0, 0x00, 0x00, 0xef, 0xf0, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00,
  0xff, 0xfc, 0x00, 0x00, 0xfc, 0x1c, 0x00, 0x00, 0xff, 0x0c, 0x00, 0x00,
  0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
  0x00, 0xff, 0x00, 0x00, 0x30, 0x7f, 0x00, 0x00, 0x38, 0x3f, 0x00, 0x00,
  0x3f, 0xff, 0x00, 0x00, 0x1f, 0xff, 0x00, 0x00, 0x0f, 0xf7, 0x00, 0x00,
  0x07, 0xe3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
