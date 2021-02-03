#include "stdafx.h"
#include "DrawGraph.h"

#define PEN_SIZE	1

DrawGraph::DrawGraph()
{
}

DrawGraph::~DrawGraph()
{
	if( m_hBrush )
	{
		DeleteObject(m_hBrush);
	}
	if( m_hLineBrush )
	{
		DeleteObject(m_hLineBrush);
	}
}

bool DrawGraph::Init( const DWORD color )
{
	m_hLineBrush	= (HBRUSH)GetStockObject(NULL_BRUSH);
	m_hBrush		= CreateSolidBrush(color);

	return true;
}

void DrawGraph::OnSize( const RECT& rect )
{
	m_rcRect = rect;

	int i32OldHeight	= m_i32Height;

	m_i32Height			= rect.bottom - rect.top;

	int i32Width		= rect.right - rect.left - (PEN_SIZE*2);
	float fWidthGap		= static_cast<float>(i32Width) / (GRAPH_COUNT+1);
	int i32StartX		= rect.left + PEN_SIZE;
	int i32EndX			= rect.right - PEN_SIZE;

	for( int i = 0 ; i < GRAPH_COPY_COUNT ; i++ )
	{
		m_ptGraph[i].x					= i32StartX + static_cast<int>(fWidthGap*i);
		m_ptGraph[i].y					= static_cast<LONG>((static_cast<float>(m_ptGraph[i].y) * m_i32Height) / i32OldHeight);
	}

	m_ptGraph[ GRAPH_INSERT_IDX ].x		= i32EndX;

	m_ptGraph[ GRAPH_COUNT ].x			= i32EndX;
	m_ptGraph[ GRAPH_COUNT ].y			= m_i32Height;
	m_ptGraph[ GRAPH_COUNT+1 ].x		= i32StartX;
	m_ptGraph[ GRAPH_COUNT+1 ].y		= m_i32Height;
}

void DrawGraph::OnDraw( const HDC& hdc ) const
{
	//HPEN hPen			= CreatePen( PS_DOT, PEN_SIZE, RGB(0,0,0) );
	//HBRUSH hOldBrush	= (HBRUSH)SelectObject(hdc, m_hLineBrush);
	//Rectangle( hdc, m_rcRect.left, m_rcRect.top, m_rcRect.right, m_rcRect.bottom );
	//DeleteObject( hPen );

	HBRUSH hOldBrush	= SelectBrush( hdc, m_hBrush );
	Polygon( hdc, m_ptGraph, GRAPH_RECT_COUNT );

	SelectBrush(hdc, hOldBrush);
}

void DrawGraph::AddValue( const int lastValue )
{
	for( int i = 0 ; i < GRAPH_COPY_COUNT ; i++ )
	{
		m_ptGraph[i].y = m_ptGraph[i + 1].y;
	}

	m_ptGraph[GRAPH_INSERT_IDX].y = ( m_i32Height - static_cast<int>( static_cast<float>(m_i32Height) * lastValue * 0.01f ) );

}