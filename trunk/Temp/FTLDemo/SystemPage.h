#pragma once


// CSystemPage 对话框

class CSystemPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CSystemPage)

public:
	CSystemPage();
	virtual ~CSystemPage();

// 对话框数据
	enum { IDD = IDD_PAGE_SYSTEM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnTempFpuRcReset();
	afx_msg void OnBnClickedBtnSystemMetrics();
};
