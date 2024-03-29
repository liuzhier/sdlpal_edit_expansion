
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

//#define _AFXDLL

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持
#include <afxdisp.h>
#include <afxcmn.h>
#include <afxwin.h>
#include <afxdisp.h>


#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

;

enum WM_Sate
{
	WM_POST_MESSAGE_CSTR = WM_USER + 100,
	WM_POST_MESSAGE_SCRIPT,
	WM_POSTTEXT,
	WM_Deploy_On_The_Map,//Deploy on the map在地图上部署
	WM_EDIT_0,
	WM_EDIT_DIR,
	WM_EDIT_DEFALSE,//基本属性
	WM_EDIT_1,
	WM_EDIT_PLAYER = WM_EDIT_1 + 20,
	WM_EDIT_ENEMY,//怪物
	WM_EDIT_MAGIC,//魔法
	WM_EditBattleField,//战斗场所
	WM_EditEventObject,  //事件对象
	WM_EditStore,//商店
	WM_EditObjectItem,//物品
	WM_EditEnemyTeam,//敌方队伍
	WM_ListScriptEntry,//显示脚本调用列表
	WM_PLAYERLEVELUPMAGIG,//升级魔法
	WM_ScriptEntry,//脚本入口
	WM_ScriptCaller,//查看调用者列表
	WM_MapEdit,//地图编辑
	WM_MapEdit_Map,//单个地图修改
	WM_Test_Eentry_Edit,//测试入口编辑
	WM_Run_Default_Using_Archive,//使用存档运行缺省
	WM_Test_Run,//测试
	WM_EditObjectName,//对象名称编辑
	WM_EditExplain,//对象说明编辑
	WM_EditDialog,//对话编辑
	WM_EditObjectPict,//对象图像编辑
	WM_EditBettlePict,//战斗图像编辑
	WM_BackgroundImageEdite,//背景图片编辑
	WM_CharacterFaceEdite,//头像编辑
	WM_EditPoison,//毒药编辑
	WM_EditScene,//场景编辑
	WM_EDIT_PARAMETERS,//基本属性编辑
	WM_Inventory,//背包物品编辑
	WM_UNPACKPAL,//解包
	WM_PACKPAL,//打包
	WM_OKReturn,//确定返回
	WM_CanselReturn,//取消返回
	WM_EDITCHANGED,
	WM_EditFormat,//cell 编辑格式字符串
	WM_SEND_MSG_STR,//向信息窗口发送信息字符串
	WM_GRID_SELECT_CHANGE,//表选择变化
	WM_GRID_CELL_CHARGE,//下拉数据返回值导致单元格内容变化
	WM_GRID_ROW_CHARGE,//编辑单元格导致行变化
	WM_GRID_POPMENU,//表弹出菜单开始
	WM_GRID_POPMENU_END = WM_GRID_POPMENU +20,//

	WM_MAP_POPMENU,//地图修改弹出菜单开始
	WM_MAP_POPMENU_END = WM_MAP_POPMENU + 20,//地图修改弹出菜单结束标识
	WM_OBJECT_POPMENU,//对象图形修改弹出菜单开始
	WM_OBJECT_POPMENU_END = WM_OBJECT_POPMENU + 20,//对象图形修改弹出菜单结束标识

	WM_TEST_TEXT,//输出调试信息文本
	WM_TEST_MSG,//输出调试信息数据
	WM_LIST_SELECTED_ROW,//列表控件选择的行号

	WM_PAL_OPTIONS,//游戏设置
	WM_PAL_FONT_FIBRARY_NAME,//字库文件名
};
