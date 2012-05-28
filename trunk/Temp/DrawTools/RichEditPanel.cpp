#include "stdafx.h"
#include "RichEditPanel.h"

#ifdef FTL_DEBUG
#include <ftlwindow.h>
#include <ftlComDetect.h>
#include <ftlControls.h>
#include <ftlGdi.h>
#endif 

#include <limits>

#pragma comment(lib, "riched20.lib")

#include <ftlConfigDetect.h>

#define LY_PER_INCH   1440
// HIMETRIC units per inch (used for conversion)
#define HIMETRIC_PER_INCH 2540

// These constants are for backward compatibility. They are the 
// sizes used for initialization and reset in RichEdit 1.0
const LONG cInitTextMax = (32 * 1024) - 1;
const LONG cResetTextMax = (64 * 1024);

// The IID in riched20.lib in the Plaform SDK appears to be incorrect.
// This one is from the Microsoft Knowledge Base, article Q270161.
const IID IID_ITextServices = 
{	
	// 8d33f740-cf58-11ce-a89d-00aa006cadc5
	0x8d33f740, 0xcf58, 0x11ce,	{0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
};

//EXTERN_C const IID IID_ITextHost = { /* c5bdd8d0-d26e-11ce-a89e-00aa006cadc5 */
//	0xc5bdd8d0,
//	0xd26e,
//	0x11ce,
//	{0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
//};
//


CRichEditPanel::CRichEditPanel()
{
	FTLTRACE(TEXT("CRichEditPanel::CRichEditPanel\n"));

	ZeroMemory(&m_cRefs, sizeof(CRichEditPanel) - offsetof(CRichEditPanel, m_cRefs));

	m_cRefs = 1;
	m_dwStyle = ES_MULTILINE | ES_NOHIDESEL ;
	m_fRich = TRUE;
	m_cchTextMost = cInitTextMax;
	m_chPasswordChar = TEXT('*');
}

CRichEditPanel::~CRichEditPanel()
{
	m_spTextDocument.Release();
	m_spTextServices.Release();
	
	FTLTRACE(TEXT("CRichEditPanel::~CRichEditPanel\n"));
}

HRESULT STDMETHODCALLTYPE CRichEditPanel::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = E_NOINTERFACE;
	*ppvObject = NULL;
	
	//{13E670F5-1A5A-11CF-ABEB-00AA00B65EA1}  -- ?
	if (IsEqualIID(riid, IID_IUnknown) 
		|| IsEqualIID(riid, IID_ITextHost)) 
	{
		AddRef();
		*ppvObject = (ITextHost *) this;
		hr = S_OK;
	}
	return hr;
}

ULONG CRichEditPanel::AddRef(void)
{
	FTLTRACE(TEXT("Before CRichEditPanel::AddRef, m_cRefs=%d\n"), m_cRefs);
	return ++m_cRefs;
}

ULONG STDMETHODCALLTYPE CRichEditPanel::Release(void)
{
	ULONG c_Refs = --m_cRefs;
	FTLTRACE(TEXT("After CRichEditPanel::Release, m_cRefs=%d\n"), m_cRefs);

	if (c_Refs == 0)
	{
		delete this;
	}
	return c_Refs;
}

HRESULT CRichEditPanel::InitDefaultCharFormat(HFONT hfont) 
{
	FTLTRACE(TEXT("CRichEditPanel::InitDefaultCharFormat\n"));

#if 1 
	HRESULT hr = E_FAIL;
	BOOL bRet = FALSE;
	HWND hWnd = NULL;
	LOGFONT lf = { 0 };
	HDC hDC = NULL;
	//LONG yPixPerInch = 0;

	// Get LOGFONT for default font
	if (!hfont)
	{
		hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}

	// Get LOGFONT for passed hfont
	API_VERIFY( 0 != GetObject(hfont, sizeof(LOGFONT), &lf) );
	if (!bRet)
	{
		return E_FAIL;
	}
	// Set CHARFORMAT structure
	m_charFormat.cbSize = sizeof(CHARFORMAT);

	hWnd = GetDesktopWindow();
	hDC = GetDC(hWnd);

	FTL::HDCProperty	hDCProperty;
	API_VERIFY(FTL::CFGdiUtil::GetHDCProperty(hDC, &hDCProperty));
	FTLTRACE(TEXT("HDCProperty=%s\n"), hDCProperty.GetPropertyString(HDC_PROPERTY_GET_ALL));

	m_xPixPerInch = GetDeviceCaps(hDC, LOGPIXELSX);
	m_yPixPerInch = GetDeviceCaps(hDC, LOGPIXELSY);

	m_charFormat.yHeight = lf.lfHeight * LY_PER_INCH / m_yPixPerInch;
	ReleaseDC(hWnd, hDC);

	m_charFormat.yOffset = 0;
	m_charFormat.crTextColor = m_crAuto;

	m_charFormat.dwEffects = CFM_ALL | CFE_AUTOBACKCOLOR; //CFM_EFFECTS | CFE_AUTOBACKCOLOR;
	m_charFormat.dwEffects &= ~(CFE_PROTECTED | CFE_LINK);

	//if(lf.lfWeight < FW_BOLD)
	//	m_charFormat.dwEffects &= ~CFE_BOLD;
	//if(!lf.lfItalic)
	//	m_charFormat.dwEffects &= ~CFE_ITALIC;
	//if(!lf.lfUnderline)
	//	m_charFormat.dwEffects &= ~CFE_UNDERLINE;
	//if(!lf.lfStrikeOut)
	//	m_charFormat.dwEffects &= ~CFE_STRIKEOUT;

	m_charFormat.dwMask = CFM_ALL | CFM_BACKCOLOR;
	m_charFormat.bCharSet = lf.lfCharSet;
	m_charFormat.bPitchAndFamily = lf.lfPitchAndFamily;
	COM_VERIFY(StringCchCopy(m_charFormat.szFaceName, _countof(m_charFormat.szFaceName),  lf.lfFaceName));
#else
	// Get the current font settings
	NONCLIENTMETRICS nonClientMetrics = {0};
	nonClientMetrics.cbSize = sizeof (NONCLIENTMETRICS);
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS), &nonClientMetrics, 0);

	// Work out the name and point size of the font
	CString strFontName(nonClientMetrics.lfMessageFont.lfFaceName);
	HDC hDC = ::GetDC(NULL);
	int iPointSize = -1 * ::MulDiv(nonClientMetrics.lfMessageFont.lfHeight,72,::GetDeviceCaps(hDC,LOGPIXELSY));
	::ReleaseDC(NULL,hDC);

	// Create a default character format
	::ZeroMemory(&m_charFormat,sizeof(CHARFORMAT));
	m_charFormat.cbSize = sizeof(CHARFORMAT);

	//TODO:CFM_ALL
	m_charFormat.dwMask =  CFM_BOLD|CFM_CHARSET|CFM_COLOR|CFM_FACE|CFM_ITALIC|CFM_OFFSET|
		CFM_PROTECTED|CFM_SIZE|CFM_STRIKEOUT|CFM_UNDERLINE;
	//m_charFormat.dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR | CFE_BOLD| CFE_ITALIC| CFE_UNDERLINE| CFE_STRIKEOUT;
	//m_charFormat.dwEffects &= ~(CFE_PROTECTED | CFE_LINK);

	m_charFormat.yHeight = 20 * iPointSize;
	m_charFormat.crTextColor = ::GetSysColor(COLOR_BTNTEXT);
	m_charFormat.bPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
	_tcscpy(m_charFormat.szFaceName,strFontName);
