//
//Copyright(c) 2016. Huan Xia
//
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
//documentation files(the "Software"), to deal in the Software without restriction, including without limitation
//the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
//and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all copies or substantial portions
//of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
//THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
//CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//DEALINGS IN THE SOFTWARE.

#pragma once

#include <UIAnimation.h>
#include "STXAccessibleWrapperBase.h"
#include <tchar.h>
#include <memory>
#include <list>
#include <vector>
#include <queue>
#include <set>
#include <map>

#include <gdiplus.h>
#include <atlcomcli.h>

//////////////////////////////////////////////////////////////////////////
class CSTXAnimatedTreeCtrlNS;
class CSTXAnimatedTreeNodeNS;
typedef CSTXAnimatedTreeNodeNS* HSTXTREENODE;

#define STXTVI_ROOT                ((HSTXTREENODE)(ULONG_PTR)-0x10000)
#define STXTVI_FIRST               ((HSTXTREENODE)(ULONG_PTR)-0x0FFFF)
#define STXTVI_LAST                ((HSTXTREENODE)(ULONG_PTR)-0x0FFFE)
#define STXTVI_SORT                ((HSTXTREENODE)(ULONG_PTR)-0x0FFFD)

#define STXATHT_ONITEMEXPANDER			0x0001
#define STXATHT_ONITEMIMAGE				0x0002
#define STXATHT_ONITEMBOTTOM			0x0100		//on the bottom half

//Expand Operations
#define STXATVE_NONE				0x0000
#define STXATVE_COLLAPSE			0x0001
#define STXATVE_EXPAND				0x0002
#define STXATVE_TOGGLE				0x0003
#define STXATVE_COLLAPSERESET		0x0100

//Node Navigation
#define STXATVGN_ROOT               0x0000
#define STXATVGN_NEXT               0x0001
#define STXATVGN_PREVIOUS           0x0002
#define STXATVGN_PARENT             0x0003
#define STXATVGN_CHILD              0x0004
#define STXATVGN_FIRSTVISIBLE       0x0005			//Not Implemented
#define STXATVGN_NEXTVISIBLE        0x0006
#define STXATVGN_PREVIOUSVISIBLE    0x0007
#define STXATVGN_DROPHILITE         0x0008			//Not Implemented
#define STXATVGN_CARET              0x0009

//Node State
#define STXTVIS_FORCE_SHOW_EXPANDER			0x00000001
#define STXTVIS_EXPANDEDONCE				0x00000040
#define STXTVIS_HANDCURSOR					0x00000100
#define STXTVIS_SELECTED					0x00010000
#define STXTVIS_HOVER						0x00020000
#define STXTVIS_GRAY_IMAGE					0x01000000
#define STXTVIS_IMAGE_CALLBACK				0x02000000
#define STXTVIS_SUB_TEXT					0x04000000		//Use sub-text instead of sub-image at the right-bottom corner of item image 

//TreeControl State
#define STXTVS_NO_LINES						0x0001
#define STXTVS_NO_EXPANDER_FADE				0x0002
#define STXTVS_DRAW_GRAY_IMAGE				0x0004
#define STXTVS_SINGLE_CLICK_EXPAND			0x0008
#define STXTVS_HORIZONTAL_SCROLL			0x0010		//Not implemented

//Watermark Location
#define STXTV_WATERMARK_LEFT_TOP			0x00
#define STXTV_WATERMARK_LEFT_BOTTOM			0x01
#define STXTV_WATERMARK_RIGHT_TOP			0x02
#define STXTV_WATERMARK_RIGHT_BOTTOM		0x03


//////////////////////////////////////////////////////////////////////////

#define STXTV_STAGE_PREPAINT			1
#define STXTV_STAGE_BACKGROUND			2
#define STXTV_STAGE_IMAGE				3
#define STXTV_STAGE_SUBIMAGE			4
#define STXTV_STAGE_TEXT				5
#define STXTV_STAGE_POSTPAINT			10

