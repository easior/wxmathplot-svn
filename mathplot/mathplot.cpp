/////////////////////////////////////////////////////////////////////////////
// Name:        mathplot.cpp
// Purpose:     Framework for mathematical graph plotting in wxWindows
// Author:      David Schalig
// Modified by:
// Created:     21/07/2003
// Copyright:   (c) David Schalig
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
#pragma implementation "plot.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/object.h"
#include "wx/font.h"
#include "wx/colour.h"
#include "wx/settings.h"
#include "wx/sizer.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/dcclient.h"
#endif

#include "mathplot.h"
#include "wx/bmpbuttn.h"
#include "wx/module.h"

#include <math.h>

// ----------------------------------------------------------------------------
// XPMs
// ----------------------------------------------------------------------------

//#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "plot_enl.xpm"
    #include "plot_shr.xpm"
    #include "plot_zin.xpm"
    #include "plot_zot.xpm"
    #include "plot_up.xpm"
    #include "plot_dwn.xpm"
//#endif

//-----------------------------------------------------------------------------
// mpLayer
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(mpLayer, wxObject)

mpLayer::mpLayer()
{
    SetPen( *wxBLACK_PEN);
    SetFont(*wxNORMAL_FONT);
}

//-----------------------------------------------------------------------------
// mpLayer implementations - functions
//-----------------------------------------------------------------------------

IMPLEMENT_CLASS(mpFX, mpLayer)

mpFX::mpFX(wxString name, int flags)
{ 
    SetName(name);
    m_flags = flags; 
}

void mpFX::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);

    for (wxCoord i = -(w.GetScrX()>>1); i < (w.GetScrX()>>1); ++i)
    {
        dc.DrawPoint(i, (wxCoord) ((w.GetPosY() - GetY( (double)i / w.GetScaleX() + w.GetPosX()) ) * w.GetScaleY()));
    }

    if (!m_name.IsEmpty())
    {
        dc.SetFont(m_font);

        wxCoord tx, ty;
        dc.GetTextExtent(m_name, &tx, &ty);

        if (m_flags & mpALIGN_RIGHT)
            tx = (w.GetScrX()>>1) - tx - 8;
        else if (m_flags & mpALIGN_CENTER)
            tx = -tx/2;
        else
            tx = -(w.GetScrX()>>1) + 8;

        dc.DrawText( m_name, tx, (wxCoord) ((w.GetPosY() - GetY( (double)tx / w.GetScaleX() + w.GetPosX())) * w.GetScaleY()) );
    }
}

IMPLEMENT_CLASS(mpFY, mpLayer)

mpFY::mpFY(wxString name, int flags)
{ 
    SetName(name);
    m_flags = flags; 
}

void mpFY::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);

    for (wxCoord i = -(w.GetScrY()>>1); i < (w.GetScrY()>>1); ++i)
    {
        dc.DrawPoint((wxCoord) ((GetX( (double)i / w.GetScaleY() + w.GetPosY()) - w.GetPosX()) * w.GetScaleX()), -i);
    }

    if (!m_name.IsEmpty())
    {
        dc.SetFont(m_font);

        wxCoord tx, ty;
        dc.GetTextExtent(m_name, &tx, &ty);

        if (m_flags & mpALIGN_TOP)
            ty = (w.GetScrY()>>1) - 8;
        else if (m_flags & mpALIGN_CENTER)
            ty = 16 - ty/2;
        else
            ty = -(w.GetScrY()>>1) + 8;

        dc.DrawText( m_name, (wxCoord) ((GetX( (double)i / w.GetScaleY() + w.GetPosY()) - w.GetPosX()) * w.GetScaleX()), -ty);
    }
}

IMPLEMENT_CLASS(mpFXY, mpLayer)

mpFXY::mpFXY(wxString name, int flags)
{ 
    SetName(name);
    m_flags = flags; 
}

void mpFXY::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);

    const int numsamples = GetNumSamples();
    double x, y;
    for (int i = 0; i <= numsamples; ++i)
    {
        GetXY(i, x, y);
        dc.DrawPoint( (wxCoord) ((x - w.GetPosX()) * w.GetScaleX()) ,
                      (wxCoord) ((w.GetPosY() - y) * w.GetScaleY()) );
    }
}

//-----------------------------------------------------------------------------
// mpLayer implementations - furniture (scales, ...)
//-----------------------------------------------------------------------------

#define mpLN10 2.3025850929940456840179914546844

IMPLEMENT_CLASS(mpScaleX, mpLayer)

mpScaleX::mpScaleX(wxString name)
{ 
    SetName(name);
    SetFont(*wxSMALL_FONT);
    SetPen(*wxGREY_PEN);
}