#endif 

	return S_OK;
}

HRESULT CRichEditPanel::InitDefaultParaFormat() 
{	
#if 0
	memset(ppf, 0, sizeof(PARAFORMAT));
	ppf->cbSize = sizeof(PARAFORMAT);
	ppf->dwMask = PFM_ALL;
	ppf->wAlignment = PFA_LEFT;
	ppf->cTabCount = 1;
	ppf->rgxTabs[0] = lDefaultTab;
#else
	::ZeroMemory(&m_paraFormat ,sizeof(PARAFORMAT));
	m_paraFormat.cbSize = sizeof(PARAFORMAT);
	m_paraFormat.dwMask = PFM_ALL; 
		//PFM_ALIGNMENT|PFM_NUMBERING|PFM_OFFSET|PFM_OFFSETINDENT|PFM_RIGHTINDENT|PFM_RTLPARA|PFM_STARTINDENT|PFM_TABSTOPS;
	m_paraFormat.wAlignment = PFA_LEFT;
#endif 
	return S_OK;
}

HRESULT CRichEditPanel::Init(HWND hWndOwner, const RECT* prcClient, PNOTIFY_CALLBACK pNotifyCallback /* = NULL */)
{
	HRESULT hr = E_FAIL;

	FTLTRACE(TEXT("CRichEditPanel::Init, hWndOwner=0x%x, prcClient=[%d,%d]-[%d,%d]\n"),
		hWndOwner, prcClient->left, prcClient->top, prcClient->right, prcClient->bottom);

	m_hWndOwner = hWndOwner;
	m_rcClient = *prcClient;
	m_pNotifyCallback = pNotifyCallback;

	//m_rcViewInset = m_rcClient;
#pragma TODO(szlExtent is Himetric)
	m_szlExtent.cx = m_rcClient.Width();
	m_szlExtent.cy = m_rcClient.Height();

	COM_VERIFY(InitDefaultCharFormat(NULL));
	COM_VERIFY(InitDefaultParaFormat());

	CComPtr<IUnknown>	spUnknown;
	COM_VERIFY(CreateTextServices(NULL, this, &spUnknown));
	if (SUCCEEDED(hr) && spUnknown)
	{
		COM_VERIFY(spUnknown->QueryInterface(IID_ITextServices, (void**)&m_spTextServices));
		COM_VERIFY(spUnknown->QueryInterface(__uuidof(ITextDocument), (void**)&m_spTextDocument));
		//COM_DETECT_INTERFACE_FROM_REGISTER(m_spTextServices);
		//COM_DETECT_INTERFACE_FROM_LIST(m_spTextServices);

		//COM_VERIFY(m_spTextServices->TxSetText(TEXT("this is sample")));
	}
	if (SUCCEEDED(hr))
	{
		//FTLTRACE(TEXT("Before Call m_spTextServices->OnTxInPlaceActivate, rcClient=[%d,%d]-[%d,%d]\n"),
		//	m_rcClient.left, m_rcClient.top, m_rcClient.right, m_rcClient.bottom);
		//m_spTextServices->OnTxInPlaceActivate(&m_rcClient);
		//FTLTRACE(TEXT("After Call m_spTextServices->OnTxInPlaceActivate\n"));
	}

	return hr;
}

BOOL CRichEditPanel::SetActive(BOOL bActive)
{
	BOOL bRet = FALSE;
	if (bActive != m_fInplaceActive)
	{
		HRESULT hr = E_FAIL;
		if (bActive)
		{
			COM_VERIFY(m_spTextServices->OnTxInPlaceActivate(m_rcClient));
			COM_VERIFY(m_spTextServices->TxSendMessage(WM_SETFOCUS, 0, 0, NULL));
		}
		else
		{
			COM_VERIFY(m_spTextServices->OnTxInPlaceDeactivate());
			COM_VERIFY(m_spTextServices->TxSendMessage(WM_KILLFOCUS, (WPARAM)NULL, 0, NULL));
		}
		if (SUCCEEDED(hr))
		{
			m_fInplaceActive = bActive;
		}
	}
	else
	{
		bRet = TRUE;
	}
	return bRet;
}

