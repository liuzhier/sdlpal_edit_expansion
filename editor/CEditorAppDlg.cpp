
// sDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "CEditorApp.h"
#include "CEditorAppDlg.h"
#include "afxdialogex.h"

#include "../palgpgl.h"
#include "CFileDir.h"
#include "ctestdata.h"
#include "../cpalevent.h"
#include "../cscript.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <string>
#include <cassert>

//下拉表筛选 0 物品 1 法术 2 毒药 3 攻击附加 4.怪
const DWORD32 select_flag[] =
{
	kIsItem,
	kIsMagic,
	kIsPoison,
	kIsPoison| kIsItem | kIsMagic,
	kIsEnemy,
};
static BOOL is_InPalThread{ 0 };


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


LPCSTR CEditorAppDlg::p_SrcToStr(int i)
{
	//显示怪队伍时使用
	if (i == 0xffff) return"禁用";
	//以下正常
	if (i < 0 || i >= MAX_OBJECTS)
		return NULL;
	return (Pal->PAL_GetWord(i));
}


//typedef LPCSTR(CPalEdit::p_Script)(int);
LPCSTR  p_Script(int i)
{
	//返回脚本说明
	LPCSTR s = "";
	switch (i)
	{
	case 0:
		s = "停止运行";
		break;
	case 0x0001:
		s = "停止运行，用下行替换，用于下一轮运行";
		break;
	case 0x0002:
		s = "停止运行，用参数1替换，用于下一轮运行,条件是参数 2 = 0 或小于对象空闲帧数";
		break;
	case 0x0003:
		s = "无条件跳转到参数 1位置,如参数2不等于0且未达到参数2指定的帧数 运行下一条";
		break;
	case 0x0004:
		s = "运行 参数1 指向的子脚本";
		break;
	case 0x0005:
		s = "刷新场景";
		break;
	case 0x0006:
		s = "条件跳转  参数1 概率  满足执行下一条，不满足跳转到参数2指向的地址";
		break;
	case 0x0007:
		s = "开始战斗  参数1 队伍号， 参数3 为 0 BOSS 参数2 战斗失败执行 参数3 战斗中逃跑执行 ";
		break;
	case 0x0008:
		s = "用下一条指令替换 ";
		break;
	case 0x0009:
		s = "等待 参数1 指定的时间（帧数） ";
		break;
	case 0x000a:
		s = "跳转到 参数1 指定地址，如玩家选择 No ";
		break;
	case 0x008e:
		s = "恢复场景";
		break;
	case 0xffff:
		s = "显示文字 参数1 文字地址";
		break;
	case 0x00ff:
		s = "随机执行 1-参数1 之间的任意一条指令  之后跳过 参数1 条数继续执行";
		break;
	case 0x000b:
	case 0x000c:
	case 0x000d:
	case 0x000e:
		s = "行走1步  入口 - 0x000b 方向";
		break;
	case 0x000f:
		s = "设置对象的方向和手势 参数1 方向 ，参数2 手势";
		break;
	case 0x0010:
		s = "直走到指定的位置 ,参数1 X，参数2 Y，参数3 高度";
		break;
	case 0x0011:
		s = "慢走到指定的位置 ,参数1 X，参数2 Y，参数3 高度";
		break;
	case 0x0012:
		s = "设置对象到相对于队伍的位置 参数1对象,参数2 X，参数3 Y";
		break;
	case 0x0013:
		s = "设置对象到指定的位置 参数1对象,参数2 X，参数3 Y";
		break;
	case 0x0014:
		s = "设置对象的形象（手势），参数1 手势 ，方向南";
		break;
	case 0x0015:
		s = "设置队伍成员的方向，形象（手势），参数1 队伍方向 ，参数3，成员序号，参数2，该成员的相对方向";
		break;
	case 0x0016:
		s = "设置对象的，方向和（ 手势），参数1对象不为0 ，参数2，方向，参数3，形象";
		break;
	case 0x0017:
		s = "设置额外属性，战后清除，参数1 位置，头0x0b,以下顺延，参数2属性09:体力,0A：真气.11：武术.12：灵力.13：防御.14：身法.15：吉运.16：毒抗17：风抗";
		break;
	case 0x0018:
		s = "指定部位的装备 参数1 = 0xb 头，参数2 新装备";
		break;
	case 0x0019:
		s = "属性增加，参数2 值，参数3 1 李，0本人，参数1= 06 等级，07 最大体力，08 最大真气，09:体力,0A：真气.11：武术.12：灵力.13：防御.14：身法.15：吉运.16：毒抗17：风抗";
		break;
	case 0x001a:
		s = "属性写入，参数2 值，参数3 1 李，0本人，参数1= 06 等级，07 最大体力，08 最大真气，09:体力,0A：真气.11：武术.12：灵力.13：防御.14：身法.15：吉运.16：毒抗17：风抗";
		break;
	case 0x001b:
		s = "体力增加减少，参数1 = 1 全体 0 单人，参数2 值";
		break;
	case 0x001c:
		s = "真气增加减少，参数1 = 1 全体 0 单人，参数2 值";
		break;
	case 0x001d:
		s = "体力、真气增加减少，参数1 = 1 全体 0 单人，参数2 体力值，参数3 真气值，参数3为0时=参数2";
		break;
	case 0x001e:
		s = "金钱改变，参数1 = 值，如参数1为且钱不足时，跳到参数2执行";
		break;
	case 0x001f:
		s = "向库存中添加物品，参数1 品种，参数2 数量";
		break;
	case 0x0020:
		s = "从库存中移除物品，参数1 品种，参数2 数量，如库存没有移除已装备物品，物品不足跳到参数3  ";
		break;
	case 0x0021:
		s = "伤敌指令，参数1 ！=0 全体，0单人。参数2 整数，小于0 增加体力";
		break;
	case 0x0022:
		s = "角色复活指令，参数1 ！=0 全体，0单人。参数2 整数，10满血，";
		break;
	case 0x0023:
		s = "装备移除指令，参数1 =0 李 2 赵，参数2 =0 所有的，=1 头";
		break;
	case 0x0024:
		s = "设置对象自动脚本地址，参数1对象 不等于0 参数2 地址";
		break;
	case 0x0025:
		s = "设置对象触发脚本地址，参数1对象 不等于0 参数2 地址";
		break;
	case 0x0026:
		s = "显示商店，参数1 商店号";
		break;
	case 0x0027:
		s = "显示当铺";
		break;
	case 0x0028:
		s = "敌方中毒指令 参数1 ！=0 全体，参数2 毒,失败跳转到参数3，参数3=0xffff,无视毒抗";
		break;
	case 0x0029:
		s = "我方中毒指令 参数1 ！=0 全体，参数2 毒,失败跳转到参数3,参数3=0xffff,无视毒抗";
		break;
	case 0x002a:
		s = "敌方解毒指令 参数1 ！=0 全体，参数2 毒";
		break;
	case 0x002b:
		s = "我方解毒指令 参数1 ！=0 全体，参数2 毒";
		break;
	case 0x002c:
		s = "我方多重解毒指令 参数1 ！=0 全体，参数2 解除不高于其等级的毒";
		break;
	case 0x002d:
		s = "我方特殊状态指令，参数1 =0 疯、睡、定、封、傀儡、攻、防、身、双击，参数2 回合数。失败跳到参数3";
		break;
	case 0x002e:
		s = "敌方特殊状态指令，参数1 =0 疯、睡、定、傀儡、攻、防、身、双击，参数2 回合数，参数3 如不成功 跳转到参数3指向的地址。";
		break;
	case 0x002f:
		s = "我方解除特殊状态指令，参数1 = 0 疯、睡、定、封、傀儡、攻、防、身、双击";
		break;
	case 0x0030:
		s = "按百分比暂时增加角色属性，参数1 = 11 武、12 灵、13 防、14 身、15 吉、16 毒、17 风、18 雷、19 水、1a 火 1b 土";
		break;
	case 0x0031:
		s = "战斗中改变形象 参数1 形象";
		break;
	case 0x0033:
		s = "收集妖怪炼丹,参数1 失败转到";
		break;
	case 0x0034:
		s = "灵壶炼丹指令,参数1 失败转到";
		break;
	case 0x0035:
		s = "摇动场景";
		break;
	case 0x0036:
		s = "设置当前正在播放的RNG动画 参数1";
		break;
	case 0x0037:
		s = "播放的RNG动画 参数1 开始帧数，参数2 播放帧数 （缺省999） 参数3 速度（缺省16） ";
		break;
	case 0x0038:
		s = "将队伍从现场传送出去 失败跳到参数1 指向地址 ";
		break;
	case 0x0039:
		s = "吸取生命 从选定对象吸取 参数1 点数生命补充自己";
		break;
	case 0x003a:
		s = "从战斗中逃走";
		break;
	case 0x003b:
		s = "在场景中间显示对话框,参数1 字体颜色 参数 3 是否显示人像 ";
		break;
	case 0x003c:
		s = "在场景上方显示对话框,参数2 字体颜色 ，参数1 人像号，参数 3 是否显示人像 ";
		break;
	case 0x003D:
		s = "在场景下方显示对话框,参数1 字体颜色 ，参数2 人像号，参数 3 是否显示人像 ";
		break;
	case 0x003e:
		s = "在场景上面显示文字,参数1 字体颜色  ";
		break;
	case 0x003f:
		s = "将指定对象低速移动到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0040:
		s = "设置对象触发模式 如参数1对象 ！= 0 ，参数2 设置";
		break;
	case 0x0041:
		s = "令脚本执行失败";
		break;
	case 0x0042:
		s = "模拟法术，参数1 法术，参数2 基础伤害，参数3 对象";
		break;
	case 0x0043:
		s = "更换背景音乐，参数1 音乐号";
		break;
	case 0x0044:
		s = "将指定对象移动到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0045:
		s = "设置战斗时音乐，参数1 音乐号";
		break;
	case 0x0046:
		s = "设置队伍在地图上的位置， 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0047:
		s = "播放音乐效果，参数1 音乐号";
		break;
	case 0x0049:
		s = "设置对象状态，参数2 状态";
		break;
	case 0x004a:
		s = "设置战斗场所，参数1 场所";
		break;
	case 0x004b:
		s = "使对象短时间消失";
		break;
	case 0x004c:
		s = "追逐队伍，参数1 最大距离，参数1 速度，参数2 是否浮动";
		break;
	case 0x004d:
		s = "按任意键继续";
		break;
	case 0x004e:
		s = "读入最近使用的文档";
		break;
	case 0x004f:
		s = "将场景淡入红色(游戏结束)";
		break;
	case 0x0050:
		s = "场景淡出";
		break;
	case 0x0051:
		s = "场景淡入";
		break;
	case 0x0052:
		s = "使对象消失一段时间，参数1 时间（缺省800）";
		break;
	case 0x0053:
		s = "使用白天调色版";
		break;
	case 0x0054:
		s = "使用夜间调色版";
		break;
	case 0x0055:
		s = "添加魔法，参数1魔法，参数2 对象，0缺省";
		break;
	case 0x0056:
		s = "移除仙术，参数1魔法，参数2 对象，0缺省";
		break;
	case 0x0057:
		s = "根据真气设定基础伤害";
		break;
	case 0x0058:
		s = "如果库存数量不足则跳过，参数1 品种，参数2 要求数量，参数3 跳转到";
		break;
	case 0x0059:
		s = "更换地图，参数1 地图号";
		break;
	case 0x005a:
		s = "体力减半";
		break;
	case 0x005b:
		s = "敌方体力减半，参数1 对象";
		break;
	case 0x005c:
		s = "隐藏  参数1 回合数";
		break;
	case 0x005d:
		s = "如我方没有中特定的毒，则跳转  参数1 毒种类，参数2 跳转到";
		break;
	case 0x005e:
		s = "如敌方没有中特定的毒，则跳转  参数1 毒种类，参数2 跳转到";
		break;
	case 0x005f:
		s = "我方立刻死亡";
		break;
	case 0x0060:
		s = "敌方立刻死亡";
		break;
	case 0x0061:
		s = "没有中毒，跳转，参数1 跳转到";
		break;
	case 0x0062:
		s = "暂停敌方 追赶，参数1 时间";
		break;
	case 0x0063:
		s = "加速敌方 追赶，参数1 时间";
		break;
	case 0x0064:
		s = "跳转，如敌方生命值高于 设定的百分比 ，参数1 比值，参数2 目标";
		break;
	case 0x0065:
		s = "设置图像，参数1 我方成员代码，参数2 图像，参数3 是否刷新";
		break;
	case 0x0066:
		s = " 投掷武器，参数2 伤害值";
		break;
	case 0x0067:
		s = "设置敌方魔法，参数1魔法，参数2 概率";
		break;
	case 0x0068:
		s = "如敌方行动，跳转到参数1";
		break;
	case 0x0069:
		s = "敌方逃跑";
		break;
	case 0x006a:
		s = "偷敌人，参数1 概率";
		break;
	case 0x006b:
		s = "吹走敌人，";
		break;
	case 0x006C:
		s = "NPC走一步， 参数1 事件对象，参数2 X，参数3 Y";
		break;
	case 0x006D:
		s = "设定一个场景的进入和传送脚本，参数1，场景号，参数2 进入脚本，参数3 传送脚本，参数2 和参数3同时为0 清除脚本";
		break;
	case 0x006E:
		s = "将队伍移动到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x006F:
		s = "将当前事件对象状态与·另一个事件对象同步";
		break;
	case 0x0070:
		s = "队伍走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0071:
		s = "波动场景 参数1，参数2";
		break;
	case 0x0073:
		s = "淡入场景";
		break;
	case 0x0074:
		s = "如不是全体满血，跳到参数1";
		break;
	case 0x0075:
		s = "设置队伍，参数1，参数2，参数3";
		break;
	case 0x0076:
		s = "Show FBP picture 参数1 ，参数2";
		break;
	case 0x0077:
		s = "停止正在播放音乐，参数1";
		break;
	case 0x0078:
		s = "未知";
		break;
	case 0x0079:
		s = "如果指定成员在队伍中，则跳到参数2";
		break;
	case 0x007a:
		s = "队伍快速走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x007b:
		s = "队伍最快速走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x007c:
		s = "队伍直接走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x007D:
		s = "移动对象位置，参数1对象 参数2 X，参数3 Y";
		break;
	case 0x007E:
		s = "设置对象的层，参数1对象，参数2 ";
		break;
	case 0x007f:
		s = "移动视点，参数1 X，参数2 Y，参数3,不等于FFFF 更新场景";
		break;
	case 0x0080:
		s = "白天黑夜转换，参数1";
		break;
	case 0x0081:
		s = "跳过，如果没有面对对象，参数1 对象，参数2 改变触发方式，参数3地址";
		break;
	case 0x0082:
		s = "队伍快速直接走到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0083:
		s = "如果事件对象不在当前事件对象的指定区域，则跳过,参数1对象，参数3地址";
		break;
	case 0x0084:
		s = "将玩家用作事件对象的物品放置到场景中,参数1，参数2，失败跳到参数3";
		break;
	case 0x0085:
		s = "延迟一段时间，参数1";
		break;
	case 0x0086:
		s = "如果未装备指定物品则跳过，参数1 物品，参数3 地址";
		break;
	case 0x0087:
		s = "画对象";
		break;
	case 0x0088:
		s = "根据所剩金钱设置基础伤害值";
		break;
	case 0x0089:
		s = "设置战斗结果，参数1";
		break;
	case 0x008a:
		s = "设置下一场战斗为自动战斗";
		break;
	case 0x008b:
		s = "改变调色版，参数1";
		break;
	case 0x008c:
		s = "淡入到颜色，参数1 颜色，参数2 延时，参数3";
		break;
	case 0x008d:
		s = "升级，参数1 增加的等级";
		break;
	case 0x008f:
		s = "金钱减半指令";
		break;
	case 0x0090:
		s = "设置对象脚本，参数1 对象，参数3 偏移，参数2 值";
		break;
	case 0x0091:
		s = "如果敌方不是孤单，则跳转 ，参数1 地址";
		break;
	case 0x0092:
		s = "显示玩家魔法动画，参数1 魔法";
		break;
	case 0x0093:
		s = "褪色的场景。更新过程中的场景";
		break;
	case 0x0094:
		s = "如果事件对象的状态是指定的，则跳转,参数1 对象，参数2 状态，参数3 地址";
		break;
	case 0x0095:
		s = "如果当前场景等于参数1，则跳转,参数2 ";
		break;
	case 0x0096:
		s = "播放结束动画";
		break;
	case 0x0097:
		s = "将指定对象最快移动到指定位置 参数1 X，参数2 Y，参数3 H";
		break;
	case 0x0098:
		s = "设置跟随对象,参数1对象 >= 0 设置跟随，0 取消";
		break;
	case 0x0099:
		s = "更改指定场景的映射,参数1 = 0xffff 当前场景，否则参数1指定，参数2 场景";
		break;
	case 0x009A:
		s = "为多个对象设置状态，参数1 到参数2，设置为参数3";
		break;
	case 0x009b:
		s = "淡入当前场景";
		break;
	case 0x009c:
		s = "怪物复制自身,失败跳转到 参数2";
		break;
	case 0x009e:
		s = "怪物召唤，参数1 召唤的怪物ID，参数2 数量，参数3 失败跳转";
		break;
	case 0x009f:
		s = "怪物变身，参数1 要变的怪物ID";
		break;
	case 0x00a0:
		s = "离开游戏";
		break;
	case 0x00a1:
		s = "设置所有成员位置与第一名一样";
		break;
	case 0x00a2:
		s = "随机跳转到之后的第1 到参数1条 指令之一";
		break;
	case 0x00a3:
		s = "播放音乐，参数1，参数2";
		break;
	case 0x00a4:
		s = "Scroll FBP to the screen，参数1，参数2，参数3";
		break;
	case 0x00a5:
		s = "Show FBP picture with sprite ，参数1，参数2，参数3";
		break;
	case 0x00a6:
		s = "备份场景";
		break;
	case 0x00a7:
		s = "WIN95版，对象说明前缀";
		break;
	case 0x00b0:
		s = "消灭水魔兽";
		break;
	case 0x00b1:
		s = "结尾动画";
		break;
	case 0x00b2:
		s = "穿越重新开始";
		break;
	case 0x0100:
		s = "增加队伍中的人员";
		break;
	case 0x0101:
		s = "移除我方人员的装备额外效果";
		break;
	case 0x0102:
		s = "增加我方成员到3人以上，参数1 标志，按位或";
		break;
	case 0x0103:
		s = "队伍中没有指定角色则跳转，参数1 角色，参数2 跳转目标";
		break;
	default:
		s = "错误入口";
		break;
	}
	return s;
}



