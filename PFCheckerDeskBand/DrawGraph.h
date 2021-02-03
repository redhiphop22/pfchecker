#pragma once

#define GRAPH_COUNT			30
#define GRAPH_INSERT_IDX	29		// GRAPH_COUNT-1 
#define GRAPH_COPY_COUNT	29		// GRAPH_COUNT-1
#define GRAPH_RECT_COUNT	32		// GRAPH_COUNT+2

class DrawGraph
{
public:
	DrawGraph();
	~DrawGraph();

	bool					Init( const DWORD color );
	void					OnSize( const RECT& rect );
	void					OnDraw( const HDC& hdc ) const;
	void					AddValue( const int lastValue );

	const RECT&				GetRect() const					{	return m_rcRect;	}

private:
	HBRUSH					m_hLineBrush	= NULL;
	HBRUSH					m_hBrush		= NULL;

	RECT					m_rcRect;
	int						m_i32Height						= 1;
	POINT					m_ptGraph[ GRAPH_RECT_COUNT ]	= {0, };
};