BOOL CRichEditPanel::IsActive()
{
	return m_fInplaceActive;
}

HRESULT CRichEditPanel::_GetTextRange(long nStart, long nEnd, CComPtr<ITextRange>& spTextRange)
{
	HRESULT hr = E_FAIL;
	if (0 == nStart && 0 == nEnd)
	{
		//current select
		CComPtr<ITextSelection> spCurtSelection;
		COM_VERIFY(m_spTextDocument->GetSelection(&spCurtSelection));
		if (spCurtSelection)
		{
			spTextRange = spCurtSelection;
		}
	}
	else if (0 == nStart && -1 == nEnd)
	{
		nEnd = m_cchTextMost;// std::numeric_limits<long>::max();
		COM_VERIFY(m_spTextDocument->Range(nStart, nEnd, &spTextRange));
	}
	else
	{
		COM_VERIFY(m_spTextDocument->Range(nStart, nEnd, &spTextRange));
	}
	if (SUCCEEDED(hr))
	{
		FTL::CFTextRangeDumper rangeDumper(spTextRange, FTL::CFOutputWindowInfoOutput::Instance(), 0);
	}
	return hr;
}

HRESULT CRichEditPanel::SetTextFont(long nStart, long nEnd, PLOGFONT pLogFont, DWORD dwFontMask)
{
	CHECK_POINTER_READABLE_DATA_RETURN_VALUE_IF_FAIL(pLogFont, sizeof(LOGFONT), FALSE);

	HRESULT hr = E_FAIL;
	CComPtr<ITextRange>		spRange;
	COM_VERIFY(_GetTextRange(nStart, nEnd, spRange));
	if (SUCCEEDED(hr) && spRange)
	{
		CComPtr<ITextFont> spFont;
		COM_VERIFY(spRange->GetFont(&spFont));
		FTL::CFTextFontDumper fontDumper(spFont, FTL::CFOutputWindowInfoOutput::Instance(), 0);

		if (dwFontMask & RICH_EDIT_PANEL_FONT_MASK_NAME)
		{
			COM_VERIFY(spFont->SetName(CComBSTR(pLogFont->lfFaceName)));
		}
		if (dwFontMask & RICH_EDIT_PANEL_FONT_MASK_SIZE)
		{
#pragma TODO(Height sign)
			COM_VERIFY(spFont->SetSize(FTL_ABS(pLogFont->lfHeight)));
		}
		//if (RICH_EDIT_PANEL_FONT_MASK_WEIGHT & dwFontMask)
		//{
		//	COM_VERIFY(spFont->SetWeight(pLogFont->lfWidth));
		//}
		if (RICH_EDIT_PANEL_FONT_MASK_BOLD & dwFontMask)
		{
			long isBold = (FW_BOLD == pLogFont->lfWeight ? tomTrue : tomFalse);
			COM_VERIFY(spFont->SetBold( isBold ));
		}
		if (RICH_EDIT_PANEL_FONT_MASK_ITALIC & dwFontMask)
		{
			long isItalic = (0 != pLogFont->lfItalic ? tomTrue : tomFalse);
			COM_VERIFY(spFont->SetItalic( isItalic ));
		}
		if (RICH_EDIT_PANEL_FONT_MASK_UNDERLINE & dwFontMask)
		{
			long isUnderLine = (0 != pLogFont->lfUnderline ? tomTrue : tomFalse);
			COM_VERIFY(spFont->SetUnderline( isUnderLine ));
		}
	}
	return hr;
}

HRESULT CRichEditPanel::SetTextFont(long nStart, long nEnd, HFONT	hFont, DWORD dwFontMask)
{
	HRESULT hr = E_FAIL;
	BOOL bRet = FALSE;

#ifdef FTL_DEBUG
	FTL::CFGdiObjectInfoDump fontInfoDump;
	fontInfoDump.GetGdiObjectInfo(hFont);
	FTLTRACE(TEXT("hFont Info = %s\n"), fontInfoDump.GetGdiObjectInfoString());
#endif 

	LOGFONT logFont = {0};
	API_VERIFY(0 != ::GetObject(hFont, sizeof(LOGFONT), &logFont));
	if (bRet)
	{
		COM_VERIFY(SetTextFont(nStart, nEnd, &logFont, dwFontMask));
	}
	return hr;
}

HRESULT CRichEditPanel::SetTextForeColor(long nStart, long nEnd, COLORREF clr)
{
	HRESULT hr = E_FAIL;
	CComPtr<ITextRange>		spRange;
	COM_VERIFY(_GetTextRange(nStart, nEnd, spRange));
	if (SUCCEEDED(hr) && spRange)
	{
		CComPtr<ITextFont> spFont;
		COM_VERIFY(spRange->GetFont(&spFont));

		if ((COLORREF)(-1) == clr)
		{
			COM_VERIFY(spFont->SetForeColor(tomAutoColor));
		}
		else
		{
			COM_VERIFY(spFont->SetForeColor(clr));
		}
	}
	return hr;
}

HRESULT CRichEditPanel::SetTextBackColor(long nStart, long nEnd, COLORREF clr)
{
	HRESULT hr = E_FAIL;
	CComPtr<ITextRange>		spRange;
	COM_VERIFY(_GetTextRange(nStart, nEnd, spRange));
	if (SUCCEEDED(hr) && spRange)
	{
		CComPtr<ITextFont> spFont;
		COM_VERIFY(spRange->GetFont(&spFont));

		if ((COLORREF)(-1) == clr)
		{
			COM_VERIFY(spFont->SetBackColor(tomAutoColor));
		}
		else
		{
			COM_VERIFY(spFont->SetForeColor(clr));
		}
	}
	return hr;
}