#define STXTV_CDRF_DODEFAULT			0x00000000
#define STXTV_CDRF_SKIPDEFAULT			0x00000001

//////////////////////////////////////////////////////////////////////////

typedef int (CALLBACK * STXAnimatedTreeSearchCompareFuncType)(LPARAM lParamItem, LPARAM lParamFind);
typedef int (CALLBACK * STXAnimatedTreeSearchItemCompareFuncType)(HSTXTREENODE hItem, LPARAM lParamFind);
typedef int (CALLBACK * STXAnimatedTreeSortFuncType)(LPARAM lParamItem1, LPARAM lParamItem2, LPARAM lParamSort);
typedef int (CALLBACK * STXAnimatedTreeSortItemFuncType)(HSTXTREENODE hItem1, HSTXTREENODE hItem2, LPARAM lParamSort);

typedef struct tagSTXTVITEMDRAW
{
	HSTXTREENODE hItem;
	DWORD dwWindowStyle;
	DWORD dwItemStyle;
	DWORD dwItemState;
	Gdiplus::Graphics *pGraphics;
	LPARAM lParamItem;
	DWORD dwStage;
	Gdiplus::RectF* rectItem;
	Gdiplus::RectF* rectPart;
	DOUBLE fOpacity;
}STXTVITEMDRAW, *LPSTXTVITEMDRAW;

typedef int (CALLBACK * STXAnimatedTreeItemDrawFuncType)(LPSTXTVITEMDRAW lpItemDraw);


typedef struct tagSTXTVSORTFUNC
{
	STXAnimatedTreeSortFuncType pfnSortFunc;
	LPARAM lParamSort;
}STXTVSORTFUNC, *LPSTXTVSORTFUNC;

//////////////////////////////////////////////////////////////////////////
#define STXATVN_ITEMCLICK				1
#define STXATVN_ITEMDBLCLICK			2
#define STXATVN_HOVERITEMCHANGED		5
#define STXATVN_SELECTEDITEMCHANGED		6
#define STXATVN_PREDELETEITEM			7
#define STXATVN_POSTDELETEITEM			8
#define STXATVN_ITEMMOUSEFLOAT			9
#define STXATVN_ENDEDIT					10
#define STXATVN_ITEMEXPANDING			20
#define STXATVN_ITEMEXPANDED			21
#define STXATVN_CLICK					22


typedef struct tagSTXATVNITEM
{
	NMHDR hdr;
	DWORD_PTR dwNotifySpec;
	HSTXTREENODE hNode;
	DWORD_PTR dwItemData;
}STXATVNITEM, *LPSTXATVNITEM;

//////////////////////////////////////////////////////////////////////////
// CSTXAnimatedTreeNodeNS

class CSTXAnimatedTreeNodeNS
{
	friend class CSTXAnimatedTreeCtrlNS;
protected:
	CSTXAnimatedTreeNodeNS(CSTXAnimatedTreeCtrlNS *pParentControl);
public:
	virtual ~CSTXAnimatedTreeNodeNS();

protected:
	CComPtr<IUIAnimationVariable> m_pAVLeftOffset;
	int m_nLeftOffsetFix;		//Non-animation offset. used as a base offset when calculating
	CComPtr<IUIAnimationVariable> m_pAVTopOffset;		//Offset of first child is 0
	CComPtr<IUIAnimationVariable> m_pAVOpacity;			//0.0 - 1.0
	CComPtr<IUIAnimationVariable> m_pAVImageScale;		//1.5 - 1.0
	CComPtr<IUIAnimationVariable> m_pAVImageOpacity;	//0.0 - 1.0
	CComPtr<IUIAnimationVariable> m_pAVImageOccupy;		//0-
	CComPtr<IUIAnimationVariable> m_pAVItemHeight;		//
	CComPtr<IUIAnimationVariable> m_pAVHoverOpacity;	//0.0 - 1.0


protected:
	std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > m_arrChildNodes;
	std::queue<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > m_queDeletedNodes;
	std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> >::iterator m_itPositionInParent;
	CSTXAnimatedTreeNodeNS *m_pParentNode;
	std::tr1::shared_ptr<Gdiplus::Image> m_pImgImage;		//Node Image
	std::tr1::shared_ptr<Gdiplus::Image> m_pImgSubImage;	//Node SubImage (small image at the right-bottom of the main image)
	CSTXAnimatedTreeCtrlNS *m_pParentControl;

protected:
	BOOL m_bExpanded;
	BOOL m_bSelected;
	BOOL m_bHover;
	int m_iDropHighlight;		//0: none, 1: before, 2: after
	int m_nVOffsetDeleting;
	long m_nAccID;
	DWORD m_dwItemStyle;
	int m_nBoundsMaxX;			//the right-most edge of this item. -1 means not calculated or need update

