// CsTree.cpp: 实现文件
//

//#define _AFXDLL

#include "stdafx.h"
#include "CEditorApp.h"
#include "CsTree.h"


// CsTree

IMPLEMENT_DYNAMIC(CsTree, CTreeCtrl)

CsTree::CsTree()
{
}

CsTree::~CsTree()
{
}


BEGIN_MESSAGE_MAP(CsTree, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CsTree::OnNMDblclk)
END_MESSAGE_MAP()



// CsTree 消息处理程序




//void CsTree::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	// TODO: 在此添加控件通知处理程序代码
//	*pResult = 0;
//}


void CsTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
		//双击鼠标左键
	POINT pt;
	//取得鼠标座标
	::GetCursorPos(&pt);
	//座标转换
	ScreenToClient(&pt);
	//取得结点
	HTREEITEM hItem = HitTest(pt);
	if (hItem)
	{
		//选择
		SelectItem(hItem);
		if (!GetChildItem(hItem))
		{
			//没有子结点，进行下一步处理
			//取节点数据
			UINT nod = GetItemData(hItem);
			if (nod > WM_EDIT_DEFALSE && nod < WM_EDIT_PLAYER)
			{
				::PostMessage(AfxGetMainWnd()->m_hWnd, WM_EDIT_1, 0, nod - WM_EDIT_1);
			}
			//发送消息
			::PostMessage(AfxGetMainWnd()->m_hWnd, nod, 0, 0);
		}
	}


	*pResult = 0;
}