void mpScaleX::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);
    dc.SetFont( m_font);

    const int orgy   = (int)(w.GetPosY() * w.GetScaleY());
    const int extend = w.GetScrX()/2;

    dc.DrawLine( -extend, orgy, extend, orgy);

    const double dig  = floor( log( 128.0 / w.GetScaleX() ) / mpLN10 );
    const double step = exp( mpLN10 * dig);
    const double end  = w.GetPosX() + (double)extend / w.GetScaleX();

    wxCoord tx, ty;
    wxString s;
    wxString fmt;
    int tmp = (int)dig;
    if (tmp>=1)
    {
        fmt = wxT("%.f");
    }
    else
    {
        tmp=4-tmp;
        fmt.Printf(wxT("%%.%df"), tmp >= -1 ? 2 : -tmp);
    }

    double n = floor( (w.GetPosX() - (double)extend / w.GetScaleX()) / step ) * step ;

    tmp=-65535;
    for (;n < end; n += step)
    {
        const int p = (n - w.GetPosX()) * w.GetScaleX();
        dc.DrawLine( p, orgy, p, orgy+4);

        s.Printf(fmt, n);
        dc.GetTextExtent(s, &tx, &ty);
        if ((p-tx/2-tmp) > 64)
        {
            dc.DrawText( s, p-tx/2, orgy+4);
            tmp=p+tx/2;
        }
    }
    
    dc.GetTextExtent(m_name, &tx, &ty);
    dc.DrawText( m_name, extend - tx - 4, orgy + 4 + ty);
}

IMPLEMENT_CLASS(mpScaleY, mpLayer)

mpScaleY::mpScaleY(wxString name)
{ 
    SetName(name);
    SetFont(*wxSMALL_FONT);
    SetPen(*wxGREY_PEN);
}

void mpScaleY::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);
    dc.SetFont( m_font);

    const int orgx   = -(int)(w.GetPosX() * w.GetScaleX());
    const int extend = w.GetScrY()/2;

    dc.DrawLine( orgx, -extend, orgx, extend);

    const double dig  = floor( log( 128.0 / w.GetScaleY() ) / mpLN10 );
    const double step = exp( mpLN10 * dig);
    const double end  = w.GetPosY() + (double)extend / w.GetScaleY();

    wxCoord tx, ty;
    wxString s;
    wxString fmt;
    int tmp = (int)dig;
    if (tmp>=1)
    {
        fmt = wxT("%.f");
    }
    else
    {
        tmp=4-tmp;
        fmt.Printf(wxT("%%.%df"), tmp >= -1 ? 2 : -tmp);
    }

    double n = floor( (w.GetPosY() - (double)extend / w.GetScaleY()) / step ) * step ;

    tmp=65536;
    for (;n < end; n += step)
    {
        const int p = (w.GetPosY() - n) * w.GetScaleY();
        dc.DrawLine( orgx, p, orgx+4, p);

        s.Printf(fmt, n);
        dc.GetTextExtent(s, &tx, &ty);
        if ((tmp-p+ty/2) > 32)
        {
            dc.DrawText( s, orgx+4, p-ty/2);
            tmp=p-ty/2;
        }
    }

    dc.GetTextExtent(m_name, &tx, &ty);
    dc.DrawText( m_name, orgx-tx-4, -extend + ty + 4);
}

//-----------------------------------------------------------------------------
// mpWindow
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mpWindow, wxScrolledWindow)

BEGIN_EVENT_TABLE(mpWindow, wxScrolledWindow)
    EVT_PAINT    ( mpWindow::OnPaint)
    EVT_SIZE     ( mpWindow::OnSize)
    EVT_SCROLLWIN( mpWindow::OnScroll2)

    EVT_MIDDLE_UP( mpWindow::OnShowPopupMenu)
    EVT_RIGHT_UP ( mpWindow::OnShowPopupMenu)
    EVT_MENU( mpID_FIT,     mpWindow::OnFit)
    EVT_MENU( mpID_ZOOM_IN, mpWindow::OnZoomIn)
    EVT_MENU( mpID_ZOOM_OUT,mpWindow::OnZoomOut)
END_EVENT_TABLE()

