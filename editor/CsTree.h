#pragma once


// CsTree

class CsTree : public CTreeCtrl
{
	DECLARE_DYNAMIC(CsTree)

public:
	CsTree();
	virtual ~CsTree();

protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
};