	INT_PTR m_iIndexBeforeSort;		//Used internally to keep track of the original index
	BOOL m_bPreDeleteNotified;		//Used internally
	BOOL m_bInitialized;			//Used internally


protected:
	DWORD_PTR m_dwNodeData;

#ifdef UNICODE
	std::wstring m_strText;
	std::wstring m_strSubText;		//Will be drawn at the right-bottom corner of the item image if STXTVIS_SUB_TEXT specified
#else
	std::string m_strText;
	std::string m_strSubText;
#endif

protected:
	static void DrawImage(Gdiplus::Graphics *pGraphics, Gdiplus::Image *pImage, DOUBLE fOpacity, Gdiplus::RectF *rectImage);

protected:
	DOUBLE GetCurrentHeight();		//Total Height, including the node itself and its children
	DOUBLE GetCurrentHeightWithoutLastChildExpand();
	DOUBLE GetFinalHeight();		//Total Height, including the node itself and its children
	DOUBLE GetFinalHeightExpanded();
	void DrawItem( Gdiplus::Graphics *pGraphics, Gdiplus::RectF* rectItem, Gdiplus::RectF* rectItemFinal, DOUBLE fBaseOffset, Gdiplus::REAL fHorzOffset, int nHeightLimit, DOUBLE fBaseOpacity, DOUBLE fGlobalExpanderOpacity);
	void DrawExpandMark(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, DOUBLE fGlobalExpanderOpacity);
	void DrawLines(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, DOUBLE fGlobalLineOpacity);
	void DrawHover(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, int iImageOccupie);
	void DrawSelection(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, int iImageOccupie);
	void DrawDropHighlight( Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, DOUBLE fNodeOpacity);
	int DrawItemImage(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, LPSTXTVITEMDRAW lpItemDraw);
	std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> HitTestInternal(POINT pt, int iControlWidth, int iOffsetPatch, UINT *pFlags);
	HSTXTREENODE HitTest(POINT pt, int iControlWidth, int iOffsetPatch, UINT *pFlags);
	void TriggerItemPreDeleteEvent();
	void TriggerItemPostDeleteEvent();
	void OnMouseEnter();
	void OnMouseLeave();
	void MarkDeleting();
	void GetItemScreenRect(__out LPRECT lprcRect);

protected:

};

//////////////////////////////////////////////////////////////////////////
// CSTXAnimatedTreeCtrlNS

