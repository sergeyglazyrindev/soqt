/**************************************************************************\
 *
 *  This file is part of the Coin family of 3D visualization libraries.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and / or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use this library in software that is incompatible
 *  with the GNU GPL, and / or you would like to take advantage of the
 *  additional benefits with regard to our support services, please
 *  contact Systems in Motion about acquiring a Coin Professional
 *  Edition License.  See <URL:http://www.coin3d.org> for more
 *  information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

// Class documentation in common/SoGuiGLWidgetCommon.cpp.in.

// *************************************************************************

//  FIXME:
//    - use the add/removeVisibilityCB methods in SoQtComponent?
//    - there's a heap of methods missing from SoXtGLWidget, none (or few)
//      of which seems relevant for the Qt implementation -- check the truth of
//      this statement

// *************************************************************************

// The following define is a work-around for a problem where:
//
//   - The <X11/Xmd.h> file is included from <GL/glx.h> (which is
//   included close to the bottom of this source code file).
//
// and
//
//   - <X11/Xmd.h> typedefs some fixed bitwidth types that crashes
//   with the same defines set up by <qglobal.h> from Qt. The types
//   are INT8 and INT32.
//
// By setting up the QT_CLEAN_NAMESPACE define before including any
// Qt-headers, we make sure that <qglobal.h> won't try to make those
// typedefs, and there will be no clash when <X11/Xmd.h> is included.
//
// The problem was reported to happen for Solaris by Bert Bril.
//
// FIXME: the QT_CLEAN_NAMESPACE define should probably be used
// throughout the compilation of the SoQt library, not just for this
// file. 20021021 mortene.

#define QT_CLEAN_NAMESPACE 1

#include <qevent.h>
#include <qframe.h>

#include <qmetaobject.h>
#include <Inventor/Qt/moc_SoQtGLWidgetP.icc>

#include <Inventor/Qt/SoAny.h>
#include <Inventor/Qt/SoQtBasic.h>
#include <Inventor/Qt/SoQtGLWidget.h>
#include <Inventor/Qt/SoQtGLWidgetP.h>
#include <Inventor/Qt/widgets/SoQtGLArea.h>
#include <Inventor/SbTime.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/misc/SoBasic.h>
#include <soqtdefs.h>
#include <stdlib.h>

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

// Abstract the overlay handling methods on QGLFormat and QGLWidget,
// because they may or may not be there depending on the version of
// the QGL library installed on the developer's system.
//
// (And we'd like to avoid scattering #ifdef's all over the code, so
// we make some new functions.)
//
#if HAVE_QGLFORMAT_SETOVERLAY
static inline bool QGLFormat_hasOverlay(const QGLFormat * f)
{ return f->hasOverlay(); }
static inline void QGLFormat_setOverlay(QGLFormat * f, bool enable)
{ f->setOverlay(enable); }
static inline void QGLFormat_makeOverlayCurrent(QGLWidget * w)
{ w->makeOverlayCurrent(); }
static inline const QGLContext * QGLWidget_overlayContext(QGLWidget * w)
{ return w->overlayContext(); }
static inline const QColor QGLContext_overlayTransparentColor(const QGLContext * c)
{ return c->overlayTransparentColor(); }
#else // !HAVE_QGLFORMAT_SETOVERLAY
static inline bool QGLFormat_hasOverlay(const QGLFormat * f) { return false; }
static inline void QGLFormat_setOverlay(QGLFormat * f, bool enable) { }
static inline void QGLFormat_makeOverlayCurrent(QGLWidget * w) { }
static inline const QGLContext * QGLWidget_overlayContext(QGLWidget * w) { return NULL; }
static inline const QColor QGLContext_overlayTransparentColor(const QGLContext * c) { return QColor(0, 0, 0); }
#endif // !HAVE_QGLFORMAT_SETOVERLAY


// For Qt v2.2.2 at least, Troll Tech forgot to include
// operator==(QGLFormat&,QGLFormat&) in the publicly exported API for
// MSWindows DLLs. This abstracts the workaround into a method
// QGLFormat_eq().
//
#if HAVE_QGLFORMAT_EQ_OP
static inline bool
QGLFormat_eq(const QGLFormat & a, const QGLFormat & b)
{
  return (a == b);
}
#else // !HAVE_QGLFORMAT_EQ_OP
static inline bool
QGLFormat_eq(const QGLFormat & a, const QGLFormat & b)
{
  if (a.doubleBuffer() != b.doubleBuffer()) return false;
  if (a.depth() != b.depth()) return false;
  if (a.rgba() != b.rgba()) return false;
  if (a.stereo() != b.stereo()) return false;
  if (QGLFormat_hasOverlay(&a) != QGLFormat_hasOverlay(&b)) return false;
  return true;
}
#endif // !HAVE_QGLFORMAT_EQ_OP

// *************************************************************************

SOQT_OBJECT_ABSTRACT_SOURCE(SoQtGLWidget);

static const int SO_BORDER_THICKNESS = 2;

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SoQtGLWidget::SoQtGLWidget(QWidget * const parent,
                           const char * const name,
                           const SbBool embed,
                           const int glmodes,
                           const SbBool build)
  : inherited(parent, name, embed),
    waitForExpose(TRUE),
    drawToFrontBuffer(FALSE)
{
  PRIVATE(this) = new SoQtGLWidgetP(this);

  PRIVATE(this)->glSize = SbVec2s(0, 0);
  PRIVATE(this)->wasresized = FALSE;

  PRIVATE(this)->glformat = new QGLFormat;
  PRIVATE(this)->glformat->setDoubleBuffer((glmodes & SO_GL_DOUBLE) ? true : false);
  PRIVATE(this)->glformat->setDepth((glmodes & SO_GL_ZBUFFER) ? true : false);
  PRIVATE(this)->glformat->setRgba((glmodes & SO_GL_RGB) ? true : false);
  PRIVATE(this)->glformat->setStereo((glmodes & SO_GL_STEREO) ? true : false);
  bool enableoverlay = (glmodes & SO_GL_OVERLAY) ? true : false;
  QGLFormat_setOverlay(PRIVATE(this)->glformat, enableoverlay);

  PRIVATE(this)->glparent = NULL;
  PRIVATE(this)->currentglwidget = NULL;
  PRIVATE(this)->previousglwidget = NULL;
  PRIVATE(this)->borderwidget = NULL;

  if (! QGLFormat::hasOpenGL()) {
    SoDebugError::post("SoQtGLWidget::SoQtGLWidget",
                        "OpenGL not available!");
    // FIXME: this is not the way to handle this -- we should call the
    // fatal error handler. 20011112 mortene.
    return;
  }

  if (! build) { return; }

  this->setClassName("SoQtGLWidget");
  QWidget * parentwidget = this->getParentWidget();
  QWidget * widget = this->buildWidget(parentwidget);
  this->setBaseWidget(widget);
}

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SoQtGLWidget::~SoQtGLWidget()
{
  // Don't delete the Qt widgets we've allocated, as they are
  // destructed implicitly when their parent widgets die. (Destructing
  // "our" child widgets in this destructor can in fact lead to
  // crashes due to the widgets being deallocated multiple times.)

  this->unregisterWidget(PRIVATE(this)->borderwidget);

  if (PRIVATE(this)->currentglwidget) {
    SoAny::si()->unregisterGLContext((void *)this);
  }
  
  delete PRIVATE(this)->glformat;
  delete PRIVATE(this);
}

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
QWidget *
SoQtGLWidget::buildWidget(QWidget * parent)
{
  if (parent != NULL && this->isTopLevelShell()) {
    parent->installEventFilter(PRIVATE(this));
  }

  PRIVATE(this)->borderwidget = new QFrame(parent);
  this->registerWidget(PRIVATE(this)->borderwidget);

  PRIVATE(this)->borderwidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
  PRIVATE(this)->borderwidget->setLineWidth(PRIVATE(this)->borderthickness);
  PRIVATE(this)->borderwidget->setMargin(0);
  PRIVATE(this)->borderwidget->move(0, 0);

  // Remember our parent widget so we can use it in tests in the
  // eventFilter().
  PRIVATE(this)->glparent = parent;

  PRIVATE(this)->buildGLWidget();

  return PRIVATE(this)->borderwidget;
}

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::setBorder(const SbBool enable)
{
  PRIVATE(this)->borderthickness = (enable ? SO_BORDER_THICKNESS : 0);
  assert(PRIVATE(this)->borderwidget != NULL);

  PRIVATE(this)->borderwidget->setLineWidth(PRIVATE(this)->borderthickness);

  QSize total(PRIVATE(this)->borderwidget->size());
  QRect r(PRIVATE(this)->borderthickness, // xpos
          PRIVATE(this)->borderthickness, // ypos
          total.width() - 2 * PRIVATE(this)->borderthickness, // width
          total.height() - 2 * PRIVATE(this)->borderthickness); // height
  PRIVATE(this)->currentglwidget->setGeometry(r);
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool
SoQtGLWidget::isBorder(void) const
{
  return PRIVATE(this)->borderthickness ? TRUE : FALSE;
}

// *************************************************************************


// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::setOverlayRender(const SbBool onoff)
{
  SbBool ison = QGLFormat_hasOverlay(PRIVATE(this)->glformat);
  if ((onoff && ison) || (!onoff && !ison)) { return; }

  QGLFormat_setOverlay(PRIVATE(this)->glformat, onoff);

  ison = QGLFormat_hasOverlay(PRIVATE(this)->glformat);
  if (onoff && !ison) {
    SoDebugError::postWarning("SoQtGLWidget::setOverlayRender",
                              "overlay planes not supported");
    return;
  }

  // Rebuild if a GL widget has already been built.
  if (PRIVATE(this)->currentglwidget) PRIVATE(this)->buildGLWidget();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool
SoQtGLWidget::isOverlayRender(void) const
{
  return QGLFormat_hasOverlay(PRIVATE(this)->glformat);
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::setDoubleBuffer(const SbBool enable)
{
  if ((enable && PRIVATE(this)->glformat->doubleBuffer()) ||
       (!enable && !PRIVATE(this)->glformat->doubleBuffer()))
    return;

  PRIVATE(this)->glformat->setDoubleBuffer(enable);
  // Rebuild if a GL widget has already been built.
  if (PRIVATE(this)->currentglwidget) PRIVATE(this)->buildGLWidget();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool
SoQtGLWidget::isDoubleBuffer(void) const
{
  return PRIVATE(this)->glformat->doubleBuffer();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::setQuadBufferStereo(const SbBool enable)
{
  if ((enable && PRIVATE(this)->glformat->stereo()) ||
       (!enable && !PRIVATE(this)->glformat->stereo()))
    return;

  PRIVATE(this)->glformat->setStereo(enable);

  // Rebuild if a GL widget has already been built.
  if (PRIVATE(this)->currentglwidget) PRIVATE(this)->buildGLWidget();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool
SoQtGLWidget::isQuadBufferStereo(void) const
{
  return PRIVATE(this)->glformat->stereo();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void 
SoQtGLWidget::setAccumulationBuffer(const SbBool enable)
{
  if ((enable && PRIVATE(this)->glformat->accum()) ||
       (!enable && !PRIVATE(this)->glformat->accum()))
    return;

  PRIVATE(this)->glformat->setAccum(enable);

  // Rebuild if a GL widget has already been built.
  if (PRIVATE(this)->currentglwidget) PRIVATE(this)->buildGLWidget();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool 
SoQtGLWidget::getAccumulationBuffer(void) const
{
  return (SbBool) (PRIVATE(this)->glformat->accum());
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void 
SoQtGLWidget::setStencilBuffer(const SbBool enable)
{
  if ((enable && PRIVATE(this)->glformat->stencil()) ||
       (!enable && !PRIVATE(this)->glformat->stencil()))
    return;

  PRIVATE(this)->glformat->setStencil(enable);

  // Rebuild if a GL widget has already been built.
  if (PRIVATE(this)->currentglwidget) PRIVATE(this)->buildGLWidget();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool 
SoQtGLWidget::getStencilBuffer(void) const
{
  return (SbBool) (PRIVATE(this)->glformat->stencil());
}


// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::setDrawToFrontBufferEnable(const SbBool enable)
{
  this->drawToFrontBuffer = enable;
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool
SoQtGLWidget::isDrawToFrontBufferEnable(void) const
{
  return this->drawToFrontBuffer;
}

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
QWidget *
SoQtGLWidget::getNormalWidget(void) const
{
  return this->getGLWidget();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
/*!
  Returns widget associated with overlay planes, or \c NULL if no
  overlay planes are available.

  For the Qt toolkit, overlay planes is a feature of the QGLWidget,
  and not seen at a separate widget entity. So this call will just
  return the same widget reference as the SoQt::getGLWidget() call (if
  overlay planes are supported).
*/
QWidget *
SoQtGLWidget::getOverlayWidget(void) const
{
  // Cast away this-pointer constness, as
  // SoQtGLWidget::getOverlayContext() is not defined as const.
  SoQtGLWidget * that = (SoQtGLWidget *)this;

  return PRIVATE(that)->getOverlayContext() ? this->getGLWidget() : NULL;
}

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::setGLSize(const SbVec2s size)
{
  if (size == PRIVATE(this)->glSize) return;
#if SOQT_DEBUG && 0 // debug
  SoDebugError::postInfo("SoQtGLWidget::setGLSize",
                         "[invoked (%d, %d)]", size[0], size[1]);
#endif // debug
  PRIVATE(this)->glSize = size;
  if (PRIVATE(this)->currentglwidget) {
    int frame = this->isBorder() ? PRIVATE(this)->borderthickness : 0;
    PRIVATE(this)->currentglwidget->setGeometry(QRect(frame, frame, PRIVATE(this)->glSize[0], PRIVATE(this)->glSize[1]));
  }

// Due to an internal hack in Qt/Mac 3.1.x, sometimes the OpenGL context
// is destroyed and re-created (see QGLWidget::macInternalRecreateContext()). 
// In this case, we must register a new context as well in order to get
// a new SoGLRenderAction cache context id.
#if (defined Q_WS_MAC && QT_VERSION >= 0x030100) 
  QGLWidget * w = (QGLWidget*) this->getGLWidget();
  if (w && PRIVATE(this)->oldcontext != w->context()) { 
    if (SOQT_DEBUG && 0) { 
      SoDebugError::postInfo("SoQtGLWidget::setGLSize", 
                             "OpenGL context recreated by Qt, "
                             "registering new context.");
    }
    SoAny::si()->unregisterGLContext((void *)this);
    SoAny::si()->registerGLContext((void *)this, 0, 0);  
  }
  if (w) PRIVATE(this)->oldcontext = w->context();
#endif

}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbVec2s
SoQtGLWidget::getGLSize(void) const
{
  return PRIVATE(this)->glSize;
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
float
SoQtGLWidget::getGLAspectRatio(void) const
{
  return float(PRIVATE(this)->currentglwidget->width()) /
    float(PRIVATE(this)->currentglwidget->height());
}

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
QWidget *
SoQtGLWidget::getGLWidget(void) const
{
  return PRIVATE(this)->currentglwidget;
}


// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::widgetChanged(QWidget * w)
{
  // virtual
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::processEvent(QEvent * anyevent)
{
  // Nothing is done here for the SoQtGLWidget, as realize, resize and
  // expose events are caught by explicitly attaching signal callbacks
  // to the widget.
}

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::glLockNormal(void)
{
  assert(PRIVATE(this)->currentglwidget != NULL);
  ((SoQtGLArea *)PRIVATE(this)->currentglwidget)->makeCurrent();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::glUnlockNormal(void)
{
  // does nothing under Qt. Under BeOS the buffer needs to be unlocked
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::glSwapBuffers(void)
{
  assert(PRIVATE(this)->currentglwidget != NULL);
  ((SoQtGLArea *)PRIVATE(this)->currentglwidget)->swapBuffers();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void 
SoQtGLWidget::glFlushBuffer(void)
{
  // might be called for both normal and overlay widgets
  glFlush();

#ifdef Q_WS_MAC
  // Qt/Mac double-buffers everything internally to circumvent some of
  // AGLs limitations. Since we don't use their "recommended"
  // updateGL() way of rendering, we have to explicitly swap the
  // buffers.
  ((QGLWidget *)this->getGLWidget())->swapBuffers();
  // FIXME: this should only be a temporary hack until Apple fixes
  // AGL...  kyrah 20011129
  //
  // UPDATE: or until we pick up and integrate into the SoQt library
  // the pieces of sourcecode from QGLWidget et al that depend on. As
  // Qt Professional Edition License holders on all platforms, this is
  // something we seem to be allowed to do.  20011129 mortene.
  //

  // For reference, this is what Sam Magnusen from TrollTech says why
  // they need the ugly bitblt hack on the Mac:
  //
  // �All widgets in Qt/Mac are really just an area that you are
  // "allowed to paint on", and then when you do Qt/Mac will figure
  // out what that area is, and only paint on it. Most of this magic
  // is in QWidget/QPainter - however they can't handle the QGLWidget
  // case as when QWidget decides what that region is, QPainter can't
  // apply it. It could be done in QGLWidget (if AGL offered complex
  // region clipping, however they only allow for a rectangle. This is
  // not good enough if you had widgets overlapping the opengl widget,
  // if you are sure you don't, you can turn off the
  // QMAC_OPENGL_DOUBLEBUFFER ifdef (in qgl.h) and it will not use the
  // technique, and I think you will get acceleration as well.�

  // UPDATE: testing shows that this works in window mode, but that it
  // fails miserably in full screen mode (with lots of Core Graphics
  // Services errors). 20011201 kyrah (writeup by mortene).

#endif // Q_WS_MAC
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::glLockOverlay(void)
{
  assert(PRIVATE(this)->currentglwidget != NULL);
  QGLFormat_makeOverlayCurrent((SoQtGLArea *)PRIVATE(this)->currentglwidget);
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void
SoQtGLWidget::glUnlockOverlay(void)
{
  // does nothing under Qt. Under BeOS the buffer needs to be unlocked  
}

// *************************************************************************

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
unsigned long 
SoQtGLWidget::getOverlayTransparentPixel(void)
{
  const QGLContext * ctx = PRIVATE(this)->getOverlayContext();
  if (ctx) {
    QColor color = QGLContext_overlayTransparentColor(ctx);
    return color.pixel();
  }
  return 0;
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool 
SoQtGLWidget::isRGBMode(void)
{
  return (SbBool) PRIVATE(this)->glformat->rgba();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void 
SoQtGLWidget::redrawOverlay(void)
{
  // should be empty. It's up subclasses to do some work here
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void 
SoQtGLWidget::initGraphic(void)
{
  this->glLockNormal();
  // Need to set this explicitly when running on top of Open Inventor,
  // as it seems to have been forgotten there.
  // This code should be invoked from SoQtRenderArea::initGraphics()
  glEnable(GL_DEPTH_TEST);
  this->glUnlockNormal();
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
void 
SoQtGLWidget::initOverlayGraphic(void)
{
  // FIXME: can't see this function called from anywhere within SoQt.
  // That seems bogus. 20010831 mortene.
}

// doc in super
void
SoQtGLWidget::sizeChanged(const SbVec2s & size)
{
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool 
SoQtGLWidget::glScheduleRedraw(void)
{
  return FALSE;
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool 
SoQtGLWidget::hasOverlayGLArea(void) const 
{
  return this->getOverlayWidget() != NULL;
}

// Documented in common/SoGuiGLWidgetCommon.cpp.in.
SbBool 
SoQtGLWidget::hasNormalGLArea(void) const 
{
  return this->getNormalWidget() != NULL;
}

// *************************************************************************

#ifndef DOXYGEN_SKIP_THIS

// The private data and code for the SoQtGLWidget.

SoQtGLWidgetP::SoQtGLWidgetP(SoQtGLWidget * o)
  : SoGuiGLWidgetP(o)
{
  this->borderthickness = 0;
  this->oldcontext = NULL;
}

SoQtGLWidgetP::~SoQtGLWidgetP()
{
}

// Gets called by the SoQtGLArea instance upon keyboard presses. These
// are then forwarded to subclasses for handling.
void
SoQtGLWidgetP::GLAreaKeyEvent(QKeyEvent * e, void * userdata)
{
  SoQtGLWidget * that = (SoQtGLWidget *)userdata;
  that->processEvent(e);
}

// slot invoked upon QGLWidget initialization
void
SoQtGLWidgetP::gl_init(void)
{
  if (SOQT_DEBUG && 0) { // debug
    SoDebugError::postInfo("gl_init", "invoked");
  }

  PUBLIC(this)->initGraphic();
}

// slot invoked upon QGLWidget resizes
void
SoQtGLWidgetP::gl_reshape(int width, int height)
{
  if (SOQT_DEBUG && 0) { // debug
    SoDebugError::postInfo("gl_reshape", "<%d, %d>", width, height);
  }

  this->glSize = SbVec2s((short) width, (short) height);
  this->wasresized = TRUE;
}

// slot invoked upon QGLWidget expose events
void
SoQtGLWidgetP::gl_exposed(void)
{
  if (SOQT_DEBUG && 0) { // debug
    SoDebugError::postInfo("gl_exposed", "%f", SbTime::getTimeOfDay().getValue());
  }

  if (PUBLIC(this)->waitForExpose) {
    PUBLIC(this)->waitForExpose = FALSE; // Gets flipped from TRUE on first expose.
#if 0 // tmp disabled
    // The Qt library uses QApplication::sendPostedEvents() for
    // passing out various delayed events upon show(), among them a
    // bunch of bl**dy resize events which will overload any size
    // settings done before we show the SoQt component widgets.

    // FIXME: should probably be an afterRealizeHook() fix. 20001125 mortene.

    PUBLIC(this)->setSize(PUBLIC(this)->getSize());
#endif // tmp disabled
  }
  if (this->wasresized) {
    PUBLIC(this)->sizeChanged(this->glSize);
    this->wasresized = FALSE;
  }

  if (!PUBLIC(this)->glScheduleRedraw()) {
    PUBLIC(this)->redraw();
  }
}


static const char eventnaming[][50] = {
  "None", // 0
  "Timer",
  "MouseButtonPress",
  "MouseButtonRelease",
  "MouseButtonDblClick",
  "MouseMove",
  "KeyPress",
  "KeyRelease",
  "FocusIn",
  "FocusOut",
  "Enter",
  "Leave",
  "Paint",
  "Move",
  "Resize",
  "Create",
  "Destroy",
  "Show",
  "Hide",
  "Close",
  "Quit", // 20
  "*unknown*", "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "Accel", // 30
  "Wheel",
  "AccelAvailable", // 32
  "CaptionChange",
  "*unknown*", "*unknown*", "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "Clipboard", // 40
  "*unknown*", "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "SockAct", // 50
  "AccelOverride", "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "DragEnter", // 60
  "DragMove",
  "DragLeave",
  "Drop",
  "DragResponse", // 64
  "*unknown*", "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "ChildInserted", // 70
  "ChildRemoved",
  "LayoutHint", // 72
  "*unknown*", "*unknown*", "*unknown*", "*unknown*", "*unknown*",
  "*unknown*", "*unknown*",
  "ActivateControl", // 80
  "DeactivateControl"
};

bool
SoQtGLWidgetP::eventFilter(QObject * obj, QEvent * e)
{
  if (SOQT_DEBUG && 0) { // debug
    SbString w = obj->name();
    SbBool istoplevel = obj == this->currentglwidget->topLevelWidget();

    if (obj == this->glparent) { w = "glparent"; }
    else if (obj == this->currentglwidget) { w = "currentglwidget"; }
    else if (obj == this->borderwidget) { w = "borderwidget"; }
    else if (istoplevel) { w = "top-level"; }

    SoDebugError::postInfo("SoQtGLWidgetP::eventFilter",
                           "[invoked] QEvent==%p obj==%p==\"%s\"==%s (%s) %s (typecode==%d)",
                           e, obj, w.getString(), obj->className(),
                           istoplevel ? "TOPLEVEL" : "",
                           eventnaming[e->type()], e->type());
  }

#if QT_VERSION >= 200
  // Qt 2 introduced "accelerator" type keyboard events, which should
  // simply be ignored (all keyboard events are first attempted passed
  // by the Qt event engine as accelerator events, before they are
  // re-sent as "ordinary" keyboard events).
  if (e->type() == QEvent::Accel || e->type() == QEvent::AccelAvailable) {
    ((QKeyEvent *)e)->ignore();
    // It might not matter whether we return TRUE or FALSE here, but
    // it seems more natural to return FALSE according to the
    // semantics of the eventFilter() method (FALSE means Qt should
    // re-dispatch "normally").
    return FALSE;
  }
#endif // Qt v2.0

  // FIXME: Under Qt 3.0.0 we got buggy mouse event handling, since
  // mouse events were routed to the gl widget, even if it was
  // supposed to go somewhere else. I'm not sure if this is the
  // correct fix though. pederb, 2001-10-16
  if ((e->type() == QEvent::MouseButtonPress ||
       e->type() == QEvent::MouseButtonRelease ||
       e->type() == QEvent::MouseButtonDblClick ||
       e->type() == QEvent::MouseMove) &&
      (obj != this->currentglwidget)) return FALSE;

  SbBool keyboardevent =
    (e->type() == QEvent::KeyPress) || (e->type() == QEvent::KeyRelease);
  if (keyboardevent) {
    // Ignore keyboard-events, as they are caught directly by the
    // SoQtGLArea widget and forwarded through the
    // SoQtGLWidgetP::GLAreaKeyEvent() callback.
    return FALSE;
  }

  if (obj == (QObject *) this->glparent) {
    // If this hits, the this->glparent QWidget is a toplevelshell, so
    // we resize the GL widget along with it.
    if (e->type() == QEvent::Resize) {
      QResizeEvent * r = (QResizeEvent *)e;
      if (SOQT_DEBUG && 0) {  // debug
        SoDebugError::postInfo("SoQtGLWidgetP::eventFilter",
                               "resize parent %p: (%d, %d)",
                               this->glparent,
                               r->size().width(), r->size().height());
      }

      this->borderwidget->resize(r->size());
//      int newwidth = r->size().width() - 2 * this->borderthickness;
//      int newheight = r->size().height() - 2 * this->borderthickness;

//      PUBLIC(this)->glwidget->setGeometry(this->borderthickness,
//                                   this->borderthickness,
//                                   newwidth - 1, newheight -1);
//      QRect glarea = this->borderwidget->contentsRect();
//      glarea =
//      PUBLIC(this)->glwidget->setGeometry(this->borderwidget->contentsRect());

/*
      int newwidth = r->size().width();
      int newheight = r->size().height();
      PUBLIC(this)->sizeChanged(SbVec2s(newwidth - 1, newheight - 1));
*/
#if 0 // debug
      SoDebugError::postInfo("SoQtGLWidgetP::eventFilter", "resize done");
#endif // debug
    }
  }
  else if (obj == (QObject *) this->currentglwidget) {
    // We used to return ``true'' here if the event was a
    // QResizeEvent. The reason why we'd want to stop resize events
    // being passed on to the native Qt handler was not commented, and
    // I can't see why this should be necessary. Also, kyrah found out
    // that it causes a nasty problem with Qt/Mac, so it has been
    // removed.  <mortene@sim.no>.
  }

  // OBSOLETED: SoQtGLWidget does no longer inherit QObject
  // (indirectly through SoQtObject). I'm not sure whether or not
  // leaving this out causes any trouble..  If no problems are
  // detected, remove eventually. 20020613 mortene.
#if 0
  else {
    // Handle in superclass.
    bool stop = PUBLIC(this)->eventFilter(obj, e);
    if (stop) { return TRUE; }
  }
#endif // OBSOLETED

  PUBLIC(this)->processEvent(e);
  return FALSE;
}

// Registered callback on device events, set up by the
// So@Gui@RenderArea.
void
SoQtGLWidgetP::eventHandler(QWidget * widget, void * closure, QEvent * event,
                            bool *)
{
  assert(closure != NULL);
  SoQtGLWidget * component = (SoQtGLWidget *) closure;
  component->processEvent(event);
}

// The GL widget rebuilding has been written to remember the previous
// GL widget, which might be swapped back in if it fits the wanted
// format.
//
// There are two reasons for keeping track of both the current and the
// previous widget:
//
//  1) efficiency; often one swaps back and forth between only two
//  different visuals -- like single and double buffering, or stereo
//  mode on and off
//
//  2) robustness; killing off the previous widget in the build-method
//  below has nasty sideeffects (like "random" coredumps), since the
//  Qt event loop might be using it
void
SoQtGLWidgetP::buildGLWidget(void)
{
  // FIXME: use SoQtComponent registerWidget() and unregisterWidget()
  // as appropriate when setting up new or deleting old
  // GL-widgets. 20020612 mortene.

  if (SOQT_DEBUG && 0) { // debug
    SoDebugError::postInfo("SoQtGLWidgetP::buildGLWidget",
                           "%s, %s, %s, %s, %s",
                           this->glformat->doubleBuffer() ? "double" : "single",
                           this->glformat->depth() ? "z-buffer" : "no z-buffer",
                           this->glformat->rgba() ? "RGBA" : "colorindex",
                           this->glformat->stereo() ? "stereo" : "mono",
                           QGLFormat_hasOverlay(this->glformat) ? "overlay" : "no overlay");
  }

  SoQtGLArea * wascurrent = this->currentglwidget;
  SoQtGLArea * wasprevious = this->previousglwidget;

  void * display = NULL;
  void * screen = NULL;

#if defined(Q_WS_X11) // Qt defines this under X11
  // FIXME: should make context sharing work for other Qt
  // base-platforms (MSWin, MacOS X) aswell. 20020118 mortene.

  // the following Qt methods are only available under X11
  display = (void*) QPaintDevice::x11AppDisplay();
  screen = (void*) ((unsigned int) QPaintDevice::x11AppScreen());
#endif // Q_WS_X11

  if (wascurrent) {
    // Do _not_ turn off mousetracking or remove the eventfilter, as
    // we'd loose events after the switch has happened if the user is
    // already interacting with the canvas (e.g. when starting a drag
    // in BUFFER_INTERACTIVE mode).
#if 0 // Keep this code around so we don't accidentally reinsert it. :^}
    wascurrent->removeEventFilter(this);
    wascurrent->setMouseTracking(FALSE);
#endif // Permanently disabled.
    QObject::disconnect(wascurrent, SIGNAL(expose_sig()), this, SLOT(gl_exposed()));
    QObject::disconnect(wascurrent, SIGNAL(init_sig()), this, SLOT(gl_init()));
    //    QObject::disconnect(wascurrent, SIGNAL(reshape_sig()), this, SLOT(gl_reshape()));
    this->previousglwidget = wascurrent;
  }

  if (wasprevious && QGLFormat_eq(*this->glformat, wasprevious->format())) {
    // Reenable the previous widget.
    if (this->currentglwidget) SoAny::si()->unregisterGLContext((void *)PUBLIC(this));
    this->currentglwidget = wasprevious;
    SoAny::si()->registerGLContext((void *)PUBLIC(this), display, screen);
    if (SOQT_DEBUG && 0) { // debug
      SoDebugError::postInfo("SoQtGLWidgetP::buildGLWidget",
                             "reused previously used GL widget");
    }
  }
  else {
    // Couldn't use the previous widget, make a new one.
    SoQtGLWidget * sharewidget = (SoQtGLWidget*) SoAny::si()->getSharedGLContext(display, screen);
    if (this->currentglwidget) SoAny::si()->unregisterGLContext((void *)PUBLIC(this));
    this->currentglwidget =
      new SoQtGLArea(this->glformat, this->borderwidget,
                     sharewidget ? (const QGLWidget*) sharewidget->getGLWidget() : NULL);
    this->currentglwidget->registerQKeyEventHandler(SoQtGLWidgetP::GLAreaKeyEvent, PUBLIC(this));
    SoAny::si()->registerGLContext((void *)PUBLIC(this), display, screen);
    // Send this one to the final hunting grounds.    
    delete wasprevious;
  }

  if (!this->currentglwidget->isValid()) {
    SbString s =
      "Can't set up a valid OpenGL canvas, "
      "something is seriously wrong with your system!";
    SbBool handled =
      SoAny::si()->invokeFatalErrorHandler(s, SoQt::NO_OPENGL_CANVAS);
    if (handled) { return; }
    exit(1);
  }

  if (SOQT_DEBUG) { // Warn about requested features that we didn't get.
    QGLFormat * w = this->glformat; // w(anted)
    QGLFormat g = this->currentglwidget->format(); // g(ot)

#define GLWIDGET_FEATURECMP(_glformatfunc_, _truestr_, _falsestr_) \
  do { \
    if (w->_glformatfunc_() != g._glformatfunc_()) { \
      SoDebugError::postWarning("SoQtGLWidgetP::buildGLWidget", \
                                "wanted %s, but that is not supported " \
                                "by the OpenGL driver", \
                                w->_glformatfunc_() ? _truestr_ : _falsestr_); \
    } \
  } while (0)

    GLWIDGET_FEATURECMP(doubleBuffer, "doublebuffer visual", "singlebuffer visual");
    GLWIDGET_FEATURECMP(depth, "visual with depthbuffer", "visual without depthbuffer");
    GLWIDGET_FEATURECMP(rgba, "RGBA buffer", "colorindex buffer");
    GLWIDGET_FEATURECMP(stereo, "stereo buffers", "mono buffer");

    if (QGLFormat_hasOverlay(w) != QGLFormat_hasOverlay(&g)) {
      SoDebugError::postWarning("SoQtGLWidgetP::buildGLWidget",
                                "wanted %s, but that is not supported "
                                "by the OpenGL driver",
                                QGLFormat_hasOverlay(w) ?
                                "overlay plane(s)" :
                                "visual without overlay plane(s)");
    }
  }
#undef GLWIDGET_FEATURECMP

  *this->glformat = this->currentglwidget->format();

  int frame = PUBLIC(this)->isBorder() ? this->borderthickness : 0;
  this->currentglwidget->setGeometry(frame, frame,
                                     this->glSize[0] - 2*frame,
                                     this->glSize[1] - 2*frame);

  QObject::connect(this->currentglwidget, SIGNAL(init_sig()),
                   this, SLOT(gl_init()));
  //  QObject::connect(this->currentglwidget, SIGNAL(reshape_sig(int, int)),
  //                    this, SLOT(gl_reshape(int, int)));
  QObject::connect(this->currentglwidget, SIGNAL(expose_sig()),
                   this, SLOT(gl_exposed()));

  this->currentglwidget->setMouseTracking(TRUE);
  this->currentglwidget->installEventFilter(this);

  // Reset to avoid unnecessary scenegraph redraws.
  PUBLIC(this)->waitForExpose = TRUE;

  // We've changed to a new widget, so notify subclasses through this
  // virtual method.
  PUBLIC(this)->widgetChanged(this->currentglwidget);

  if (wascurrent) {
    // If we are rebuilding, we need to explicitly call show() here,
    // as no message to show will be given from an already visible
    // parent. (If the glwidget was built but not shown before the
    // rebuild, the call below doesn't do any harm, as the glwidget
    // still won't become visible until all parents are visible.)
    this->currentglwidget->show();
    this->currentglwidget->raise();
  }
  this->currentglwidget->setFocus();
}

// Returns the normal GL context.
const QGLContext * 
SoQtGLWidgetP::getNormalContext(void)
{
  QGLWidget * w = (QGLWidget*) PUBLIC(this)->getGLWidget();
  if (w) return w->context();
  return NULL;
}

// Returns the overlay GL context.
const QGLContext * 
SoQtGLWidgetP::getOverlayContext(void)
{
  QGLWidget * w = (QGLWidget*) PUBLIC(this)->getGLWidget();
  if (w) { return QGLWidget_overlayContext(w); }
  return NULL;
}

#if HAVE_GLX
// There is something in this header file that fools the g++ 2.95.4
// compiler to run into problems with some of the code in
// SoQtGLWidgetP::eventFilter() (it gives a completely non-sensical
// error message), so we just include it at the bottom like this.
#include <GL/glx.h> // For glXIsDirect().
#endif // HAVE_GLX

// Return a flag indicating whether or not OpenGL rendering is
// happening directly from the CPU(s) to the GPU(s), ie on a local
// display. With GLX on X11, it is possible to do remote rendering.
SbBool
SoQtGLWidgetP::isDirectRendering(void)
{
#if defined(Q_WS_X11)
  PUBLIC(this)->glLockNormal();
  GLXContext ctx = glXGetCurrentContext();
  if (!ctx) {
    SoDebugError::postWarning("SoQtGLWidgetP::isDirectRendering",
                              "Could not get hold of current context.");
    return TRUE;
  }
  Bool isdirect = glXIsDirect(qt_xdisplay(), ctx);
  PUBLIC(this)->glUnlockNormal();
  return isdirect ? TRUE : FALSE;
#else // ! X11
  return TRUE; // Neither MSWindows nor Mac OS X is capable of remote display.
#endif // ! X11
}

#endif // DOXYGEN_SKIP_THIS

// *************************************************************************
