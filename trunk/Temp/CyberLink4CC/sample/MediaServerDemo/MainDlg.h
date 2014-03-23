// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "AVMediaServer.h"
#include "AVMediaPlayer.h"

class CMainDlg : public CDialogImpl<CMainDlg>,
	public IAVMediaServerCallback,
	public IAVMediaPlayerCallback
{
public:
	CMainDlg();
	~CMainDlg();

public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)

		MSG_WM_DESTROY(OnDestroy)
		COMMAND_ID_HANDLER_EX(IDC_BTN_SERVER_START, OnBtnServerStartClick)
		COMMAND_ID_HANDLER_EX(IDC_BTN_SERVER_DUMP_INFO, OnBtnServerDumpInfo)
		COMMAND_ID_HANDLER_EX(IDC_BTN_SERVER_STOP, OnBtnServerStopClick)

		COMMAND_ID_HANDLER_EX(IDC_BTN_PLAYER_START, OnBtnPlayerStartClick)
		COMMAND_ID_HANDLER_EX(IDC_BTN_PLAYER_DUMP_INFO, OnBtnPlayerDumpInfo)
		COMMAND_ID_HANDLER_EX(IDC_BTN_PLAYER_START, OnBtnPlayerStopClick)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void OnDestroy();
	void OnBtnServerStartClick(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnBtnServerDumpInfo(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnBtnServerStopClick(UINT uNotifyCode, int nID, CWindow wndCtl);

	void OnBtnPlayerStartClick(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnBtnPlayerDumpInfo(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnBtnPlayerStopClick(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	CAVMediaServer*	m_pMediaServer;
	CAVMediaPlayer*	m_pMediaPlayer;
};