class CSTXAnimatedTreeCtrlNS
	: public IUIAnimationManagerEventHandler
	, public CSHAccessibleHWNDBase
{
	friend class CSTXAnimatedTreeNodeNS;
public:
	CSTXAnimatedTreeCtrlNS(void);
	virtual ~CSTXAnimatedTreeCtrlNS(void);

protected:
	struct SORT_STRUCTURE
	{
		int nSortType;
		union
		{
			STXAnimatedTreeSortFuncType pfnSort;
			STXAnimatedTreeSortItemFuncType pfnSortItem;
		};
		LPARAM lParamSort;
	};

public:
	STDMETHOD(QueryInterface)(REFIID riid, __out void **ppv)
	{
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)()
	{
		return 1;
	}
	STDMETHOD_(ULONG, Release)()
	{
		return 1;
	}

protected:
	CComPtr<IUIAnimationManager> m_AnimationManager;
	CComPtr<IUIAnimationTimer> m_AnimationTimer;
	CComPtr<IUIAnimationTransitionLibrary> m_AnimationTransitionLibrary;

protected:
	UI_ANIMATION_SECONDS m_nDefaultAnimationDuration;
	CComPtr<IUIAnimationVariable> m_pAVExpanderOpacity;

protected:
	std::tr1::shared_ptr<Gdiplus::Image> m_pImgBackground;		//Control Background Image
	std::tr1::shared_ptr<Gdiplus::CachedBitmap> m_pImgBackgroundCached;
	std::tr1::shared_ptr<Gdiplus::Image> m_pImgWatermark;
	std::tr1::shared_ptr<Gdiplus::CachedBitmap> m_pImgWatermarkCached;
	UINT m_nWatermarkLocation;
	double m_fWatermarkOpacity;

	std::tr1::shared_ptr<Gdiplus::Image> m_pImgExpanderExpanded;
	std::tr1::shared_ptr<Gdiplus::Image> m_pImgExpanderCollapsed;


protected:
	std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > m_arrRootNodes;
	std::queue<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > m_queDeletedNodes;
	std::set<CSTXAnimatedTreeNodeNS*> m_setValidItemPtr;
	HSTXTREENODE m_hSelectedNode;
	HSTXTREENODE m_hDragOverNode;
	Gdiplus::Color m_clrBackground;
	std::vector<HSTXTREENODE> m_arrEnumItems;
	std::map<long, HSTXTREENODE> m_mapAccIdToItem;

	HSTXTREENODE m_hLastHoverNode;
	HSTXTREENODE m_hLastFloatTriggeredNode;
	STXAnimatedTreeItemDrawFuncType m_pfnItemDrawFunc;

	STXTVSORTFUNC m_sortFuncDefault;
	static SORT_STRUCTURE s_sortStructure;		//Thread unsafe

	LOGFONT m_lfDefaultFont;
	LOGFONT m_lfDefaultSubTextFont;
	Gdiplus::Font *m_pDefaultFont;
	Gdiplus::Font *m_pDefaultSubTextFont;


protected:
	HWND m_hwndControl;
	BOOL m_bMouseInControl;
	BOOL m_bLButtonDown;
	POINT m_ptLButtonDown;
	UINT m_nLButtonDownHitFlags;
	BOOL m_bDragging;
	BOOL m_bAllowDragDrop;
	int m_nItemIndent;

protected:
	static LPCTSTR s_lpszAnimatedTreeCtrlClassName;
	static LRESULT CALLBACK STXAnimatedTreeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static bool STXAnimatedTreeDefaultSortFuncWrapper(std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pNode1, std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pNode2);

protected:
	void OnTimer(UINT nIDEvent);
	void OnPaint(HDC hDC);
	void OnHScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar);
	void OnVScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar);
	void OnMouseMove(int x, int y, UINT nFlags);
	void OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	void OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton = FALSE);
	void OnRButtonDown(int x, int y, UINT nFlags);
	void OnRButtonUp(int x, int y, UINT nFlags);
	void OnLButtonDblClk(int x, int y, UINT nFlags);
	void OnMouseWheel(UINT nFlags, short zDelta, int x, int y);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyDown_Left();
	void OnKeyDown_Right();
	void OnMouseLeave();
	void OnSetFocus(HWND hWndOldFocus);
	void OnKillFocus(HWND hWndNewFocus);
	void OnDestroy();
	BOOL OnSetCursor(HWND hWnd, UINT nHitTest, UINT message);
	UINT OnGetDlgCode();
	LRESULT OnGetObject(DWORD dwFlags, DWORD dwObjId);
	void OnSize(UINT nType, int cx, int cy);


	void DrawControl(HDC hDC);
	void DrawBackground(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis);
	void DrawWatermark(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis);
	void DrawContent(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis);
	void DrawLine(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis);