LPCWSTR  FileNames[] = {
	L"SSS.MKF",
	L"DATA.MKF",
	L"M.MSG",
	L"WORD.DAT",
	L"DESC.DAT",
	L"WOR16.ASC",
	L"GOP.MKF",
	L"map.MKF",
	nullptr,
};

// CEditorAppDlg 对话框



CEditorAppDlg::CEditorAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_APP_MAIN_DIALOG, pParent)
	,Pal(nullptr)
	,JobCtrl(0)
	,WorkCtrl(0)
	,okExit(0)
	,pTestData(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CEditorAppDlg::~CEditorAppDlg()
{
	if (pTestData)
		delete pTestData;
}

void CEditorAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_Edit);
	DDX_Control(pDX, IDC_TREE1, m_Tree);
	DDX_Control(pDX, IDCANCEL, m_ButtonCancel);
	DDX_Control(pDX, IDOK, m_ButtonOK);
	DDX_Control(pDX, IDC_GRID, m_Grid);
	DDX_Control(pDX, IDC_LIST4, m_ListCtrl);

	//DDX_Control(pDX, IDC_STATIC1, CPal_Main);
}

BEGIN_MESSAGE_MAP(CEditorAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(WM_MapEdit_Map, &CEditorAppDlg::Edit_The_Map)
	ON_MESSAGE(WM_Deploy_On_The_Map, &CEditorAppDlg::OnDeployOnTheMap)
	ON_MESSAGE(WM_Test_Run, &CEditorAppDlg::OnTestRun)

	ON_MESSAGE(WM_POST_MESSAGE_SCRIPT, &CEditorAppDlg::OnPostMessageScript)
	ON_MESSAGE(WM_POST_MESSAGE_CSTR, &CEditorAppDlg::Text_Message)
	ON_MESSAGE(WM_ListScriptEntry, &CEditorAppDlg::OnListscriptentry)
	ON_MESSAGE(WM_ScriptCaller, &CEditorAppDlg::OnScriptCaller)

	/*
	ON_MESSAGE(WM_EDIT_DIR, &CEditorAppDlg::Edit_Dir)
	ON_MESSAGE(WM_EDIT_DEFALSE, &CEditorAppDlg::Edit_Defalse)
	ON_MESSAGE(WM_EDIT_1, &CEditorAppDlg::Edit_1)

	ON_MESSAGE(WM_EDIT_PLAYER, &CEditorAppDlg::Edit_Player)
	ON_MESSAGE(WM_EDIT_ENEMY, &CEditorAppDlg::Edit_Enemy)
	ON_MESSAGE(WM_EDIT_MAGIC, &CEditorAppDlg::Edit_Magic)
	ON_MESSAGE(WM_EditEventObject, &CEditorAppDlg::Edit_EventObject)
	ON_MESSAGE(WM_PLAYERLEVELUPMAGIG, &CEditorAppDlg::Edit_PlayerLevelUPMagic)
	ON_MESSAGE(WM_EditBattleField, &CEditorAppDlg::Edit_BattleField)
	ON_MESSAGE(WM_EditStore, &CEditorAppDlg::Edit_Store)
	ON_MESSAGE(WM_EditPoison, &CEditorAppDlg::Edit_Poison)
	ON_MESSAGE(WM_EditScene, &CEditorAppDlg::Edit_Scene)
	ON_MESSAGE(WM_EditObjectItem, &CEditorAppDlg::Edit_ObjectItem)
	ON_MESSAGE(WM_EditEnemyTeam, &CEditorAppDlg::Edit_EnemyTeam)
	ON_MESSAGE(WM_PACKPAL, &CEditorAppDlg::PackPal)
	ON_MESSAGE(WM_UNPACKPAL, &CEditorAppDlg::UnPackPal)
	ON_MESSAGE(WM_ScriptEntry, &CEditorAppDlg::Edit_ScriptEntry)
	ON_MESSAGE(WM_Inventory, &CEditorAppDlg::Edit_Invenyory)
	ON_MESSAGE(WM_OKReturn, &CEditorAppDlg::OK_Return)
	ON_MESSAGE(WM_CanselReturn, &CEditorAppDlg::Cansel_Return)
	ON_MESSAGE(WM_SEND_MSG_STR, &CEditorAppDlg::OnSendMsgStr)
	ON_MESSAGE(WM_EDIT_PARAMETERS, &CEditorAppDlg::Edit_Parameters)
	ON_MESSAGE(WM_MapEdit, &CEditorAppDlg::Edit_Map)
	ON_MESSAGE(WM_EditExplain, &CEditorAppDlg::Edit_Explain)
	ON_MESSAGE(WM_Test_Eentry_Edit, &CEditorAppDlg::OnTestEentryEdit)
	ON_MESSAGE(WM_EditObjectName, &CEditorAppDlg::Edit_ObjectName)
	ON_MESSAGE(WM_EditDialog, &CEditorAppDlg::Edit_Dialog)
	ON_MESSAGE(WM_EditObjectPict, &CEditorAppDlg::Edit_ObjectPict)
	ON_MESSAGE(WM_EditBettlePict, &CEditorAppDlg::Edit_BettlePict)

	ON_MESSAGE(WM_PAL_OPTIONS, &CEditorAppDlg::Edit_Pal_Options)
	ON_MESSAGE(WM_PAL_FONT_FIBRARY_NAME, &CEditorAppDlg::Edit_PalBibrary_name)
	*/
	ON_EN_CHANGE(IDC_EDIT1, &CEditorAppDlg::OnEnChangeEdit1)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, &CEditorAppDlg::OnTvnSelchangedTree1)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDOK, &CEditorAppDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CEditorAppDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_WM_KEYDOWN()
//	ON_NOTIFY(NM_CLICK, IDC_TREE1, &CEditorAppDlg::OnClickTree1)
END_MESSAGE_MAP()


// CEditorAppDlg 消息处理程序
void CEditorAppDlg::MessageText(LPCTSTR text)
{
	m_Edit.SetLimitText(60000);
	m_Edit.ShowWindow(TRUE);
	m_Edit.ShowWindow(SW_SHOW);
	CString s = text;
	s +=  _T("\r\n");
	int  nLens = m_Edit.GetWindowTextLength();
	if (nLens > 59900)
	{
		m_Edit.SetSel(0, 20000);
		m_Edit.ReplaceSel(_T(""));
	}
	m_Edit.SetSel(nLens, nLens);
	m_Edit.SetFocus();
	m_Edit.ReplaceSel(s);
	::PostMessage(m_ListCtrl.m_hWnd, WM_KILLFOCUS, -1, 0);

}


LRESULT CEditorAppDlg::Text_Message(WPARAM w_wparam, LPARAM w_lparam)
{
	//处理异步信息
	//WM_POST_MESSAGE_CSTR
	CString* p = (CString *)w_lparam;
	MessageText((p->GetBuffer()));
	delete p;
	return 0;
}