void CRichEditPanel::DoPaint(CDCHandle dcParent)
{
	HRESULT hr = E_FAIL;
	//CDCHandle dcHandle(hDC);
	//dcParent.DrawText(TEXT("CRichEditPanel::Draw"), -1, m_rcClient, DT_VCENTER | DT_CENTER | DT_SINGLELINE);

	CMemoryDC	memDC(dcParent, m_rcClient);

	CRect rcClient = m_rcClient;
	rcClient.NormalizeRect();

	memDC.Draw3dRect(rcClient, RGB(255,0,0), RGB(0,0,255));
	rcClient.DeflateRect(1, 1);

	memDC.FillSolidRect(&rcClient, RGB(255,0,0));

	//rcClient.DeflateRect(5, 5);
	RECT *prc = &rcClient;
	RECTL rcL = { rcClient.left, rcClient.top, rcClient.right, rcClient.bottom };
	LONG lViewId = TXTVIEW_ACTIVE;

	//if (!m_fInplaceActive)
	//{
	//	//GetControlRect(&rcClient);
	//	prc = &rcClient;
	//	lViewId = TXTVIEW_ACTIVE;//TXTVIEW_INACTIVE;
	//}

	CRect rcNewClient;
	rcNewClient.SetRectEmpty();

	COM_VERIFY(m_spTextServices->TxDraw(
		DVASPECT_CONTENT,  		// Draw Aspect
		/*-1*/0,						// Lindex
		NULL,					// Info for drawing optimazation
		NULL,					// target device information
		memDC,				// Draw device HDC
		NULL, 				   	// Target device HDC
		&rcL,			// Bounding client rectangle
		NULL, 					// Clipping rectangle for metafiles
		NULL, //&rcNewClient, //(RECT *) m_rcClient,	// Update rectangle
		NULL, 	   				// Call back function
		0,					// Call back parameter
		lViewId));				// What view of the object				
	
	if (rcNewClient.Height() > m_rcClient.Height())
	{
		SetClientRect(&rcNewClient, TRUE);
	}

	// Put a frame around the control so it can be seen
	//FrameRect(dcParent, &m_rcClient, (HBRUSH) GetStockObject(GRAY_BRUSH));

	//if(TxGetEffects() == TXTEFFECT_SUNKEN)
	//	DrawSunkenBorder(hwnd, (HDC) wparam);
}

HRESULT CRichEditPanel::SetClientRect(const RECT *prc, BOOL fUpdateExtent /* = TRUE */)
{
	FTLTRACE(TEXT("CRichEditPanel::SetClientRect, newRect=[%d,%d]-[%d,%d]\n"),
		prc->left, prc->top, prc->right, prc->bottom);
	m_rcClient = *prc;
	m_rcClient.NormalizeRect();

	//m_rcViewInset = m_rcClient;
	m_szlExtent.cx = m_rcClient.Width();
	m_szlExtent.cy = m_rcClient.Height();

	return S_OK;
}

void CRichEditPanel::SetExtent(SIZEL *psizelExtent, BOOL fNotify)
{
	// Set our extent
	m_sizelExtent = *psizelExtent; 

	// Notify the host that the extent has changed
	if (fNotify)
	{
		m_spTextServices->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
	}
}

PNOTIFY_CALLBACK CRichEditPanel::SetNotifyCallback(PNOTIFY_CALLBACK pNotifyCallback)
{
	PNOTIFY_CALLBACK pOldCallback = m_pNotifyCallback;
	m_pNotifyCallback = pNotifyCallback;
	return pOldCallback;
}

HRESULT CRichEditPanel::OnTxInPlaceActivate(LPCRECT prcClient)
{
	m_fInplaceActive = TRUE;
	HRESULT hr = E_FAIL;
	COM_VERIFY_EXCEPT1(m_spTextServices->OnTxInPlaceActivate(prcClient), E_FAIL);
	//COM_VERIFY(m_spTextServices->TxSendMessage(WM_SETFOCUS, 0, 0, NULL));
	
	if (FAILED(hr))
	{
		m_fInplaceActive = FALSE;
	}

	return hr;
}

HRESULT CRichEditPanel::OnTxInPlaceDeactivate()
{
	HRESULT hr = E_FAIL;
	COM_VERIFY(m_spTextServices->OnTxInPlaceDeactivate());

	m_fInplaceActive = FALSE;
	return hr;
}

HDC CRichEditPanel::TxGetDC()
{
	ATLASSERT(::IsWindow(m_hWndOwner));
	HDC hDC = ::GetDC(m_hWndOwner);
	FTLTRACEEX(FTL::tlDetail, TEXT("CRichEditPanel::TxGetDC, getDC=0x%x\n"), hDC);
	return hDC;
}

INT CRichEditPanel::TxReleaseDC( HDC hdc )
{
	ATLASSERT(::IsWindow(m_hWndOwner));
	FTLTRACEEX(FTL::tlDetail, TEXT("CRichEditPanel::TxReleaseDC hdc=0x%x\n"), hdc);
	return ReleaseDC(m_hWndOwner, hdc);
}

BOOL CRichEditPanel::TxShowScrollBar( INT fnBar, BOOL fShow )
{
	FTLTRACE(TEXT("CRichEditPanel::TxShowScrollBar\n"));
	FTLASSERT(FALSE);
	return FALSE;
}

BOOL CRichEditPanel::TxEnableScrollBar( INT fuSBFlags, INT fuArrowflags )
{
	FTLTRACE(TEXT("CRichEditPanel::TxEnableScrollBar\n"));
	FTLASSERT(FALSE);
	return FALSE;
}

BOOL CRichEditPanel::TxSetScrollRange( INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw )
{
	FTLTRACE(TEXT("CRichEditPanel::TxSetScrollRange\n"));
	FTLASSERT(FALSE);
	return FALSE;
}