protected:
	static std::tr1::shared_ptr<Gdiplus::Image> GetResizedImage(IStream *pStream, int nWidthHeight);
	static std::tr1::shared_ptr<Gdiplus::Image> GetResizedImage(HBITMAP hBitmap, int nWidthHeight);
	static std::tr1::shared_ptr<Gdiplus::Image> GetResizedImage(LPCTSTR lpszFile, int nWidthHeight);
	static std::tr1::shared_ptr<Gdiplus::Image> GetResizedImage(std::tr1::shared_ptr<Gdiplus::Image> pImage, int nWidthHeight);

protected:
	STDMETHOD(OnManagerStatusChanged)(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus);

protected:
	virtual HWND v_GetWindow();
	virtual long _GetChildCount();

	IFACEMETHODIMP accHitTest(long xLeft, long yTop, __out VARIANT *pvarChildAtPoint);
	IFACEMETHODIMP accLocation(__out long *pxLeft, __out long *pyTop, __out long *pcxWidth, __out long *pcyHeight, VARIANT varChild);
	IFACEMETHODIMP get_accName(VARIANT varChild, __out BSTR *pbstrName);
	//IFACEMETHODIMP get_accChildCount(__out long* pChildCount);
	IFACEMETHODIMP get_accDefaultAction(VARIANT varChild, __out BSTR* pszDefaultAction);
	IFACEMETHODIMP accDoDefaultAction(VARIANT varChild);
	IFACEMETHODIMP accNavigate(long navDir, VARIANT varStart, __out VARIANT * pvarEndUpAt);
	IFACEMETHODIMP get_accSelection(__out VARIANT * pvarSelectedChildren);
	IFACEMETHODIMP get_accState(VARIANT varChild, __out VARIANT *pvarState);
	IFACEMETHODIMP accSelect(long flagsSel, VARIANT varChild);
	IFACEMETHODIMP get_accRole(VARIANT varChild, __out VARIANT *pvarRole);


	IFACEMETHODIMP Reset(void);
	IFACEMETHODIMP Next(ULONG celt, __out_ecount_part(celt, *pceltFetched) VARIANT* rgvar, __out_opt ULONG * pceltFetched);

protected:
	DOUBLE GetCurrentTotalHeightBefore(std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > *pList, HSTXTREENODE hNodeBefore);
	DOUBLE GetFinalTotalHeightBefore(std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > *pList, HSTXTREENODE hNodeBefore);
	void ApplyOffsetAfter(std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > *pList, HSTXTREENODE hNodeAfter, DOUBLE fOffset, HSTXTREENODE hNodeBaseline, BOOL bParentApply);
	void UpdateAnimationManager();
	UI_ANIMATION_SECONDS GetCurrentTime();
	
	//When drawing vertical lines, We need to know the height from the top to the last sibling node.
	DOUBLE GetCurrentHeightWithoutLastChildExpand();
	
	void ResetScrollBars();
	void ResetHorizontalScrollBar();
	BOOL ModifyStyle(int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT nFlags);
	int GetScrollLimit(int nBar);
	BOOL IsNeedApplyOffset(HSTXTREENODE hNode);
	void SetAllChildrenOpacity(HSTXTREENODE hNode, DOUBLE fTargetOpacity);
	void AdjustSelection(HSTXTREENODE hNodeCollapsed);
	void SelectNode(HSTXTREENODE hNode);
	BOOL IsAscendant(HSTXTREENODE hNodeAscendant, HSTXTREENODE hNode);
	BOOL IsItemVisible(HSTXTREENODE hItem);	//The term 'Visible' here means not cover by its Ascendants. aka, it's Ascendants are expanded
	HSTXTREENODE GetNearestVisibleAscendant(HSTXTREENODE hItem);
	void SelectNextVisibleItem();
	DOUBLE GetItemTopOffsetAbsolute(HSTXTREENODE hItem);	//assume hItem is visible/expanded
	HSTXTREENODE GetFurthestVisibleItem(HSTXTREENODE hItem);
	void SelectPrevVisibleItem();
	void ApplySmoothStopTransition(IUIAnimationVariable *pVar, UI_ANIMATION_SECONDS duration, DOUBLE fTargetValue, IUIAnimationStoryboard *pStoryboard = NULL, BOOL bResetVelocity = FALSE);
	void CheckDrag(int x, int y);
	void OnBeginDrag();
	BOOL IsMouseInButtomSpace(int x, int y);
	void OnDrop(HSTXTREENODE hNodeDropTo);
	HSTXTREENODE Internal_InsertItem(std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pNewNode, HSTXTREENODE hParent = STXTVI_ROOT, HSTXTREENODE hInsertBefore = STXTVI_LAST, int iItemHeight = -1);
	int GetAllVisibleItem(HSTXTREENODE hSearchRoot = STXTVI_ROOT, std::vector<HSTXTREENODE> *pArrItems = NULL);
	int GetAllItem(HSTXTREENODE hSearchRoot = STXTVI_ROOT, std::vector<HSTXTREENODE> *pArrItems = NULL);
	int GetItemCount(HSTXTREENODE hSearchRoot = STXTVI_ROOT);
	void RecalculateItemOffset( std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > * pList );
	void OnInternalItemPreDelete( CSTXAnimatedTreeNodeNS* pItem );
	void OnInternalItemPostDelete( CSTXAnimatedTreeNodeNS* pItem );
	int GetDescendantItems(HSTXTREENODE hItem, std::vector<HSTXTREENODE> *pArrItems);
	BOOL IsValidItem(HSTXTREENODE hItem);
	BOOL IsAnimationBusy();
	LRESULT SendCommonNotifyMessage(UINT nNotifyCode, HSTXTREENODE hNode, DWORD_PTR dwNotifySpec);
	LRESULT SendSimpleNotifyMessage(UINT nNotifyCode);
	void OnInternalHoverItemChanged(HSTXTREENODE hItem);
	void OnItemFloatCheckTriggered();
	int GetItemMaxBoundsX(HSTXTREENODE hItem);
	void GetDefaultFontInfo();

