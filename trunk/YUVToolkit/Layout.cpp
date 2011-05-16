#include "../Plugins/YT_Interface.h"

#include "Layout.h"
#include "VideoView.h"

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

	connect(vv, SIGNAL(ViewPortUpdated(VideoView*,double,double)), this, SLOT(OnViewPortUpdated(VideoView*,double,double)));

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
	if (count>0)
	{
		if (count == 1)
		{
			m_CountX = 1;
		}else if (count<=4)
		{
			m_CountX = 2;
		}else if (count<=9)
		{
			m_CountX = 3;
		}else if (count<=12)
		{
			m_CountX = 4;
		}else if (count<=20)
		{
			m_CountX = 5;
		}else if (count<=24)
		{
			m_CountX = 6;
		}else
		{
			m_CountX = ((int)qSqrt(count))+1;
		}

		m_CountY = qCeil(((double)count)/m_CountX);
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

void Layout::OnViewPortUpdated( VideoView* _vv, double x, double y)
{
	for (int i=0; i<m_VideoList.size(); ++i) 
	{
		VideoView* vv = m_VideoList.at(i);
		if (vv != _vv)
		{
			vv->UpdateViewPort(x,y);
		}
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