BOOL CRichEditPanel::TxSetScrollPos( INT fnBar, INT nPos, BOOL fRedraw )
{
	FTLTRACE(TEXT("CRichEditPanel::TxSetScrollPos\n"));
	FTLASSERT(FALSE);
	return FALSE;
}

void CRichEditPanel::TxInvalidateRect( LPCRECT prc, BOOL fMode )
{
	if (prc)
	{
		FTLTRACE(TEXT("CRichEditPanel::TxInvalidateRect, *pRC=[%d,%d]-[%d,%d], fMode=%d\n"),
			prc->left, prc->top, prc->right, prc->bottom, fMode);
	}
	else
	{
		FTLTRACE(TEXT("CRichEditPanel::TxInvalidateRect, prc is NULL, fMode=%d\n"),
			fMode);
	}
	::InvalidateRect(m_hWndOwner, prc, fMode);
}

void CRichEditPanel::TxViewChange( BOOL fUpdate )
{
	FTLTRACE(TEXT("CRichEditPanel::TxViewChange, fUpdate =%d\n"), fUpdate);
	if (fUpdate)
	{
		InvalidateRect(m_hWndOwner, &m_rcClient, FALSE);
		//::UpdateWindow (m_hWndOwner);
	}
}

BOOL CRichEditPanel::TxCreateCaret( HBITMAP hbmp, INT xWidth, INT yHeight )
{
	FTLTRACE(TEXT("CRichEditPanel::TxCreateCaret, hbmp=0x%x, xWidth=%d, yHeight=%d\n"),
		hbmp, xWidth, yHeight);
	BOOL bRet = FALSE;
	API_VERIFY(::CreateCaret (m_hWndOwner, hbmp, xWidth, yHeight));
	return bRet;
}

BOOL CRichEditPanel::TxShowCaret( BOOL fShow )
{
	FTLTRACE(TEXT("CRichEditPanel::TxShowCaret, fShow=%d\n"), fShow);

	BOOL bRet = FALSE;
	if (fShow)
	{
		API_VERIFY_EXCEPT1(::ShowCaret(m_hWndOwner), ERROR_ACCESS_DENIED);
	}
	else
	{
		API_VERIFY_EXCEPT1(::HideCaret(m_hWndOwner), ERROR_ACCESS_DENIED);
	}
	return bRet;	
}

BOOL CRichEditPanel::TxSetCaretPos( INT x, INT y )
{
	FTLTRACE(TEXT("CRichEditPanel::TxSetCaretPos, x=%d, y=%d\n"), x, y);
	BOOL bRet = FALSE;
	API_VERIFY(::SetCaretPos(x, y));
	return bRet;
}

BOOL CRichEditPanel::TxSetTimer( UINT idTimer, UINT uTimeout )
{
	FTLTRACE(TEXT("CRichEditPanel::TxSetTimer, idTimer=%d, uTimeOut=%d\n"), idTimer, uTimeout);
	//m_fTimer = TRUE;
	BOOL bRet = FALSE;
	//API_VERIFY(::SetTimer(m_hWndOwner, idTimer, uTimeout, NULL));
	return bRet;
}

void CRichEditPanel::TxKillTimer( UINT idTimer )
{
	FTLTRACE(TEXT("CRichEditPanel::TxKillTimer, idTimer=%d\n"), idTimer);
	BOOL bRet = FALSE;
	//API_VERIFY(::KillTimer(m_hWndOwner, idTimer));
	//m_fTimer = FALSE;
}

void CRichEditPanel::TxScrollWindowEx( INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll )
{
	FTLTRACE(TEXT("CRichEditPanel::TxScrollWindowEx\n"));
	//FTLTRACE(FALSE);
	//m_spTextServices->
}

void CRichEditPanel::TxSetCapture( BOOL fCapture )
{
	FTLTRACE(TEXT("CRichEditPanel::TxSetCapture, fCapture=%d\n"), fCapture);
	BOOL bRet = FALSE;
	if (fCapture)
	{
		//::SetCapture(m_hWndOwner);
	}
	else
	{
		//API_VERIFY(::ReleaseCapture());
	}
	m_fCapture = fCapture;
}

void CRichEditPanel::TxSetFocus()
{
	FTLTRACE(TEXT("CRichEditPanel::TxSetFocus\n"));

	::SetFocus(m_hWndOwner);
}

void CRichEditPanel::TxSetCursor( HCURSOR hcur, BOOL fText )
{
	FTLTRACE(TEXT("CRichEditPanel::TxSetCursor, hCur=0x%x, fText=%d\n"),
		hcur, fText);
	::SetCursor(hcur);	
}

BOOL CRichEditPanel::TxScreenToClient( LPPOINT lppt )
{
	BOOL bRet = FALSE;
	POINT ptOld = *lppt;
	API_VERIFY(::ScreenToClient(m_hWndOwner, lppt));
	//FTLTRACE(TEXT("CRichEditPanel::TxScreenToClient, [%d,%d] => [%d,%d]\n"), ptOld.x, ptOld.y, lppt->x, lppt->y);
	return bRet;
}

BOOL CRichEditPanel::TxClientToScreen( LPPOINT lppt )
{
	BOOL bRet = FALSE;
	API_VERIFY(::ClientToScreen(m_hWndOwner, lppt));

	POINT ptOld = *lppt;
	FTLTRACE(TEXT("CRichEditPanel::TxClientToScreen, [%d,%d] => [%d,%d]\n"), ptOld.x, ptOld.y, lppt->x, lppt->y);
	return bRet;
}

HRESULT CRichEditPanel::TxActivate( LONG * plOldState )
{
	FTLTRACE(TEXT("CRichEditPanel::TxActivate"));
	*plOldState = m_lState;
	return S_OK;
}