protected:
	virtual void OnEndEdit();
	virtual BOOL PreSendNotifyMessage(UINT nCode, LPNMHDR pNMHDR);
	virtual std::tr1::shared_ptr<Gdiplus::Image> OnItemImageCallback(HSTXTREENODE hItem);

public:
	// "Internal" methods are thread-unsafe. they are designed to be called by Windows message dispatching
	// Todo: Add non-Internal methods by defining a WM_XXX message handler for each Internal one.

	// Register the window class for this control.
	static void RegisterAnimatedTreeCtrlClass();

	//Create window.
	//Be sure to call RegisterAnimatedTreeCtrlClass before create window
	BOOL Create(LPCTSTR lpszWindowText, DWORD dwStyle, int x, int y, int cx, int cy, HWND hWndParent, UINT nID);

	HSTXTREENODE Internal_InsertItem(LPCTSTR lpszText, HSTXTREENODE hParent = STXTVI_ROOT, HSTXTREENODE hInsertBefore = STXTVI_LAST, DWORD_PTR dwItemData = 0, int iItemHeight = -1);
	HSTXTREENODE Internal_NodeHitTest(POINT pt, UINT *pFlags);
	HWND GetSafeHwnd();

	//Expand specified node. 
	//nCode : refer to STXATVE_ macros
	BOOL Internal_Expand(HSTXTREENODE hNode, UINT nCode);

	HSTXTREENODE Internal_GetChildItem(HSTXTREENODE hItem);
	
	//nCode : refer to STXATVGN_ macros
	HSTXTREENODE Internal_GetNextItem(HSTXTREENODE hItem, UINT nCode);

	HSTXTREENODE GetSelectedItem();
	HSTXTREENODE Internal_GetNextVisibleItem(HSTXTREENODE hItem);
	HSTXTREENODE Internal_GetNextSiblingItem(HSTXTREENODE hItem);
	HSTXTREENODE Internal_GetPrevVisibleItem(HSTXTREENODE hItem);
	HSTXTREENODE Internal_GetPrevSiblingItem(HSTXTREENODE hItem);
	BOOL Internal_EnsureVisible(HSTXTREENODE hItem);
	BOOL SetItemImage(HSTXTREENODE hItem, IStream *pStream, BOOL bResizeImage = FALSE);
	void SetItemImage(HSTXTREENODE hItem, LPCTSTR lpszImageFile, BOOL bResizeImage = FALSE);
	void SetItemImageCallback(HSTXTREENODE hItem, BOOL bUseImageCallback);
	BOOL SetItemSubImage(HSTXTREENODE hItem, IStream *pStream);
	void SetItemSubImage(HSTXTREENODE hItem, LPCTSTR lpszImageFile);
	BOOL Internal_DeleteItem(HSTXTREENODE hItem);
	BOOL Internal_SetItemHeight(HSTXTREENODE hItem, int iHeight);
	BOOL Internal_SortChildren(HSTXTREENODE hItem, STXAnimatedTreeSortFuncType pfnSortFunc, LPARAM lParamSort);
	BOOL Internal_SortChildrenByItem(HSTXTREENODE hItem, STXAnimatedTreeSortItemFuncType pfnSortFunc, LPARAM lParamSort);
	int Internal_GetItemText(HSTXTREENODE hItem, LPTSTR pszTextBuffer, int cchBufferLen);
	DWORD_PTR Internal_GetItemData(HSTXTREENODE hItem);
	HSTXTREENODE Internal_FindItem(HSTXTREENODE hItem, STXAnimatedTreeSearchCompareFuncType pfnSearchFunc, LPARAM lParamSearch);
	//Search Items.
	//If pOutBuffer is NULL, this function returns the number of items that match the search condition
	//If pOutBuffer is not NULL, nBufferSize is the maximum number of items that can be filled in pOutBuffer.
	int Internal_FindItems(HSTXTREENODE hItem, STXAnimatedTreeSearchCompareFuncType pfnSearchFunc, LPARAM lParamSearch, HSTXTREENODE *pOutBuffer, int nBufferSize);
	BOOL Internal_SetItemData(HSTXTREENODE hItem, DWORD_PTR dwItemData);
	BOOL Internal_ItemHasChildren(HSTXTREENODE hItem);
	BOOL Internal_SetBackgroundImage(IStream *pImgStream);
	BOOL Internal_SetWatermarkImage(IStream *pImgStream);
	DWORD GetStyle();
	void ModifyStyle(DWORD dwRemove, DWORD dwAdd);
	BOOL Internal_SetExpanderImage(IStream *pImgStreamExpanded, IStream *pImgStreamCollapsed);
	void Internal_SetDefaultSortFunction(LPSTXTVSORTFUNC lpSortFunc);
	void Internal_ModifyItemStyle(HSTXTREENODE hItem, DWORD dwAdd, DWORD dwRemove);
	void Internal_SetItemDrawFunction(STXAnimatedTreeItemDrawFuncType lpfnItemDrawFunc);
	DWORD Internal_GetItemState(HSTXTREENODE hItem);
	BOOL Internal_GetItemRect(HSTXTREENODE hItem, LPRECT lprcItemRect);
	void Internal_SetAnimationDuration(DWORD dwMilliseconds);
	void Internal_SetBackgroundColor(COLORREF clrBackground);
	HSTXTREENODE Internal_GetParentItem(HSTXTREENODE hItem);
	int GetItemIndent();
	BOOL SetItemIndent(int nItemIndent);
	BOOL Internal_SetItemText(HSTXTREENODE hItem, LPCTSTR pszText);
	BOOL Internal_SetItemSubText(HSTXTREENODE hItem, LPCTSTR pszSubText);
	int GetCurrentContentWidth();
	Gdiplus::Font* GetDefaultFont();
	Gdiplus::Font* GetDefaultSubTextFont();
	BOOL SetWatermarkLocation(UINT nLocation);		//see STXTV_WATERMARK_ macros
	void SetWatermarkOpacity(double fOpacity);		//0.0f (transparent) to 1.0f (opacity)
};