mpWindow::mpWindow( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int flag )
    : wxScrolledWindow( parent, id, pos, size, flag, wxT("mathplot") )
{
    m_scaleX = m_scaleY = 1.0;
    m_posX   = m_posY   = 0;
    m_scrX   = m_scrX   = 64;
    m_minX   = m_minY   = 0;
    m_maxX   = m_maxY   = 0;

    m_popmenu.Append( mpID_FIT,      _("Fit"),      _("Set plot view to show all items"));
    m_popmenu.Append( mpID_ZOOM_IN,  _("Zoom in"),  _("Zoom in plot view."));
    m_popmenu.Append( mpID_ZOOM_OUT, _("Zoom out"), _("Zoom out plot view."));

    m_layers.DeleteContents(TRUE);
    SetBackgroundColour( *wxWHITE );
    EnableScrolling(FALSE, FALSE);

    UpdateAll();
}

mpWindow::~mpWindow()
{
}

void mpWindow::Fit()
{
    if (UpdateBBox())
    {
        int cx, cy;
        GetClientSize( &cx, &cy);

        double d;
        d = m_maxX - m_minX;
        if (d!=0)
        {
            m_scaleX = cx/d;
            m_posX = m_minX + d/2;
        }
        d = m_maxY - m_minY;
        if (d!=0)
        {
            m_scaleY = cy/d;
            m_posY = m_minY + d/2;
        }

        UpdateAll();
    }
}

void mpWindow::ZoomIn()
{
    m_scaleX = m_scaleX * 2;
    m_scaleY = m_scaleY * 2;
    UpdateAll();
}

void mpWindow::ZoomOut()
{
    m_scaleX = m_scaleX / 2;
    m_scaleY = m_scaleY / 2;
    UpdateAll();
}

void mpWindow::OnShowPopupMenu(wxMouseEvent &event)
{
    PopupMenu( &m_popmenu, event.GetX(), event.GetY());
}

void mpWindow::OnFit(wxCommandEvent &event)
{
    Fit();
}

void mpWindow::OnZoomIn(wxCommandEvent &event)
{
    ZoomIn();
}

void mpWindow::OnZoomOut(wxCommandEvent &event)
{
    ZoomOut();
}

void mpWindow::OnSize( wxSizeEvent &event )
{
    UpdateAll();
}

void mpWindow::AddLayer( mpLayer* layer)
{
    m_layers.Append( layer);
    UpdateAll();
}

void mpWindow::DelLayer( mpLayer* layer)
{
    m_layers.DeleteObject( layer);
    UpdateAll();
}

void mpWindow::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc(this);
    dc.BeginDrawing();

    dc.GetSize(&m_scrX, &m_scrY);
    dc.SetDeviceOrigin( m_scrX>>1, m_scrY>>1);

    wxNode *node = m_layers.GetFirst();
    while (node)
    {
        ((mpLayer*)node->GetData())->Plot( dc, *this);
        node = node->GetNext();
    }

    dc.EndDrawing();
}

void mpWindow::OnScroll2(wxScrollWinEvent &event)
{
    int width, height;
    GetClientSize( &width, &height);
    int px, py;
    GetViewStart( &px, &py);

    if (event.GetOrientation() == wxHORIZONTAL)
    {
        SetPosX( (double)px / GetScaleX() + m_minX + (double)(width>>1)/GetScaleX());
        wxLogMessage(wxT("X %d -> %f"), px, GetPosX());
    }
    else
    {
        SetPosY( m_maxY - (double)py / GetScaleY() - (double)(height>>1)/GetScaleY());
    }
    event.Skip();
}

bool mpWindow::UpdateBBox()
{
    wxNode *node = m_layers.GetFirst();

    if (!node)  return FALSE;

    mpLayer* f = (mpLayer*)node->GetData();

    m_minX = f->GetMinX(); m_maxX=f->GetMaxX();
    m_minY = f->GetMinY(); m_maxY=f->GetMaxY();

    for(;;)
    {
        node = node->GetNext(); if (node==NULL) break;
        f = (mpLayer*)node->GetData();
        if (f->GetMinX()<m_minX) m_minX=f->GetMinX(); if (f->GetMaxX()>m_maxX) m_maxX=f->GetMaxX();
        if (f->GetMinY()<m_minY) m_minY=f->GetMinY(); if (f->GetMaxY()>m_maxY) m_maxY=f->GetMaxY();
    }

    return TRUE;
}

void mpWindow::UpdateAll()
{
    if (UpdateBBox())
    {
        int cx, cy;
        GetClientSize( &cx, &cy);

        const int sx = (int)((m_maxX - m_minX) * GetScaleX());
        const int sy = (int)((m_maxY - m_minY) * GetScaleY());
        const int px = (GetPosX() - m_minX) * GetScaleX() - (cx>>1);
        const int py = (GetPosY() - m_minY) * GetScaleY() - (cy>>1);;
        SetScrollbars( 1, 1, sx, sy, px, py);
    }

    FitInside();
    Refresh( TRUE );
}
