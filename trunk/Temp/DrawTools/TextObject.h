#pragma once
#include "DrawObject.h"
#include "RichEditPanel.h"

class CTextObject : public CDrawObject
{
public:
	CTextObject(IDrawCanvas* pDrawCanvas, const CRect& position, DrawObjectType objType);
	~CTextObject();

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (m_pRichEditPanel)
		{
			return m_pRichEditPanel->PreTranslateMessage(pMsg);
		}
		return FALSE;
	}

	// Implementation
public:
	virtual void Draw(HDC hDC, BOOL bOriginal);
	virtual void MoveHandleTo(int nHandle, CPoint point);
	virtual CDrawObject* Clone();

protected:
	CRichEditPanel*	m_pRichEditPanel;	
};