void  CEditorAppDlg::Set_Defalse_Tree()
{
	//
	HTREEITEM root, res, nod;
	m_Grid.ShowWindow(FALSE);
	//m_Tree.SetExtendedStyle(0, m_Tree.GetExStyle() | TVS_HASLINES);
	m_Tree.GetStyle();
	WorkCtrl = -1;
	//删除树结点
	m_Tree.DeleteAllItems();
	//取得根结点
	root = m_Tree.InsertItem(_T("初始属性"));
	//插入二级结点
	res = m_Tree.InsertItem(_T("基本属性"), 1, 1, root);

	nod = m_Tree.InsertItem(_T("我方属性"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EDIT_PLAYER);
	nod = m_Tree.InsertItem(_T("敌方属性"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EDIT_ENEMY);
	nod = m_Tree.InsertItem(_T("敌方队伍"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditEnemyTeam);
	nod = m_Tree.InsertItem(_T("魔法"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EDIT_MAGIC);
	nod = m_Tree.InsertItem(_T("物品"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditObjectItem);
	nod = m_Tree.InsertItem(_T("毒药"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditPoison);
	nod = m_Tree.InsertItem(_T("升级魔法"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_PLAYERLEVELUPMAGIG);
	nod = m_Tree.InsertItem(_T("战斗场所"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditBattleField);
	nod = m_Tree.InsertItem(_T("事件对象"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditEventObject);
	nod = m_Tree.InsertItem(_T("场景"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditScene);
	nod = m_Tree.InsertItem(_T("商店"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditStore);
	nod = m_Tree.InsertItem(_T("对象名称编辑"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditObjectName);
	nod = m_Tree.InsertItem(_T("对话编辑"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditDialog);
	if (!Pal->PAL_IsPalWIN())
	{
		nod = m_Tree.InsertItem(_T("对象说明编辑"), 2, 2, res);
		m_Tree.SetItemData(nod, WM_EditExplain);
	}
	nod = m_Tree.InsertItem(_T("对象图像编辑"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditObjectPict);
	nod = m_Tree.InsertItem(_T("脚本入口"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_ScriptEntry);
	nod = m_Tree.InsertItem(_T("地图编辑"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_MapEdit);

	//nod = m_Tree.InsertItem(_T("打包"), 2, 2, res);
	//m_Tree.SetItemData(nod, WM_PACKPAL);
	//nod = m_Tree.InsertItem(_T("解包"), 2, 2, res);
	//m_Tree.SetItemData(nod, WM_UNPACKPAL);
	m_Tree.InsertItem(_T("--------"), 2, 2, res);
	nod = m_Tree.InsertItem(L"修改游戏设置", 2, 2, res);
	m_Tree.SetItemData(nod, WM_PAL_OPTIONS);
	m_Tree.InsertItem(_T("--------"), 2, 2, res);

	nod = m_Tree.InsertItem(_T("存盘返回"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_OKReturn);
	nod = m_Tree.InsertItem(_T("取消返回"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_CanselReturn);

	//展开树
	m_Tree.Expand(res, TVE_EXPAND);
	m_Tree.Expand(root, TVE_EXPAND);

}

//数据更新
void CEditorAppDlg::DataUpDate(int Work)
{
	if (Work <= 0) return;
	DataArray &m_DataArray = m_Grid.GetDataClass();
	DWORD nsize = m_DataArray.size();
	GAMEDATA* p = &Pal->gpGlobals->g;

	// 检查尺寸变化
	switch (Work)
	{
	case WM_EditEnemyTeam:
	{
		if (nsize > p->nEnemyTeam)
		{
			p->lprgEnemyTeam = (LPENEMYTEAM)realloc(p->lprgEnemyTeam, nsize * sizeof(ENEMYTEAM));
			if (p->lprgEnemyTeam == nullptr) exit(11);
			p->nEnemyTeam = nsize;
		}
		break;
	}
	case WM_EditBattleField:
	{
		if (nsize > p->nBattleField)
		{
			p->lprgBattleField = (LPBATTLEFIELD)realloc(p->lprgBattleField, nsize * sizeof(BATTLEFIELD));
			if (p->lprgBattleField == nullptr)exit(11);
			p->nBattleField = nsize;
		}
		break;
	}
	case WM_PLAYERLEVELUPMAGIG://升级魔法
	{
		if (nsize > p->nLevelUpMagic)
		{
			p->lprgLevelUpMagic = (LPLEVELUPMAGIC_ALL)realloc(p->lprgLevelUpMagic, nsize * sizeof(LEVELUPMAGIC_ALL));
			if (p->lprgLevelUpMagic == nullptr)exit(11);
			p->nLevelUpMagic = nsize;
		}
		break;
	}
	case WM_EditStore://商店编辑
	{
		if (nsize > p->nStore)
		{
			p->lprgStore = (LPSTORE)realloc(p->lprgStore, nsize * sizeof(STORE));
			if (p->lprgStore == nullptr)exit(11);
			p->nStore = nsize;
		}
		break;
	}
	case WM_EditEventObject:  //事件对象
	{
		if (nsize != p->nEventObject)
		{
			p->lprgEventObject = (LPEVENTOBJECT)realloc(p->lprgEventObject, nsize * sizeof(EVENTOBJECT));
			if (p->lprgEventObject == nullptr)exit(11);
			p->nEventObject = nsize;
		}
		break;
	}
	case WM_ScriptEntry://脚本入口
	{
		if (nsize > p->nScriptEntry)
		{
			p->lprgScriptEntry =
				(LPSCRIPTENTRY)realloc(p->lprgScriptEntry, nsize * sizeof(SCRIPTENTRY));
			if (p->lprgScriptEntry == nullptr)exit(11);
			p->nScriptEntry = nsize;
		}
		//以下进行数据存储
		//将存档文件载入至内存变量中
		Pal->loadSaveFile();
		//Pal->PAL_SaveGame(1, 1);
		for (DWORD r = 0; r < nsize; r++)
		{
			ASSERT(get<int>(m_DataArray[r].Col[0]) == (int)r);
			p->lprgScriptEntry[r].wOperation = get<int>(m_DataArray[r].Col[1]);
			p->lprgScriptEntry[r].rgwOperand[0] = get<int>(m_DataArray[r].Col[2]);
			p->lprgScriptEntry[r].rgwOperand[1] = get<int>(m_DataArray[r].Col[3]);
			p->lprgScriptEntry[r].rgwOperand[2] = get<int>(m_DataArray[r].Col[4]);
		}
		for (DWORD r = 0; r < nsize; r++)
			if (m_DataArray[r].oldRow != r + 1 && m_DataArray[r].oldRow)
			{
				//行号已经变动
				//单个脚本变动调整
				//Single script changes
				if (Pal->SingleScriptChange(m_DataArray[r].oldRow - 1, m_DataArray[r].oldRow - 1, r))
				{
					//失败恢复原数据
					JobCtrl = 0;
					delete Pal;
					Pal = new CGetPalData(0, 1);
					return;
				}
			}

		//完成、存储修改后的数据
		Pal->PAL_Make_MKF();
		//存储存档文件
		Pal->saveSaveFile();

		JobCtrl = 0;
		delete Pal;
		Pal = new CGetPalData(0, 1);
		return;
		break;
	}
	case WM_EditDialog://对话编辑
	{
		string dir = CPalEvent::PalDir;
		dir += "m.msg";
		string dirNew = dir + ".new";
		FILE* f = fopen(dirNew.c_str(), "w");
		if (f == nullptr)
			return;
		int ret{};

		if (nsize != Pal->g_TextLib.nMsgs)
			Pal->g_TextLib.lpMsgOffset =(LPDWORD)realloc(Pal->g_TextLib.lpMsgOffset,(nsize + 1) * sizeof(DWORD));
		if (Pal->g_TextLib.lpMsgOffset == nullptr)exit(11);
		Pal->g_TextLib.nMsgs = nsize;
		for (int r = 0; r < nsize; r++)
		{
			string p = std::get<string>(m_DataArray[r].Col[3]);
			//去除尾部空格
			p.erase(p.find_last_not_of(" ") + 1);
			//string s1 = p;
			Pal->Utf8ToSys(p);
			fputs(p.c_str(), f);
			fputs("\0\0", f);
			//fwrite(p.c_str(), 1, strlen(p.c_str()), f);
			Pal->g_TextLib.lpMsgOffset[r + 1] = ftell(f);
		}
		fclose(f);

		ret = DeleteFileA(dir.c_str());
		ret = MoveFileA(dirNew.c_str(), dir.c_str());
		ret = DeleteFileA(dirNew.c_str());
		//Pal->PAL_Make_MKF();
		return;
	}
	case WM_EditExplain://说明编辑
	{
		string dir = CPalEvent::PalDir;
		dir += "desc.dat";
		string dirNew = dir + ".new";
		FILE* f = fopen(dirNew.c_str(), "w");
		if (f == nullptr)
			return;
		int ret{};

		for (int r = 0; r < nsize; r++)
		{
			string p = std::get<string>(m_DataArray[r].Col[2]);
			//去除尾部空格
			p.erase(p.find_last_not_of(" ") + 1);
			if (p.empty())
				continue;
			CStringA cs;
			cs.Format("%3.3x(%s)=%s\r\n", r,
				Pal->PAL_GetWord(r), p.c_str());
			p = cs.GetString();
			Pal->Utf8ToSys(p);
			fputs(p.c_str(), f);
		}
		fclose(f);

		ret = DeleteFileA(dir.c_str());
		ret = MoveFileA(dirNew.c_str(), dir.c_str());
		ret = DeleteFileA(dirNew.c_str());
		//Pal->PAL_Make_MKF();
		return;
		break;
	}
	case WM_EditScene:
		break;
	default:
		break;
	}

	//更新行数据
	for (DWORD r = 0; r < nsize; r++)
	{
		if (m_DataArray[r].Col.size() == 0)
			continue;
		switch (Work)
		{
		case WM_EDIT_PLAYER://我方人物
		{
			WORD* p_list = (WORD*)&(Pal->gpGlobals->g.PlayerRoles);
			for (int n = 1, i; n < 61; n++)
			{
				if (n == 1)
					i = n + 2;//名称
				else if (n > 1 && n <= 6)
					i = n + 4;//等级-真气
				else if (n > 6 && n <= 20)
					i = n + 10;//武力--巫抗
				else if (n > 20 && n < 27)
					i = n - 10;//装备
				else if (n == 27)
					i = 0x1f;//救援
				else if (n == 28)
				{
					i = 0x41;//合体法术
					WORD obj = std::get<int>(m_DataArray[r].Col[n]);
					if (obj && (obj >= MAX_OBJECTS ||
						Pal->gpObject_classify[obj] != kIsMagic))
					{
						::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T("作废修改"), _T("输入错"), MB_OK);
						return;
					}
				}
				else if (n > 28 && n < 61)
				{
					i = n + 3;//魔法
					WORD obj = (WORD)std::get<int>(m_DataArray[r].Col[n]);
					if (obj && (obj >= MAX_OBJECTS ||
						Pal->gpObject_classify[obj] != kIsMagic))
					{
						CString str = L"";
						str += Pal->PAL_GetWord(obj);
						MessageText(str);
						::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T("作废修改"), _T("输入错"), MB_OK);
						return;
					}
				}
				(WORD)p_list[(i)*MAX_PLAYER_ROLES + r] = std::get<int>(m_DataArray[r].Col[n]);

			}

			WORD s1 = 0, s2 = 0;
			s1 = std::get<int>(m_DataArray[r].Col[61]);
			s2 = std::get<int>(m_DataArray[r].Col[62]);
			WORD name = ((Pal->gpGlobals->g.PlayerRoles)).rgwName[r];
			((LPOBJECT)Pal->gpGlobals->g.rgObject)[name].player.wScriptOnFriendDeath = s1;
			((LPOBJECT)Pal->gpGlobals->g.rgObject)[name].player.wScriptOnDying = s2;
			break;
		}
		case WM_EDIT_ENEMY://怪物
		{
			if (r >= (WORD)Pal->gpGlobals->g.nEnemy)
				break;
			WORD wEnemyID = get<int>(m_DataArray[r].Col[41]);
			WORD wOenemyID = get<int>(m_DataArray[r].Col[0]);
			LPENEMY  p_enemy = &(Pal->gpGlobals->g.lprgEnemy[wEnemyID]);//ID
			WORD* p_list = &p_enemy->wIdleFrames;
			for (int n = 2; n < 37; n++)
			{
				(WORD)p_list[(n - 2)] = get<int>(m_DataArray[r].Col[n]);
			}
			//LPWORD pWord;
			LPOBJECT lpObj = (LPOBJECT)Pal->gpGlobals->g.rgObject;
			lpObj[wOenemyID].enemy.wResistanceToSorcery = get<int>(m_DataArray[r].Col[37]);
			break;
		}
		case WM_EDIT_MAGIC://魔法
		{
			if (r >= (WORD)Pal->nMagic) break;
			WORD wMagicID = get<int>(m_DataArray[r].Col[22]);
			WORD wOMagicID = get<int>(m_DataArray[r].Col[1]);
			LPMAGIC  p_Magic = &(Pal->gpGlobals->g.lprgMagic[wMagicID]);//ID
			WORD* p_list = &p_Magic->wEffect;
			for (int n = 2; n < 18; n++)
			{
				(WORD)p_list[(n - 2)] = get<int>(m_DataArray[r].Col[n]);
			}
			((LPOBJECT)Pal->gpGlobals->g.rgObject)[wOMagicID].magic.wFlags = get<int>(m_DataArray[r].Col[18]);
			((LPOBJECT)Pal->gpGlobals->g.rgObject)[wOMagicID].magic.wScriptOnUse = get<int>(m_DataArray[r].Col[19]);
			((LPOBJECT)Pal->gpGlobals->g.rgObject)[wOMagicID].magic.wScriptOnSuccess = get<int>(m_DataArray[r].Col[20]);
			break;
		}
		case WM_EditBattleField://战斗场所
		{
			for (int k = 0; k < NUM_MAGIC_ELEMENTAL; k++)
			{
				p->lprgBattleField[r].rgsMagicEffect[k] = 0xffff & get<int>(m_DataArray[r].Col[k + 1]);
			}
			p->lprgBattleField[r].wScreenWave = 0xffff & get<int>(m_DataArray[r].Col[6]);
			break;
		}
		case WM_EditEventObject:  //事件对象
		{
			//
			LPWORD d = (LPWORD)(&p->lprgEventObject[r]);
			int scene = get<int>(m_DataArray[r].Col[1]) - 1;
			for (int n = 0; n < 16; n++)
			{
				d[n] = get<int>(m_DataArray[r].Col[n + 2]) & 0xffff;
			}
			break;
		}
		case WM_EditStore://商店
		{
			for (size_t n_Row = 0; n_Row < nsize; n_Row++)
			{
				for (int n_Item = 0; n_Item < MAX_STORE_ITEM; n_Item++)
				{
					p->lprgStore[n_Row].rgwItems[n_Item] = get<int>(m_DataArray[n_Row].Col[n_Item + 2]);
				}
			}
			break;
		}
		case WM_EditObjectItem://物品
		{
			int j = get<int>(m_DataArray[r].Col[0]);
			assert(Pal->gpObject_classify[j] == kIsItem);
			((LPOBJECT)p->rgObject)[j].item.wPrice = get<int>(m_DataArray[r].Col[3]);//
			((LPOBJECT)p->rgObject)[j].item.wBitmap = get<int>(m_DataArray[r].Col[19]);//
			WORD wFlags = 0;
			for (int n = 7; n < 19; n++)
				wFlags |= get<int>(m_DataArray[r].Col[n]) << (n - 7);
			//assert(p->rgObject[j].item.wFlags == wFlags);
			((LPOBJECT)p->rgObject)[j].item.wFlags = wFlags;
			break;
		}
		case WM_EditEnemyTeam://敌方队伍
			for (int n = 0; n < MAX_ENEMIES_IN_TEAM; n++)
			{
				WORD k_Enemy = get<int>(m_DataArray[r].Col[n + 1]);
				if (!(k_Enemy == 0 || k_Enemy == 0xffff ||
					(k_Enemy < MAX_OBJECTS && Pal->gpObject_classify[k_Enemy] == kIsEnemy)))
				{
					::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T("作废修改"), _T("输入错"), MB_OK);
					return;
				}
				p->lprgEnemyTeam[r].rgwEnemy[n] = k_Enemy;
			}
			break;
		case WM_PLAYERLEVELUPMAGIG://升级魔法
		{
			GAMEDATA* p = &Pal->gpGlobals->g;
			for (int r = 0; r < p->nLevelUpMagic; r++)
			{
				for (int j = 0; j < MAX_PLAYABLE_PLAYER_ROLES; j++)
				{
					p->lprgLevelUpMagic[r].m[j].wLevel = get<int>(m_DataArray[r].Col[j * 2 + 1]);
					p->lprgLevelUpMagic[r].m[j].wMagic = get<int>(m_DataArray[r].Col[j * 2 + 2]);
				}
			}
			break;
		}
		case WM_EditPoison://毒药编辑
		{
			int j = get<int>(m_DataArray[r].Col[0]);
			assert(Pal->gpObject_classify[j] == kIsPoison);
			((LPOBJECT)p->rgObject)[j].poison.wPoisonLevel = get<int>(m_DataArray[r].Col[2]);//
			((LPOBJECT)p->rgObject)[j].poison.wColor = get<int>(m_DataArray[r].Col[3]);//
			((LPOBJECT)p->rgObject)[j].poison.wPlayerScript = get<int>(m_DataArray[r].Col[4]);//
			((LPOBJECT)p->rgObject)[j].poison.wEnemyScript = get<int>(m_DataArray[r].Col[5]);//
			break;
		}
		case WM_ScriptEntry://脚本入口
		{
			break;
		}
		case WM_EDIT_PARAMETERS://基本属性编辑
		{
			Pal->gpGlobals->dwCash = get<int>(m_DataArray[0].Col[1]);
			Pal->gpGlobals->wCollectValue = get<int>(m_DataArray[1].Col[1]);
			break;
		}
		case WM_EditObjectName://对象名称编辑
		{
			string s = get<string>(m_DataArray[r].Col[1]);
			if (s == Pal->PAL_GetWord(r))
				break;
			//
			Pal->Utf8ToSys(s);
			s += "           ";
			memcpy(Pal->g_TextLib.lpWordBuf + r * 10, s.c_str(), 10);
			s = Pal->PAL_GetWord(r);
			break;
		}
		case WM_EditExplain://说明编辑
		{
			break;
		}
		case WM_EditScene://场景编辑
		{
			auto sence = Pal->gpGlobals->g.rgScene;
			sence->wScriptOnEnter = get<int>(m_DataArray[r].Col[1]);
			sence->wScriptOnTeleport = get<int>(m_DataArray[r].Col[2]);
			sence->wEventObjectIndex = get<int>(m_DataArray[r].Col[3]);
			sence->wMapNum = get<int>(m_DataArray[r].Col[5]);
			break;
		}
		case WM_Inventory://背包编辑
		{
			DWORD j = r;
			WORD wItem = get<int>(m_DataArray[r].Col[1]);

			if (wItem && (wItem >= MAX_OBJECTS ||
				(Pal->gpObject_classify[wItem] != kIsItem) &&
				Pal->gpObject_classify[wItem] != kIsPoison))
			{
				::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T("作废修改"), _T("输入错"), MB_OK);
				return;
			}

			//if (Pal->gpGlobals->rgInventory[j].wItem != wItem && wItem)

			Pal->gpGlobals->rgInventory[j].wItem = wItem;
			Pal->gpGlobals->rgInventory[j].nAmount = get<int>(m_DataArray[r].Col[2]);
			break;
		}
		case WM_Test_Eentry_Edit:
		{
			for (int n = 0; n < t_End; n++)
			{
				pTestData->sTestData[r][n] = get<int>(m_DataArray[r].Col[n]);
			}
			break;
		}
		case WM_PAL_OPTIONS:
		{
			Pal->gConfig->m_Function_Set[r] = get<int>(m_DataArray[r].Col[1]);
			break;
		}
		default:
			break;
		}
	}

	//更新后的存储
	switch (Work)
	{
	case WM_Inventory:
	{
		if (MAX_INVENTORY < nsize)
		{
			Pal->gpGlobals->rgInventory[nsize].wItem = 0 ;
			Pal->gpGlobals->rgInventory[nsize].nAmount = 0;
		}

		break;
	}
	case WM_Test_Eentry_Edit:
	{
		break;
	}
	case WM_ScriptEntry://脚本入口
	{
		break;
	}

	case WM_EditEventObject:  //事件对象
	{
		MAPScript st;//生成新旧行号对照表
		for (int k = 0; k < nsize; k++)
		{
			st[m_DataArray[k].oldRow] = get<int>(m_DataArray[k].Col[0]) + 1;
		}
		//更新脚本
		for (int n = 0; n < Pal->gpGlobals->g.nScriptEntry; n++)
		{
			LPWORD p1{ 0 }, p2{ 0 };
			switch (Pal->gpGlobals->g.lprgScriptEntry[n].wOperation)
			{
			case 0x0012://设置对象到相对于队伍的位置 参数1对象,参数2 X，参数3 Y
			case 0x0013://设置对象到指定的位置 参数1对象, 参数2 X，参数3 Y
			case 0x0016://设置对象的，方向和（ 手势），参数1对象不为0 ，参数2，方向，参数3，形象
			case 0x0024://设置对象自动脚本地址，参数1 对象不等于0 参数2 地址
			case 0x0025://设置对象触发脚本地址，参数1 对象不等于0 参数2 地址
			case 0x0040 ://设置对象触发模式 如参数1对象 ！= 0 ，参数2 设置
			case 0x0049 ://设置对象状态，参数2 状态
			case 0x006c://參數1 !=0，0xffff 對象  NPC走一步，参数2 X，参数3 Y
			case 0x006f ://将当前事件对象状态与·另一个事件对象同步
			case 0x007d ://移动对象位置，参数1对象 参数2 X，参数3 Y
			case 0x007e ://设置对象的层，参数1对象，参数2
			case 0x0081 ://跳过，如果没有面对对象，参数1 对象，参数2 改变触发方式，参数3地址
			case 0x0083 ://如果事件对象不在当前事件对象的指定区域，则跳过,参数1对象，参数3地址
			case 0x0084 ://将玩家用作事件对象的物品放置到场景中,参数1,位置，，失败跳到参数3
			case 0x0094 ://如果事件对象的状态是指定的，则跳转,参数1 对象，参数2 状态，参数3 地址
				p1 = &Pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[0];
				break;
			case 0x009A ://为多个对象设置状态，参数1 到参数2，设置为参数3
				p1 = &Pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[0];
				p2 = &Pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[1];
				break;
			case 0x0004:
				p2 = &Pal->gpGlobals->g.lprgScriptEntry[n].rgwOperand[1];
				break;
			default:
				break;
			}
			if (p1 && *p1!=0xffff && *p1 != 0)
			{
				MAPScript::iterator pw;
				pw = st.find(*p1);
				if (pw == st.end())
				{
					//没有找到出错
					ShowMsg("数据错，原有对象 %4.4X 被删除", *p1);
					return;
					break;
				}
				if (pw->first != pw->second)
				{
					//新旧入口不一致，需要修改，用新入口替换旧入口
					*p1 = pw->second;
				}
			}
			if (p2 && *p2)
			{
				MAPScript::iterator pw;
				pw = st.find(*p2);
				ASSERT(pw != st.end());
				if (pw->first != pw->second)
				{
					//新旧入口不一致，需要修改，用新入口替换旧入口
					*p2 = pw->second;
				}
			}
		}
		//更新场景表
		for (int n = 0; n < MAX_SCENES; n++)
		{
			WORD *p = &Pal->gpGlobals->g.rgScene[n].wEventObjectIndex;
			if (!*p) continue;
			MAPScript::iterator pw = st.find(*p);
			ASSERT(pw != st.end());
			if (pw->first != pw->second)
			{
				//新旧入口不一致，需要修改，用新入口替换旧入口
				*p = pw->second;
			}
		}
		//修改存档文件 的事件对象
		//修改存档文件
		Pal->loadSaveFile();

		for (int n = 0; n < 10; n++)
		{
			if ((Pal->pSaveData[n].size()) == 0)
				continue;
			int sIns = 0, sDel = 0;
			int chage = m_DataArray.size() - Pal->gpGlobals->g.nEventObject;
			for (int r = 0; r < m_DataArray.size(); r++)
			{
				if (m_DataArray[r].oldRow)
				{
					int oldRow = m_DataArray[r].oldRow;
					if (oldRow + sIns - sDel > r + 1)
					{
						int nDel = oldRow + sIns - sDel - r - 1;
						//刪除之後的nDel行
						size_t pos = Pal->getSaveFileOffset(1) + sizeof(EVENTOBJECT) * r;
						Pal->pSaveData[n].erase(Pal->pSaveData[n].begin() + pos,
							Pal->pSaveData[n].begin() + pos + sizeof(EVENTOBJECT) * nDel);
						sDel += nDel;
					}
					continue;
				}
				//新行，插入
				PDATABUF p(sizeof(EVENTOBJECT));
				//拷贝需插入的数据
				memcpy(&p[0], &Pal->gpGlobals->g.lprgEventObject[r], p.size());
				//计算插入位置
				size_t pos = Pal->getSaveFileOffset(1) + sizeof(EVENTOBJECT) * r;
				//对象区起始位置newLen - Pal->gpGlobals->g.nEventObject * sizeof(EVENTOBJECT)
				Pal->pSaveData[n].insert(Pal->pSaveData[n].begin() + pos, p.begin(), p.end());
				sIns++;
			}
			int savefilelen = Pal->getSaveFileLen();
			ASSERT(Pal->pSaveData[n].size() == savefilelen);
			//更新对象位置数据
			//更新场景数据
			LPSCENE p = Pal->getSecensPoint(&Pal->pSaveData[n][0]);
			for (int k = 0; k < MAX_SCENES; k++)
			{
				p[k].wEventObjectIndex = Pal->gpGlobals->g.rgScene[k].wEventObjectIndex;
			}
		}
		//完成、存储修改后的数据
		Pal->PAL_Make_MKF();
		//存储存档文件
		Pal->saveSaveFile();

		JobCtrl = 0;
		delete Pal;
		Pal = new CGetPalData(0, 1);
		return;
		break;
	}
	case WM_EditObjectName://对象名称编辑
	{
		string dir = CPalEvent::PalDir;
		dir += "WORD.DAT";
		string dirbak  = dir + ".BAK";
		FILE * f = fopen(dirbak.c_str(), "wb");
		if (f == nullptr)
			return;
		int ret = fwrite(Pal->g_TextLib.lpWordBuf,10, Pal->g_TextLib.nWords, f);
		fclose(f);
		ret = DeleteFileA(dir.c_str());
		ret = MoveFileA(dirbak.c_str(), dir.c_str());
		break;
	}
	case WM_EditDialog://对话编辑
	{
		break;
	}
	case WM_EditExplain://说明编辑
	{
		//
		break;
	}
	default:
		break;
	}

}


//脚本入口编辑
LRESULT CEditorAppDlg::Edit_ScriptEntry(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);
	WorkCtrl = WM_ScriptEntry;
	m_Grid.UndoCtrl = WM_ScriptEntry;
	CString s;
	s.Format(_T("脚本入口编辑"));
	MessageText(s);
	ColArray w_ColData;
	w_ColData.resize(9);
	// 制做表头
	// 参数：文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	w_ColData[0].GetData("脚本ID", 80, 0, ctrl_Fix,  tHEX);
	w_ColData[1].GetData("入口",   80, 1, ctrl_Edit, tHEX);
	w_ColData[2].GetData("参数1",  80, 2, ctrl_Edit, tHEX);
	w_ColData[3].GetData("参数2",  80, 3, ctrl_Edit, tHEX);
	w_ColData[4].GetData("参数3",  80, 4, ctrl_Edit, tHEX);
	w_ColData[5].GetData("调 用",  80, 5, ctrl_Null, tINT, 0, 0, 0, 2);//查看脚本调用

	SrcToStr obj = p_Script;

	w_ColData[6].GetData("行号", 70, 6, ctrl_Null, tWORD);
	w_ColData[7].GetData("原行号", 70, 7, ctrl_Null, tWORD);
	w_ColData[8].GetData("说明", 1500, 8, ctrl_Null, tINT, nullptr, obj);

	m_Grid.SetColClass(9, w_ColData);
	//标记脚本有效性
	Pal->PAL_MarkScriptAll();

	//表数据
	int n_Row;
	GAMEDATA * p = &Pal->gpGlobals->g;
	DataArray s_RowData;
	s_RowData.resize( p->nScriptEntry);
	//以下测试脚本有效性

	for (n_Row = 0; n_Row < p->nScriptEntry; n_Row++)
	{
			s_RowData[n_Row].Col.resize(9);
			s_RowData[n_Row].oldRow = n_Row + 1;
			s_RowData[n_Row].Col[0] = (int) n_Row;
			s_RowData[n_Row].Col[1] = p->lprgScriptEntry[n_Row].wOperation;
			s_RowData[n_Row].Col[2] = p->lprgScriptEntry[n_Row].rgwOperand[0];
			s_RowData[n_Row].Col[3] = p->lprgScriptEntry[n_Row].rgwOperand[1];
			s_RowData[n_Row].Col[4] = p->lprgScriptEntry[n_Row].rgwOperand[2];
			s_RowData[n_Row].Col[5] = (INT)Pal->pMark[n_Row].s.size();
			s_RowData[n_Row].Col[6] = n_Row + 1;
			s_RowData[n_Row].Col[7] = n_Row + 1;
			s_RowData[n_Row].Col[8] = p->lprgScriptEntry[n_Row].wOperation;
	}

	m_Grid.SetDataClass(n_Row, 9, s_RowData);
	m_Grid.m_popMenuFlags = 0b0111111;
	//
	SET_COL_DATA s_col_data = [](ColumnClass& s)->VOID
	{
		s.Col[6] = get<int>(s.Col[0]) + 1;
		s.Col[7] = (int)s.oldRow;
		s.Col[8] = s.Col[1];
	};
	m_Grid.Set_Set_Col_Data(s_col_data);
	return 0;
}


//敌方队伍编辑
LRESULT CEditorAppDlg::Edit_EnemyTeam(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_EditEnemyTeam;
	m_Grid.UndoCtrl = WM_EditEnemyTeam;
	CString s;
	s.Format(_T("敌方队伍"));
	MessageText(s);
	ColArray w_ColData;
	w_ColData.resize(6);
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	auto p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	pSelect_Item p_Select_Enemy = &Select_Item_Array[4];

	w_ColData[0].GetData("队伍号", 140, 0, ctrl_Fix, tWORD);
	w_ColData[1].GetData("位1名", 140, 1, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
	w_ColData[2].GetData("位2名", 140, 2, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
	w_ColData[3].GetData("位3名", 140, 3, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
	w_ColData[4].GetData("位4名", 140, 4, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
	w_ColData[5].GetData("位5名", 140, 5, ctrl_List, tNull, p_Select_Enemy, p_SrcToStr);
	m_Grid.SetColClass(6, w_ColData);

	//生成数据
	DataArray s_RowData;
	GAMEDATA * p = &Pal->gpGlobals->g;
	s_RowData.resize(p->nEnemyTeam);
	int n_EnemyTeam;
	for (n_EnemyTeam = 0; n_EnemyTeam < p->nEnemyTeam; n_EnemyTeam++)
	{
		s_RowData[n_EnemyTeam].Col.resize(6);
		s_RowData[n_EnemyTeam].oldRow = n_EnemyTeam + 1;

		s_RowData[n_EnemyTeam].Col[0] = n_EnemyTeam + 1;
		for (int n = 0; n < MAX_ENEMIES_IN_TEAM; n++)
		{
			s_RowData[n_EnemyTeam].Col[n + 1] = p->lprgEnemyTeam[n_EnemyTeam].rgwEnemy[n];
		}
	}
	m_Grid.SetDataClass(n_EnemyTeam, 6, s_RowData);
	m_Grid.m_popMenuFlags = 0b010111;

	return 0;
}


LRESULT CEditorAppDlg::Edit_BattleField(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	// WM_EditBattleField;
	//事件对象
	WorkCtrl = WM_EditBattleField;
	m_Grid.UndoCtrl = WM_EditBattleField;
	CString s;
	s.Format(_T("修改战斗场所"));
	MessageText(s);

	//制作表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）

	ColArray w_ColData;
	w_ColData.resize(7);
	w_ColData[0].GetData("序号", 90, 0, ctrl_Fix, tWORD);
	w_ColData[1].GetData("风", 90, 1, ctrl_Edit, tSHORT);
	w_ColData[2].GetData("雷", 90, 2, ctrl_Edit, tSHORT);
	w_ColData[3].GetData("水", 90, 3, ctrl_Edit, tSHORT);
	w_ColData[4].GetData("火", 90, 4, ctrl_Edit, tSHORT);
	w_ColData[5].GetData("土", 90, 5, ctrl_Edit, tSHORT);
	w_ColData[6].GetData("波动水平", 90, 6, ctrl_Edit, tWORD);
	m_Grid.SetColClass(7, w_ColData);

	//表数据
	DataArray s_RowData;
	GAMEDATA * p = &Pal->gpGlobals->g;
	s_RowData.resize(p->nBattleField );//行数
	int j, k;
	for (j = 0; j < p->nBattleField; j++)
	{
		s_RowData[j].Col.resize(7);
		s_RowData[j].Col[0] = j + 1;
		for (k = 0; k < NUM_MAGIC_ELEMENTAL; k++)
		{
			s_RowData[j].Col[k + 1] = p->lprgBattleField[j].rgsMagicEffect[k];
		}
		s_RowData[j].Col[6] = p->lprgBattleField[j].wScreenWave;
	}
	m_Grid.SetDataClass(p->nBattleField, 7, s_RowData);
	m_Grid.m_popMenuFlags = 0b010111;
	return 0;
}

LRESULT CEditorAppDlg::Edit_ObjectItem(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_EditObjectItem;
	m_Grid.UndoCtrl = WM_EditObjectItem;

	//物品
	CString s;
	s.Format(_T("修改物品"));
	MessageText(s);
	ColArray w_ColData;
	w_ColData.resize(20);
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串,6 布尔）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	SrcToStr p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	w_ColData[0].GetData("ID", 90, 0, ctrl_Fix, tHEX);
	w_ColData[1].GetData("品名", 120, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr);
	w_ColData[2].GetData("标志", 90, 2, ctrl_Edit, tHEX);
	w_ColData[3].GetData("价格", 90, 3, ctrl_Edit, tWORD);
	w_ColData[4].GetData("装备", 90, 4, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[5].GetData("投掷", 90, 5, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[6].GetData("使用", 90, 6, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[7].GetData("可使用", 90, 7, ctrl_Edit, tBOOL);
	w_ColData[8].GetData("可装备", 90, 8, ctrl_Edit, tBOOL);
	w_ColData[9].GetData("可投掷", 90, 9, ctrl_Edit, tBOOL);
	w_ColData[10].GetData("可消耗", 90, 10, ctrl_Edit, tBOOL);
	w_ColData[11].GetData("全体", 90, 11, ctrl_Edit, tBOOL);
	w_ColData[12].GetData("可出售", 90, 12, ctrl_Edit, tBOOL);
	w_ColData[13].GetData("李装备", 90, 13, ctrl_Edit, tBOOL);
	w_ColData[14].GetData("赵装备", 90, 14, ctrl_Edit, tBOOL);
	w_ColData[15].GetData("林装备", 90, 15, ctrl_Edit, tBOOL);
	w_ColData[16].GetData("巫装备", 90, 16, ctrl_Edit, tBOOL);
	w_ColData[17].GetData("阿装备", 90, 17, ctrl_Edit, tBOOL);
	w_ColData[18].GetData("盖装备", 90, 18, ctrl_Edit, tBOOL);
	w_ColData[19].GetData("图片号", 90, 3, ctrl_Edit, tWORD);

	m_Grid.SetColClass(20, w_ColData);

	//生成数据
	DataArray s_RowData;
	s_RowData.resize(Pal->nItem + 1);
	GAMEDATA * p = &Pal->gpGlobals->g;
	int n_Row = 0,j;
	for (j = 0; j < MAX_OBJECTS; j++)
	{
		if (!(Pal->gpObject_classify[j] & kIsItem))
			continue;
		LPOBJECT lpObj = (LPOBJECT)p->rgObject;

		WORD nFlags = lpObj[j].item.wFlags;
		s_RowData[n_Row].Col.resize(20);
		s_RowData[n_Row].oldRow = n_Row + 1;
		s_RowData[n_Row].Col[0] = j;
		s_RowData[n_Row].Col[1] = j;
		s_RowData[n_Row].Col[2] = lpObj[j].item.wFlags;
		s_RowData[n_Row].Col[3] = lpObj[j].item.wPrice;
		s_RowData[n_Row].Col[4] = lpObj[j].item.wScriptOnEquip;
		s_RowData[n_Row].Col[5] = lpObj[j].item.wScriptOnThrow;
		s_RowData[n_Row].Col[6] = lpObj[j].item.wScriptOnUse;
		s_RowData[n_Row].Col[7] = nFlags & (1 << 0) ? 1 : 0;
		s_RowData[n_Row].Col[8] = nFlags & (1 << 1) ? 1 : 0;
		s_RowData[n_Row].Col[9] = nFlags & (1 << 2) ? 1 : 0;
		s_RowData[n_Row].Col[10] = nFlags & (1 << 3) ? 1 : 0;
		s_RowData[n_Row].Col[11] = nFlags & (1 << 4) ? 1 : 0;
		s_RowData[n_Row].Col[12] = nFlags & (1 << 5) ? 1 : 0;
		s_RowData[n_Row].Col[13] = nFlags & (1 << 6) ? 1 : 0;
		s_RowData[n_Row].Col[14] = nFlags & (1 << 7) ? 1 : 0;
		s_RowData[n_Row].Col[15] = nFlags & (1 << 8) ? 1 : 0;
		s_RowData[n_Row].Col[16] = nFlags & (1 << 9) ? 1 : 0;
		s_RowData[n_Row].Col[17] = nFlags & (1 << 10) ? 1 : 0;
		s_RowData[n_Row].Col[18] = nFlags & (1 << 11) ? 1 : 0;
		s_RowData[n_Row].Col[19] = lpObj[j].item.wBitmap;

		n_Row++;
	}
	m_Grid.SetDataClass(n_Row, 20, s_RowData);
	//不允许添加删除行
	m_Grid.m_popMenuFlags = 0b000111;

	//assert(n_Row == Pal->nItem);

	return LRESULT();
}

LRESULT CEditorAppDlg::Edit_Poison(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_EditPoison;
	m_Grid.UndoCtrl = WM_EditPoison;
	//事件对象
	CString s;
	s.Format(_T("毒药属性编辑"));
	MessageText(s);
	ColArray w_ColData;
	w_ColData.resize(6);
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	auto p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	w_ColData[0].GetData("ID", 90, 0, ctrl_Fix, tHEX);
	w_ColData[1].GetData("品名", 120, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr);
	w_ColData[2].GetData("等级", 90, 2, ctrl_Edit, tWORD);
	w_ColData[3].GetData("颜色", 90, 3, ctrl_Edit, tWORD);
	w_ColData[4].GetData("我方", 90, 4, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[5].GetData("敌方", 90, 5, ctrl_Edit, tHEX, 0, 0, 0, 1);
	m_Grid.SetColClass(6, w_ColData);

	//生成数据
	DataArray s_RowData;
	s_RowData.resize(Pal->nPoisonID);
	GAMEDATA * p = &Pal->gpGlobals->g;
	int n_Row = 0, j;

	for (j = 0; j < MAX_OBJECTS; j++)
	{
		if (!(Pal->gpObject_classify[j] & kIsPoison))
			continue;
		s_RowData[n_Row].Col.resize(6);
		s_RowData[n_Row].oldRow = n_Row + 1;
		s_RowData[n_Row].Col[0] = j;
		s_RowData[n_Row].Col[1] = j;
		LPOBJECT lpObj = (LPOBJECT)p->rgObject;
		s_RowData[n_Row].Col[2] = lpObj[j].poison.wPoisonLevel;
		s_RowData[n_Row].Col[3] = lpObj[j].poison.wColor & 0xff;
		s_RowData[n_Row].Col[4] = lpObj[j].poison.wPlayerScript;
		s_RowData[n_Row].Col[5] = lpObj[j].poison.wEnemyScript;
		n_Row++;
	}
	m_Grid.SetDataClass(n_Row, 6, s_RowData);
	assert(n_Row == Pal->nPoisonID);
	//不允许添加删除行
	m_Grid.m_popMenuFlags = 0b000111;
	SET_COL_DATA s_col_data = [](ColumnClass& s)->VOID{ s.Col[5] = s.Col[1]; };
	return LRESULT();
}

LRESULT CEditorAppDlg::Edit_Scene(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_EditScene;
	m_Grid.UndoCtrl = WM_EditScene;
	//事件对象
	CString s;
	s.Format(_T("场景属性编辑"));
	MessageText(s);

	ColArray w_ColData;
	w_ColData.resize(6);
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	w_ColData[0].GetData("场景号", 100, 0, ctrl_Fix, tINT);
	w_ColData[1].GetData("进入脚本", 100, 1, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[2].GetData("传送脚本", 100, 2, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[3].GetData("开始索引", 100, 3, ctrl_Edit, tINT);
	w_ColData[4].GetData("结束索引", 100, 4, ctrl_Edit, tINT);
	w_ColData[5].GetData("地图号", 100, 5, ctrl_Edit, tINT);

	m_Grid.SetColClass(6, w_ColData);

	//生成数据
	DataArray s_RowData;
	GAMEDATA * p = &Pal->gpGlobals->g;
	s_RowData.resize(MAX_SCENES);
	int n_Row = 0, j;

	for (j = 0; j < MAX_SCENES; j++)
	{
		s_RowData[n_Row].Col.resize(6);
		s_RowData[n_Row].oldRow = n_Row + 1;
		s_RowData[n_Row].Col[0] = n_Row + 1;
		s_RowData[n_Row].Col[1] = p->rgScene[j].wScriptOnEnter;
		s_RowData[n_Row].Col[2] = p->rgScene[j].wScriptOnTeleport;
		s_RowData[n_Row].Col[3] = p->rgScene[j].wEventObjectIndex;
		s_RowData[n_Row].Col[4] = j < MAX_SCENES - 1 ? p->rgScene[j + 1].wEventObjectIndex - 1 : 0;
		s_RowData[n_Row].Col[5] = p->rgScene[j].wMapNum;
		n_Row++;
	}
	m_Grid.SetDataClass(n_Row, 6, s_RowData);
	m_Grid.m_popMenuFlags = 0b1000000111;

	return LRESULT();
}

LRESULT CEditorAppDlg::Edit_Invenyory(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_Inventory;
	m_Grid.UndoCtrl = WM_Inventory;
	//事件对象
	CString s;
	s.Format(_T("背包物品编辑"));
	MessageText(s);
	ColArray w_ColData;
	w_ColData.resize(3);
	auto p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	//制作下拉框数据
	pSelect_Item p_Select_Item = &Select_Item_Array[0];
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	w_ColData[0].GetData("序号", 100, 0, ctrl_Fix, tNull);
	w_ColData[1].GetData("品名", 120, 1, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
	w_ColData[2].GetData("数量", 100, 2, ctrl_Edit, tINT);
	m_Grid.SetColClass(3, w_ColData);

	//生成数据
	DataArray s_RowData;
	s_RowData.resize(MAX_INVENTORY);
	int n_Row = 0, j;

	for (j = 0; j < MAX_INVENTORY; j++)
	{
		if (Pal->gpGlobals->rgInventory[j].nAmount == 0)
			continue;
		s_RowData[n_Row].Col.resize(3);
		s_RowData[n_Row].oldRow = j + 1;
		s_RowData[n_Row].Col[0] = j + 1;
		s_RowData[n_Row].Col[1] = Pal->gpGlobals->rgInventory[j].wItem;
		s_RowData[n_Row].Col[2] = Pal->gpGlobals->rgInventory[j].nAmount;
		n_Row++;
	}
	s_RowData.resize(n_Row);
	m_Grid.SetDataClass(n_Row, 3, s_RowData);
	m_Grid.m_popMenuFlags = 0b000111111;
	return LRESULT();
}

LRESULT CEditorAppDlg::PackPal(WPARAM w_wparam, LPARAM w_lparam)
{
	WorkCtrl = WM_PACKPAL;
	m_Grid.UndoCtrl = WM_PACKPAL;
	//事件对象
	CString s;
	s.Format(_T("打包"));
	MessageText(s);
	//Pal->PAL_PackObject();
	return LRESULT();
}

LRESULT CEditorAppDlg::UnPackPal(WPARAM w_wparam, LPARAM w_lparam)
{
	WorkCtrl = WM_UNPACKPAL;
	m_Grid.UndoCtrl = WM_UNPACKPAL;

	//事件对象
	CString s;
	s.Format(_T("解包"));
	MessageText(s);
	s = Pal->PalDir.c_str();
	s += _T("MAKMKF");
	if (!IsDirExist(s))
	{
		//目录不存在，建立目录
		if (CreateDirectory(s, 0) == 0)
		{
			::MessageBox(NULL, _T("无法建立目录"), s, MB_OK);
			return 0;
		}
	}
	Pal->PAL_UnPackObject();
	return LRESULT();
}

LRESULT CEditorAppDlg::Edit_EventObject(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_EditEventObject;
	m_Grid.UndoCtrl = WM_EditEventObject;
	//事件对象
	CString s;
	s.Format(_T("修改事件对象"));
	MessageText(s);
	ColArray w_ColData;
	w_ColData.resize(20);
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	w_ColData[0].GetData("序号", 90, 1, ctrl_Fix, tWORD);
	w_ColData[1].GetData("场景号", 90, 0, ctrl_Fix, tWORD);
	w_ColData[2].GetData("隐时间", 90, 2, ctrl_Edit, tWORD);
	w_ColData[3].GetData("X", 90, 3, ctrl_Edit, tWORD);
	w_ColData[4].GetData("Y", 90, 4, ctrl_Edit, tWORD);
	w_ColData[5].GetData("层次", 90, 5, ctrl_Edit, tSHORT);
	w_ColData[6].GetData("目标脚本", 120, 6, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[7].GetData("自动脚本", 120, 7, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[8].GetData("状态", 90, 8, ctrl_Edit, tWORD);
	w_ColData[9].GetData("触发模式", 90, 9, ctrl_Edit, tWORD);
	w_ColData[10].GetData("形象号", 90, 10, ctrl_Edit, tWORD);
	w_ColData[11].GetData("形象数", 90, 11, ctrl_Edit, tWORD);
	w_ColData[12].GetData("方向", 90, 12, ctrl_Edit, tWORD);
	w_ColData[13].GetData("当前帧数", 90, 13, ctrl_Edit, tSHORT);
	w_ColData[14].GetData("空闲帧数", 90, 14, ctrl_Edit, tSHORT);
	w_ColData[15].GetData("无效", 90, 15, ctrl_Edit, tSHORT);
	w_ColData[16].GetData("总帧数", 90, 16, ctrl_Edit, tWORD);
	w_ColData[17].GetData("空闲计数", 90, 17, ctrl_Edit, tWORD);
	w_ColData[18].GetData("行号", 90, 18, ctrl_Null, tWORD);
	w_ColData[19].GetData("原行号", 90, 19, ctrl_Null, tWORD);

	m_Grid.SetColClass(20, w_ColData);

	//生成数据
	DataArray s_RowData;
	GAMEDATA * p = &Pal->gpGlobals->g;
	s_RowData.resize(p->nEventObject );

	int  n_Row, n_Scens,n_Object;
	for (n_Row = 0,n_Scens = 1; n_Scens <= MAX_SCENES; n_Scens++)
	{
		for (n_Object = p->rgScene[n_Scens - 1].wEventObjectIndex;
			(n_Object < (p->rgScene[n_Scens].wEventObjectIndex) || n_Scens == MAX_SCENES)
			&& n_Row < p->nEventObject; n_Object++,n_Row++)
		{
			s_RowData[n_Row].Col.resize(20);
			s_RowData[n_Row].oldRow = n_Row + 1;
			s_RowData[n_Row].Col[18] = n_Row + 1;
			s_RowData[n_Row].Col[19] = n_Row + 1;

			LPEVENTOBJECT  peo = &(p->lprgEventObject[n_Object]);
			s_RowData[n_Row].Col[0] = n_Row;
			s_RowData[n_Row].Col[1] = n_Scens;
			WORD* wp = (WORD*)peo;
			for (int n = 2; n < 18; n++)
			{
				s_RowData[n_Row].Col[n] = wp[n - 2];
			}
		}
	}

	m_Grid.SetDataClass(n_Row, 20, s_RowData);
	if (JobCtrl)
		m_Grid.m_popMenuFlags = 0b00111;//存档文件不允许插入删除
	else
		m_Grid.m_popMenuFlags = 0b0101111;//可插入删除，不请允许尾部添加
	//
	SET_COL_DATA s_col_data = [](ColumnClass& s)->VOID
	{
		s.Col[18] = get<int>(s.Col[0]) + 1;
		s.Col[19] = (int)s.oldRow ;
	};
	m_Grid.Set_Set_Col_Data(s_col_data);

	return 0;
}

LRESULT CEditorAppDlg::Edit_Store(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);
	//事件对象
	WorkCtrl = WM_EditStore;
	m_Grid.UndoCtrl = WM_EditStore;
	ShowMsg("修改商店");

	//下拉表选择数据
	pSelect_Item p_Select_all = &Select_Item_Array[3];

	ColArray w_ColData;
	w_ColData.resize(11);
	//字段显示生成函数
	auto p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	//lambda
	LPSTR(*p_IntToStr)(int d) = [](int d)-> LPSTR {
		static char buf[8]; sprintf_s(buf, " %3d", d);  return (LPSTR)buf; };
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	w_ColData[0].GetData("序号", 90, 0, ctrl_Fix, tWORD);
	w_ColData[1].GetData("商店号", 90, 1, ctrl_Fix, tWORD,NULL,p_IntToStr);
	w_ColData[2].GetData("位置1", 90, 2, ctrl_List, tNull, p_Select_all, p_SrcToStr);
	w_ColData[3].GetData("位置2", 90, 3, ctrl_List, tNull, p_Select_all, p_SrcToStr);
	w_ColData[4].GetData("位置3", 90, 4, ctrl_List, tNull, p_Select_all, p_SrcToStr);
	w_ColData[5].GetData("位置4", 90, 5, ctrl_List, tNull, p_Select_all, p_SrcToStr);
	w_ColData[6].GetData("位置5", 90, 6, ctrl_List, tNull, p_Select_all, p_SrcToStr);
	w_ColData[7].GetData("位置6", 90, 7, ctrl_List, tNull, p_Select_all, p_SrcToStr);
	w_ColData[8].GetData("位置7", 90, 8, ctrl_List, tNull, p_Select_all, p_SrcToStr);
	w_ColData[9].GetData("位置8", 90, 9, ctrl_List, tNull, p_Select_all, p_SrcToStr);
	w_ColData[10].GetData("位置9", 90, 10, ctrl_List, tNull, p_Select_all, p_SrcToStr);

	m_Grid.SetColClass(11, w_ColData);

	//表数据
	int n_Row, n_Item;
	GAMEDATA * p = &Pal->gpGlobals->g;
	DataArray s_RowData;
	s_RowData.resize(p->nStore);
	for (n_Row = 0; n_Row < p->nStore; n_Row++)
	{
		s_RowData[n_Row].Col.resize(11);
		s_RowData[n_Row].oldRow = n_Row + 1;
		s_RowData[n_Row].Col[0] = n_Row + 1;
		s_RowData[n_Row].Col[1] = n_Row ;

		for (n_Item = 0; n_Item < MAX_STORE_ITEM; n_Item++)
		{
			s_RowData[n_Row].Col[n_Item + 2] = p->lprgStore[n_Row].rgwItems[n_Item];
		}
	}

	m_Grid.SetDataClass(n_Row, 11, s_RowData);
	m_Grid.m_popMenuFlags = 0b00010111;

	return 0;
}


LRESULT CEditorAppDlg::Edit_PlayerLevelUPMagic(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_PLAYERLEVELUPMAGIG;
	m_Grid.UndoCtrl = WM_PLAYERLEVELUPMAGIG;
	//下拉表选择数据
	pSelect_Item p_Select_Magic = &Select_Item_Array[1];
	//升级魔法
	CString s;
	s.Format(_T("修改升级魔法"));
	MessageText(s);
	ColArray w_ColData;
	w_ColData.resize(11);
		SrcToStr p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	w_ColData[0].GetData("序号", 60, 0, ctrl_Fix, tWORD);
	w_ColData[1].GetData("李等级", 90, 1,ctrl_Edit,tWORD);
	w_ColData[2].GetData("魔法", 90, 2, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
	w_ColData[3].GetData("赵等级", 90, 3, ctrl_Edit, tWORD);
	w_ColData[4].GetData("魔法", 90, 4, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
	w_ColData[5].GetData("林等级", 90, 5, ctrl_Edit, tWORD);
	w_ColData[6].GetData("魔法", 90, 6, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
	w_ColData[7].GetData("巫后等级", 90, 7, ctrl_Edit, tWORD);
	w_ColData[8].GetData("魔法", 90, 8, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
	w_ColData[9].GetData("阿奴等级", 90, 9, ctrl_Edit, tWORD);
	w_ColData[10].GetData("魔法", 90, 10, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);

	m_Grid.SetColClass(11, w_ColData);

	//表数据
	DataArray s_RowData;
	s_RowData.resize(Pal->gpGlobals->g.nLevelUpMagic );//行数
	int k = 0;
	for (k = 0; k < Pal->gpGlobals->g.nLevelUpMagic; k++)
	{
		s_RowData[k].Col.resize(11);
		s_RowData[k].oldRow = k + 1;

		s_RowData[k].Col[0] = k + 1;
		for (size_t j = 0; j < MAX_PLAYABLE_PLAYER_ROLES; j++)
		{
			s_RowData[k].Col[j * 2 + 1] = Pal->gpGlobals->g.lprgLevelUpMagic[k].m[j].wLevel;
			s_RowData[k].Col[j * 2 + 2] = Pal->gpGlobals->g.lprgLevelUpMagic[k].m[j].wMagic;
		}
	}

	m_Grid.SetDataClass(k, 11, s_RowData);
	m_Grid.m_popMenuFlags = 0b111111;

	return 0;
}


LRESULT CEditorAppDlg::Edit_Player(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_EDIT_PLAYER;
	m_Grid.UndoCtrl = WM_EDIT_PLAYER;
	CString s;
	s.Format(_T("修改我方人员属性"));
	MessageText(s);
	ColArray w_ColData;
	w_ColData.resize(0);
	w_ColData.resize(64);
	//下拉表选择数据
	pSelect_Item p_Select_Item = &Select_Item_Array[0];
	pSelect_Item p_Select_Magic = &Select_Item_Array[1];

	auto p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	//制做表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	w_ColData[0].GetData("编号",    60, 0, ctrl_Fix,  tWORD);
	w_ColData[1].GetData("姓名",   120, 1, ctrl_Fix,  tWORD, nullptr, p_SrcToStr);
	w_ColData[2].GetData("等级",    90, 2, ctrl_Edit, tWORD);
	w_ColData[3].GetData("体力MAX", 90, 3, ctrl_Edit, tWORD);
	w_ColData[4].GetData("真气MAX", 90, 4, ctrl_Edit, tWORD);
	w_ColData[5].GetData("体力",    90, 5, ctrl_Edit, tWORD);
	w_ColData[6].GetData("真气",    90, 6, ctrl_Edit, tWORD);
	w_ColData[7].GetData("武力",    90, 7, ctrl_Edit, tWORD);
	w_ColData[8].GetData("灵气",    90, 8, ctrl_Edit, tWORD);
	w_ColData[9].GetData("防御",    90, 9, ctrl_Edit , tWORD);
	w_ColData[10].GetData("速度",    90,10, ctrl_Edit, tWORD);
	w_ColData[11].GetData("吉运",	90, 11, ctrl_Edit, tWORD);
	w_ColData[12].GetData("毒抗",	90, 12, ctrl_Edit, tWORD);
	w_ColData[13].GetData("风抗",	90, 13, ctrl_Edit, tWORD);
	w_ColData[14].GetData("雷抗",	90, 14, ctrl_Edit, tWORD);
	w_ColData[15].GetData("水抗",	90, 15, ctrl_Edit, tWORD);
	w_ColData[16].GetData("火抗",	90, 16, ctrl_Edit, tWORD);
	w_ColData[17].GetData("土抗",	90, 17, ctrl_Edit, tWORD);
	w_ColData[18].GetData("巫抗",	90, 18, ctrl_Edit, tWORD);
	w_ColData[19].GetData("物抗",	90, 19, ctrl_Edit, tWORD);
	w_ColData[20].GetData("巫攻",	90, 20, ctrl_Edit, tWORD);
	w_ColData[21].GetData("头",		120, 21, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
	w_ColData[22].GetData("肩",		120, 22, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
	w_ColData[23].GetData("身",		120, 23, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
	w_ColData[24].GetData("手",		120, 24, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
	w_ColData[25].GetData("脚",		120, 25, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
	w_ColData[26].GetData("挂",		120, 26, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
	w_ColData[27].GetData("救援",	120, 27, ctrl_Edit, tWORD);
	w_ColData[28].GetData("合体法术",	120, 28, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
	for (WORD n = 29; n < 61; n++)
	{
		w_ColData[n].GetData(Pal->va("法术%2.2d", n - 28), 100, n, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
	}
	//队友死 位2
	w_ColData[61].GetData("队友死", 100, 61, ctrl_Edit, tHEX, 0, 0, 0, 1);
	//濒死 位3
	w_ColData[62].GetData("濒死", 100, 62, ctrl_Edit, tHEX, 0, 0, 0, 1);
	//行号
	w_ColData[63].GetData("行号", 60, 63, ctrl_Null, tINT);

	//w_ColData[62].GetData(" ",		20,  29, 0, 1);

	m_Grid.SetColClass(64, w_ColData);

	//生成表数据
				//插入子项数据
			/*
			参数1：主角属性地址——基本属性
			00：状态表情图像
			01：战斗模型
			02：地图模型
			03：名字
			04：可否攻击全体
			05：无效？
			06：等级
			07：体力最大值
			08：真气最大值
			09：体力
			0A：真气
			0B-10,装备
			11：武术
			12：灵力
			13：防御
			14：身法
			15：吉运
			16：毒抗
			17：风抗
			18：雷抗
			19：水抗
			1A：火抗
			1B：土抗
			1C:	巫抗
			1D：物抗
			1E：巫攻
			1F：救援
			20--3F魔法
			40？......
			41：合体法术
			*/
	DataArray s_RowData;
	s_RowData.resize(MAX_PLAYER_ROLES);//行数

	WORD  * p_list = (WORD *)&(Pal->gpGlobals->g.PlayerRoles);

	for (int r = 0; r < MAX_PLAYER_ROLES; r++)
	{
		s_RowData[r].Col.resize(64);
		s_RowData[r].Col[0] = r + 1;//行号从1开始
		for (int n = 1, i; n < 62; n++)
		{
			if (n == 1)
				i = n + 2;//名称
			else if (n > 1 && n <= 6)
				i = n + 4;//等级-真气
			else if (n > 6 && n <= 20)
				i = n + 10;//武力--巫抗
			else if (n > 20 && n < 27)
				i = n - 10;//装备
			else if (n == 27)
				i = 0x1f;//救援
			else if (n == 28)
				i = 0x41;//合体法术
			else if (n >= 29 && n < 61)
				i = n + 3;

			s_RowData[r].Col[n] = (WORD)p_list[(i)*MAX_PLAYER_ROLES + r];
		}
		WORD s1 = 0, s2 = 0;
		WORD name = ((Pal->gpGlobals->g.PlayerRoles)).rgwName[r];
		s1 = ((LPOBJECT)Pal->gpGlobals->g.rgObject)[name].player.wScriptOnFriendDeath;
		s2 = ((LPOBJECT)Pal->gpGlobals->g.rgObject)[name].player.wScriptOnDying;
		//队友死
		s_RowData[r].Col[61] = s1;
		//虚弱
		s_RowData[r].Col[62] = s2;
		//行号
		s_RowData[r].Col[63] = r + 1;

	}
	m_Grid.SetDataClass( MAX_PLAYER_ROLES, 64, s_RowData);
	//弹出菜单项
	m_Grid.m_popMenuFlags = 0b000111;

	return 0;
	//assert;
}

LRESULT CEditorAppDlg::Edit_Enemy(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);
	//编辑怪物属性
	WorkCtrl = WM_EDIT_ENEMY;
	m_Grid.UndoCtrl = WM_EDIT_ENEMY;
	CString s;
	s.Format(_T("修改敌方怪物属性"));
	MessageText(s);
	//下拉表物品选择数据
	pSelect_Item p_Select_Item = &Select_Item_Array[0];
	pSelect_Item p_Select_Magic = &Select_Item_Array[1];
	pSelect_Item p_Select_All = &Select_Item_Array[3];
	//制作表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）

	ColArray w_ColData;
	w_ColData.resize(42);
	auto p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	w_ColData[0].GetData("ID", 60, 0, ctrl_Fix, tHEX);
	w_ColData[1].GetData("名称", 120, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr);
	w_ColData[2].GetData("空闲帧数", 90, 2, ctrl_Edit, tWORD);
	w_ColData[3].GetData("魔法帧数", 90, 2, ctrl_Edit, tWORD);
	w_ColData[4].GetData("普攻帧数", 90, 2, ctrl_Edit, tWORD);
	w_ColData[5].GetData("空闲动速", 90, 2, ctrl_Edit, tWORD);
	w_ColData[6].GetData("等待帧数", 90, 2, ctrl_Edit, tWORD);
	w_ColData[7].GetData("Y位移", 90, 2, ctrl_Edit, tSHORT);
	w_ColData[8].GetData("物攻声", 90, 2, ctrl_Edit, tWORD);
	w_ColData[9].GetData("动作声", 90, 2, ctrl_Edit, tWORD);
	w_ColData[10].GetData("魔法声", 90, 2, ctrl_Edit, tWORD);
	w_ColData[11].GetData("死亡声", 90, 2, ctrl_Edit, tWORD);
	w_ColData[12].GetData("进入声", 90, 2, ctrl_Edit, tWORD);
	w_ColData[13].GetData("体力", 90, 2, ctrl_Edit, tWORD);
	w_ColData[14].GetData("经验", 90, 3, ctrl_Edit, tWORD);
	w_ColData[15].GetData("金钱", 90, 4, ctrl_Edit, tWORD);
	w_ColData[16].GetData("等级", 90, 5, ctrl_Edit, tWORD);
	w_ColData[17].GetData("魔法ID", 90, 6, ctrl_List, tNull, p_Select_Magic, p_SrcToStr);
	w_ColData[18].GetData("概率", 90, 7, ctrl_Edit, tWORD);
	w_ColData[19].GetData("攻击附加", 90, 8, ctrl_List, tNull, p_Select_All, p_SrcToStr);
	w_ColData[20].GetData("概率", 90, 9, ctrl_Edit, tWORD);
	w_ColData[21].GetData("偷窃物品", 90, 10, ctrl_List, tNull, p_Select_Item, p_SrcToStr);
	w_ColData[22].GetData("数量", 90, 11, ctrl_Edit, tWORD);
	w_ColData[23].GetData("物攻", 90, 12, ctrl_Edit, tSHORT);
	w_ColData[24].GetData("灵攻", 90, 13, ctrl_Edit, tSHORT);
	w_ColData[25].GetData("防御", 90, 14, ctrl_Edit, tSHORT);
	w_ColData[26].GetData("速度", 90, 15, ctrl_Edit, tSHORT);
	w_ColData[27].GetData("吉运", 90, 16, ctrl_Edit, tSHORT);
	w_ColData[28].GetData("毒抗", 90, 17, ctrl_Edit, tWORD);
	w_ColData[29].GetData("风抗", 90, 18, ctrl_Edit, tWORD);
	w_ColData[30].GetData("雷抗", 90, 19, ctrl_Edit, tWORD);
	w_ColData[31].GetData("水抗", 90, 20, ctrl_Edit, tWORD);
	w_ColData[32].GetData("火抗", 90, 21, ctrl_Edit, tWORD);
	w_ColData[33].GetData("土抗", 90, 22, ctrl_Edit, tWORD);
	w_ColData[34].GetData("物抗", 90, 23, ctrl_Edit, tWORD);
	w_ColData[35].GetData("双击", 90, 24, ctrl_Edit, tWORD);
	w_ColData[36].GetData("灵葫值", 90, 25, ctrl_Edit, tWORD);
	w_ColData[37].GetData("巫抗", 90, 26, ctrl_Edit, tWORD);
	w_ColData[38].GetData("战前脚本", 90, 27, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[39].GetData("战斗脚本", 90, 28, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[40].GetData("战后脚本", 90, 29, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[41].GetData("行号", 90, 30, ctrl_Null, tWORD);

	m_Grid.SetColClass(42, w_ColData);

	//生成数据
	DataArray s_RowData;
	GAMEDATA* p = &Pal->gpGlobals->g;
	s_RowData.resize(p->nEnemy);//行数
	int n_Count = 0;


	for (WORD j = 0; j < MAX_OBJECTS; j++)
	{
		if (Pal->gpObject_classify[j] != kIsEnemy)
			continue;
		WORD t = ((LPOBJECT)p->rgObject)[j].enemy.wEnemyID;
		ASSERT(t >= 0 && t < Pal->gpGlobals->g.nEnemy);

		LPENEMY pEnemy = &(p->lprgEnemy[t]);
		WORD* wp_State = &(pEnemy->wIdleFrames);

		s_RowData[n_Count].Col.resize(42);
		s_RowData[n_Count].oldRow = n_Count + 1;

		s_RowData[n_Count].Col[0] = j;
		s_RowData[n_Count].Col[1] = j;
		for (int k = 2; k < 37; k++)
		{
			s_RowData[n_Count].Col[k] = (WORD)wp_State[k - 2];
		}
		LPOBJECT lpObj = (LPOBJECT)p->rgObject;
		s_RowData[n_Count].Col[37] = lpObj[j].enemy.wResistanceToSorcery;
		s_RowData[n_Count].Col[38] = lpObj[j].enemy.wScriptOnTurnStart;
		s_RowData[n_Count].Col[39] = lpObj[j].enemy.wScriptOnReady;
		s_RowData[n_Count].Col[40] = lpObj[j].enemy.wScriptOnBattleEnd;
		s_RowData[n_Count].Col[41] = t;
		n_Count++;
	}
	assert(n_Count == Pal->nEnemy);
	//
	m_Grid.SetDataClass(n_Count, 42, s_RowData);
	m_Grid.m_popMenuFlags = 0b000111;
	return 0;
}

LRESULT CEditorAppDlg::Edit_Magic(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_EDIT_MAGIC;
	m_Grid.UndoCtrl = WM_EDIT_MAGIC;
	MessageText(_T("修改魔法"));
	//建立表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	ColArray w_ColData;
	auto p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	w_ColData.resize(24);
	w_ColData[0].GetData("ID", 60, 0, ctrl_Fix, tHEX);
	w_ColData[1].GetData("名称", 120, 1, ctrl_Fix, tWORD, nullptr, p_SrcToStr, nullptr);
	w_ColData[2].GetData("形象号", 90, 2, ctrl_Edit, tSHORT);
	w_ColData[3].GetData("类型", 90, 3, ctrl_Edit, tSHORT);
	w_ColData[4].GetData("X位移", 90, 4, ctrl_Edit, tSHORT);
	w_ColData[5].GetData("Y位移", 90, 5, ctrl_Edit, tSHORT);
	w_ColData[6].GetData("效果号", 90, 6, ctrl_Edit, tSHORT);
	w_ColData[7].GetData("速度", 90, 7, ctrl_Edit, tSHORT);
	w_ColData[8].GetData("保持形象", 90, 8, ctrl_Edit, tSHORT);
	w_ColData[9].GetData("声效延迟", 90, 9, ctrl_Edit, tSHORT);
	w_ColData[10].GetData("耗时", 90, 10, ctrl_Edit, tSHORT);
	w_ColData[11].GetData("场景震动", 90, 11, ctrl_Edit, tSHORT);
	w_ColData[12].GetData("场景波动", 90, 12, ctrl_Edit, tSHORT);
	w_ColData[13].GetData("保留", 90, 13, ctrl_Edit, tWORD);
	w_ColData[14].GetData("耗兰", 90, 14, ctrl_Edit, tSHORT);
	w_ColData[15].GetData("基础伤害", 90, 15, ctrl_Edit, tSHORT);
	w_ColData[16].GetData("属性", 90, 16, ctrl_Edit, tSHORT);
	w_ColData[17].GetData("声效", 90, 17, ctrl_Edit, tWORD);
	w_ColData[18].GetData("标志", 90, 18, ctrl_Edit, tSHORT);
	w_ColData[19].GetData("使用脚本", 90, 19, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[20].GetData("成功脚本", 90, 20, ctrl_Edit, tHEX, 0, 0, 0, 1);
	w_ColData[21].GetData("魔法号", 90, 21, ctrl_Null, tSHORT);
	w_ColData[22].GetData("序号", 90, 22, ctrl_Null, tSHORT);
	w_ColData[23].GetData("行号", 60, 23, ctrl_Null, tINT);

	m_Grid.SetColClass(24, w_ColData);

	//表数据
	DataArray s_RowData;
	GAMEDATA* p = &Pal->gpGlobals->g;
	s_RowData.resize(Pal->nMagic);//行数
	int n_Count = 0;
	int j;
	for (j = 0; j < MAX_OBJECTS; j++)
	{
		if (!(Pal->gpObject_classify[j] & kIsMagic))
			continue;
		WORD k;
		WORD* obj;
		k = ((LPOBJECT)p->rgObject)[j].magic.wMagicNumber;
		obj = ((LPOBJECT)p->rgObject)[j].rgwData;
		//= p->rgObject[j].magic.wMagicNumber;
		LPMAGIC pMagic = &(p->lprgMagic[k]);
		WORD* wp_State = (WORD*)pMagic;
		s_RowData[n_Count].Col.resize(24);
		s_RowData[n_Count].Col[0] = j;
		s_RowData[n_Count].Col[1] = j;
		for (int s = 2; s < 18; s++)
		{
			s_RowData[n_Count].Col[s] = (WORD)wp_State[s - 2];
		}
		s_RowData[n_Count].Col[18] = obj[6];// lpObj[j].magic.wFlags;//5-6

		s_RowData[n_Count].Col[19] = obj[3];//lpObj[j].magic.wScriptOnUse;//3
		s_RowData[n_Count].Col[20] = obj[2];// lpObj[j].magic.wScriptOnSuccess;//2
		s_RowData[n_Count].Col[21] = obj[0];// lpObj[j].magic.wMagicNumber;//0

		s_RowData[n_Count].Col[22] = k;//=21
		//
		s_RowData[n_Count].Col[23] = n_Count + 1;

		n_Count++;
	}
	assert(n_Count == Pal->nMagic);
	m_Grid.SetDataClass(n_Count, 24, s_RowData);
	m_Grid.m_popMenuFlags = 0b000111;

	return 0;
}


LRESULT CEditorAppDlg::Edit_1(WPARAM w_wparam, LPARAM w_lparam)
{
	//修改存档文件
	m_Grid.ShowWindow(FALSE);
	CString s;
	s.Format(_T("修改存档文件 %1d.rpg"), (int)w_lparam);
	MessageText(s);
	WorkCtrl = -1;
	JobCtrl = w_lparam;
	Set_Archived_Tree();
	return 0;
}

LRESULT CEditorAppDlg::Post_Text(WPARAM w_wparam, LPARAM w_lparam)
{
	CString s = *(CString *)w_lparam;
	return 0;
}

LRESULT CEditorAppDlg::Edit_Dir(WPARAM w_wparam, LPARAM w_lparam)
{
	if (MessageBox(L"确认吗？", L"本操作将修改目录,并作废之前的修改", MB_YESNO) == IDNO)
	{
		return 0;
	}
	m_Grid.ShowWindow(FALSE);
	MessageText(_T("修改目录"));
	CString dir(Pal->PalDir.c_str());
	int ret;
	dirBuf = dir;
	while ((ret = GetCorrectDir(dir)) != 1)
	{
		if (ret == -1)
			return 0;
		if (::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T(" 目录错"), dir , MB_RETRYCANCEL) == IDCANCEL)
			return 0;
	}
	if (Pal->PalDir != CStringA( dir).GetString())
	{
		//修改目录后的备份
		CString dir1(Pal->PalDir.c_str());
		delete Pal;
		Pal = nullptr;
		//恢复原目录文件
		restoreFile(dir1);
		CPalEvent::PalDir = CStringA(dir).GetString();
		vector<INT32> p;
		p.resize(40);
		//先变更工作目录再初始系统
		SetCurrentDirectory(dir);
		backupFile(dir);

		Pal = new CGetPalData(0 , 1);

		Set_Begin_Tree();
		CString newdirstr;
		newdirstr.Format(L"新目录为 %s", dir.GetBuffer());
		MessageText(newdirstr);
	}
	PostMessage(WM_OKReturn, 0, 1);
	return 0;
}

LRESULT CEditorAppDlg::Edit_Defalse(WPARAM w_wparam, LPARAM w_lparam)
{

	MessageText(_T("选择初始项目")); //输出
	if (Pal == nullptr)
	{
		vector<INT32> p;
		p.resize(40);
		Pal = new CGetPalData(0, 1);
	}
	Pal->PAL_LoadDefaultGame();

	if (Pal->gpGlobals->lpObjectDesc == NULL)
		Pal->gpGlobals->lpObjectDesc = Pal->PAL_LoadObjectDesc("desc.dat");

	Set_Defalse_Tree();
	return 0;
}

LRESULT CEditorAppDlg::OK_Return(WPARAM w_wparam, LPARAM w_lparam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	MessageText(_T("存盘返回")); //输出
	//存盘
	if (JobCtrl > 0)
	{
		//string s;
		//s = Pal->va("%s%1d.rpg", Pal->PalDir.c_str(), JobCtrl);

		//存档文件
		//Pal->PAL_SaveGame(s.c_str(), 2);
		//Pal->PAL_ReloadPAL(FALSE);
		//isUpData = TRUE;
	}
	else
	{
		if (w_lparam == 0)
		{
			//存储缺省项目
			if (Pal->gpGlobals == nullptr)
			{
				MessageText(L"存储项目出错");
			}
			else
			{
				Pal->PAL_Make_MKF();
				isUpData = TRUE;
			}
		}
	}
	Pal->PAL_ReloadPAL(FALSE);
	//
	m_Grid.ShowWindow(FALSE);
	Set_Begin_Tree();
	return 0;
}

LRESULT CEditorAppDlg::Cansel_Return(WPARAM w_wparam, LPARAM w_lparam)
{
	MessageText(_T("取消返回")); //输出
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	m_Grid.ShowWindow(FALSE);
	Set_Begin_Tree();
	return 0;
}

#include <direct.h>
#include <thread>
#include "CGetLing.h"
#include "cMap_Dlg.h"
#include "CListScriptEntry.h"
#include "CScriptCaller.h"

BOOL CEditorAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	MoveWindow(0, 0, 960, 640);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//WCHAR name[256]{ 0 };
	//GetWindowText(name,255);
	CString gameRootDir;
	int ret;
	{
		//EXE 文件所在目录
		CHAR wDIR[512]{ 0 };

		GetModuleFileNameA(NULL, wDIR, 511);
		char drive[_MAX_DRIVE]{ 0 };
		char dir[_MAX_DIR]{ 0 };
		_splitpath(wDIR,drive ,dir , NULL, NULL);
		ExeDir = drive;
		ExeDir += dir;
	}

	//测试当前目录是否是合法目录，如不是进行选择
	CHAR AppDir[256]{ 0 };
	LPSTR sDir =  getcwd(AppDir, 255);
	gameRootDir = AppDir;//工作目录

	gameRootDir += _T("\\");
	CString palSubDir = gameRootDir + "pal\\";

	// 如果指定的根目录、及pal子目录，都不是有效游戏资源路径
	if ((!IsCorrectDir(gameRootDir)) && (!IsCorrectDir(palSubDir)))
	{
		while ((ret = GetCorrectDir(gameRootDir)) != 1)
		{
			if (ret == -1||
				::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, gameRootDir +
					L"  不是正确的游戏目录", _T("目录错"), MB_RETRYCANCEL) == IDCANCEL)
			{
				okExit = -1;
				PostMessage(WM_CLOSE);
				return 0;
			}
				//exit(2);
			//if (::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, gameRootDir + L"  不是正确的游戏目录", _T("目录错"), MB_RETRYCANCEL) == IDCANCEL)
			continue;
		}
	}

	// 如果根目录不是有效目录，则以 pal 子目录作为有效目录
	if (!IsCorrectDir(gameRootDir))
		gameRootDir = palSubDir;

	CPalEvent::PalDir = CStringA(gameRootDir);

	//检查原始文件备份情况
	CString oldfileDir = gameRootDir + _T("OLDFILE");
	if (!IsDirExist(oldfileDir))
	{
		//目录不存在，建立目录
		if (CreateDirectory(oldfileDir, 0) == 0)
		{
			::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T("无法建立目录"), oldfileDir, MB_OK);
			return 0;
		}
		else
		{
			//备份关键文件
			CString dirOld = gameRootDir + _T("OLDFILE\\");
			CString s1;

			for (int n = 0; FileNames[n]; n++)
			{
				oldfileDir = gameRootDir + FileNames[n];
				s1 = dirOld + FileNames[n];
				{
					//覆盖
					CopyFile((oldfileDir), (s1), TRUE);
				}
			}
		}
	}

	//恢复上次未正常退出前备份的文件
	if (IsDirExist(gameRootDir + L"BAK\\"))
		restoreFile(gameRootDir);

	//拷贝需要备份的文件
	backupFile(gameRootDir);

	//
	m_Grid.m_pList = &m_ListCtrl;
	m_Tree.ShowWindow(FALSE);

	//初始化游戏数据
	Pal->PalDir = CStringA(gameRootDir).GetString();

	vector<INT32> p;
	p.resize(40);

	if (Pal)
		delete Pal;

	Pal = new CGetPalData(0, 1);

	m_Grid.pPal = &Pal;
	//
	//置控制树初始值
	JobCtrl = WM_EDIT_0;
	//初始化

	Set_Begin_Tree();

	int nWidth = GetSystemMetrics(SM_CXSCREEN);  //屏幕宽度
	int nHeight = GetSystemMetrics(SM_CYSCREEN); //屏幕高度
	RECT rect{ 0,0,nWidth * 0.75,nHeight * 0.75 };
	MoveWindow(&rect);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CEditorAppDlg::Set_Archived_Tree()
{
	//生成修改存档文件树
	//删除命令树全部叶子
	//USES_CONVERSION;
	string s;
	s = Pal->va("%s%1d.rpg", Pal->PalDir.c_str(), JobCtrl);
	//装入存档文件
	Pal->PAL_LoadGame(s.c_str());

	m_Tree.ShowWindow(FALSE);
	HTREEITEM root, res, nod;
	//WorkCtrl = -1;
	//删除树结点
	m_Tree.DeleteAllItems();
	//取得根结点

	root = m_Tree.InsertItem(_T("初始属性"));
	//插入二级结点
	res = m_Tree.InsertItem(_T("属性"), 1, 1, root);
	nod = m_Tree.InsertItem(_T("基本属性"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EDIT_PARAMETERS);
	nod = m_Tree.InsertItem(_T("我方属性"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EDIT_PLAYER);

	nod = m_Tree.InsertItem(_T("事件对象"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EditEventObject);

	nod = m_Tree.InsertItem(_T("背包物品"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_Inventory);

	nod = m_Tree.InsertItem(_T("存盘返回"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_OKReturn);
	nod = m_Tree.InsertItem(_T("取消返回"), 2, 2, res);
	m_Tree.SetItemData(nod, WM_CanselReturn);

	//展开树
	m_Tree.Expand(res, TVE_EXPAND);
	m_Tree.Expand(root, TVE_EXPAND);
	m_Tree.ShowWindow(TRUE);

}


void CEditorAppDlg::Set_Begin_Tree()
{
	//生成开始树
	//删除命令树全部叶子
	JobCtrl = 0;
	WorkCtrl = -1;
	m_Grid.ShowWindow(FALSE);
	m_Tree.ShowWindow(FALSE);
	m_Tree.DeleteAllItems();
	HTREEITEM root, res, nod;
	root = m_Tree.InsertItem(_T("操作"), NULL, NULL);
	m_Tree.SetItemState(root, TVIS_BOLD, TVIS_BOLD);
	res = m_Tree.InsertItem(_T("目录"), 1, 1, root);
	nod = m_Tree.InsertItem(CString(CPalEvent::PalDir.c_str()), 2, 2, res);
	m_Tree.SetItemData(nod, WM_EDIT_DIR);
	m_Tree.InsertItem(_T("--------"), 1, 1, root);
	m_Tree.Expand(res, TVE_EXPAND);
	nod = m_Tree.InsertItem(_T("修改初始数据"), 1, 1, root);
	m_Tree.SetItemData(nod, WM_EDIT_DEFALSE);
	m_Tree.InsertItem(_T("--------"), 1, 1, root);
	res = m_Tree.InsertItem(_T("修改存档数据"), 1, 1, root);

	for (int i = 1; i <= 10; i++)
	{
		string sp;
		sp = Pal->va(("%s%1d.rpg"), Pal->PalDir.c_str(), i);
		if (IsFileExist(CString(sp.c_str())))
		{
			CString p1;
			p1.Format(_T("%1d.rpg"), i);
			nod = m_Tree.InsertItem(p1, 2, 2, res);
			m_Tree.SetItemData(nod, WM_EDIT_1 + i);
		}
	}
	//m_Tree.InsertItem(_T("........"), 2, 2, res);

	nod = m_Tree.InsertItem(_T("测试"), 1, 1, root);
	m_Tree.SetItemData(nod, WM_Test_Eentry_Edit);
	nod = m_Tree.InsertItem(_T("运行"), 1, 1, root);
	m_Tree.SetItemData(nod, WM_Test_Run);

	Select_Item_Array.clear();
	Select_Item_Array.resize(10);

	//以下制作下拉表数据 0 物品 1 法术 2 毒药 3 攻击附加 4 怪
	for (int i = 0; i < 5; i++)
	{
		pSelect_Item p_Select_Item = &Select_Item_Array[i];

		p_Select_Item->data.resize(MAX_OBJECTS);

		p_Select_Item->row = 0;
		//插入空项
		p_Select_Item->data[0].item = 0;
		p_Select_Item->data[0].row = 0;
		p_Select_Item->data[0].s = "";
		int row = 1;
		if (i == 4)
		{
			//多插入一行 值FFFF
			p_Select_Item->data[1].item =0xffff;
			p_Select_Item->data[1].row = 1;
			p_Select_Item->data[1].s = "禁用";
			row = 2;
		}
		for (int n = 0; n < MAX_OBJECTS; n++)
		{
			if (!(Pal->gpObject_classify[n] & select_flag[i]))
				continue;
			p_Select_Item->data[row].item = n;
			p_Select_Item->data[row].row = row;
			p_Select_Item->data[row].s = p_SrcToStr(n);
			row++;
		}
		p_Select_Item->data.resize(row);
	}

	m_Tree.Expand(res, TVE_EXPAND);
	//展开命令树
	m_Tree.Expand(root, TVE_EXPAND);
	m_Tree.ShowWindow(TRUE);
}


void CEditorAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEditorAppDlg::OnPaint()
{
	//auto getwind();
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEditorAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEditorAppDlg::PostNcDestroy()
{
	// TODO: 在此添加专用代码和/或调用基类

	CString  dir = CString(Pal->PalDir.c_str());
	delete Pal;

	//在此,检查进程是否运行
	CPalEvent::PalQuit = 1;
	while (is_InPalThread)
		Sleep(100);//等待

	Pal = nullptr;

	if ((okExit == 1))
	{
		//清除备份文件
		removeFile(dir);
	}
	else
		//恢复文件清除备份
		restoreFile(dir);
	CDialogEx::PostNcDestroy();
}




void CEditorAppDlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CEditorAppDlg::OnTvnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW p = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	//TODO
	//bug 上一表格处理未结束会发生问题
	// TODO: 在此添加控件通知处理程序代码
	//点击项目变化
	m_Grid.EndEditing();

	if (m_Grid.mUndoArray.size())
	{

		int ret = ::MessageBox(AfxGetApp()->m_pMainWnd->m_hWnd, _T(
			"保存吗？(Y) 保存，(N)不保存,取消 返回"), _T("数据已经修改"),
			MB_YESNOCANCEL);
		if (ret == IDCANCEL)
		{
			//不做处理返回
			return;
		}
		else if (ret != IDYES)
		{
			//清除UNDO
			m_Grid.mUndoArray.clear();
		}
		else
		{
			//更新数据
			DataUpDate(m_Grid.UndoCtrl);
			m_Grid.mUndoArray.clear();
			if (!JobCtrl)
			{
				//存修改后的數據
				Pal->PAL_Make_MKF();
				isUpData = TRUE;
			}
			else if (JobCtrl > 0)
			{
				Pal->PAL_SaveGame(Pal->va("%s%d.rpg", Pal->PalDir.c_str(), JobCtrl), Pal->gpGlobals->bSavedTimes);
				delete Pal;
				Pal = new CGetPalData(JobCtrl, 1);
				isUpData = FALSE;;
				CString ss;
				ss.Format(L"存档 %3d 数据已经更新!", JobCtrl);
				MessageText(ss);
			};
		}
	}
	if (Pal == nullptr || Pal->gpGlobals == nullptr || isUpData)
	{
		CString ss = L"数据已经更新";
		delete Pal;
		Pal = new CGetPalData(0, 1);
		isUpData = FALSE;
		MessageText(ss);
	}
	if (p->itemNew.lParam)
	{
		switch (p->itemNew.lParam)
		{
		case WM_EDIT_DIR:
			Edit_Dir(0, 0);
			break;
		case WM_EDIT_DEFALSE:
			Edit_Defalse(0, 0);
			break;
		case WM_EDIT_PLAYER:
			Edit_Player(0, 0);
			break;
		case WM_EDIT_ENEMY:
			Edit_Enemy(0, 0);
			break;
		case WM_EDIT_MAGIC:
			Edit_Magic(0, 0);
			break;
		case WM_EditEventObject:
			Edit_EventObject(0, 0);
			break;
		case WM_PLAYERLEVELUPMAGIG:
			Edit_PlayerLevelUPMagic(0, 0);
			break;
		case WM_POST_MESSAGE_CSTR:
			//Text_Message(0, 0);
			break;
		case WM_EditBattleField:
			Edit_BattleField(0, 0);
			break;
		case WM_EditStore:
			Edit_Store(0, 0);
			break;
		case WM_EditPoison:
			Edit_Poison(0, 0);
			break;
		case WM_EditScene:
			Edit_Scene(0, 0);
			break;
		case WM_EditObjectItem:
			Edit_ObjectItem(0, 0);
			break;
		case WM_EditEnemyTeam:
			Edit_EnemyTeam(0, 0);
			break;
		case WM_PACKPAL:
			PackPal(0, 0);
			break;
		case WM_UNPACKPAL:
			UnPackPal(0, 0);
			break;
		case WM_ScriptEntry:
			Edit_ScriptEntry(0, 0);
			break;
		case WM_Inventory:
			Edit_Invenyory(0, 0);
			break;
		case WM_OKReturn:
			OK_Return(0, 0);
			break;
		case WM_CanselReturn:
			Cansel_Return(0, 0);
			break;
		case WM_SEND_MSG_STR:
			OnSendMsgStr(0, 0);
			break;
		case WM_EDIT_PARAMETERS:
			Edit_Parameters(0, 0);
			break;
		case WM_MapEdit:
			Edit_Map(0, 0);
			break;
		case WM_EditExplain:
			Edit_Explain(0, 0);
			break;
		case WM_Test_Run:
			OnTestRun(0, 0);
			break;
		case WM_Test_Eentry_Edit:
			OnTestEentryEdit(0, 0);
			break;
		case WM_POST_MESSAGE_SCRIPT:
			OnPostMessageScript(0, 0);
			break;
		case WM_Deploy_On_The_Map:
			OnDeployOnTheMap(0, 0);
			break;
		case WM_EditObjectName:
			Edit_ObjectName(0, 0);
			break;
		case WM_EditDialog:
			Edit_Dialog(0, 0);
			break;
		case WM_EditObjectPict:
			Edit_ObjectPict(0, 0);
			break;
		case WM_EditBettlePict:
			//Edit_BettlePict(0, 0);
			break;
		case WM_PAL_OPTIONS:
			Edit_Pal_Options(0, 0);
			break;
		default:

			if (p->itemNew.lParam > WM_EDIT_1 && p->itemNew.lParam < WM_EDIT_PLAYER)
				Edit_1(0, p->itemNew.lParam - WM_EDIT_1);

			break;
		}
	}
	//取消选择
	m_Tree.SelectItem(0);

	*pResult = 0;
}




void CEditorAppDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (m_Tree.GetSafeHwnd() == NULL)
		return;

	RECT rc;
	GetClientRect( &rc );
	m_Tree.MoveWindow(4,4,200,cy-60);
	m_Grid.MoveWindow(210,4,cx - 220,cy/2 + 60);
	m_Edit.MoveWindow(210,cy/2 +65,cx -220,cy /2 -120);
	m_ButtonCancel.GetClientRect(&rc);
	m_ButtonCancel.MoveWindow(cx - 120,cy - 50,rc.right,rc.bottom);
	m_ButtonOK.MoveWindow(cx - 240, cy - 50, rc.right, rc.bottom);

}


void CEditorAppDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//控制最小尺寸
	lpMMI->ptMinTrackSize.x = 1000;
	lpMMI->ptMinTrackSize.y = 700;
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CEditorAppDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (MessageBoxW(L"退出吗？", L"本操作将确认本次修改", MB_YESNO) == IDNO)
		return;
	okExit = 1;

	CDialogEx::OnOK();
}


void CEditorAppDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码

	if (!okExit && MessageBox(L"退出吗？", L"本操作将退出,并作废本次修改", MB_YESNO) == IDNO)
	{
		return;
	}

	okExit = 0;
	CDialogEx::OnCancel();
}



afx_msg LRESULT CEditorAppDlg::OnSendMsgStr(WPARAM wParam, LPARAM lParam)
{
	//处理同步发送到信息窗口信息串

	MessageText((LPCTSTR)lParam);
	return 0;
}

afx_msg LRESULT CEditorAppDlg::Edit_Explain(WPARAM wParam, LPARAM lParam)
{
	ShowMsg("对象说明编辑");

	if (Pal->gpGlobals->lpObjectDesc == NULL)
		Pal->gpGlobals->lpObjectDesc = Pal->PAL_LoadObjectDesc("desc.dat");

	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);
	WorkCtrl = WM_EditExplain;
	m_Grid.UndoCtrl = WM_EditExplain;
	//建立表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	ColArray w_ColData;
	auto p_SrcToStr = std::bind(&CEditorAppDlg::p_SrcToStr, this, std::placeholders::_1);
	w_ColData.resize(3);
	w_ColData[0].GetData("对象号", 90, 0, ctrl_Fix, tINT);
	w_ColData[1].GetData("名称", 120, 1, ctrl_Fix, tINT, nullptr, p_SrcToStr);
	w_ColData[2].GetData("说明", 4000, 2,ctrl_Edit,tSTR );
	m_Grid.SetColClass(3, w_ColData);
	DataArray s_RowData;
	s_RowData.resize(Pal->g_TextLib.nWords);
	for (int n = 0; n < Pal->g_TextLib.nWords; n++)
	{
		s_RowData[n].Col.resize(3);
		s_RowData[n].oldRow = n + 1;
		s_RowData[n].Col[0] = n;
		s_RowData[n].Col[1] = n;
		LPCSTR pDesc = Pal->PAL_GetObjectDesc(Pal->gpGlobals->lpObjectDesc, n);
		if (pDesc == NULL)
			s_RowData[n].Col[2] = "";
		else
			s_RowData[n].Col[2] = pDesc;
	}
	m_Grid.SetDataClass(s_RowData.size(), 3, s_RowData);
	//不允许添加删除行
	m_Grid.m_popMenuFlags = 0b0111;
	return 0;
}

afx_msg LRESULT CEditorAppDlg::Edit_Parameters(WPARAM wParam, LPARAM lParam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_EDIT_PARAMETERS;
	m_Grid.UndoCtrl = WM_EDIT_PARAMETERS;
	MessageText(_T("修改基本属性"));
	//建立表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	ColArray w_ColData;
	w_ColData.resize(2);
	w_ColData[0].GetData("项目", 90, 0, ctrl_Fix, tSTR);
	w_ColData[1].GetData("数额", 120, 1, ctrl_Edit, tDWORD);
	m_Grid.SetColClass(2, w_ColData);

	DataArray s_RowData;
	LPGLOBALVARS p = Pal->gpGlobals;
	s_RowData.resize(2);//行数
	s_RowData[0].Col.resize(2);
	s_RowData[0].Col[0] = "金钱";
	s_RowData[0].Col[1] =(int) p->dwCash;
	s_RowData[1].Col.resize(2);
	s_RowData[1].Col[0] = "灵壶值";
	s_RowData[1].Col[1] = p->wCollectValue;

	m_Grid.SetDataClass(2, 2, s_RowData);
	return 0;
}




afx_msg LRESULT CEditorAppDlg::Edit_Map(WPARAM wParam, LPARAM lParam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	WorkCtrl = WM_MapEdit;
	m_Grid.UndoCtrl = WM_MapEdit;
	MessageText(_T("地图编辑"));
	//选择目录

	FILE* fpMAP = Pal->UTIL_OpenRequiredFile("map.mkf");
	FILE* fpGOP = Pal->UTIL_OpenRequiredFile("gop.mkf");

	int s_nMap = Pal->PAL_MKFGetChunkCount(fpMAP);
	ColArray w_ColData;
	w_ColData.resize(3);
	w_ColData[0].GetData("地图号", 90, 0, ctrl_Fix, tINT);
	w_ColData[1].GetData("压缩索引长度", 120, 1, ctrl_MapEdit, tNull);
	w_ColData[2].GetData("图像块长度", 120, 2, ctrl_MapEdit, tNull);

	m_Grid.SetColClass(3, w_ColData);

	DataArray s_RowData;
	s_RowData.clear();
	s_RowData.resize(s_nMap);//行数


	for (int n = 0; n < s_nMap; n++)
	{
		s_RowData[n].Col.resize(3);
		s_RowData[n].Col[0] = n;
		int len = Pal->PAL_MKFGetChunkSize(n, fpMAP);
		s_RowData[n].Col[1] = len;
		len = Pal->PAL_MKFGetChunkSize(n, fpGOP);
		s_RowData[n].Col[2] = len;
	}
	m_Grid.SetDataClass(s_nMap, 3, s_RowData);
	m_Grid.m_popMenuFlags = 0b000111;

	fclose(fpMAP);
	fclose(fpGOP);
	return 0;
}



void CEditorAppDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (okExit != -1)
		okExit = 0;
	__super::OnClose();
}


void CAboutDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//调整最小高度与宽度,如果需要的话
	lpMMI->ptMinTrackSize.x = 1300;
	lpMMI->ptMinTrackSize.y = 800;

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void CEditorAppDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nChar == 27 || nChar == 13)
		return;
	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);

}



BOOL CEditorAppDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	//输入CTRL_Z
	if (pMsg->message == WM_KEYDOWN && m_Grid.GetFocus() != &m_ListCtrl &&
		IsCTRLpressed() && (pMsg->wParam == 'Z' ))
	{
		m_Grid.SendMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}


afx_msg LRESULT CEditorAppDlg::OnTestRun(WPARAM wParam, LPARAM lParam)
{

	if (is_InPalThread)//已经运行，返回
		return 0;
	m_Grid.ShowWindow(FALSE);
	//运行测试
	if (wParam )
	{
		DataUpDate(WM_Test_Eentry_Edit);
		SetCurrentDirectory(CString(Pal->PalDir.c_str()));

		CString s;
		int save  = pTestData->sTestData[lParam][0];


		if (wParam == 1)
		{
			s.Format(L"测试运行存档 第 %d 项", (int)lParam);
		}
		else
		{
			s.Format(L"测试系统缺省 第 %d 项", (int)lParam);
			save *= -1;
		}
		MessageText(s);


		is_InPalThread = 1;
		TestData * t = &(pTestData->sTestData[lParam]);
		thread theThread([&](TestData* t, CGetPalData* p)->void {

			GetDlgItem(IDC_TREE1)->EnableWindow(FALSE);
			SDL_Init((Uint32)(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE));
			CGetPalData* pal = new CGetPalData(0, 1);
			CPalEvent::PalQuit = FALSE;
			class RunInThread* a = new RunInThread(t, pal);
			delete a;
			CPalEvent::PalQuit = 0;
			is_InPalThread = 0;
			delete pal;
			SDL_Quit();
			GetDlgItem(IDC_TREE1)->EnableWindow(TRUE);
			}, t, Pal);
		theThread.detach();
		m_Grid.ShowWindow(FALSE);

	}
	else
	{
		//运行
		MessageText(L"运行");
		SetCurrentDirectory(CString(Pal->PalDir.c_str()));

		is_InPalThread = 1;
		if (Pal)delete Pal;
		Pal = nullptr;

		thread theThread([]()->void {
			SDL_Init((Uint32)(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE));
			CPalEvent::PalQuit = FALSE;
			class CGetPalData* a = new CGetPalData;
			delete a;
			SDL_Quit();
			CPalEvent::PalQuit = 0;
			is_InPalThread = 0;//结束标识
			});
		theThread.join();
		m_Grid.ShowWindow(FALSE);


		//删除所有鼠标信息
		MSG msg;
		while (PeekMessage(&msg, this->m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE));

		//再次载入数据
		Pal = new CGetPalData(0, 1);
	}
	return 0;
}

afx_msg LRESULT CEditorAppDlg::OnTestEentryEdit(WPARAM wParam, LPARAM lParam)
{
	MessageText(L"修改测试入口");
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	WorkCtrl = WM_Test_Eentry_Edit;
	m_Grid.UndoCtrl = WM_Test_Eentry_Edit;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	if (!pTestData) pTestData = new CTestData(Pal);
	//建立表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	ColArray w_ColData;
	w_ColData.resize(t_End+1);
	w_ColData[0].GetData("存档号", 90, 0, ctrl_Fix, tDWORD);
	w_ColData[1].GetData("位置X", 90, 1, ctrl_Edit, tDWORD);
	w_ColData[2].GetData("位置Y", 90, 2, ctrl_Edit, tDWORD);
	w_ColData[3].GetData("组队数", 90, 3, ctrl_Edit, tDWORD);
	w_ColData[4].GetData("场景号", 90, 4, ctrl_Edit, tDWORD);
	w_ColData[5].GetData("夜间", 90, 5, ctrl_Edit, tDWORD);
	w_ColData[6].GetData("队伍方向", 90, 6, ctrl_Edit, tDWORD);
	w_ColData[7].GetData("音乐", 90, 7, ctrl_Edit, tDWORD);
	w_ColData[8].GetData("战斗音乐", 90, 8, ctrl_Edit, tDWORD);
	w_ColData[9].GetData("战斗场景", 90, 9, ctrl_Edit, tDWORD);
	w_ColData[10].GetData("屏幕波动", 90, 10, ctrl_Edit, tDWORD);
	w_ColData[11].GetData("灵葫值", 90, 11, ctrl_Edit, tDWORD);
	w_ColData[12].GetData("排列", 90, 12, ctrl_Edit, tDWORD);
	w_ColData[13].GetData("引怪值", 90, 13, ctrl_Edit, tDWORD);
	w_ColData[14].GetData("剩余周期", 90, 14, ctrl_Edit, tDWORD);
	w_ColData[15].GetData("跟随", 90, 15, ctrl_Edit, tDWORD);
	w_ColData[16].GetData("金钱", 90, 16, ctrl_Edit, tDWORD);
	w_ColData[17].GetData("无敌", 90, 17, ctrl_Edit, tBOOL);
	w_ColData[18].GetData("快速灭怪", 90, 18, ctrl_Edit, tBOOL);
	w_ColData[19].GetData("显示对象", 90, 19, ctrl_Edit, tBOOL);
	w_ColData[20].GetData("装备脚本", 90, 20, ctrl_Edit, tBOOL);
	w_ColData[21].GetData("自动脚本", 90, 21, ctrl_Edit, tBOOL);

	m_Grid.SetColClass(t_End, w_ColData);

	DataArray s_RowData;
	s_RowData.resize(pTestData->sTestData.size());
	for (int n = 0; n < pTestData->sTestData.size(); n++)
	{
		s_RowData[n].Col.resize(t_End);
		const TestData& td = pTestData->sTestData[n];
		for (int i = 0; i < t_End; i++)
		{
			s_RowData[n].Col[i] = td[i];
		}
	}

	m_Grid.SetDataClass(pTestData->sTestData.size(), t_End , s_RowData);
	m_Grid.m_popMenuFlags = 0b0110000111;

	return 0;
}

afx_msg LRESULT CEditorAppDlg::OnPostMessageScript(WPARAM wParam, LPARAM lParam)
{
	//在信息窗口显示调试信息，需实时传递
	LPSCRIPTENTRY     pScript = (LPSCRIPTENTRY)lParam;
	WORD Script = PAL_X(wParam);
	WORD EventId = PAL_Y(wParam);
	CString s;
	s.Format(L"[SCRIPT]入口 %.4X:发起对象 %.4X:操作 %.4X %.4X %.4X %.4X\t %s\n", Script, EventId,
		pScript->wOperation, pScript->rgwOperand[0], pScript->rgwOperand[1],
		pScript->rgwOperand[2], (LPCWSTR)CString(p_Script(pScript->wOperation)));

	MessageText(s);

	return 0;
}

afx_msg LRESULT CEditorAppDlg::OnDeployOnTheMap(WPARAM wParam, LPARAM lParam)
{
	MessageText(L"在地图上部署");
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.ShowWindow(FALSE);
	//
	CMap_Dlg dMap(this,1);
	dMap.m_nCscene = lParam + 1;
	dMap.m_nMap = Pal->gpGlobals->g.rgScene[lParam].wMapNum;
	INT_PTR nResponse = dMap.DoModal();
	m_Grid.ShowWindow(TRUE);

	return 0;

}

afx_msg LRESULT CEditorAppDlg::Edit_The_Map(WPARAM wParam, LPARAM lParam)
{
	//地图编辑
	CString s;
	s.Format(L"选择地图  %d", lParam);
	MessageText(s.GetBuffer());
	CMap_Dlg s_MapDlg(this);
	s_MapDlg.m_nMap = lParam;
	s_MapDlg.DoModal();
	if (s_MapDlg.isChanged())
		//地图已经改动，重新载入
		PostMessage(WM_MapEdit, 0, 0);
	return 0;
}

afx_msg LRESULT CEditorAppDlg::Edit_ObjectName(WPARAM wParam, LPARAM lParam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	ShowMsg("对象名称编辑");
	WorkCtrl = WM_EditObjectName;
	m_Grid.UndoCtrl = WM_EditObjectName;
	//建立表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	ColArray w_ColData;
	w_ColData.resize(2);
	w_ColData[0].GetData("对象号", 90, 0, ctrl_Fix, tINT);
	w_ColData[1].GetData("名称", 120, 1, ctrl_Edit, tSTR);
	m_Grid.SetColClass(2, w_ColData);
	DataArray s_RowData;
	s_RowData.resize(Pal->g_TextLib.nWords);
	for (int n = 0; n < Pal->g_TextLib.nWords; n++)
	{
		s_RowData[n].Col.resize(2);
		s_RowData[n].oldRow = n + 1;
		s_RowData[n].Col[0] = n ;
		s_RowData[n].Col[1] = Pal->PAL_GetWord(n );
	}
	m_Grid.SetDataClass(s_RowData.size(), 2, s_RowData);
	//不允许添加删除行
	m_Grid.m_popMenuFlags = 0b0111;
	return 0;
}

afx_msg LRESULT CEditorAppDlg::Edit_Dialog(WPARAM wParam, LPARAM lParam)
{
	//备份对话文件
	CString fname ( Pal->PalDir.c_str());
	fname +="m.msg";

	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	ShowMsg("对话编辑");
	ShowMsg("符号说明:字体颜色 _ _ 青色 \' \' 红色 \" \" 黄色" );
	ShowMsg("~00 延迟并退出 ( )设置等待图标 $00 文本显示延迟时间");
	WorkCtrl = WM_EditDialog;
	m_Grid.UndoCtrl = WM_EditDialog;
	//建立表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	ColArray w_ColData;
	w_ColData.resize(4);
	w_ColData[0].GetData("序号", 90, 0, ctrl_Fix, tINT);
	w_ColData[1].GetData("16进制", 90, 1, ctrl_Fix, tHEX);
	w_ColData[2].GetData("有效", 50, 1, ctrl_Fix, tBOOL);
	w_ColData[3].GetData("内容", 3000, 1, ctrl_Edit, tSTR);
	m_Grid.SetColClass(4, w_ColData);
	DataArray s_RowData;
	s_RowData.resize(Pal->g_TextLib.nMsgs);
	for (int n = 0; n < Pal->g_TextLib.nMsgs; n++)
	{
		s_RowData[n].Col.resize(4);
		s_RowData[n].oldRow = n + 1;
		s_RowData[n].Col[0] = n;
		s_RowData[n].Col[1] = n;
		s_RowData[n].Col[2] = 0;
		LPSTR sStr = Pal->PAL_GetMsg(n);
		s_RowData[n].Col[3] = string(sStr);
	}
	//确定对话是否被使用
	for (int n_Row = 0; n_Row < Pal->gpGlobals->g.nScriptEntry; n_Row++)
	{
		auto p = &Pal->gpGlobals->g;
		if (p->lprgScriptEntry[n_Row].wOperation == 0xffff &&
			p->lprgScriptEntry[n_Row].rgwOperand[0] < Pal->g_TextLib.nMsgs)
		{
			//已经被使用
			s_RowData[p->lprgScriptEntry[n_Row].rgwOperand[0]].Col[2] = 1;
		}
	}
	m_Grid.SetDataClass(s_RowData.size(), 4, s_RowData);
	//允许从尾部添加
	m_Grid.m_popMenuFlags = 0b010111;
	return 0;
}

#include "PackedPict_Dlg.h"
#include "NoPackedPict_Dlg.h"
#include "MapPict_Dlg.h"
afx_msg LRESULT CEditorAppDlg::Edit_ObjectPict(WPARAM wParam, LPARAM lParam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	ShowMsg("对象图像编辑");
	//选择文件
	CFileDialog file(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"对象图像|mgo.mkf|敌方战斗图像|abc.mkf|我方战斗图像|f.mkf|魔法特效|fire.mkf|"
		L"背景图片 |fbp.mkf| 头像 |rgm.mkf| 对象图片 |ball.mkf|"
		L" 地图文件 |map.mkf| All Files(*.*) | *.*|||");
	CString fileDir = CString(Pal->PalDir.c_str());
	file.m_ofn.lpstrInitialDir = fileDir.GetString();
	if (file.DoModal() != IDOK)
		return 0;
	CString UPfilename = file.GetFileName().MakeUpper();//转换成大写文件名
	if (UPfilename == CString(L"BALL.MKF")
		|| UPfilename == CString(L"RGM.MKF")
		|| UPfilename == CString(L"FBP.MKF"))
	{
		//单个图像
		NoPackedPict_Dlg m(this, &file.m_ofn);
		m.DoModal();
	}
	else if (UPfilename == CString(L"MAP.MKF"))
	{
		//地图图像
		MapPict_Dlg m(this,&file.m_ofn);
		m.DoModal();
	}
	else
	{
		//压缩图像
		//打开对话框
		PackedPict_Dlg m(this, &file.m_ofn);
		m.DoModal();
	}
	Pal->PAL_ReloadPAL(FALSE);
	//
	return 0;
}

//
afx_msg LRESULT CEditorAppDlg::Edit_BettlePict(WPARAM wParam, LPARAM lParam)
{
	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	ShowMsg("战斗图像编辑");
	return 0;
}

LPCSTR szsm[] =
{
	"增加毒的烈度",
	"修改伤害计算公式（包括敌人的属性计算方式）",
	"在战斗中显示相关数据",
	"在后期加强前期的敌人、队伍多于3人，敌人加强",
	"修改灵葫炼丹为商店",
	"当mp减少时也显示数值",
	"在一定情况下添加状态总是成功，无视巫抗",
	"补体力真气时已满返回失败",
	"显示驱魔香和十里香步数",
	"主動防禦j时，防御加強",
	"设置冷却值 在之后的 n 次夺魂无效",
	"修改附加属性",
	"修改战后衩覆盖的脚本",
	"修改附加经验计算方式",
	"额外附加经验",
	"额外恢复",
	"不完全是随机选择目标",
	"敌人最多行动次数，如果为2则是经典版",
	"某些毒可以对任何人均命中（无视敌方巫抗或我方毒抗）",
	"有特色的加强主角，灵儿初始五灵抗性20%，阿奴毒抗巫抗各30%，林月如额外恢复",
	"天罡战气后，投掷偷武器伤害增加",
	"使用梦蛇后，各项属性经验增加",
	"自动防御比率",
	"BATTLE_FRAME_TIME 战时每帧毫秒数 40",
	"非战时每帧毫秒数 100",
	"= 1 怪物混乱攻击同伴, = 2 怪物混乱无同伴攻击我方",
	"怪物分裂体力减半",

	"音频设备号 -1 系统缺省",
	"SurroundOPLOffset 缺省值384",
	"声道数2 双声道，1 单声道",
	"采样率 缺省为44100",
	"OPL 采样率，缺省为 49716",
	"采样质量 0 到 4 缺省为 4",
	"音乐音量 缺省为100",
	"效果音量 缺省为100",
	"音乐格式 0 MIDE,1 RIX 2 MP3",
	"OPL核心",
	"OPLChip",
	"MIDI设置",
	"优先使用MP3 1 是",
	"是否使用Big5 繁体字，1 是",
	"使用环绕OPL",
	"保持窗口显示比例， 1 是",
	"是否使用过场动画AVI，1 是",
	"音乐缓存区大小 为1024",
	"字体设置，0 宋体，1 仿宋体 ，2 黑体 3 幼圆体 ，4 楷体",
	"字体粗细， 1-10",
	"最大存档文件数5-10",
	"加速或减速 -80%--+500%。" ,
};

//修改系统设置
afx_msg LRESULT CEditorAppDlg::Edit_Pal_Options(WPARAM wParam, LPARAM lParam)
{

	WorkCtrl = WM_PAL_OPTIONS;
	m_Grid.UndoCtrl = WM_PAL_OPTIONS;
	const  int rowSize = sizeof(szsm) / sizeof(char*);

	//清空上一轮的撤消操作
	m_Grid.mUndoArray.clear();
	m_Grid.UndoCount = 0;
	m_Grid.Set_Set_Col_Data(nullptr);
	m_Grid.ShowWindow(FALSE);

	ShowMsg("修改游戏设置");
	//建立表头
	//参数 文字，列宽，列号，（类型 0 什么都不做，1，列头固定(永远显示)，2，Edit），
	//（原数据类型 0  整数 ，1 WORD, 2 SHORT, 3 16进制数，4 浮点数，5 字符串）
	//(下拉框数据索引)，（指向函数的指针，将源数据转化为显示数据字串），（指向函数的指针，将显示字串转化为源）
	ColArray w_ColData;
	w_ColData.resize(4);
	w_ColData[0].GetData("序号", 90, 1, ctrl_Fix, tINT);
	w_ColData[1].GetData(" 值 ", 90, 0, ctrl_Edit, tINT);
	w_ColData[2].GetData("说明", 3000, 1, ctrl_Fix, tSTR);
	m_Grid.SetColClass(3, w_ColData);
	//内容数据
	DataArray s_RowData;
	s_RowData.resize(rowSize);
	for (int n = 0; n < rowSize; n++)
	{
		s_RowData[n].Col.resize(3);
		s_RowData[n].Col[0] = n + 1;
		s_RowData[n].Col[1] = Pal->gConfig->m_Function_Set[n];
		s_RowData[n].Col[2] = szsm[n];
	}
	m_Grid.SetDataClass(rowSize, 3, s_RowData);
	//不能插入
	m_Grid.m_popMenuFlags = 0b00111;
	return 0;
}


afx_msg LRESULT CEditorAppDlg::OnListscriptentry(WPARAM wParam, LPARAM lParam)
{
	//查看脚本
	CListScriptEntry m(this,wParam);
	m.DoModal();
	return 0;
}

afx_msg LRESULT CEditorAppDlg::OnScriptCaller(WPARAM wParam, LPARAM lParam)
{
	//查看脚本调用
	CScriptCaller m(this, wParam);
	m.DoModal();
	return 0;
}
