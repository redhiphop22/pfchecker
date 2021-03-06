#pragma once

////////////////////////////////////////////////////////////////////////////////
// 

class CVisualStyle
{
public:
    enum VisualStyle
    {
        Auto,
        Classic,
        Themed,
    };

public:
    CVisualStyle() : m_hFont(NULL), m_clrText(0) {}
    virtual ~CVisualStyle();

    HFONT GetFont() const { return m_hFont; }
    COLORREF GetTextColor() const { return m_clrText; }

    virtual void DrawBackground(HWND hWnd, HDC hdc, const RECT& rcPaint) const = 0;

    static CVisualStyle* Create(VisualStyle vs = Auto);

protected:
    HFONT m_hFont;
    COLORREF m_clrText;

private:
    // notcopyable
    CVisualStyle(const CVisualStyle&);
    CVisualStyle& operator=(const CVisualStyle&);
};

////////////////////////////////////////////////////////////////////////////////