HRESULT CRichEditPanel::TxDeactivate( LONG lNewState )
{
	FTLTRACE(TEXT("CRichEditPanel::TxDeactivate, lNewState=%d\n"), lNewState);
	return S_OK;
}

HRESULT CRichEditPanel::TxGetClientRect( LPRECT prc )
{
	FTLTRACE(TEXT("CRichEditPanel::TxGetClientRect, m_rcClient=[%d,%d]-[%d,%d]\n"),
		m_rcClient.left, m_rcClient.top, m_rcClient.right, m_rcClient.bottom);
	*prc = m_rcClient;
	return S_OK;
}

HRESULT CRichEditPanel::SetText(LPCTSTR pszText)
{
	HRESULT hr = E_FAIL;
	//m_spTextServices->TxSetText(pszText);
	FTLTRACE(TEXT("CRichEditPanel::SetText, Text=%s\n"), pszText);
	// Get a range for the whole of the text in the control
	CComPtr<ITextRange> spTextRange;
	COM_VERIFY(m_spTextDocument->Range(0, 0, &spTextRange));
	COM_VERIFY_EXCEPT1(spTextRange->MoveEnd(tomStory, 1, NULL), S_FALSE);
	
	// Replace the whole text
	COM_VERIFY(spTextRange->SetText(CComBSTR(pszText)));

#ifdef FTL_DEBUG
	USES_CONVERSION;
	CComBSTR bstrNewText;
	COM_VERIFY(spTextRange->GetText(&bstrNewText));
	FTLTRACE(TEXT("After SetText, New String=%s\n"), COLE2CT(bstrNewText));
#endif 
	return hr;
}

HRESULT CRichEditPanel::Range(long cpFirst, long cpLim, ITextRange** ppRange)
{
	// Get the given range of text in the control
	HRESULT hr = E_FAIL;
	COM_VERIFY(m_spTextDocument->Range(cpFirst,cpLim,ppRange));
	if (SUCCEEDED(hr))
	{
		USES_CONVERSION;
		CComBSTR bstrText;
		COM_VERIFY((*ppRange)->GetText(&bstrText));
		FTLTRACE(TEXT("After Call Range For [%d,%d] Text is %s\n"), cpFirst, cpLim, COLE2CT(bstrText));
	}
	return hr;
}


//
HRESULT CRichEditPanel::TxGetViewInset( LPRECT prc )
{
	*prc = CRect(0, 0, 0, 0);//100 ,200, 300, 400);
	//FTLTRACE(TEXT("CRichEditPanel::TxGetViewInset, prc=[%d,%d]-[%d,%d]\n"),
	//	prc->left, prc->top, prc->right, prc->bottom);
	return S_OK;
}

HRESULT CRichEditPanel::TxGetCharFormat( const CHARFORMATW **ppCF )
{
	FTL::CFStringFormater strFormaterEffects;
	FTL::CFStringFormater strFormaterMasks;

	FTLTRACE(TEXT("CRichEditPanel::TxGetCharFormat,szFaceName=%s, crTextColor=0x%x, yHeight=%d, yOffset=%d\n")
		TEXT("Effects=%s, Mask=%s\n"),
		m_charFormat.szFaceName, m_charFormat.crTextColor, m_charFormat.yHeight, m_charFormat.yOffset,
		FTL::CFControlUtil::GetCharFormatEffectAndMaskString(strFormaterEffects, m_charFormat.dwEffects),
		FTL::CFControlUtil::GetCharFormatEffectAndMaskString(strFormaterMasks, m_charFormat.dwMask)
		);

	*ppCF = &m_charFormat;
	return S_OK;
}

HRESULT CRichEditPanel::TxGetParaFormat( const PARAFORMAT **ppPF )
{
	FTLTRACE(TEXT("CRichEditPanel::TxGetParaFormat,wNumbering=%d, dxStartIndent=%d, dxRightIndent=%d, dxOffset=%d, cTabCount=%d\n"),
		m_paraFormat.wNumbering, m_paraFormat.dxStartIndent, 
		m_paraFormat.dxRightIndent, m_paraFormat.dxOffset, m_paraFormat.cTabCount);
	*ppPF = &m_paraFormat;
	return S_OK;
}

COLORREF CRichEditPanel::TxGetSysColor( int nIndex )
{
	FTLTRACEEX(FTL::tlInfo, TEXT("CRichEditPanel::TxGetSysColor, nIndex=%s\n"), 
		FTL::CFWinUtil::GetColorString(FTL::ctSysColor, nIndex));

	if (nIndex == COLOR_WINDOW)
	{
		if(!m_fNotSysBkgnd)
			return GetSysColor(COLOR_WINDOW);
		return m_crBackground;
	}
	return GetSysColor(nIndex);
}

HRESULT CRichEditPanel::TxGetBackStyle( TXTBACKSTYLE *pstyle )
{
	FTLTRACE(TEXT("CRichEditPanel::TxGetBackStyle, m_fTransparent=%d\n"), m_fTransparent);
	*pstyle = !m_fTransparent ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
	return S_OK;
}

HRESULT CRichEditPanel::TxGetMaxLength( DWORD *plength )
{
	FTLTRACE(TEXT("CRichEditPanel::TxGetMaxLength, m_cchTextMost=%d\n"), m_cchTextMost);

	*plength = m_cchTextMost;
	return S_OK;

}

HRESULT CRichEditPanel::TxGetScrollBars( DWORD *pdwScrollBar )
{
	//if return 0, then do not allow scrollbars
	*pdwScrollBar = 0; 


	//m_dwStyle & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | 
	//	ES_AUTOHSCROLL | ES_DISABLENOSCROLL);

	//FTLTRACE(TEXT("CRichEditPanel::TxGetScrollBars, *pdwScrollBar=0x%x\n"), *pdwScrollBar);

	return S_OK;
}

