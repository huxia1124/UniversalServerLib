//////////////////////////////////////////////////////////////////////
//
// STXAnchor.h: interface for the CSTXAnchor class.
//
// CSTXAnchor: An easy-use Anchor class
// Version 0.3
// Last Update: 2005-08-12
// by XiaHuan
// Copyright (C) 2005.
//
// This file is part of the STXControlLibrary by XiaHuan.
// No guarantee or warranty if used without notification.
// To get more information on STXControlLibrary, please contact.
// 
// E-mail:	xihhh_1@163.com
// QQ:		123449028
//
//////////////////////////////////////////////////////////////////////


#pragma once

#include <vector>

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

#define STXANCHOR_NOANCHOR		0x0000
#define STXANCHOR_LEFT			0x0001
#define STXANCHOR_TOP			0x0002
#define STXANCHOR_RIGHT			0x0004
#define STXANCHOR_BOTTOM			0x0008
#define STXANCHOR_TOPLEFT		(STXANCHOR_TOP|STXANCHOR_LEFT)
#define STXANCHOR_BOTTOMRIGHT	(STXANCHOR_BOTTOM|STXANCHOR_RIGHT)
#define STXANCHOR_ALL			(STXANCHOR_TOPLEFT|STXANCHOR_BOTTOMRIGHT)

#define STXANCHOR_ITEM_WINDOW	1		//Pointer to a window
#define STXANCHOR_ITEM_DLGITEM	2		//ID of a child window


//////////////////////////////////////////////////////////////////////////

class CSTXAnchor;
class CSTXAnchorItem;

//////////////////////////////////////////////////////////////////////////


typedef struct tagSTXANCHORITEMINFO
{
	BYTE iItemType;
	DWORD dwAnchor;
	BOOL bInitialized;
	RECT rcItem;	//client rect in parent window
	DWORD_PTR dwAnchorTool;	//CWnd* or UINT
}STXANCHORITEMINFO,*LPSTXANCHORITEMINFO;

class CSTXAnchorItem
{
public:
	CSTXAnchorItem(HWND hToolWnd,DWORD dwAnchor);
	CSTXAnchorItem(UINT uDlgItemID,DWORD dwAnchor);
	virtual ~CSTXAnchorItem();

public:
	void Adjust(int cxDelta,int cyDelta);
	void SetParentAnchor(CSTXAnchor *pParentAnchor);

protected:
	STXANCHORITEMINFO m_AnchorItemInfo;
	CSTXAnchor *m_pParentAnchor;

};


//////////////////////////////////////////////////////////////////////////

class CSTXAnchor  
{
public:
	CSTXAnchor();
	CSTXAnchor(HWND hSizeTraceWnd);
	virtual ~CSTXAnchor();

public:
	void DeleteAllItems();
	void InsertItem(int iIndex,HWND pToolWnd,DWORD dwAnchor);
	void InsertItem(int iIndex,UINT uDlgItemID,DWORD dwAnchor);
	void AddItem(HWND pToolWnd,DWORD dwAnchor);
	void AddItem(UINT uDlgItemID,DWORD dwAnchor);
	void AddAllChildren(DWORD dwAnchor);

	void Adjust(int cxOld,int cyOld,int cxNew,int cyNew);
	void Adjust(int cxDelta,int cyDelta);
	HWND GetTraceWindow();

protected:
	HWND m_hSizeTraceWnd;
	WNDPROC m_pfnOldProc;
	SIZE m_sizeTrace;
	std::vector<CSTXAnchorItem*> m_arrpItems;
	int m_iAnchorID;

protected:
	int GetTraceWindowWidth() const;
	int GetTraceWindowHeight() const;
	void SetTraceSize(int cx,int cy);

private:
	static LRESULT CALLBACK NewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam);

private:
	int s_AnchorBaseID;
};

