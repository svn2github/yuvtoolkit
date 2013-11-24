#include "../Plugins/YT_Interface.h"

#include "Layout.h"
#include "VideoView.h"
#include <QMouseEvent>

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif


Layout::Layout(QWidget* p) : parent(p), m_ActiveVidew(0),
m_ViewWidth(0), m_ViewHeight(0)
{
	
}

Layout::~Layout(void)
{
}

void Layout::AddView( VideoView* vv)
{
	m_VideoList.append(vv);

	UpdateGrid();
}

void Layout::RemoveView( VideoView* vv)
{
	m_ActiveVidew = NULL;
	m_VideoList.removeOne(vv);
	
	UpdateGrid();
	UpdateGeometry();
}

void Layout::UpdateGrid()
{
	int count = m_VideoList.size();
	if (count==0)
	{
		return;
	}

	QSize szClient = parent->size();
	float maxCoverage = 0;
	for (int x=1; x<=count; x++)
	{
		int y = qCeil(((double)count)/x);

		int viewWidth = (szClient.width()+1)/x;
		int viewHeight = (szClient.height()+1)/y;
		int area = 0;
		for (int i=0; i<m_VideoList.size(); ++i) 
		{
			VideoView* vv = m_VideoList.at(i);
			int w=viewWidth, h=viewHeight;
			vv->computeAR( vv->width(), vv->height(), w, h);
			area += w*h;
		}
		
		float coverage = ((float)area)/(viewWidth*viewHeight);
		if (coverage>=maxCoverage)
		{
			m_CountX = x;
			m_CountY = y;
			maxCoverage = coverage;
		}
	}
}

void Layout::UpdateGeometry()
{
	if (m_CountX==0 || m_CountY == 0)
	{
		m_ViewWidth = 0;
		m_ViewHeight = 0;
		return;
	}

	UpdateGrid();

	QSize szClient = parent->size();
	m_ViewWidth = (szClient.width()+1)/m_CountX;
	m_ViewHeight = (szClient.height()+1)/m_CountY;

	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		VideoView* vv = m_VideoList.at(i);

		int x = i % m_CountX;
		int y = i / m_CountX;
		vv->SetGeometry(x*m_ViewWidth, y*m_ViewHeight, m_ViewWidth-1, m_ViewHeight-1);
	}
}

int Layout::GetVideoCount()
{
	return m_VideoList.size();
}

void Layout::GetDisplaySize( QSize& displaySize )
{
	UpdateGrid();
	QSize max, cur, actual;

	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		VideoView* vv = m_VideoList.at(i);
		vv->GetVideoSize(actual, cur);

		max.setWidth(MAX(cur.width(), max.width()));
		max.setHeight(MAX(cur.height(), max.height()));
	}

	displaySize.setWidth((max.width()+1)*m_CountX-1);
	displaySize.setHeight((max.height()+1)*m_CountY-1);
}

void Layout::OnMouseMoveEvent( QMouseEvent* e )
{
	if (m_ActiveVidew)
	{
		QPoint pt = e->pos();
		pt.setX(pt.x()-m_ActiveX*m_ViewWidth);
		pt.setY(pt.y()-m_ActiveY*m_ViewHeight);
		m_ActiveVidew->OnMouseMoveEvent(pt);
	}
}

void Layout::OnMousePressEvent( QMouseEvent* e)
{
	if (m_ViewWidth==0 || m_ViewHeight==0)
	{
		return;
	}

	m_ActiveX = e->x()/m_ViewWidth;
	m_ActiveY = e->y()/m_ViewHeight;

	int pos = m_ActiveX+m_ActiveY*m_CountX;

	if (pos>=m_VideoList.size())
	{
		return;
	}

	m_ActiveVidew = m_VideoList.at(pos);

	if (m_ActiveVidew)
	{
		QPoint pt = e->pos();
		pt.setX(pt.x()-m_ActiveX*m_ViewWidth);
		pt.setY(pt.y()-m_ActiveY*m_ViewHeight);
		m_ActiveVidew->OnMousePressEvent(pt);
	}
}

void Layout::OnMouseReleaseEvent( QMouseEvent* e )
{
	if (m_ActiveVidew)
	{
		QPoint pt = e->pos();
		pt.setX(pt.x()-m_ActiveX*m_ViewWidth);
		pt.setY(pt.y()-m_ActiveY*m_ViewHeight);
		m_ActiveVidew->OnMouseReleaseEvent(pt);
	}
}


VideoView* Layout::FindVideoAtMoisePosition()
{
	QPoint pos = parent->mapFromGlobal(QCursor::pos());

	if (pos.x()<=0 || pos.x()>=parent->width() ||
		pos.y()<=0 || pos.y()>=parent->height() || 
		m_ViewHeight==0 || m_ViewWidth==0)
	{
		return NULL;
	}

	int x = pos.x()/m_ViewWidth;
	int y = pos.y()/m_ViewHeight;

	int item = x+y*m_CountX;
	if (item>=m_VideoList.size())
	{
		return NULL;
	}

	return m_VideoList.at(item);
}