HRESULT CRichEditPanel::TxGetPasswordChar( TCHAR *pch )
{
	*pch = m_chPasswordChar;
	FTLTRACE(TEXT("CRichEditPanel::TxGetPasswordChar, char=%c\n"), m_chPasswordChar);
	return S_OK;
}

HRESULT CRichEditPanel::TxGetAcceleratorPos( LONG *pcp )
{
	*pcp = -1;
	FTLTRACE(TEXT("CRichEditPanel::TxGetAcceleratorPos, AcceleratorPos=%d\n"), *pcp);
	return S_OK;
}

HRESULT CRichEditPanel::TxGetExtent( LPSIZEL lpExtent )
{
	//FTLTRACE(TEXT("CRichEditPanel::TxGetExtent, m_szlExtent={%d,%d}\n"), 
	//	m_szlExtent.cx, m_szlExtent.cy);
	*lpExtent = m_szlExtent;
	return E_NOTIMPL;
}

HRESULT CRichEditPanel::OnTxCharFormatChange( const CHARFORMATW * pcf )
{
	FTLTRACE(TEXT("CRichEditPanel::OnTxCharFormatChange\n"));
	FTLASSERT(FALSE);
	return E_NOTIMPL;
}

HRESULT CRichEditPanel::OnTxParaFormatChange( const PARAFORMAT * ppf )
{
	FTLTRACE(TEXT("CRichEditPanel::OnTxParaFormatChange\n"));
	FTLASSERT(FALSE);
	return E_NOTIMPL;
}

HRESULT CRichEditPanel::TxGetPropertyBits( DWORD dwMask, DWORD *pdwBits )
{
	FTL::CFStringFormater propertyFormater;
	FTLTRACE(TEXT("CRichEditPanel::TxGetPropertyBits, dwMask=%s\n"),
		FTL::CFControlUtil::GetRichEditPropertyBits(propertyFormater, dwMask, TEXT("|")));
	DWORD dwProperties =  TXTBIT_MULTILINE | TXTBIT_RICHTEXT | TXTBIT_WORDWRAP | TXTBIT_SHOWACCELERATOR;

#if 0
	if (m_fRich)
	{
		dwProperties = TXTBIT_RICHTEXT;
	}

	if (m_dwStyle & ES_MULTILINE)
	{
		dwProperties |= TXTBIT_MULTILINE;
	}

	if (m_dwStyle & ES_READONLY)
	{
		dwProperties |= TXTBIT_READONLY;
	}


	if (m_dwStyle & ES_PASSWORD)
	{
		dwProperties |= TXTBIT_USEPASSWORD;
	}

	if (!(m_dwStyle & ES_NOHIDESEL))
	{
		dwProperties |= TXTBIT_HIDESELECTION;
	}

	if (m_fEnableAutoWordSel)
	{
		dwProperties |= TXTBIT_AUTOWORDSEL;
	}

	if (m_fVertical)
	{
		dwProperties |= TXTBIT_VERTICAL;
	}

	if (m_fWordWrap)
	{
		dwProperties |= TXTBIT_WORDWRAP;
	}

	if (m_fAllowBeep)
	{
		dwProperties |= TXTBIT_ALLOWBEEP;
	}

	if (m_fSaveSelection)
	{
		dwProperties |= TXTBIT_SAVESELECTION;
	}
#endif 
	*pdwBits = dwProperties & dwMask; 
	return S_OK;
}

HRESULT CRichEditPanel::TxNotify( DWORD iNotify, void *pv )
{
	FTLTRACE(TEXT("CRichEditPanel::TxNotify, iNotify=%s, pv=0x%x\n"),
		FTL::CFControlUtil::GetEditNotifyCodeString(iNotify), pv);

	if (m_pNotifyCallback)
	{
		(*m_pNotifyCallback)(iNotify, pv);
	}
	// Claim to have handled the notifcation, even though we always ignore it
	return S_OK;
}

HIMC CRichEditPanel::TxImmGetContext()
{
	FTLTRACE(TEXT("CRichEditPanel::TxImmGetContext\n"));
	return NULL;
}

void CRichEditPanel::TxImmReleaseContext( HIMC himc )
{
	FTLTRACE(TEXT("CRichEditPanel::TxImmReleaseContext, himc=0x%x\n"), himc);
}

HRESULT CRichEditPanel::TxGetSelectionBarWidth( LONG *lSelBarWidth )
{
	*lSelBarWidth = m_lSelBarWidth;

	FTLTRACEEX(FTL::tlDetail, TEXT("CRichEditPanel::TxGetSelectionBarWidth, m_lSelBarWidth=%d\n"), m_lSelBarWidth);
	return E_NOTIMPL;
}

BOOL CRichEditPanel::_IsNeedHandleMsg(MSG* pMsg)
{
	BOOL bRet = FALSE;

	if (m_fCapture)
	{
		bRet = TRUE;
	}
	else if (IsActive())
	{
		if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
		{
			POINT ptLocalClient = pMsg->pt;
			TxScreenToClient(&ptLocalClient);
			if (m_rcClient.PtInRect(ptLocalClient))
			{
				bRet = TRUE;
			}
		}
		else if(WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
		{
			bRet = TRUE;
		}
	}
	return bRet;
}

LRESULT CRichEditPanel::OnKeyMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HRESULT hr = E_FAIL;
	LRESULT lResult = 0;
	hr = m_spTextServices->TxSendMessage(uMsg, wParam, lParam, &lResult);
	if (S_OK != hr)
	{
		bHandled = FALSE;
	}
	else
	{
		//COM_VERIFY(m_spTextServices->OnTxPropertyBitsChange(TXTBIT_SCROLLBARCHANGE, TXTBIT_SCROLLBARCHANGE));
	}
	return lResult;
}

LRESULT CRichEditPanel::OnMouseMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HRESULT hr = E_FAIL;
	LRESULT lResult = 0;
	hr = m_spTextServices->TxSendMessage(uMsg, wParam, lParam, &lResult);
	if (S_OK != hr)
	{
		bHandled = FALSE;
	}
	return lResult;
}

BOOL CRichEditPanel::PreTranslateMessage(MSG* pMsg)
{
#ifdef FTL_DEBUG
	//DEFAULT_DUMP_FILTER_MESSAGE | 
	DUMP_WINDOWS_MSG(__FILE__LINE__, (DEFAULT_DUMP_FILTER_MESSAGE | DUMP_FILTER_TIMER) &~ DUMP_FILTER_MOUSE_MOVE, pMsg->message, pMsg->wParam, pMsg->lParam);
#endif 
	BOOL bRet = FALSE;
	HRESULT hr = E_FAIL;
	LONG lResult = 0;
	//BOOL bWillHandle = FALSE;
	if (_IsNeedHandleMsg(pMsg))
	{
		bRet = ProcessWindowMessage(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam, lResult, 0);
		if (!bRet)
		{
			hr = m_spTextServices->TxSendMessage(pMsg->message, pMsg->wParam, pMsg->lParam, &lResult);
			if (S_OK == hr)
			{
				return TRUE;
			}
		}

	//	if (WM_MOUSEMOVE != pMsg->message)
	//	{
	//		FTLTRACE(TEXT("CRichEditPanel::PreTranslateMessage TxSendMessage for %s return 0x%x\n"), 
	//			FTL::CFMessageInfo(pMsg->message, pMsg->wParam, pMsg->lParam).GetConvertedInfo(), hr);
	//	}

//#if 1
//		//Handle Mouse Message
//		if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
//		{
//			POINT ptLocalClient = pMsg->pt;
//			TxScreenToClient(&ptLocalClient);
//			if (m_rcClient.PtInRect(ptLocalClient))
//			{
//				//if in Rich Edit Panel Client
//				hr = m_spTextServices->TxSendMessage(pMsg->message, pMsg->wParam, pMsg->lParam, &lResult);
//				if (WM_MOUSEMOVE != pMsg->message)
//				{
//					FTLTRACE(TEXT("CRichEditPanel::PreTranslateMessage TxSendMessage for %s return 0x%x\n"), 
//						FTL::CFMessageInfo(pMsg->message, pMsg->wParam, pMsg->lParam).GetConvertedInfo(), hr);
//				}
//				if (S_OK == hr)
//				{
//					return TRUE;
//				}
//			}
//		}
//#else
//		bRet = ProcessWindowMessage(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam, lResult, 0);
//
//		if (bRet)
//		{
//			FTLTRACE(TEXT("CRichEditPanel::PreTranslateMessage Handle Msg(%s) return TRUE\n"),
//				FTL::CFMessageInfo(pMsg->message, pMsg->wParam, pMsg->lParam).GetConvertedInfo());
//		}
//#endif 
	}

	//HRESULT hr = m_spTextServices->TxSendMessage(uMsg, wParam, lParam, &lResult);
	//if (hr == S_FALSE)
	//{
	//	lResult = ::DefWindowProc(m_hWndOwner, uMsg, wParam, lParam);
	//}

	return bRet;
}

//LRESULT CRichEditPanel::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	FTLTRACE(TEXT("CRichEditPanel::OnSetFocus\n"));
//
//	return 0;
//}
//
//LRESULT CRichEditPanel::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	HRESULT hr = E_FAIL;
//	LRESULT lResult = 0;
//	COM_VERIFY(m_spTextServices->TxSendMessage(uMsg, wParam, lParam, &lResult));
//
//	COM_VERIFY(m_spTextServices->OnTxPropertyBitsChange(TXTBIT_SCROLLBARCHANGE, TXTBIT_SCROLLBARCHANGE));
//
//	return lResult;
//}

//LRESULT CRichEditPanel::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	HRESULT hr = E_FAIL;
//	LRESULT lResult = 0;
//	switch(TCHAR(wParam))
//	{
//	case VK_BACK:
//	//	FTLTRACE(TEXT("In KeyDown for Back\n"));
//	//	break;
//	default:
//		hr = m_spTextServices->TxSendMessage(uMsg, wParam, lParam, &lResult);
//		break;
//	}
//	if (S_FALSE == hr)
//	{
//		bHandled = FALSE;
//	}
//	return lResult;
//}


DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie,
						 LPBYTE pbBuff,
						 LONG cb,
						 LONG *pcb
						 )
{
	CStringA strInfo((CHAR*)pbBuff);
	USES_CONVERSION;
	FTLTRACE(TEXT("On EditStreamCallback, cb=%d, pbBufff=%s\n"), cb, CA2T(strInfo));
	return 0;
}

//LRESULT CRichEditPanel::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
//{
//	HRESULT hr = E_FAIL;
//
//	//CComVariant varFileName(CComBSTR(TEXT("DocumentRtf.dat")));
//	//COM_VERIFY(m_spTextDocuemtn->Save(&varFileName, tomCreateNew|tomRTF ,0 ));
//	
//	EDITSTREAM	editStream = {0};
//	editStream.dwCookie = DWORD_PTR(this);
//	editStream.dwError = 0;
//	editStream.pfnCallback = EditStreamCallback;
//
//	COM_VERIFY(m_spTextServices->TxSendMessage(EM_STREAMOUT, (WPARAM)(SF_RTF), (LPARAM)&editStream, NULL));
//	return 0;
//}


//HRESULT CRichEditPanel::GetTextStream(long nStart, long nEnd, IStream** ppStream)
//{
//	CHECK_POINTER_RETURN_VALUE_IF_FAIL(ppStream, E_POINTER);
//	HRESULT hr = E_FAIL;
//
//
//	FTLASSERT(FALSE);
//	return hr;
//}