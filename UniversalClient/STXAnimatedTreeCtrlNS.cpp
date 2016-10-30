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

#include "StdAfx.h"
#include "STXAnimatedTreeCtrlNS.h"
#include <Shlwapi.h>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif

#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

//////////////////////////////////////////////////////////////////////////

#define STXATC_EXPANDER_WIDTH				18
#define STXATC_ITEM_DEFAULT_HEIGHT			20
#define STXATC_ITEM_IMAGE_SPACING			2
#define STXATC_ITEM_SUB_IMAGE_MAX_WIDTH		16

#define STXATC_ITEM_INDENT					16


#define STXATC_ITEM_DELETE_ANIMATE_OFFSET		24

#define STXTC_TIMER_ID_ANIMATION				1600
#define STXTC_TIMER_ID_ITEM_FLOAT_CHECK			1601

#define STXTC_TIMER_INTERVAL_ITEM_FLOAT_CHECK		1000

#define STXAT_FONT_NAME						"ËÎÌו"
#define STXAT_FONT_SIZE						13
#define STXAT_MEASURE_STRING_WIDTH_FIX		10240

//////////////////////////////////////////////////////////////////////////

typedef std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> >  STXTREENODELIST;

//////////////////////////////////////////////////////////////////////////

CSTXAnimatedTreeNodeNS::CSTXAnimatedTreeNodeNS(CSTXAnimatedTreeCtrlNS *pParentControl)
: m_pParentControl(pParentControl)
{
	static long sAccID = 1;
	m_pParentNode = NULL;
	m_bExpanded = TRUE;
	m_bSelected = FALSE;
	m_bHover = FALSE;
	m_dwNodeData = 0;
	m_nLeftOffsetFix = 0;
	m_iDropHighlight = 0;
	m_iIndexBeforeSort = -1;
	m_bPreDeleteNotified = FALSE;
	m_nVOffsetDeleting = 0;
	m_bInitialized = FALSE;
	m_dwItemStyle = 0;
	m_nAccID = sAccID++;
	m_nBoundsMaxX = -1;
}

CSTXAnimatedTreeNodeNS::~CSTXAnimatedTreeNodeNS()
{
	if(m_bInitialized)
		TriggerItemPostDeleteEvent();
}

DOUBLE CSTXAnimatedTreeNodeNS::GetCurrentHeight()
{
	DOUBLE fHeight = 0;
	m_pAVItemHeight->GetValue(&fHeight);

	if(!m_bExpanded)
		return fHeight;

	STXTREENODELIST::iterator it = m_arrChildNodes.begin();
	for(;it!=m_arrChildNodes.end();it++)
	{
		fHeight += (*it)->GetCurrentHeight();
	}

	return fHeight;
}

DOUBLE CSTXAnimatedTreeNodeNS::GetFinalHeight()
{
	DOUBLE fHeight = 0.0;
	m_pAVItemHeight->GetFinalValue(&fHeight);

	if(!m_bExpanded)
		return fHeight;

	STXTREENODELIST::iterator it = m_arrChildNodes.begin();
	for(;it!=m_arrChildNodes.end();it++)
	{
		fHeight += (*it)->GetFinalHeight();
	}

	return fHeight;
}

DOUBLE CSTXAnimatedTreeNodeNS::GetFinalHeightExpanded()
{
	DOUBLE fHeight = 0;
	m_pAVItemHeight->GetValue(&fHeight);
	STXTREENODELIST::iterator it = m_arrChildNodes.begin();
	for(;it!=m_arrChildNodes.end();it++)
	{
		fHeight += (*it)->GetFinalHeight();
	}

	return fHeight;
}

DOUBLE CSTXAnimatedTreeNodeNS::GetCurrentHeightWithoutLastChildExpand()
{
	DOUBLE fHeight = 0.0;
	if(m_arrChildNodes.size() > 0)
	{
		//Get last sibling item's height first
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pLastItem = *m_arrChildNodes.rbegin();
		pLastItem->m_pAVItemHeight->GetValue(&fHeight);
	}

	STXTREENODELIST::iterator it = m_arrChildNodes.begin();
	STXTREENODELIST::iterator itMore;
	for(;it!=m_arrChildNodes.end();it++)
	{
		if(it != m_arrChildNodes.end())
		{
			itMore = it;
			itMore++;
			if(itMore == m_arrChildNodes.end())
			{
				//fHeight;
				break;
			}
		}

		fHeight += (*it)->GetCurrentHeight();
	}

	return fHeight;
}

void CSTXAnimatedTreeNodeNS::DrawItem(Gdiplus::Graphics *pGraphics, Gdiplus::RectF* rectItem, Gdiplus::RectF* rectItemFinal, DOUBLE fBaseOffset, Gdiplus::REAL fHorzOffset, int nHeightLimit, DOUBLE fBaseOpacity, DOUBLE fGlobalExpanderOpacity)
{
	//Gdiplus::FontFamily  fontFamily(m_pParentControl->GetFontName());
	//Gdiplus::Font drawFont(&fontFamily, STXAT_FONT_SIZE, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	DOUBLE fNodeOpacity = 0;
	m_pAVOpacity->GetValue(&fNodeOpacity);
	fNodeOpacity = fNodeOpacity * fBaseOpacity;

	if(fNodeOpacity < 0.05f)
		return;

	STXTVITEMDRAW itemDraw;
	itemDraw.pGraphics = pGraphics;
	itemDraw.dwItemStyle = m_dwItemStyle;
	itemDraw.dwWindowStyle = m_pParentControl->GetStyle();
	itemDraw.fOpacity = fNodeOpacity;
	itemDraw.hItem = this;
	itemDraw.lParamItem = m_dwNodeData;
	itemDraw.rectItem = rectItem;
	itemDraw.dwItemState = m_pParentControl->Internal_GetItemState(this);

	itemDraw.rectPart = rectItem;
	itemDraw.dwStage = STXTV_STAGE_PREPAINT;

	if(m_pParentControl->m_pfnItemDrawFunc)
	{
		if(m_pParentControl->m_pfnItemDrawFunc(&itemDraw) == STXTV_CDRF_SKIPDEFAULT)
			return;
	}

	COLORREF clrTextColor = GetSysColor(COLOR_WINDOWTEXT);
	if(m_bSelected)
		clrTextColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
	//else if(m_bHover)
	//	clrTextColor = GetSysColor(COLOR_HOTLIGHT);


	Gdiplus::SolidBrush textBrush(Gdiplus::Color(static_cast<BYTE>(255 * fNodeOpacity), GetRValue(clrTextColor), GetGValue(clrTextColor), GetBValue(clrTextColor)));

	// Draw the Main Caption
	DOUBLE fItemHeight = 0;
	m_pAVItemHeight->GetValue(&fItemHeight);
	if (fBaseOffset + fItemHeight > 0)
	{
		DrawDropHighlight(pGraphics, rectItem, rectItemFinal, fNodeOpacity);

		int iImageOccupie = DrawItemImage(NULL, rectItem, rectItemFinal, fHorzOffset, fNodeOpacity, &itemDraw);

		itemDraw.rectPart = rectItem;
		itemDraw.dwStage = STXTV_STAGE_BACKGROUND;
		if(m_pParentControl->m_pfnItemDrawFunc == NULL || m_pParentControl->m_pfnItemDrawFunc(&itemDraw) == STXTV_CDRF_DODEFAULT)
		{
			DrawHover(pGraphics, rectItem, rectItemFinal, fHorzOffset, fNodeOpacity, iImageOccupie);
			DrawSelection(pGraphics, rectItem, rectItemFinal, fHorzOffset, fNodeOpacity, iImageOccupie);
		}

		DrawItemImage(pGraphics, rectItem, rectItemFinal, fHorzOffset, fNodeOpacity, &itemDraw);

		Gdiplus::RectF rectTextMain(rectItem->X + iImageOccupie - fHorzOffset, rectItem->Y, rectItem->Width - iImageOccupie + fHorzOffset + STXAT_MEASURE_STRING_WIDTH_FIX, rectItem->Height);
		itemDraw.rectPart = &rectTextMain;
		itemDraw.dwStage = STXTV_STAGE_TEXT;
		if(m_pParentControl->m_pfnItemDrawFunc == NULL || m_pParentControl->m_pfnItemDrawFunc(&itemDraw) == STXTV_CDRF_DODEFAULT)
		{
			Gdiplus::PointF ptTextMain(rectItem->X + iImageOccupie, rectItem->Y);
			Gdiplus::StringFormat strFormat;
			
			strFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
			if ((m_pParentControl->GetStyle() & STXTVS_HORIZONTAL_SCROLL) == 0)
			{
				strFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisWord);
			}

			Gdiplus::RectF rectTextMeasured;
			pGraphics->MeasureString(m_strText.c_str(), -1, m_pParentControl->GetDefaultFont(), rectTextMain, &strFormat, &rectTextMeasured);
			int iTextTopOffsetPatch = static_cast<int>((rectTextMain.Height - rectTextMeasured.Height) / 2);
			if(iTextTopOffsetPatch > 0)
				rectTextMain.Y += iTextTopOffsetPatch;

			pGraphics->DrawString(m_strText.c_str(), -1, m_pParentControl->GetDefaultFont(), rectTextMain, &strFormat, &textBrush);
		}
	}

	if(fBaseOffset + GetCurrentHeight() > 0)
	{
		DrawLines(pGraphics, rectItem, rectItemFinal, fHorzOffset, fNodeOpacity, fGlobalExpanderOpacity);
	}
	
	if (fBaseOffset + fItemHeight> 0)
	{
		DrawExpandMark(pGraphics, rectItem, rectItemFinal, fHorzOffset, fNodeOpacity, fGlobalExpanderOpacity);
	}

	if(m_pParentControl->m_pfnItemDrawFunc)
	{
		itemDraw.rectPart = rectItem;
		itemDraw.dwStage = STXTV_STAGE_POSTPAINT;
		m_pParentControl->m_pfnItemDrawFunc(&itemDraw);
	}

	//if(!m_bExpanded)
	//	return;

	std::queue<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > m_queueAllToDraw;

	//Draw Deleted items
	size_t nDrawDelete = m_queDeletedNodes.size();
	for(size_t i=0;i<nDrawDelete;i++)
	{
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pItem = m_queDeletedNodes.front();
		m_queDeletedNodes.pop();

		DOUBLE fItemOpacity = 0;
		pItem->m_pAVOpacity->GetValue(&fItemOpacity);
		if (fItemOpacity > 0.02f)
 		{
 			m_queDeletedNodes.push(pItem);
 			m_queueAllToDraw.push(pItem);
 		}
	}

	STXTREENODELIST::iterator it = m_arrChildNodes.begin();
	for(;it!=m_arrChildNodes.end();it++)
	{
		m_queueAllToDraw.push(*it);
	}

	DOUBLE fItemHeightFinal = 0.0;
	m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);

	size_t nItemToDraw = m_queueAllToDraw.size();
	for(size_t i=0;i<nItemToDraw;i++)
	{
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pItem = m_queueAllToDraw.front();
		m_queueAllToDraw.pop();

		DOUBLE fTopOffset = 0;
		pItem->m_pAVTopOffset->GetValue(&fTopOffset);
		DOUBLE fLeftOffset = 0;
		pItem->m_pAVLeftOffset->GetValue(&fLeftOffset);
		DOUBLE fTopOffsetFinal;
		pItem->m_pAVTopOffset->GetFinalValue(&fTopOffsetFinal);
		DOUBLE fLeftOffsetFinal;
		pItem->m_pAVLeftOffset->GetFinalValue(&fLeftOffsetFinal);

		DOUBLE fAbsoluteTopOffset = fBaseOffset + fTopOffset;
		if(fAbsoluteTopOffset > nHeightLimit)
			break;

		DOUBLE fItemHeight = 0;
		m_pAVItemHeight->GetValue(&fItemHeight);
		Gdiplus::PointF ptText(0, 0);
		ptText.X = static_cast<Gdiplus::REAL>(fLeftOffset);;
		ptText.Y = static_cast<Gdiplus::REAL>(fTopOffset + rectItem->Y + fItemHeight);

		Gdiplus::PointF ptTextFinal(0, 0);
		DOUBLE fChildItemHeightFinal = 0.0;
		pItem->m_pAVItemHeight->GetFinalValue(&fChildItemHeightFinal);
		DOUBLE fChildItemHeight = 0.0;
		pItem->m_pAVItemHeight->GetValue(&fChildItemHeight);

		ptTextFinal.X = static_cast<Gdiplus::REAL>(fLeftOffsetFinal);;
		ptTextFinal.Y = static_cast<Gdiplus::REAL>(fTopOffsetFinal + rectItemFinal->Y + fItemHeightFinal);

		Gdiplus::RectF rectItem(ptText.X, ptText.Y, static_cast<Gdiplus::REAL>(rectItem->Width - m_pParentControl->GetItemIndent()), static_cast<Gdiplus::REAL>(fChildItemHeight));
		Gdiplus::RectF rectItemFinal(ptTextFinal.X, ptTextFinal.Y, static_cast<Gdiplus::REAL>(rectItemFinal->Width - m_pParentControl->GetItemIndent()), static_cast<Gdiplus::REAL>(fChildItemHeightFinal));

		pItem->DrawItem(pGraphics, &rectItem, &rectItemFinal, fTopOffset + fBaseOffset + fItemHeight, fHorzOffset, nHeightLimit, fNodeOpacity, fGlobalExpanderOpacity);
	}
}

void CSTXAnimatedTreeNodeNS::DrawExpandMark(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, DOUBLE fGlobalExpanderOpacity)
{
	if(m_arrChildNodes.size() == 0 && (m_dwItemStyle & STXTVIS_FORCE_SHOW_EXPANDER) == 0)
		return;

	Gdiplus::RectF rectItemExpander = *rectItem;
	rectItemExpander.X -= STXATC_EXPANDER_WIDTH;
	rectItemExpander.Width = STXATC_EXPANDER_WIDTH;
	rectItemExpander.Offset(-fHorzOffset, 0);

	DOUBLE fExpanderOpacity = fNodeOpacity * fGlobalExpanderOpacity;

	std::tr1::shared_ptr<Gdiplus::Image> pImgUsing = m_pParentControl->m_pImgExpanderExpanded;
	if(!m_bExpanded)
		pImgUsing = m_pParentControl->m_pImgExpanderCollapsed;
	if(pImgUsing)
	{
		if(rectItemExpander.Height > rectItemExpander.Width)
		{
			rectItemExpander.Y += ((rectItemExpander.Height - rectItemExpander.Width) / 2);
			rectItemExpander.Height = rectItemExpander.Width;
		}

		DrawImage(pGraphics, pImgUsing.get(), fExpanderOpacity, &rectItemExpander);
	}
	else
	{
		rectItemExpander.Inflate(-4, 0);
		rectItemExpander.Inflate(0, -((rectItemExpander.Height - rectItemExpander.Width) / 2));

		Gdiplus::SolidBrush bkBrush(Gdiplus::Color(static_cast<BYTE>(124 * fExpanderOpacity), 224, 224, 224));

		Gdiplus::Pen pen(Gdiplus::Color(static_cast<BYTE>(255 * fExpanderOpacity), 128, 128, 128), 1);
		pGraphics->DrawRectangle(&pen, rectItemExpander);
		pGraphics->FillRectangle(&bkBrush, rectItemExpander);

		Gdiplus::PointF ptLeft(rectItemExpander.X + 2, rectItemExpander.Y + rectItemExpander.Height / 2);

		pGraphics->DrawLine(&pen, ptLeft.X, ptLeft.Y, ptLeft.X + rectItemExpander.Width - 4, ptLeft.Y);

		if (!m_bExpanded)
		{
			Gdiplus::PointF ptTop(rectItemExpander.X + rectItemExpander.Width / 2, rectItemExpander.Y + 2);
			pGraphics->DrawLine(&pen, ptTop.X, ptTop.Y, ptTop.X, ptTop.Y + rectItemExpander.Height - 4);
		}
	}


}

void CSTXAnimatedTreeNodeNS::DrawLines(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, DOUBLE fGlobalLineOpacity)
{
	if(m_pParentControl->GetStyle() & STXTVS_NO_LINES)
		return;

	DOUBLE fLineOpacity = fNodeOpacity * fGlobalLineOpacity;
	Gdiplus::Pen pen(Gdiplus::Color(static_cast<BYTE>(255 * fLineOpacity), 192, 192, 192), 1);
	pen.SetDashStyle(Gdiplus::DashStyleDot);

	pGraphics->DrawLine(&pen, rectItemFinal->X - STXATC_EXPANDER_WIDTH / 2 - fHorzOffset, rectItemFinal->Y + rectItemFinal->Height / 2, rectItemFinal->X - fHorzOffset, rectItemFinal->Y + rectItemFinal->Height / 2);

	if(m_bExpanded && m_arrChildNodes.size() > 0)
	{
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pFirstItem = *m_arrChildNodes.begin();
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pLastItem = *m_arrChildNodes.rbegin();

		DOUBLE fItemHeightFinal = 0.0;
		m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
		DOUBLE fItemHeightLast = 0.0;
		pLastItem->m_pAVItemHeight->GetValue(&fItemHeightLast);

		Gdiplus::PointF ptTop(static_cast<Gdiplus::REAL>(rectItemFinal->X + m_pParentControl->GetItemIndent() - STXATC_EXPANDER_WIDTH / 2), rectItemFinal->Y + static_cast<Gdiplus::REAL>(fItemHeightFinal));
		pGraphics->DrawLine(&pen, ptTop.X - fHorzOffset, ptTop.Y, ptTop.X - fHorzOffset, ptTop.Y + static_cast<Gdiplus::REAL>(GetCurrentHeightWithoutLastChildExpand() - fItemHeightLast * 0.5));
	}
}

void CSTXAnimatedTreeNodeNS::DrawHover(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, int iImageOccupie)
{
// 	if(!m_bHover)
// 		return;

	COLORREF clrHighlight = RGB(232, 240, 255);
	DOUBLE fHoverOpacity = 0;
	m_pAVHoverOpacity->GetValue(&fHoverOpacity);

	if (fHoverOpacity < 0.02)
		return;

	Gdiplus::SolidBrush bkBrush(Gdiplus::Color(static_cast<BYTE>(192 * fNodeOpacity * fHoverOpacity), GetRValue(clrHighlight), GetGValue(clrHighlight), GetBValue(clrHighlight)));
	Gdiplus::RectF rectSelection(*rectItem);
	rectSelection.Offset(-fHorzOffset, 0);
	rectSelection.Width += fHorzOffset;
	//rectSelection.X += iImageOccupie;
	//rectSelection.Width -= iImageOccupie;

	pGraphics->FillRectangle(&bkBrush, rectSelection);
}

void CSTXAnimatedTreeNodeNS::DrawSelection(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, int iImageOccupie)
{
	if(!m_bSelected)
		return;

	COLORREF clrHighlight = GetSysColor(COLOR_HIGHLIGHT);

	Gdiplus::SolidBrush bkBrush(Gdiplus::Color(static_cast<BYTE>(192 * fNodeOpacity), GetRValue(clrHighlight), GetGValue(clrHighlight), GetBValue(clrHighlight)));
	Gdiplus::RectF rectSelection(*rectItem);
	rectSelection.Offset(-fHorzOffset, 0);
	rectSelection.Width += fHorzOffset;
	//rectSelection.X += iImageOccupie;
	//rectSelection.Width -= iImageOccupie;

	pGraphics->FillRectangle(&bkBrush, rectSelection);
}

void CSTXAnimatedTreeNodeNS::DrawDropHighlight( Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, DOUBLE fNodeOpacity)
{
	if(m_iDropHighlight == 0)
		return;

	Gdiplus::Pen pen(Gdiplus::Color(static_cast<BYTE>(255 * fNodeOpacity), 192, 192, 192), 1);
	if(m_iDropHighlight == 1)
		pGraphics->DrawLine(&pen, rectItem->X, rectItem->Y, rectItem->X + rectItem->Width, rectItem->Y);
	else
		pGraphics->DrawLine(&pen, rectItem->X, rectItem->Y + STXATC_ITEM_DEFAULT_HEIGHT, rectItem->X + rectItem->Width, rectItem->Y + STXATC_ITEM_DEFAULT_HEIGHT);
}

int CSTXAnimatedTreeNodeNS::DrawItemImage(Gdiplus::Graphics * pGraphics, Gdiplus::RectF *rectItem, Gdiplus::RectF* rectItemFinal, Gdiplus::REAL fHorzOffset, DOUBLE fNodeOpacity, LPSTXTVITEMDRAW lpItemDraw)
{
	DOUBLE fImageOccupy = 0;
	m_pAVImageOccupy->GetValue(&fImageOccupy);

	int iResult = static_cast<int>(fImageOccupy);

	if(pGraphics == NULL)
		return iResult;
	
	if(m_pImgImage == NULL && (m_dwItemStyle & STXTVIS_IMAGE_CALLBACK) == 0)
		return iResult;

	std::tr1::shared_ptr<Gdiplus::Image> imgUse = m_pImgImage;
	if(m_dwItemStyle & STXTVIS_IMAGE_CALLBACK)
		imgUse = m_pParentControl->OnItemImageCallback(this);

	if(imgUse == NULL)
		return iResult;

	DOUBLE fItemHeight = 0;
	m_pAVItemHeight->GetValue(&fItemHeight);
	Gdiplus::REAL rItemHeight = static_cast<Gdiplus::REAL>(fItemHeight) - STXATC_ITEM_IMAGE_SPACING;

	DOUBLE fImageScale = 0;
	m_pAVImageScale->GetValue(&fImageScale);

	Gdiplus::REAL rScale = static_cast<Gdiplus::REAL>(fImageScale);
	Gdiplus::REAL rPartIncrease = (rScale - 1.0f) * rItemHeight;
	Gdiplus::REAL rPartOffset = rPartIncrease / 2;

	DOUBLE fItemImageOpacity = 0.0;
	m_pAVImageOpacity->GetValue(&fItemImageOpacity);
	Gdiplus::REAL rImageExtraOpacity = static_cast<Gdiplus::REAL>(fItemImageOpacity);


	Gdiplus::RectF rectImgDest(rectItem->X - rPartOffset - fHorzOffset, rectItem->Y - rPartOffset, static_cast<Gdiplus::REAL>(rItemHeight)+rPartIncrease, static_cast<Gdiplus::REAL>(rItemHeight)+rPartIncrease);
	Gdiplus::REAL rOpacity = static_cast<Gdiplus::REAL>(fNodeOpacity) * rImageExtraOpacity;
	Gdiplus::ColorMatrix *pCMUse = NULL;
	Gdiplus::ColorMatrix cm = { 
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, rOpacity, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	pCMUse = &cm;

	Gdiplus::ColorMatrix cmGray = { 
		0.299f, 0.299f, 0.299f, 0.0f, 0.0f,
		0.587f, 0.587f, 0.587f, 0.0f, 0.0f,
		0.114f, 0.114f, 0.114f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, rOpacity * 0.3f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};


	lpItemDraw->rectPart = &rectImgDest;
	lpItemDraw->dwStage = STXTV_STAGE_IMAGE;
	if(m_pParentControl->m_pfnItemDrawFunc == NULL || m_pParentControl->m_pfnItemDrawFunc(lpItemDraw) == STXTV_CDRF_DODEFAULT)
	{
		if((m_pParentControl->GetStyle() & STXTVS_DRAW_GRAY_IMAGE)
			|| (lpItemDraw->dwItemState & STXTVIS_GRAY_IMAGE))
		{
			pCMUse = &cmGray;
		}

		Gdiplus::ImageAttributes ImgAttr;

		ImgAttr.SetColorMatrix(pCMUse, Gdiplus::ColorMatrixFlagsDefault, 
			Gdiplus::ColorAdjustTypeBitmap);

		UINT nImgWidth = imgUse->GetWidth();
		UINT nImgHeight = imgUse->GetHeight();
		Gdiplus::RectF rectImg(0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight));

		rectImgDest.Offset(STXATC_ITEM_IMAGE_SPACING / 2, STXATC_ITEM_IMAGE_SPACING / 2);

		pGraphics->DrawImage(imgUse.get(), rectImgDest, 0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight)
			, Gdiplus::UnitPixel, &ImgAttr);
	}

	//Draw SubImage / SubText
	//SubImage height is half of Item Image but not larger than STXATC_ITEM_SUB_IMAGE_MAX_WIDTH
	Gdiplus::REAL fSubImageWidthHeight = rectImgDest.Width > rectImgDest.Height ? rectImgDest.Height : rectImgDest.Width;
	fSubImageWidthHeight /= 2;
	if (fSubImageWidthHeight > STXATC_ITEM_SUB_IMAGE_MAX_WIDTH)
		fSubImageWidthHeight = STXATC_ITEM_SUB_IMAGE_MAX_WIDTH;

	if ((m_dwItemStyle & STXTVIS_SUB_TEXT) == 0)
	{	
		if(m_pImgSubImage)
		{
			Gdiplus::RectF rectSubImgDest(rectImgDest.GetLeft() + rectImgDest.Width - fSubImageWidthHeight, rectImgDest.GetTop() + rectImgDest.Height - fSubImageWidthHeight, fSubImageWidthHeight, fSubImageWidthHeight);

			lpItemDraw->rectPart = &rectSubImgDest;
			lpItemDraw->dwStage = STXTV_STAGE_SUBIMAGE;
			if(m_pParentControl->m_pfnItemDrawFunc == NULL || m_pParentControl->m_pfnItemDrawFunc(lpItemDraw) == STXTV_CDRF_DODEFAULT)
			{
				Gdiplus::ImageAttributes ImgAttr;

				ImgAttr.SetColorMatrix(pCMUse, Gdiplus::ColorMatrixFlagsDefault, 
					Gdiplus::ColorAdjustTypeBitmap);

				UINT nSubImgWidth = m_pImgSubImage->GetWidth();
				UINT nSubImgHeight = m_pImgSubImage->GetHeight();
				pGraphics->DrawImage(m_pImgSubImage.get(), rectSubImgDest, 0, 0, static_cast<Gdiplus::REAL>(nSubImgWidth), static_cast<Gdiplus::REAL>(nSubImgHeight)
					, Gdiplus::UnitPixel, &ImgAttr);
			}
		}
	}
	else
	{
		if (m_strSubText.size() > 0)
		{
			//Gdiplus::RectF rectSubImgDest(rectImgDest.GetLeft() + rectImgDest.Width - fSubImageWidthHeight, rectImgDest.GetTop() + rectImgDest.Height - fSubImageWidthHeight, fSubImageWidthHeight, fSubImageWidthHeight);

			//Gdiplus::PointF ptTextSub(rectImgDest.GetLeft() + rectImgDest.Width - fSubImageWidthHeight, rectImgDest.GetTop() + rectImgDest.Height - fSubImageWidthHeight);
			Gdiplus::StringFormat strFormat;
			Gdiplus::RectF rectTextSub(rectImgDest.GetLeft() + rectImgDest.Width - fSubImageWidthHeight, rectImgDest.GetTop() + rectImgDest.Height - fSubImageWidthHeight, STXAT_MEASURE_STRING_WIDTH_FIX, fSubImageWidthHeight);

			
			strFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

			Gdiplus::RectF rectTextMeasured;
			pGraphics->MeasureString(m_strSubText.c_str(), -1, m_pParentControl->GetDefaultSubTextFont(), rectTextSub, &strFormat, &rectTextMeasured);


			Gdiplus::REAL rScale = static_cast<Gdiplus::REAL>(fImageScale);
			Gdiplus::REAL rImageExtraOpacity = 1.0f - (rScale - 1.0f);
			Gdiplus::REAL rOpacity = static_cast<Gdiplus::REAL>(fNodeOpacity)* rImageExtraOpacity;


			//Gdiplus::SolidBrush textBrush(Gdiplus::Color(static_cast<BYTE>(255 * fNodeOpacity), GetRValue(clrTextColor), GetGValue(clrTextColor), GetBValue(clrTextColor)));
			Gdiplus::SolidBrush textBrush(Gdiplus::Color(static_cast<BYTE>(255 * fNodeOpacity * rOpacity), 0, 0, 0));
			rectTextMeasured.Offset(fSubImageWidthHeight - rectTextMeasured.Width, fSubImageWidthHeight - rectTextMeasured.Height);

			Gdiplus::SolidBrush brushBk(Gdiplus::Color(static_cast<BYTE>(192 * fNodeOpacity * rOpacity), 255, 255, 255));
			pGraphics->FillRectangle(&brushBk, rectTextMeasured.X, rectTextMeasured.Y, rectTextMeasured.Width, rectTextMeasured.Height);


			pGraphics->DrawString(m_strSubText.c_str(), -1, m_pParentControl->GetDefaultSubTextFont(), rectTextMeasured, &strFormat, &textBrush);

		}
	}

	return iResult;
}

HSTXTREENODE CSTXAnimatedTreeNodeNS::HitTest(POINT pt, int iControlWidth,  int iOffsetPatch, UINT *pFlags)
{
	DOUBLE fYOffset = 0;
	m_pAVTopOffset->GetValue(&fYOffset);

	int iYOffset = static_cast<int>(fYOffset);
	if(pt.y < iYOffset)
		return NULL;

	//Convert to local offset
	pt.y -= iOffsetPatch;

	DOUBLE fLeftOffset = 0;
	m_pAVLeftOffset->GetValue(&fLeftOffset);
	DOUBLE fItemHeight = 0;
	m_pAVItemHeight->GetValue(&fItemHeight);

	RECT rcNode;
	rcNode.left = static_cast<int>(fLeftOffset);
	rcNode.top = 0;
	rcNode.right = iControlWidth;
	rcNode.bottom = static_cast<LONG>(fItemHeight);

	if(PtInRect(&rcNode, pt))		//In the item area?
	{
		if(pFlags && m_pImgImage)
		{
			RECT rcImage = rcNode;
			rcImage.right = rcImage.left + (rcImage.bottom - rcImage.top);
			if(PtInRect(&rcImage, pt))
				*pFlags |= STXATHT_ONITEMIMAGE;
		}
		rcNode.top += static_cast<LONG>((fItemHeight / 2));
		if(pFlags && PtInRect(&rcNode, pt))
			*pFlags |= STXATHT_ONITEMBOTTOM;

		return (HSTXTREENODE)this;
	}

	OffsetRect(&rcNode, -STXATC_EXPANDER_WIDTH, 0);

	if(PtInRect(&rcNode, pt))		//In the expander area
	{
		if(pFlags)
			*pFlags |= STXATHT_ONITEMEXPANDER;

		return (HSTXTREENODE)this;
	}

	if(m_bExpanded)
	{
		HSTXTREENODE hNodeHit = NULL;
		STXTREENODELIST::iterator it = m_arrChildNodes.begin();
		for(;it!=m_arrChildNodes.end();it++)
		{
			DOUBLE fChildTopOffset = 0;
			(*it)->m_pAVTopOffset->GetValue(&fChildTopOffset);
			hNodeHit = (*it)->HitTest(pt, iControlWidth, static_cast<int>(fItemHeight)+static_cast<int>(fChildTopOffset), pFlags);
			if(hNodeHit)
			{
				return hNodeHit;
			}
		}
	}

	return NULL;
}

void CSTXAnimatedTreeNodeNS::TriggerItemPreDeleteEvent()
{
	if(!m_bPreDeleteNotified)
	{
		OutputDebugString(_T("Item PreDelete : "));
		OutputDebugString(m_strText.c_str());
		OutputDebugString(_T("\n"));
		m_nVOffsetDeleting = GetScrollPos(m_pParentControl->GetSafeHwnd(), SB_VERT);
		m_bPreDeleteNotified = TRUE;
		m_pParentControl->OnInternalItemPreDelete(this);
	}
}

void CSTXAnimatedTreeNodeNS::TriggerItemPostDeleteEvent()
{
	m_pParentControl->OnInternalItemPostDelete(this);
}

void CSTXAnimatedTreeNodeNS::OnMouseEnter()
{
	m_bHover = TRUE;

	CComPtr<IUIAnimationStoryboard> pStory;
	m_pParentControl->m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_pParentControl->m_AnimationTransitionLibrary->CreateInstantaneousTransition(1.0, &pTrans);
	pStory->AddTransition(m_pAVHoverOpacity, pTrans);

	pStory->Schedule(m_pParentControl->GetCurrentTime(), NULL);
}

void CSTXAnimatedTreeNodeNS::OnMouseLeave()
{
	m_bHover = FALSE;

	m_pParentControl->ApplySmoothStopTransition(m_pAVHoverOpacity, m_pParentControl->m_nDefaultAnimationDuration / 8.0
		, 0.0);
}

void CSTXAnimatedTreeNodeNS::DrawImage( Gdiplus::Graphics *pGraphics, Gdiplus::Image *pImage, DOUBLE fOpacity, Gdiplus::RectF *rectImage)
{
	Gdiplus::REAL rOpacity = static_cast<Gdiplus::REAL>(fOpacity);
	Gdiplus::ColorMatrix *pCMUse = NULL;
	Gdiplus::ColorMatrix cm = { 
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, rOpacity, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	pCMUse = &cm;

	// 	Gdiplus::ColorMatrix cmGray = { 
	// 		0.299, 0.299, 0.299, 0.0f, 0.0f,
	// 		0.587, 0.587, 0.587, 0.0f, 0.0f,
	// 		0.114, 0.114, 0.114, 0.0f, 0.0f,
	// 		0.0f, 0.0f, 0.0f, rOpacity * 0.3, 0.0f,
	// 		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	// 	};
	// 
	// 	if(pItem->m_bUseGrayImage || m_pParentAnimationList->m_bGlobalUseGrayImage)
	// 	{
	// 		pCMUse = &cmGray;
	// 	}

	Gdiplus::ImageAttributes ImgAttr;

	ImgAttr.SetColorMatrix(pCMUse, Gdiplus::ColorMatrixFlagsDefault, 
		Gdiplus::ColorAdjustTypeBitmap);

	UINT nImgWidth = pImage->GetWidth();
	UINT nImgHeight = pImage->GetHeight();

	pGraphics->DrawImage(pImage, *rectImage, 0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight)
		, Gdiplus::UnitPixel, &ImgAttr);
}

void CSTXAnimatedTreeNodeNS::GetItemScreenRect( __out LPRECT lprcRect )
{
	RECT rcControlClient;
	GetClientRect(m_pParentControl->GetSafeHwnd(), &rcControlClient);
	int nControlWidth = rcControlClient.right - rcControlClient.left;

	INT32 fValue = 0;
	m_pAVTopOffset->GetFinalIntegerValue(&fValue);
	long nTop = fValue;
	m_pAVLeftOffset->GetFinalIntegerValue(&fValue);
	long nLeft = fValue;
	INT32 fHeight = 0;
	m_pAVItemHeight->GetFinalIntegerValue(&fHeight);
	CSTXAnimatedTreeNodeNS *pNode = m_pParentNode;
	while(pNode)
	{
		pNode->m_pAVTopOffset->GetFinalIntegerValue(&fValue);
		nTop += fValue;
		pNode->m_pAVItemHeight->GetFinalIntegerValue(&fValue);
		nTop += fValue;
		pNode = pNode->m_pParentNode;
	}

	POINT ptLocal = {nLeft, nTop};
	ClientToScreen(m_pParentControl->GetSafeHwnd(), &ptLocal);
	int nItemWidth = nControlWidth - nLeft;

	RECT rcItem = {ptLocal.x, ptLocal.y, 0, 0};
	rcItem.right = rcItem.left + nItemWidth;
	rcItem.bottom = rcItem.top + fHeight;
	
	*lprcRect = rcItem;
}

//////////////////////////////////////////////////////////////////////////

LPCTSTR CSTXAnimatedTreeCtrlNS::s_lpszAnimatedTreeCtrlClassName = _T("STXAnimatedTreeCtrlNS");
CSTXAnimatedTreeCtrlNS::SORT_STRUCTURE CSTXAnimatedTreeCtrlNS::s_sortStructure;

CSTXAnimatedTreeCtrlNS::CSTXAnimatedTreeCtrlNS(void)
{
	m_hwndControl = NULL;
	m_nDefaultAnimationDuration = 0.8;
	m_hSelectedNode = NULL;
	m_bMouseInControl = FALSE;
	COLORREF clrWindow = GetSysColor(COLOR_WINDOW);
	m_clrBackground = Gdiplus::Color(255, GetRValue(clrWindow), GetGValue(clrWindow), GetBValue(clrWindow));
	m_bLButtonDown = FALSE;
	m_bAllowDragDrop = FALSE;
	m_bDragging = FALSE;
	m_hDragOverNode = NULL;
	m_hLastHoverNode = NULL;
	m_hLastFloatTriggeredNode = NULL;
	m_sortFuncDefault.pfnSortFunc = NULL;
	m_sortFuncDefault.lParamSort = 0;
	m_pfnItemDrawFunc = NULL;
	m_nLButtonDownHitFlags = 0;
	m_nItemIndent = STXATC_ITEM_INDENT;
	m_pDefaultFont = NULL;
	m_pDefaultSubTextFont = NULL;
	m_nWatermarkLocation = STXTV_WATERMARK_RIGHT_BOTTOM;
	m_fWatermarkOpacity = 1.0f;
}

CSTXAnimatedTreeCtrlNS::~CSTXAnimatedTreeCtrlNS(void)
{
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_InsertItem( LPCTSTR lpszText, HSTXTREENODE hParent /*= STXTVI_ROOT*/, HSTXTREENODE hInsertBefore /*= STXTVI_LAST*/, DWORD_PTR dwItemData /*= 0*/, int iItemHeight /*= -1*/ )
{
	std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pNewNode(new CSTXAnimatedTreeNodeNS(this));
	m_setValidItemPtr.insert(pNewNode.get());
	pNewNode->m_strText = lpszText;
	pNewNode->m_dwNodeData = dwItemData;
	return Internal_InsertItem(pNewNode, hParent, hInsertBefore, iItemHeight);
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_InsertItem( std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pNewNode, HSTXTREENODE hParent /*= STXTVI_ROOT*/, HSTXTREENODE hInsertBefore /*= STXTVI_LAST*/, int iItemHeight /*= -1*/ )
{
 	DOUBLE fLeftOffset = STXATC_EXPANDER_WIDTH;

	STXTREENODELIST *pListToAddIn = &m_arrRootNodes;

	HSTXTREENODE hNodeParentToSet = NULL;

	if(hParent == STXTVI_ROOT)
 	{

 	}
	else
	{
		if(hParent == NULL)
			return NULL;

		hNodeParentToSet = hParent;

		pListToAddIn = &hParent->m_arrChildNodes;
		DOUBLE fParentOffset = 0;
		hParent->m_pAVLeftOffset->GetFinalValue(&fParentOffset);
		fLeftOffset = fParentOffset + m_nItemIndent;
	}

	m_AnimationManager->CreateAnimationVariable(fLeftOffset + STXATC_EXPANDER_WIDTH, &pNewNode->m_pAVLeftOffset);
	m_AnimationManager->CreateAnimationVariable(0, &pNewNode->m_pAVOpacity);
	m_AnimationManager->CreateAnimationVariable(0.0, &pNewNode->m_pAVImageOpacity);
	m_AnimationManager->CreateAnimationVariable(0.0, &pNewNode->m_pAVImageScale);
	m_AnimationManager->CreateAnimationVariable(0, &pNewNode->m_pAVImageOccupy);
	int iItemInitialHeight = (iItemHeight > 0 ? iItemHeight : STXATC_ITEM_DEFAULT_HEIGHT);
	m_AnimationManager->CreateAnimationVariable(iItemInitialHeight, &pNewNode->m_pAVItemHeight);
	m_AnimationManager->CreateAnimationVariable(0, &pNewNode->m_pAVHoverOpacity);

	CComPtr<IUIAnimationStoryboard> pStory;
	m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, fLeftOffset, &pTrans);
	pStory->AddTransition(pNewNode->m_pAVLeftOffset, pTrans);
	pNewNode->m_nLeftOffsetFix = static_cast<int>(fLeftOffset);

	CComPtr<IUIAnimationTransition> pTransOpacity;
	if(hNodeParentToSet == NULL || hNodeParentToSet->m_bExpanded)
	{
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration, 1.0, &pTransOpacity);
		pStory->AddTransition(pNewNode->m_pAVOpacity, pTransOpacity);
	}
	else
	{

	}

	pStory->Schedule(GetCurrentTime(), NULL);

	pNewNode->m_pParentNode = hNodeParentToSet;

	if(hInsertBefore == STXTVI_FIRST)
	{
		m_AnimationManager->CreateAnimationVariable(0.0, &pNewNode->m_pAVTopOffset);
		STXTREENODELIST::iterator itInstered = pListToAddIn->insert(pListToAddIn->begin(), pNewNode);
		pNewNode->m_itPositionInParent = itInstered;
		ApplyOffsetAfter(pListToAddIn, pNewNode.get(), iItemInitialHeight, pNewNode.get(), TRUE);

		ResetScrollBars();
		InvalidateRect(m_hwndControl, NULL, TRUE);
		pNewNode->m_bInitialized = TRUE;
		m_mapAccIdToItem[pNewNode->m_nAccID] = pNewNode.get();
		return pNewNode.get();
	}
	else if(hInsertBefore == STXTVI_LAST || (hInsertBefore == STXTVI_SORT && m_sortFuncDefault.pfnSortFunc == NULL))
	{
		DOUBLE fHeightBeforeTheItem = GetCurrentTotalHeightBefore(pListToAddIn, STXTVI_LAST);
		m_AnimationManager->CreateAnimationVariable(fHeightBeforeTheItem, &pNewNode->m_pAVTopOffset);
		STXTREENODELIST::iterator itInstered = pListToAddIn->insert(pListToAddIn->end(), pNewNode);
		pNewNode->m_itPositionInParent = itInstered;

		ApplyOffsetAfter(pListToAddIn, pNewNode.get(), iItemInitialHeight, pNewNode.get(), TRUE);
		ResetScrollBars();
		InvalidateRect(m_hwndControl, NULL, TRUE);
		pNewNode->m_bInitialized = TRUE;
		m_mapAccIdToItem[pNewNode->m_nAccID] = pNewNode.get();
		return pNewNode.get();
	}
	else if(hInsertBefore == STXTVI_SORT)
	{
		STXTREENODELIST::iterator itInstered = pListToAddIn->begin();
		while(itInstered != pListToAddIn->end()
			&& m_sortFuncDefault.pfnSortFunc(pNewNode->m_dwNodeData, (*itInstered)->m_dwNodeData, m_sortFuncDefault.lParamSort) >= 0)
		{
			itInstered++;
		}

		if(itInstered == pListToAddIn->end())
		{
			return Internal_InsertItem(pNewNode->m_strText.c_str(), hParent, STXTVI_LAST, pNewNode->m_dwNodeData, iItemHeight);
		}
		else if(itInstered == pListToAddIn->begin())
		{
			return Internal_InsertItem(pNewNode->m_strText.c_str(), hParent, STXTVI_FIRST, pNewNode->m_dwNodeData, iItemHeight);
		}
		else
		{
			return Internal_InsertItem(pNewNode->m_strText.c_str(), hParent, (*itInstered).get(), pNewNode->m_dwNodeData, iItemHeight);
		}
	}
	else if(hInsertBefore != NULL)
	{
		DOUBLE fHeightBeforeTheItem = GetCurrentTotalHeightBefore(pListToAddIn, hInsertBefore);
		m_AnimationManager->CreateAnimationVariable(fHeightBeforeTheItem, &pNewNode->m_pAVTopOffset);
		STXTREENODELIST::iterator itInstered = pListToAddIn->insert(hInsertBefore->m_itPositionInParent, pNewNode);
		pNewNode->m_itPositionInParent = itInstered;
		ApplyOffsetAfter(pListToAddIn, pNewNode.get(), iItemInitialHeight, pNewNode.get(), TRUE);
		ResetScrollBars();
		InvalidateRect(m_hwndControl, NULL, TRUE);
		pNewNode->m_bInitialized = TRUE;
		m_mapAccIdToItem[pNewNode->m_nAccID] = pNewNode.get();
		return pNewNode.get();
	}

	return NULL;
 }

void CSTXAnimatedTreeCtrlNS::RegisterAnimatedTreeCtrlClass()
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = STXAnimatedTreeWindowProc;
	wc.lpszClassName = s_lpszAnimatedTreeCtrlClassName;
	wc.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
	wc.hCursor = ::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

	RegisterClass(&wc);
}

BOOL CSTXAnimatedTreeCtrlNS::Create( LPCTSTR lpszWindowText, DWORD dwStyle, int x, int y, int cx, int cy, HWND hWndParent, UINT nID )
{
	if (GetSafeHwnd())
		return FALSE;

	if (FAILED(m_AnimationManager.CoCreateInstance(CLSID_UIAnimationManager, NULL, CLSCTX_ALL)))
		return FALSE;

	if (FAILED(m_AnimationTimer.CoCreateInstance(CLSID_UIAnimationTimer, NULL, CLSCTX_ALL)))
		return FALSE;

	if (FAILED(m_AnimationTransitionLibrary.CoCreateInstance(CLSID_UIAnimationTransitionLibrary, NULL, CLSCTX_ALL)))
		return FALSE;

	m_AnimationManager->SetManagerEventHandler(this);
	m_AnimationManager->CreateAnimationVariable(0.0, &m_pAVExpanderOpacity);


	HWND hWnd = CreateWindow(s_lpszAnimatedTreeCtrlClassName, lpszWindowText, dwStyle, x, y, cx, cy, hWndParent, (HMENU)nID, GetModuleHandle(NULL), NULL);
	if(hWnd == NULL)
		return FALSE;

	SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)this);
	m_hwndControl = hWnd;

//	HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
//	if (hFont != NULL)
//	{
//		::SendMessage(m_hwndControl, WM_SETFONT, (WPARAM)hFont, FALSE);
//	}

	GetDefaultFontInfo();

	return TRUE;
}

void CSTXAnimatedTreeCtrlNS::OnPaint(HDC hDC)
{
	DrawControl(hDC);
}

void CSTXAnimatedTreeCtrlNS::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == STXTC_TIMER_ID_ANIMATION)
	{
		UpdateAnimationManager();
		InvalidateRect(m_hwndControl, NULL, TRUE);
	}
	else if(nIDEvent == STXTC_TIMER_ID_ITEM_FLOAT_CHECK)
	{
		OnItemFloatCheckTriggered();
	}
}

LRESULT CALLBACK CSTXAnimatedTreeCtrlNS::STXAnimatedTreeWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CSTXAnimatedTreeCtrlNS *pThis = (CSTXAnimatedTreeCtrlNS*)GetWindowLongPtr(hwnd, GWL_USERDATA);
	if(pThis == NULL)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	switch(uMsg)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			pThis->OnPaint(hdc);
			EndPaint(hwnd, &ps);
		}
		break;
	case WM_TIMER:
		pThis->OnTimer(static_cast<UINT>(wParam));
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_HSCROLL:
		pThis->OnHScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;
	case WM_VSCROLL:
		pThis->OnVScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;
	case WM_LBUTTONDOWN:
		pThis->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_LBUTTONUP:
		pThis->OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_RBUTTONDOWN:
		pThis->OnRButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_RBUTTONUP:
		pThis->OnRButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_LBUTTONDBLCLK:
		pThis->OnLButtonDblClk(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_MOUSEWHEEL:
		pThis->OnMouseWheel(GET_KEYSTATE_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam), LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		pThis->OnKeyDown(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		pThis->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		break;
	case WM_MOUSELEAVE:
		pThis->OnMouseLeave();
		break;
	case WM_SETFOCUS:
		pThis->OnSetFocus((HWND)wParam);
		break;
	case WM_KILLFOCUS:
		pThis->OnKillFocus((HWND)wParam);
		break;
	case WM_DESTROY:
		pThis->OnDestroy();
		break;
	case WM_GETOBJECT:
		return pThis->OnGetObject((DWORD)wParam, (DWORD)lParam);
		break;
	case WM_SETCURSOR:
		if(pThis->OnSetCursor((HWND)wParam, LOWORD(lParam), HIWORD(lParam)))
			return TRUE;
		break;
	case WM_GETDLGCODE:
		return pThis->OnGetDlgCode();
	case WM_SIZE:
		pThis->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CSTXAnimatedTreeCtrlNS::DrawControl(HDC hDC)
{
	RECT rcThis;
	::GetClientRect(m_hwndControl, &rcThis);
	Gdiplus::Rect rectThis(rcThis.left, rcThis.top, rcThis.right - rcThis.left, rcThis.bottom - rcThis.top);

	Gdiplus::Graphics g(hDC);
	Gdiplus::Bitmap bmpMem(rcThis.right - rcThis.left, rcThis.bottom - rcThis.top);
	Gdiplus::Graphics *pMemGraphics = Gdiplus::Graphics::FromImage(&bmpMem);

	DrawBackground(pMemGraphics, &rectThis);
	DrawWatermark(pMemGraphics, &rectThis);
	DrawContent(pMemGraphics, &rectThis);
	
	delete pMemGraphics;

	Gdiplus::TextureBrush brushContent(&bmpMem);
	g.FillRectangle(&brushContent, rectThis);
	g.ReleaseHDC(hDC);
}

void CSTXAnimatedTreeCtrlNS::DrawWatermark(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis)
{
	if (m_pImgWatermark)
	{
		UINT nImgWidth = m_pImgWatermark->GetWidth();
		UINT nImgHeight = m_pImgWatermark->GetHeight();

		Gdiplus::ColorMatrix *pCMUse = NULL;
		Gdiplus::ColorMatrix cm = {
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, static_cast<Gdiplus::REAL>(m_fWatermarkOpacity), 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f
		};

		pCMUse = &cm;

		Gdiplus::ImageAttributes ImgAttr;

		ImgAttr.SetColorMatrix(pCMUse, Gdiplus::ColorMatrixFlagsDefault,
			Gdiplus::ColorAdjustTypeBitmap);


		if (!m_pImgWatermarkCached)
		{
			Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(nImgWidth, nImgHeight);
			Gdiplus::Graphics graphics(pBitmap);
			Gdiplus::RectF rectImgDest(0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight));

			graphics.DrawImage(m_pImgWatermark.get(), rectImgDest, 0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight)
				, Gdiplus::UnitPixel, &ImgAttr);


			Gdiplus::Bitmap *pBitmapSrc = pBitmap;
			std::tr1::shared_ptr<Gdiplus::CachedBitmap> imgCached(new Gdiplus::CachedBitmap(pBitmapSrc, &graphics));
			m_pImgWatermarkCached = imgCached;

			delete pBitmap;
		}

		int nLocationX = 0;
		int nLocationY = 0;

		if (m_nWatermarkLocation & 0x01)
		{
			nLocationY = rectThis->Height - nImgHeight;
		}
		if (m_nWatermarkLocation & 0x02)
		{
			nLocationX = rectThis->Width - nImgWidth;
		}

		if (m_pImgWatermarkCached)
			pGraphics->DrawCachedBitmap(m_pImgWatermarkCached.get(), nLocationX, nLocationY);
		else
		{
			Gdiplus::RectF rectImgDest(static_cast<Gdiplus::REAL>(nLocationX), static_cast<Gdiplus::REAL>(nLocationY), static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight));
			pGraphics->DrawImage(m_pImgWatermark.get(), rectImgDest, 0, 0, static_cast<Gdiplus::REAL>(nImgWidth), static_cast<Gdiplus::REAL>(nImgHeight)
				, Gdiplus::UnitPixel, &ImgAttr);
		}
	}
}


void CSTXAnimatedTreeCtrlNS::DrawBackground(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis)
{
	if(m_pImgBackground)
	{
		if(!m_pImgBackgroundCached)
		{
			Gdiplus::Bitmap* pBitmap = new Gdiplus::Bitmap(rectThis->Width, rectThis->Height); 
			Gdiplus::Graphics graphics(pBitmap);
			graphics.DrawImage(m_pImgBackground.get(), 0, 0, rectThis->Width, rectThis->Height);

			Gdiplus::Bitmap *pBitmapSrc = pBitmap;
			std::tr1::shared_ptr<Gdiplus::CachedBitmap> imgCached(new Gdiplus::CachedBitmap(pBitmapSrc, pGraphics));
			m_pImgBackgroundCached = imgCached;

			delete pBitmap;
		}
		if(m_pImgBackgroundCached)
			pGraphics->DrawCachedBitmap(m_pImgBackgroundCached.get(), 0, 0);
		else
			pGraphics->DrawImage(m_pImgBackground.get(), rectThis->X, rectThis->Y, rectThis->Width, rectThis->Height);
	}
	else
	{
		Gdiplus::SolidBrush brushBk(m_clrBackground);
		pGraphics->FillRectangle(&brushBk, rectThis->X, rectThis->Y, rectThis->Width, rectThis->Height);
	}
}

void CSTXAnimatedTreeCtrlNS::DrawContent(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis)
{
	int iHScrollPos = GetScrollPos(m_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);

	DrawLine(pGraphics, rectThis);

	RECT rcClient;
	GetClientRect(m_hwndControl, &rcClient);

	std::queue<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > m_queueAllToDraw;

	//Draw Deleted items
	size_t nDrawDelete = m_queDeletedNodes.size();
	for(size_t i=0;i<nDrawDelete;i++)
	{
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pItem = m_queDeletedNodes.front();
		m_queDeletedNodes.pop();

		DOUBLE fItemOpacity = 0;
		pItem->m_pAVOpacity->GetValue(&fItemOpacity);

		if (fItemOpacity > 0.02f)
 		{
 			m_queDeletedNodes.push(pItem);
 			m_queueAllToDraw.push(pItem);
 		}
	}

	STXTREENODELIST::iterator it = m_arrRootNodes.begin();
	for(;it!=m_arrRootNodes.end();it++)
	{
		m_queueAllToDraw.push(*it);
	}

	size_t nItemToDraw = m_queueAllToDraw.size();
	for(size_t i=0;i<nItemToDraw;i++)
	{
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pItem = m_queueAllToDraw.front();
		m_queueAllToDraw.pop();

		DOUBLE fTopOffset = 0;
		pItem->m_pAVTopOffset->GetValue(&fTopOffset);
		DOUBLE fLeftOffset = 0;
		pItem->m_pAVLeftOffset->GetValue(&fLeftOffset);
		
		DOUBLE fTopOffsetFinal;
		pItem->m_pAVTopOffset->GetFinalValue(&fTopOffsetFinal);
		DOUBLE fLeftOffsetFinal;
		pItem->m_pAVLeftOffset->GetFinalValue(&fLeftOffsetFinal);

		if(!pItem->m_bPreDeleteNotified)
		{
			fTopOffset -= iVScrollPos;
			fTopOffsetFinal -= iVScrollPos;
		}
		else
		{
			fTopOffset -= pItem->m_nVOffsetDeleting;
			fTopOffsetFinal -= pItem->m_nVOffsetDeleting;
		}

		if(fTopOffset > rcClient.bottom)
			continue;

		//if(fTopOffset + STXATC_ITEM_HEIGHT < 0)
		//	continue;

		Gdiplus::PointF ptText(static_cast<Gdiplus::REAL>(fLeftOffset), static_cast<Gdiplus::REAL>(fTopOffset));
		Gdiplus::PointF ptTextFinal(static_cast<Gdiplus::REAL>(fLeftOffsetFinal), static_cast<Gdiplus::REAL>(fTopOffsetFinal));

		DOUBLE fItemHeightFinal = 0.0;
		pItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
		DOUBLE fItemHeight = 0.0;
		pItem->m_pAVItemHeight->GetValue(&fItemHeight);

		Gdiplus::RectF rectItem(ptText.X, ptText.Y, static_cast<Gdiplus::REAL>(rectThis->Width - STXATC_EXPANDER_WIDTH), static_cast<Gdiplus::REAL>(fItemHeight));
		Gdiplus::RectF rectItemFinal(ptTextFinal.X, ptTextFinal.Y, static_cast<Gdiplus::REAL>(rectThis->Width - STXATC_EXPANDER_WIDTH), static_cast<Gdiplus::REAL>(fItemHeightFinal));

		DOUBLE fExpanderOpacity = 0;
		m_pAVExpanderOpacity->GetValue(&fExpanderOpacity);

		if((GetStyle() & STXTVS_NO_EXPANDER_FADE) == 0)
			pItem->DrawItem(pGraphics, &rectItem, &rectItemFinal, fTopOffset, (Gdiplus::REAL)iHScrollPos, rcClient.bottom, 1.0, fExpanderOpacity);
		else
			pItem->DrawItem(pGraphics, &rectItem, &rectItemFinal, fTopOffset, (Gdiplus::REAL)iHScrollPos, rcClient.bottom, 1.0, 1.0);
	}
}

void CSTXAnimatedTreeCtrlNS::DrawLine(Gdiplus::Graphics *pGraphics, Gdiplus::Rect *rectThis)
{
	if(GetStyle() & STXTVS_NO_LINES)
		return;

	int iHScrollPos = GetScrollPos(m_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);

	DOUBLE fLineOpacity = 1.0;
	if((GetStyle() & STXTVS_NO_EXPANDER_FADE) == 0)
		m_pAVExpanderOpacity->GetValue(&fLineOpacity);

	Gdiplus::Pen pen(Gdiplus::Color(static_cast<BYTE>(255 * fLineOpacity), 192, 192, 192), 1);
	pen.SetDashStyle(Gdiplus::DashStyleDot);

	if(m_arrRootNodes.size() > 0)
	{
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pFirstItem = *m_arrRootNodes.begin();
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pLastItem = *m_arrRootNodes.rbegin();
		DOUBLE fItemHeightFirst = 0;
		pFirstItem->m_pAVItemHeight->GetValue(&fItemHeightFirst);
		DOUBLE fItemHeightLast = 0;
		pLastItem->m_pAVItemHeight->GetValue(&fItemHeightLast);


		Gdiplus::PointF ptTop(STXATC_EXPANDER_WIDTH / 2, 0);
		pGraphics->DrawLine(&pen, ptTop.X - iHScrollPos, ptTop.Y + static_cast<Gdiplus::REAL>(fItemHeightFirst / 2) - iVScrollPos, ptTop.X - iHScrollPos, ptTop.Y + static_cast<Gdiplus::REAL>(GetCurrentHeightWithoutLastChildExpand() - static_cast<Gdiplus::REAL>(fItemHeightLast * 0.5)) - iVScrollPos);
	}
}

HRESULT CSTXAnimatedTreeCtrlNS::OnManagerStatusChanged(UI_ANIMATION_MANAGER_STATUS newStatus, UI_ANIMATION_MANAGER_STATUS previousStatus)
{
	if(newStatus == UI_ANIMATION_MANAGER_BUSY)
	{
		//OutputDebugString(_T("Activate...\n"));
		::SetTimer(m_hwndControl, STXTC_TIMER_ID_ANIMATION, 10, NULL);
	}
	else
	{
		//OutputDebugString(_T("Deactivate...\n"));
		::KillTimer(m_hwndControl, STXTC_TIMER_ID_ANIMATION);
	}

	return S_OK;
}

DOUBLE CSTXAnimatedTreeCtrlNS::GetCurrentTotalHeightBefore(STXTREENODELIST *pList, HSTXTREENODE hNodeBefore)
{
	DOUBLE fTotalHeight = 0;
	STXTREENODELIST::iterator it = pList->begin();
	STXTREENODELIST::iterator itEnd;
	if(hNodeBefore == STXTVI_LAST)
	{
		itEnd = pList->end();
	}
	else
	{
		itEnd = hNodeBefore->m_itPositionInParent;
	}
	for(;it!=itEnd;it++)
	{
		fTotalHeight += (*it)->GetFinalHeight();
	}
	return fTotalHeight;
}

void CSTXAnimatedTreeCtrlNS::UpdateAnimationManager()
{
	UI_ANIMATION_UPDATE_RESULT result;
	UI_ANIMATION_SECONDS secTime;
	m_AnimationTimer->GetTime(&secTime);
	m_AnimationManager->Update(secTime, &result);
}

UI_ANIMATION_SECONDS CSTXAnimatedTreeCtrlNS::GetCurrentTime()
{
	UI_ANIMATION_SECONDS secTime;
	m_AnimationTimer->GetTime(&secTime);
	return secTime;
}

void CSTXAnimatedTreeCtrlNS::ApplyOffsetAfter(STXTREENODELIST *pList, HSTXTREENODE hNodeAfter, DOUBLE fOffset, HSTXTREENODE hNodeBaseline, BOOL bParentApply)
{
	STXTREENODELIST::iterator it;
	STXTREENODELIST::iterator itEnd = pList->end();
	if(hNodeAfter == STXTVI_LAST)
	{
		return;
	}
	else if(hNodeAfter == STXTVI_FIRST)
	{
		it = pList->begin();
	}
	else
	{
		it = hNodeAfter->m_itPositionInParent;
		it++;
	}

	CComPtr<IUIAnimationStoryboard> pStory;
	m_AnimationManager->CreateStoryboard(&pStory);

	int nApply = 0;
	for(;it!=itEnd;it++)
	{
		DOUBLE fOffsetOriginal;
		(*it)->m_pAVTopOffset->GetFinalValue(&fOffsetOriginal);
		DOUBLE fOffsetTarget = fOffsetOriginal + fOffset;

		CComPtr<IUIAnimationTransition> pTrans;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 2, fOffsetTarget, &pTrans);
		pStory->AddTransition((*it)->m_pAVTopOffset, pTrans);

		nApply++;
	}

	if(nApply > 0)
		pStory->Schedule(GetCurrentTime(), NULL);

	if(bParentApply)
	{
		if(hNodeBaseline != NULL && hNodeBaseline->m_pParentNode != NULL)
		{
			if(!hNodeBaseline->m_pParentNode->m_bExpanded)
				return;

			if(hNodeBaseline->m_pParentNode->m_pParentNode != NULL)
			{
				ApplyOffsetAfter(&hNodeBaseline->m_pParentNode->m_pParentNode->m_arrChildNodes, hNodeBaseline->m_pParentNode, fOffset, hNodeBaseline->m_pParentNode, bParentApply);
			}
			else
			{
				ApplyOffsetAfter(&m_arrRootNodes, hNodeBaseline->m_pParentNode, fOffset, hNodeBaseline->m_pParentNode, bParentApply);
			}
		}
	}
}

DOUBLE CSTXAnimatedTreeCtrlNS::GetCurrentHeightWithoutLastChildExpand()
{
	DOUBLE fHeight = 0.0;
	if(m_arrRootNodes.size() > 0)
	{
		//Get last sibling item's height first
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pLastItem = *m_arrRootNodes.rbegin();
		pLastItem->m_pAVItemHeight->GetValue(&fHeight);
	}

	//go through all without the last sibling item
	STXTREENODELIST::iterator it = m_arrRootNodes.begin();
	STXTREENODELIST::iterator itMore;
	for(;it!=m_arrRootNodes.end();it++)
	{
		if(it != m_arrRootNodes.end())
		{
			itMore = it;
			itMore++;
			if(itMore == m_arrRootNodes.end())
			{
				//fHeight;
				break;
			}
		}

		fHeight += (*it)->GetCurrentHeight();
	}

	return fHeight;
}

void CSTXAnimatedTreeCtrlNS::OnHScroll(UINT nSBCode, UINT nPos, HWND hWndScrollBar)
{
	OnEndEdit();

	int minpos;
	int maxpos;
	GetScrollRange(m_hwndControl, SB_HORZ, &minpos, &maxpos);
	maxpos = GetScrollLimit(SB_HORZ);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(m_hwndControl, SB_HORZ);
	int oldpos = curpos;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos -= 5;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos += 5;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
	{
		// Get the page size. 
		SCROLLINFO   info;
		info.fMask = SIF_ALL;
		GetScrollInfo(m_hwndControl, SB_HORZ, &info);

		if (curpos > minpos)
			curpos = max(minpos, curpos - (int)info.nPage);
	}
	break;

	case SB_PAGERIGHT:      // Scroll one page right.
	{
		// Get the page size. 
		SCROLLINFO   info;
		info.fMask = SIF_ALL;
		GetScrollInfo(m_hwndControl, SB_HORZ, &info);

		if (curpos < maxpos)
			curpos = min(maxpos, curpos + (int)info.nPage);
	}
	break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	SetScrollPos(m_hwndControl, SB_HORZ, curpos, TRUE);

	InvalidateRect(m_hwndControl, NULL, TRUE);
}

void CSTXAnimatedTreeCtrlNS::OnVScroll( UINT nSBCode, UINT nPos, HWND hWndScrollBar )
{
	OnEndEdit();

	int minpos;
	int maxpos;
	GetScrollRange(m_hwndControl, SB_VERT, &minpos, &maxpos); 
	maxpos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(m_hwndControl, SB_VERT);
	int oldpos = curpos;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		curpos = minpos;
		break;

	case SB_RIGHT:      // Scroll to far right.
		curpos = maxpos;
		break;

	case SB_ENDSCROLL:   // End scroll.
		break;

	case SB_LINELEFT:      // Scroll left.
		if (curpos > minpos)
			curpos-=5;
		break;

	case SB_LINERIGHT:   // Scroll right.
		if (curpos < maxpos)
			curpos+=5;
		break;

	case SB_PAGELEFT:    // Scroll one page left.
		{
			// Get the page size. 
			SCROLLINFO   info;
			info.fMask = SIF_ALL;
			GetScrollInfo(m_hwndControl, SB_VERT, &info);

			if (curpos > minpos)
				curpos = max(minpos, curpos - (int) info.nPage);
		}
		break;

	case SB_PAGERIGHT:      // Scroll one page right.
		{
			// Get the page size. 
			SCROLLINFO   info;
			info.fMask = SIF_ALL;
			GetScrollInfo(m_hwndControl, SB_VERT, &info);

			if (curpos < maxpos)
				curpos = min(maxpos, curpos + (int) info.nPage);
		}
		break;

	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos;      // of the scroll box at the end of the drag operation.
		break;

	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		curpos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	SetScrollPos(m_hwndControl, SB_VERT, curpos, TRUE);

	InvalidateRect(m_hwndControl, NULL, TRUE);
}

void CSTXAnimatedTreeCtrlNS::OnMouseWheel(UINT nFlags, short zDelta, int x, int y)
{
	if(!((GetWindowLong(m_hwndControl, GWL_STYLE) & WS_VSCROLL) == WS_VSCROLL))
	{
		return;
	}

	int minpos;
	int maxpos;
	GetScrollRange(m_hwndControl, SB_VERT, &minpos, &maxpos); 
	maxpos = GetScrollLimit(SB_VERT);

	// Get the current position of scroll box.
	int curpos = GetScrollPos(m_hwndControl, SB_VERT);
	int newpos = curpos - zDelta;
	newpos = min(newpos, maxpos);
	newpos = max(newpos, minpos);

	SetScrollPos(m_hwndControl, SB_VERT, newpos, TRUE);
	InvalidateRect(m_hwndControl, NULL, TRUE);
}

void CSTXAnimatedTreeCtrlNS::OnLButtonDblClk(int x, int y, UINT nFlags)
{
	UINT nHitFlags = 0;
	POINT pt = {x, y};
	HSTXTREENODE hNode = Internal_NodeHitTest(pt, &nHitFlags);
	if(hNode)
	{
		if((GetStyle() & STXTVS_SINGLE_CLICK_EXPAND) == 0)
		{
			Internal_Expand(hNode, STXATVE_TOGGLE);
		}
		SendCommonNotifyMessage(STXATVN_ITEMDBLCLICK, hNode, 0);
	}
}

void CSTXAnimatedTreeCtrlNS::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	OnEndEdit();

	if(nChar == VK_DOWN)
		SelectNextVisibleItem();
	else if(nChar == VK_UP)
		SelectPrevVisibleItem();
	else if(nChar == VK_LEFT)
	{
		OnKeyDown_Left();
	}
	else if(nChar == VK_RIGHT)
	{
		OnKeyDown_Right();
	}
	else if(nChar == VK_RETURN)
	{
		if(m_hSelectedNode == NULL)
			return;

		Internal_Expand(m_hSelectedNode, STXATVE_TOGGLE);
	}
}

void CSTXAnimatedTreeCtrlNS::OnKeyDown_Left()
{
	if(m_hSelectedNode == NULL)
		return;

	if(m_hSelectedNode->m_bExpanded && m_hSelectedNode->m_arrChildNodes.size() > 0)
		Internal_Expand(m_hSelectedNode, STXATVE_COLLAPSE);
	else
	{
		HSTXTREENODE hNodeAscendant = GetNearestVisibleAscendant(m_hSelectedNode);
		if(hNodeAscendant)
		{
			SelectNode(hNodeAscendant);
			Internal_EnsureVisible(hNodeAscendant);
		}
	}
}
void CSTXAnimatedTreeCtrlNS::OnKeyDown_Right()
{
	if(m_hSelectedNode == NULL)
		return;

	if(!m_hSelectedNode->m_bExpanded && (m_hSelectedNode->m_arrChildNodes.size() > 0 || m_hSelectedNode->m_dwItemStyle & STXTVIS_FORCE_SHOW_EXPANDER))
		Internal_Expand(m_hSelectedNode, STXATVE_EXPAND);
	else
	{
		HSTXTREENODE hNodeChild = Internal_GetNextItem(m_hSelectedNode, STXATVGN_CHILD);
		if(hNodeChild)
		{
			SelectNode(hNodeChild);
			Internal_EnsureVisible(hNodeChild);
		}
	}
}

void CSTXAnimatedTreeCtrlNS::OnLButtonDown(int x, int y, UINT nFlags, BOOL bForRButton)
{
	OnEndEdit();

	SetFocus(m_hwndControl);
	SetCapture(m_hwndControl);
	m_bLButtonDown = TRUE;
	m_ptLButtonDown.x = x;
	m_ptLButtonDown.y = y;

	UINT nHitFlags = 0;
	POINT pt = {x, y};
	HSTXTREENODE hNode = Internal_NodeHitTest(pt, &nHitFlags);
	m_nLButtonDownHitFlags = nHitFlags;
	if(hNode)
	{
		if(nHitFlags & STXATHT_ONITEMEXPANDER)
		{
			Internal_Expand(hNode, STXATVE_TOGGLE);
		}
	}

	if((nHitFlags & STXATHT_ONITEMEXPANDER) == 0)
	{
		SelectNode(hNode);
	}
}

void CSTXAnimatedTreeCtrlNS::OnLButtonUp(int x, int y, UINT nFlags, BOOL bForRButton)
{
	m_bLButtonDown = FALSE;
	m_bDragging = FALSE;
	if(m_hDragOverNode)
	{
		OnDrop(m_hDragOverNode);
		m_hDragOverNode->m_iDropHighlight = 0;
		InvalidateRect(m_hwndControl, NULL, TRUE);
		m_hDragOverNode = NULL;
	}
	ReleaseCapture();

	UINT nHitFlags = 0;
	POINT pt = {x, y};
	HSTXTREENODE hNode = Internal_NodeHitTest(pt, &nHitFlags);
	if(m_hSelectedNode && (nHitFlags & STXATHT_ONITEMEXPANDER) == 0 && (m_nLButtonDownHitFlags & STXATHT_ONITEMEXPANDER) == 0)
	{
		if(hNode && (GetStyle() & STXTVS_SINGLE_CLICK_EXPAND) && !bForRButton)
		{
			Internal_Expand(hNode, STXATVE_TOGGLE);
		}
		if(abs(m_ptLButtonDown.y - y) < 6)
		{
			SendCommonNotifyMessage(STXATVN_ITEMCLICK, m_hSelectedNode, bForRButton ? 1 : 0);
		}
	}
	else
	{
		SendCommonNotifyMessage(STXATVN_CLICK, NULL, bForRButton ? 1 : 0);
	}

}

void CSTXAnimatedTreeCtrlNS::OnRButtonUp(int x, int y, UINT nFlags)
{
	OnLButtonUp(x, y, nFlags, TRUE);
}
void CSTXAnimatedTreeCtrlNS::OnRButtonDown(int x, int y, UINT nFlags)
{
	OnLButtonDown(x, y, nFlags, TRUE);
}

void CSTXAnimatedTreeCtrlNS::OnMouseMove(int x, int y, UINT nFlags)
{
	KillTimer(m_hwndControl, STXTC_TIMER_ID_ITEM_FLOAT_CHECK);
	if(m_hLastHoverNode)
	{
		SetTimer(m_hwndControl, STXTC_TIMER_ID_ITEM_FLOAT_CHECK, STXTC_TIMER_INTERVAL_ITEM_FLOAT_CHECK, NULL);
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.hwndTrack = m_hwndControl;
	tme.dwFlags = TME_LEAVE;
	TrackMouseEvent(&tme);

	POINT point;
	point.x = x;
	point.y = y;
	HSTXTREENODE hNodeHover = Internal_NodeHitTest(point, NULL);

	if(m_hLastHoverNode != hNodeHover)
	{
		if(hNodeHover)
		{
			hNodeHover->OnMouseEnter();
		}

		if(m_hLastHoverNode && IsValidItem(m_hLastHoverNode))
			m_hLastHoverNode->OnMouseLeave();

		m_hLastHoverNode = hNodeHover;
		
		OnInternalHoverItemChanged(hNodeHover);
		SendCommonNotifyMessage(STXATVN_HOVERITEMCHANGED, hNodeHover, 0);

		if(!IsAnimationBusy())
			InvalidateRect(m_hwndControl, NULL, FALSE);
	}

	if(!m_bMouseInControl)
	{
		m_bMouseInControl = TRUE;

		if((GetStyle() & STXTVS_NO_EXPANDER_FADE) == 0)
			ApplySmoothStopTransition(m_pAVExpanderOpacity, m_nDefaultAnimationDuration / 4, 1.0);
	}

	CheckDrag(x, y);
}

void CSTXAnimatedTreeCtrlNS::OnMouseLeave()
{
	if(m_hLastHoverNode)
	{
		OnInternalHoverItemChanged(NULL);
		SendCommonNotifyMessage(STXATVN_HOVERITEMCHANGED, NULL, 0);

		if(IsValidItem(m_hLastHoverNode))
		{
			m_hLastHoverNode->OnMouseLeave();
			InvalidateRect(m_hwndControl, NULL, FALSE);
		}
	}
	m_hLastHoverNode = NULL;
	m_hLastFloatTriggeredNode = NULL;

	HWND hWndFocus = GetFocus();
	if(m_bMouseInControl)
	{
		m_bMouseInControl = FALSE;

		if(hWndFocus != m_hwndControl)
		{
			if((GetStyle() & STXTVS_NO_EXPANDER_FADE) == 0)
				ApplySmoothStopTransition(m_pAVExpanderOpacity, m_nDefaultAnimationDuration * 2, 0.0, NULL, TRUE);
		}
	}
}

void CSTXAnimatedTreeCtrlNS::OnSetFocus(HWND hWndOldFocus)
{
	if((GetStyle() & STXTVS_NO_EXPANDER_FADE) == 0)
		ApplySmoothStopTransition(m_pAVExpanderOpacity, m_nDefaultAnimationDuration / 4, 1.0);
}

void CSTXAnimatedTreeCtrlNS::OnKillFocus(HWND hWndNewFocus)
{
	if(!m_bMouseInControl)
	{
		if((GetStyle() & STXTVS_NO_EXPANDER_FADE) == 0)
			ApplySmoothStopTransition(m_pAVExpanderOpacity, m_nDefaultAnimationDuration * 2, 0.0, NULL, TRUE);
	}
}

void CSTXAnimatedTreeCtrlNS::OnDestroy()
{
	OnEndEdit();

	std::vector<HSTXTREENODE> arrItemsToBeDelete;
	GetDescendantItems(STXTVI_ROOT, &arrItemsToBeDelete);
	std::vector<HSTXTREENODE>::iterator itPreDel = arrItemsToBeDelete.begin();
	for(;itPreDel!=arrItemsToBeDelete.end();itPreDel++)
	{
		(*itPreDel)->TriggerItemPreDeleteEvent();
	}

	m_arrRootNodes.clear();
	while(m_queDeletedNodes.size() > 0)
		m_queDeletedNodes.pop();

	if (m_pDefaultFont)
	{
		delete m_pDefaultFont;
		m_pDefaultFont = NULL;
	}
	if (m_pDefaultSubTextFont)
	{
		delete m_pDefaultSubTextFont;
		m_pDefaultSubTextFont = NULL;
	}

	m_pImgWatermarkCached = NULL;
	m_pImgWatermark = NULL;
}

LRESULT CSTXAnimatedTreeCtrlNS::OnGetObject(DWORD dwFlags, DWORD dwObjId)
{
	LRESULT lres = 0;
 	if(dwObjId == OBJID_CLIENT)
 	{
 		lres = LresultFromObject(IID_IAccessible, dwFlags, (IAccessible*)this);
 	}
	return lres;
}

BOOL CSTXAnimatedTreeCtrlNS::OnSetCursor( HWND hWnd, UINT nHitTest, UINT message )
{
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(m_hwndControl, &point);
	HSTXTREENODE hNodeHover = Internal_NodeHitTest(point, NULL);
	if(hNodeHover)
	{
		if(hNodeHover->m_dwItemStyle & STXTVIS_HANDCURSOR)
		{
			SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
			return TRUE;
		}
	}
	return FALSE;
}

UINT CSTXAnimatedTreeCtrlNS::OnGetDlgCode()
{
	return DLGC_WANTARROWS;
}

void CSTXAnimatedTreeCtrlNS::OnSize( UINT nType, int cx, int cy )
{
	ResetScrollBars();
}

void CSTXAnimatedTreeCtrlNS::ResetScrollBars()
{
	if(!IsWindow(m_hwndControl))
		return;

	RECT rcClient;
	::GetClientRect(m_hwndControl, &rcClient);

	//Vertical Scroll Bar
	int iTotalHeightAvailable = rcClient.bottom - rcClient.top;

	int iCurPos = 0;
	BOOL bVScrollExist;
	if((GetWindowLong(m_hwndControl, GWL_STYLE) & WS_VSCROLL) == WS_VSCROLL)
	{
		iCurPos = ::GetScrollPos(m_hwndControl, SB_VERT);
		bVScrollExist = TRUE;
	}
	else
		bVScrollExist = FALSE;

	int iOldPos = ::GetScrollPos(m_hwndControl, SB_VERT);

	int nTotalHeight = static_cast<int>(GetCurrentTotalHeightBefore(&m_arrRootNodes, STXTVI_LAST));
	if(m_arrRootNodes.size() > 0 && nTotalHeight > iTotalHeightAvailable)	//Need H-ScrollBar
	{
		OutputDebugString(_T("Need V-Scrol Bar\n"));
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE|SIF_POS|SIF_RANGE;
		si.nPage = iTotalHeightAvailable;
		si.nMin = 0;
		si.nMax = nTotalHeight;
		si.nPos = min(iCurPos,si.nMax);

		SetScrollPos(m_hwndControl, SB_VERT, si.nPos, FALSE);
		SetScrollInfo(m_hwndControl, SB_VERT, &si, TRUE);
		::ShowScrollBar(m_hwndControl, SB_VERT, TRUE);
		ModifyStyle(GWL_STYLE, 0, WS_VSCROLL, 0);
	}
	else
	{
		int iCurPos = GetScrollPos(m_hwndControl, SB_VERT);
		//ScrollWindow(m_hwndControl, 0, iCurPos, NULL, NULL);
		SetScrollPos(m_hwndControl, SB_VERT, 0, TRUE);
		::ShowScrollBar(m_hwndControl, SB_VERT, FALSE);
		ModifyStyle(GWL_STYLE, WS_VSCROLL, 0, 0);
	}

	//Horizontal Scroll Bar
	ResetHorizontalScrollBar();

	InvalidateRect(m_hwndControl, NULL, FALSE);
}

void CSTXAnimatedTreeCtrlNS::ResetHorizontalScrollBar()
{
	RECT rcClient;
	::GetClientRect(m_hwndControl, &rcClient);

	int iTotalWidthAvailable = rcClient.right - rcClient.left;

	int iCurPos = 0;
	BOOL bHScrollExist;
	if ((GetWindowLong(m_hwndControl, GWL_STYLE) & WS_HSCROLL) == WS_HSCROLL)
	{
		iCurPos = ::GetScrollPos(m_hwndControl, SB_HORZ);
		bHScrollExist = TRUE;
	}
	else
		bHScrollExist = FALSE;

	int iOldPos = ::GetScrollPos(m_hwndControl, SB_HORZ);

	
	int nTotalWidth = static_cast<int>(GetCurrentContentWidth());
	if (m_arrRootNodes.size() > 0 && nTotalWidth > iTotalWidthAvailable)	//Need H-ScrollBar
	{
		//OutputDebugString(_T("Need H-Scrol Bar\n"));
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		si.nPage = iTotalWidthAvailable;
		si.nMin = 0;
		si.nMax = nTotalWidth;
		si.nPos = min(iCurPos, si.nMax);

		SetScrollPos(m_hwndControl, SB_HORZ, si.nPos, FALSE);
		SetScrollInfo(m_hwndControl, SB_HORZ, &si, TRUE);
		::ShowScrollBar(m_hwndControl, SB_HORZ, TRUE);
		ModifyStyle(GWL_STYLE, 0, WS_HSCROLL, 0);
	}
	else
	{
		int iCurPos = GetScrollPos(m_hwndControl, SB_HORZ);
		//ScrollWindow(m_hwndControl, 0, iCurPos, NULL, NULL);
		SetScrollPos(m_hwndControl, SB_HORZ, 0, TRUE);
		::ShowScrollBar(m_hwndControl, SB_HORZ, FALSE);
		ModifyStyle(GWL_STYLE, WS_HSCROLL, 0, 0);
	}
}

BOOL CSTXAnimatedTreeCtrlNS::ModifyStyle(int nStyleOffset, DWORD dwRemove, DWORD dwAdd, UINT nFlags)
{
	DWORD dwStyle = ::GetWindowLong(m_hwndControl, nStyleOffset);
	DWORD dwNewStyle = (dwStyle & ~dwRemove) | dwAdd;
	if (dwStyle == dwNewStyle)
		return FALSE;

	::SetWindowLong(m_hwndControl, nStyleOffset, dwNewStyle);
	if (nFlags != 0)
	{
		::SetWindowPos(m_hwndControl, NULL, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
	}
	return TRUE;
}

void CSTXAnimatedTreeCtrlNS::ModifyStyle( DWORD dwRemove, DWORD dwAdd )
{
	if(GetSafeHwnd() == NULL)
		return;

	ModifyStyle(GWL_STYLE, dwRemove, dwAdd, 0);
	InvalidateRect(m_hwndControl, NULL, FALSE);
}

int CSTXAnimatedTreeCtrlNS::GetScrollLimit(int nBar)
{
	int nMin, nMax;
	GetScrollRange(m_hwndControl, nBar, &nMin, &nMax);
	SCROLLINFO info;
	info.fMask = SIF_PAGE;
	if (GetScrollInfo(m_hwndControl, nBar, &info))
	{
		nMax -= __max(info.nPage-1,0);
	}
	return nMax;
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_NodeHitTest(POINT pt, UINT *pFlags)
{
	RECT rcClient;
	GetClientRect(m_hwndControl, &rcClient);

	int iHScrollPos = GetScrollPos(m_hwndControl, SB_HORZ);
	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);
	pt.x += iHScrollPos;
	pt.y += iVScrollPos;

	HSTXTREENODE hNodeHit = NULL;

	STXTREENODELIST::iterator it = m_arrRootNodes.begin();
	for(;it!=m_arrRootNodes.end();it++)
	{
		DOUBLE fTopOffset = 0;
		(*it)->m_pAVTopOffset->GetValue(&fTopOffset);

		hNodeHit = (*it)->HitTest(pt, rcClient.right + iHScrollPos, static_cast<int>(fTopOffset), pFlags);
		if(hNodeHit)
		{
			return hNodeHit;
		}
	}
	return NULL;
}

HWND CSTXAnimatedTreeCtrlNS::GetSafeHwnd()
{
	return m_hwndControl;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_Expand(HSTXTREENODE hNode, UINT nCode)
{
	if(hNode == NULL || hNode == STXTVI_ROOT || hNode == STXTVI_FIRST || hNode == STXTVI_LAST)
		return FALSE;

	int iActionToDo = STXATVE_NONE;
	BOOL bReset = FALSE;
	if(nCode & STXATVE_COLLAPSERESET)
		bReset = TRUE;
	WORD nCodeNumber = (nCode & 0x00FF);
	if(nCodeNumber == STXATVE_COLLAPSE && hNode->m_bExpanded)
	{
		iActionToDo = STXATVE_COLLAPSE;
	}
	else if(nCodeNumber == STXATVE_EXPAND && !hNode->m_bExpanded)
	{
		iActionToDo = STXATVE_EXPAND;
	}
	else if(nCodeNumber == STXATVE_TOGGLE)
	{
		if(hNode->m_bExpanded)
		{
			iActionToDo = STXATVE_COLLAPSE;
		}
		else
		{
			iActionToDo = STXATVE_EXPAND;
		}
	}

	if(nCodeNumber == STXATVE_COLLAPSE && bReset)
		hNode->m_dwItemStyle &= (~STXTVIS_EXPANDEDONCE);

	if(iActionToDo == STXATVE_COLLAPSE)
	{
		DOUBLE fItemHeightFinal = 0.0;
		hNode->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
		if(hNode->m_pParentNode)
			ApplyOffsetAfter(&hNode->m_pParentNode->m_arrChildNodes, hNode, -(hNode->GetFinalHeightExpanded() - fItemHeightFinal), hNode, TRUE);
		else
			ApplyOffsetAfter(&m_arrRootNodes, hNode, -(hNode->GetFinalHeightExpanded() - fItemHeightFinal), hNode, TRUE);

		hNode->m_bExpanded = FALSE;
		SetAllChildrenOpacity(hNode, 0.0);
		AdjustSelection(hNode);
	}
	else if(iActionToDo == STXATVE_EXPAND)
	{
		if((hNode->m_dwItemStyle & STXTVIS_EXPANDEDONCE) == 0)
		{
			SendCommonNotifyMessage(STXATVN_ITEMEXPANDING, hNode, 0);
		}

		DOUBLE fItemHeightFinal = 0.0;
		hNode->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
		if(hNode->m_pParentNode)
			ApplyOffsetAfter(&hNode->m_pParentNode->m_arrChildNodes, hNode, (hNode->GetFinalHeightExpanded() - fItemHeightFinal), hNode, TRUE);
		else
			ApplyOffsetAfter(&m_arrRootNodes, hNode, (hNode->GetFinalHeightExpanded() - fItemHeightFinal), hNode, TRUE);

		hNode->m_bExpanded = TRUE;
		SetAllChildrenOpacity(hNode, 1.0);

		if((hNode->m_dwItemStyle & STXTVIS_EXPANDEDONCE) == 0)
		{
			hNode->m_dwItemStyle |= STXTVIS_EXPANDEDONCE;

			SendCommonNotifyMessage(STXATVN_ITEMEXPANDED, hNode, 0);
		}
	}

	ResetScrollBars();
	InvalidateRect(m_hwndControl, NULL, TRUE);
	return TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::IsNeedApplyOffset(HSTXTREENODE hNode)
{
	BOOL bNeedApplyOffset = TRUE;
	hNode = hNode->m_pParentNode;
	while(hNode && bNeedApplyOffset)
	{
		bNeedApplyOffset = (bNeedApplyOffset && hNode->m_bExpanded);
		hNode = hNode->m_pParentNode;
	}
	return bNeedApplyOffset;
}

void CSTXAnimatedTreeCtrlNS::SetAllChildrenOpacity( HSTXTREENODE hNode, DOUBLE fTargetOpacity )
{
	if(hNode == NULL || hNode == STXTVI_ROOT || hNode == STXTVI_FIRST || hNode == STXTVI_LAST)
		return;

	CComPtr<IUIAnimationStoryboard> pStory;
	m_AnimationManager->CreateStoryboard(&pStory);


	STXTREENODELIST::iterator it = hNode->m_arrChildNodes.begin();
	for(;it!=hNode->m_arrChildNodes.end();it++)
	{
		CComPtr<IUIAnimationTransition> pTrans;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 2, fTargetOpacity, &pTrans);
		pStory->AddTransition((*it)->m_pAVOpacity, pTrans);
	}

	pStory->Schedule(GetCurrentTime(), NULL);
}

void CSTXAnimatedTreeCtrlNS::AdjustSelection(HSTXTREENODE hNodeCollapsed)
{
	if(m_hSelectedNode == NULL)
		return;

	HSTXTREENODE hNodeTrace = m_hSelectedNode->m_pParentNode;
	while(hNodeTrace)
	{
		if(hNodeTrace == hNodeCollapsed)
		{
			SelectNode(hNodeCollapsed);
			break;
		}
		hNodeTrace = hNodeTrace->m_pParentNode;
	}
}

void CSTXAnimatedTreeCtrlNS::SelectNode( HSTXTREENODE hNode )
{
	if(m_hSelectedNode != hNode)
	{
		if(m_hSelectedNode)
		{
			m_hSelectedNode->m_bSelected = FALSE;
		}
		m_hSelectedNode = hNode;
		if(hNode)
		{
			hNode->m_bSelected = TRUE;
		}
		SendCommonNotifyMessage(STXATVN_SELECTEDITEMCHANGED, hNode, 0);
		InvalidateRect(m_hwndControl, NULL, TRUE);
	}
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_GetChildItem( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_SORT)
		return NULL;

	if(hItem == STXTVI_ROOT)
	{
		if(m_arrRootNodes.size() == 0)
			return NULL;

		return m_arrRootNodes.front().get();
	}

	if(hItem->m_arrChildNodes.size() == 0)
		return NULL;

	return hItem->m_arrChildNodes.front().get();
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_GetNextItem( HSTXTREENODE hItem, UINT nCode )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_SORT)
		return NULL;

	STXTREENODELIST *pNodeList;
	if(hItem == STXTVI_ROOT)
		pNodeList = &m_arrRootNodes;
	else
		pNodeList = &hItem->m_arrChildNodes;

	if(nCode == STXATVGN_NEXT)
	{
		return Internal_GetNextSiblingItem(hItem);
	}
	else if(nCode == STXATVGN_PREVIOUS)
	{
		return Internal_GetPrevSiblingItem(hItem);
	}
	else if(nCode == STXATVGN_CHILD)
	{
		if(pNodeList->size() > 0)
			return pNodeList->front().get();
	}
	else if(nCode == STXATVGN_PARENT)
	{
		if(hItem == STXTVI_ROOT)
			return NULL;
		if(hItem->m_pParentNode == NULL)
			return STXTVI_ROOT;

		return hItem->m_pParentNode;
	}
	else if(nCode == STXATVGN_NEXTVISIBLE)
	{
		return Internal_GetNextVisibleItem(hItem);
	}
	else if(nCode == STXATVGN_PREVIOUSVISIBLE)
	{
		return Internal_GetPrevVisibleItem(hItem);
	}
	else if(nCode == STXATVGN_ROOT)
	{
		return STXTVI_ROOT;
	}
	else if(nCode == STXATVGN_CARET)
	{
		return m_hSelectedNode;
	}
	return NULL;
}

BOOL CSTXAnimatedTreeCtrlNS::IsAscendant( HSTXTREENODE hNodeAscendant, HSTXTREENODE hNode )
{
	if(hNode == NULL || hNode == STXTVI_FIRST || hNode == STXTVI_LAST)
		return FALSE;

	if(hNodeAscendant == NULL || hNodeAscendant == STXTVI_FIRST || hNodeAscendant == STXTVI_LAST)
		return FALSE;

	if(hNodeAscendant == STXTVI_ROOT)
		return TRUE;

	hNode = hNode->m_pParentNode;
	while(hNode)
	{
		if(hNode == hNodeAscendant)
			return TRUE;

		hNode = hNode->m_pParentNode;
	}

	return FALSE;
}

BOOL CSTXAnimatedTreeCtrlNS::IsItemVisible( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return FALSE;

	hItem = hItem->m_pParentNode;
	while(hItem)
	{
		if(!hItem->m_bExpanded)
			return FALSE;

		hItem = hItem->m_pParentNode;
	}

	return TRUE;
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::GetNearestVisibleAscendant( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return NULL;

	//if all Ascendants are expanded, then get the parent directly
	//else get the furthest collapsed Ascendant
	HSTXTREENODE hItemCurrent = hItem->m_pParentNode;
	HSTXTREENODE hItemCollapse = NULL;
	while(hItemCurrent)
	{
		if(!hItemCurrent->m_bExpanded)
			hItemCollapse = hItemCurrent;

		hItemCurrent = hItemCurrent->m_pParentNode;
	}

	return hItemCollapse ? hItemCollapse : hItem->m_pParentNode;
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::GetSelectedItem()
{
	return m_hSelectedNode;
}

void CSTXAnimatedTreeCtrlNS::SelectNextVisibleItem()
{
	if(m_hSelectedNode == NULL && m_arrRootNodes.size() > 0)
	{
		HSTXTREENODE hNodeToSelect = (*m_arrRootNodes.begin()).get();
		SelectNode(hNodeToSelect);
		Internal_EnsureVisible(hNodeToSelect);
		return;
	}

	HSTXTREENODE hItemNext = Internal_GetNextItem(m_hSelectedNode, STXATVGN_NEXTVISIBLE);
	if(hItemNext)
	{
		SelectNode(hItemNext);
		Internal_EnsureVisible(hItemNext);
	}
}

void CSTXAnimatedTreeCtrlNS::SelectPrevVisibleItem()
{
	if(m_hSelectedNode == NULL && m_arrRootNodes.size() > 0)
	{
		HSTXTREENODE hNodeToSelect = (*m_arrRootNodes.begin()).get();
		SelectNode(hNodeToSelect);
		Internal_EnsureVisible(hNodeToSelect);
		return;
	}

	HSTXTREENODE hItemNext = Internal_GetNextItem(m_hSelectedNode, STXATVGN_PREVIOUSVISIBLE);
	if(hItemNext)
	{
		SelectNode(hItemNext);
		Internal_EnsureVisible(hItemNext);
	}
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_GetNextSiblingItem(HSTXTREENODE hItem)
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_ROOT || hItem == STXTVI_SORT)
		return NULL;

	STXTREENODELIST *pNodeList = &m_arrRootNodes;
	if(hItem->m_pParentNode != NULL)
		pNodeList = &hItem->m_pParentNode->m_arrChildNodes;

	STXTREENODELIST::iterator it = hItem->m_itPositionInParent;
	it++;
	return it == pNodeList->end() ? NULL : (*it).get();
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_GetPrevSiblingItem( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return NULL;

	STXTREENODELIST *pNodeList = &m_arrRootNodes;
	if(hItem->m_pParentNode != NULL)
		pNodeList = &hItem->m_pParentNode->m_arrChildNodes;

	STXTREENODELIST::iterator it = hItem->m_itPositionInParent;
	if(it == pNodeList->begin())
		return NULL;
	
	it--;
	return (*it).get();
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_GetPrevVisibleItem( HSTXTREENODE hItem )
{
	HSTXTREENODE hItemVisibleAscendant = GetNearestVisibleAscendant(hItem);
	if(hItemVisibleAscendant == hItem->m_pParentNode)
	{
		//the Node's parent is visible
		HSTXTREENODE hPrevSiblingItem = Internal_GetPrevSiblingItem(hItem);
		if(hPrevSiblingItem)
			return GetFurthestVisibleItem(hPrevSiblingItem);

		return GetNearestVisibleAscendant(hItem);
	}
	else
	{
		return hItemVisibleAscendant;
	}

	return NULL;
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_GetNextVisibleItem( HSTXTREENODE hItem )
{
	HSTXTREENODE hItemVisibleAscendant = GetNearestVisibleAscendant(hItem);
	HSTXTREENODE hItemCurrent = hItem;
	if(hItemVisibleAscendant == hItem->m_pParentNode)
	{
		//the Node's parent is visible
		if(hItem->m_bExpanded && hItem->m_arrChildNodes.size() > 0)
		{
			return hItem->m_arrChildNodes.front().get();
		}
	}
	else
	{
		hItemCurrent = hItemVisibleAscendant;
	}
	HSTXTREENODE hItemNext = hItem;
	hItemNext = Internal_GetNextItem(hItemCurrent, STXATVGN_NEXT);
	while(hItemNext == NULL && hItemCurrent->m_pParentNode != NULL)
	{
		hItemCurrent = hItemCurrent->m_pParentNode;
		hItemNext = Internal_GetNextItem(hItemCurrent, STXATVGN_NEXT);
	}
	return hItemNext;
}

DOUBLE CSTXAnimatedTreeCtrlNS::GetItemTopOffsetAbsolute( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return 0.0;

	DOUBLE fOffset = 0;
	hItem->m_pAVTopOffset->GetValue(&fOffset);

	hItem = hItem->m_pParentNode;
	while(hItem)
	{
		DOUBLE fItemHeight = 0;
		hItem->m_pAVItemHeight->GetValue(&fItemHeight);
		DOUBLE fItemTopOffset = 0;
		hItem->m_pAVTopOffset->GetValue(&fItemTopOffset);

		fOffset += fItemHeight;
		fOffset += fItemTopOffset;
		hItem = hItem->m_pParentNode;
	}
	return fOffset;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_EnsureVisible( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return FALSE;

	DOUBLE fItemHeightFinal = 0.0;
	hItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);

	DOUBLE fOffset = 0;
	hItem->m_pAVTopOffset->GetValue(&fOffset);
	hItem = hItem->m_pParentNode;
	DOUBLE fItemHeightFinalIteration = 0.0;
	while(hItem)
	{
		hItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinalIteration);
		Internal_Expand(hItem, STXATVE_EXPAND);
		fOffset += fItemHeightFinalIteration;

		DOUBLE fTopOffset = 0;
		hItem->m_pAVTopOffset->GetValue(&fTopOffset);
		fOffset += fTopOffset;
		hItem = hItem->m_pParentNode;
	}

	//Make it visible
	int iOffsetLow = static_cast<int>(fOffset);
	int iOffsetHigh = static_cast<int>(fOffset + fItemHeightFinal);
	RECT rcClient;
	GetClientRect(m_hwndControl, &rcClient);
	int iScrollPos = GetScrollPos(m_hwndControl, SB_VERT);
	int iViewMin = iScrollPos;
	int iViewMax = iScrollPos + rcClient.bottom;

	if(iOffsetLow < iViewMin)
	{
		SetScrollPos(m_hwndControl, SB_VERT, max(0, iOffsetLow), TRUE);
	}
	else if(iOffsetHigh > iViewMax)
	{
		int nScMin = 0;
		int nScMax = 0;
		GetScrollRange(m_hwndControl, SB_VERT, &nScMin, &nScMax);
		SetScrollPos(m_hwndControl, SB_VERT, min(nScMax, iOffsetHigh - rcClient.bottom), TRUE);
	}
	return TRUE;
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::GetFurthestVisibleItem( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return NULL;

	while(hItem && hItem->m_bExpanded && hItem->m_arrChildNodes.size() > 0)
	{
		hItem = hItem->m_arrChildNodes.back().get();
	}
	return hItem;
}


std::tr1::shared_ptr<Gdiplus::Image> CSTXAnimatedTreeCtrlNS::GetResizedImage( IStream *pStream, int nWidthHeight )
{
	std::tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(pStream));
	return GetResizedImage(img, nWidthHeight);
}

std::tr1::shared_ptr<Gdiplus::Image> CSTXAnimatedTreeCtrlNS::GetResizedImage( HBITMAP hBitmap, int nWidthHeight )
{
	std::tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Bitmap(hBitmap, NULL));
	return GetResizedImage(img, nWidthHeight);
}

std::tr1::shared_ptr<Gdiplus::Image> CSTXAnimatedTreeCtrlNS::GetResizedImage( LPCTSTR lpszFile, int nWidthHeight )
{
	std::tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(lpszFile));
	return GetResizedImage(img, nWidthHeight);
}

std::tr1::shared_ptr<Gdiplus::Image> CSTXAnimatedTreeCtrlNS::GetResizedImage( std::tr1::shared_ptr<Gdiplus::Image> pImage, int nWidthHeight )
{
	UINT o_height = pImage->GetHeight();
	UINT o_width = pImage->GetWidth();

	if(nWidthHeight > 0 && (o_height > (UINT)nWidthHeight || o_width > (UINT)nWidthHeight))
	{
		INT n_width = nWidthHeight;
		INT n_height = nWidthHeight;
		double ratio = ((double)o_width) / ((double)o_height);
		if (o_width > o_height) 
		{
			// Resize down by width
			n_height = static_cast<UINT>(((double)n_width) / ratio);
		} 
		else
		{
			n_width = static_cast<UINT>(n_height * ratio);
		}

		std::tr1::shared_ptr<Gdiplus::Image> imgThumbnail(pImage->GetThumbnailImage(n_width, n_height));
		return imgThumbnail;
	}
	else
	{
		return pImage;
	}
}

BOOL CSTXAnimatedTreeCtrlNS::SetItemImage(HSTXTREENODE hItem, IStream *pStream, BOOL bResizeImage)
{
	if (hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return FALSE;

	DOUBLE fItemHeightFinal = 0.0;
	hItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
	int nResizeImageWidth = -1;		//Use original image
	if(bResizeImage)
	{
		nResizeImageWidth = static_cast<int>(fItemHeightFinal);
	}
	std::tr1::shared_ptr<Gdiplus::Image> pImg = GetResizedImage(pStream, nResizeImageWidth);
	if(pImg && pImg->GetWidth() == 0)
		pImg.reset();

	hItem->m_pImgImage = pImg;

	if(pImg && pImg->GetWidth() > 0)
	{
		CComPtr<IUIAnimationStoryboard> pStory;
		m_AnimationManager->CreateStoryboard(&pStory);

		CComPtr<IUIAnimationTransition> pTrans;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, 1.0, &pTrans);
		pTrans->SetInitialValue(1.1);
		pTrans->SetInitialVelocity(3.0);
		pStory->AddTransition(hItem->m_pAVImageScale, pTrans);

		CComPtr<IUIAnimationTransition> pTransImageOpacity;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, 1.0, &pTransImageOpacity);
		pStory->AddTransition(hItem->m_pAVImageOpacity, pTransImageOpacity);

		DOUBLE fItemHeightFinal = 0.0;
		hItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
		CComPtr<IUIAnimationTransition> pTransImageSize;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, fItemHeightFinal + 2, &pTransImageSize);
		pTransImageSize->SetInitialVelocity(0.0);
		pStory->AddTransition(hItem->m_pAVImageOccupy, pTransImageSize);

		pStory->Schedule(GetCurrentTime(), NULL);
	}
	else
	{
		CComPtr<IUIAnimationStoryboard> pStory;
		m_AnimationManager->CreateStoryboard(&pStory);

		CComPtr<IUIAnimationTransition> pTrans;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, 0.0, &pTrans);
		pStory->AddTransition(hItem->m_pAVImageScale, pTrans);

		CComPtr<IUIAnimationTransition> pTransImageOpacity;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, 0.0, &pTransImageOpacity);
		pStory->AddTransition(hItem->m_pAVImageOpacity, pTransImageOpacity);

		CComPtr<IUIAnimationTransition> pTransImageSize;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, 0, &pTransImageSize);
		pStory->AddTransition(hItem->m_pAVImageOccupy, pTransImageSize);

		pStory->Schedule(GetCurrentTime(), NULL);
		hItem->m_pImgImage = std::tr1::shared_ptr<Gdiplus::Image>();		//NULL
	}

	hItem->m_nBoundsMaxX = -1;
	ResetHorizontalScrollBar();

	if(GetSafeHwnd() && IsItemVisible(hItem))
		InvalidateRect(m_hwndControl, NULL, TRUE);

	return TRUE;
}

void CSTXAnimatedTreeCtrlNS::SetItemImageCallback(HSTXTREENODE hItem, BOOL bUseImageCallback)
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_ROOT || hItem == STXTVI_SORT)
		return;

	DOUBLE fItemHeightFinal = 0.0;
	hItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
	if(bUseImageCallback)
	{
		hItem->m_dwItemStyle |= STXTVIS_IMAGE_CALLBACK;

		CComPtr<IUIAnimationStoryboard> pStory;
		m_AnimationManager->CreateStoryboard(&pStory);

		CComPtr<IUIAnimationTransition> pTrans;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, 1.0, &pTrans);
		pTrans->SetInitialValue(2.0);
		pTrans->SetInitialVelocity(0.0);
		pStory->AddTransition(hItem->m_pAVImageScale, pTrans);

		DOUBLE fItemHeightFinal = 0.0;
		hItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
		CComPtr<IUIAnimationTransition> pTransImageSize;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, fItemHeightFinal + 2, &pTransImageSize);
		pTransImageSize->SetInitialVelocity(0.0);
		pStory->AddTransition(hItem->m_pAVImageOccupy, pTransImageSize);

		pStory->Schedule(GetCurrentTime(), NULL);
	}
	else
	{
		hItem->m_dwItemStyle &= ~STXTVIS_IMAGE_CALLBACK;

		CComPtr<IUIAnimationStoryboard> pStory;
		m_AnimationManager->CreateStoryboard(&pStory);

		CComPtr<IUIAnimationTransition> pTrans;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, 2.0, &pTrans);
		pStory->AddTransition(hItem->m_pAVImageScale, pTrans);

		CComPtr<IUIAnimationTransition> pTransImageSize;
		m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 1.0, 0, &pTransImageSize);
		pStory->AddTransition(hItem->m_pAVImageOccupy, pTransImageSize);

		pStory->Schedule(GetCurrentTime(), NULL);
	}
}


BOOL CSTXAnimatedTreeCtrlNS::SetItemSubImage( HSTXTREENODE hItem, IStream *pStream )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return FALSE;

	//DOUBLE fItemHeightFinal = 0.0;
	//hItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);
	//std::tr1::shared_ptr<Gdiplus::Image> pImg = GetResizedImage(pStream, fItemHeightFinal);
	std::tr1::shared_ptr<Gdiplus::Image> pImg = GetResizedImage(pStream, -1);		//Use original image
	if(pImg && pImg->GetWidth() == 0)
		pImg.reset();

	hItem->m_pImgSubImage = pImg;
	hItem->m_dwItemStyle &= (~STXTVIS_SUB_TEXT);

	if(GetSafeHwnd() && IsItemVisible(hItem))
		InvalidateRect(m_hwndControl, NULL, TRUE);

	return TRUE;
}

void CSTXAnimatedTreeCtrlNS::SetItemSubImage( HSTXTREENODE hItem, LPCTSTR lpszImageFile )
{
	if(lpszImageFile == NULL)
		return;

	IStream *pStream = NULL;
	SHCreateStreamOnFile(lpszImageFile, STGM_READ|STGM_SHARE_DENY_WRITE, &pStream);
	if(pStream)
	{
		SetItemSubImage(hItem, pStream);
		pStream->Release();
	}
}

void CSTXAnimatedTreeCtrlNS::SetItemImage(HSTXTREENODE hItem, LPCTSTR lpszImageFile, BOOL bResizeImage)
{
	if(lpszImageFile == NULL)
		return;

	IStream *pStream = NULL;
	SHCreateStreamOnFile(lpszImageFile, STGM_READ|STGM_SHARE_DENY_WRITE, &pStream);
	if(pStream)
	{
		SetItemImage(hItem, pStream, bResizeImage);
		pStream->Release();
	}
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_DeleteItem(HSTXTREENODE hItem)
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return FALSE;

	STXTREENODELIST *pListToDeleteFrom = &m_arrRootNodes;
	std::queue<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > *pDeleteQueue = &m_queDeletedNodes;

	HSTXTREENODE originalSelectedNode = m_hSelectedNode;

	if(hItem == STXTVI_ROOT)
	{
		//Trigger PreDelete Event
		std::vector<HSTXTREENODE> arrItemsToBeDelete;
		GetDescendantItems(hItem, &arrItemsToBeDelete);
		std::vector<HSTXTREENODE>::iterator itPreDel = arrItemsToBeDelete.begin();
		for(;itPreDel!=arrItemsToBeDelete.end();itPreDel++)
		{
			(*itPreDel)->TriggerItemPreDeleteEvent();
		}

		CComPtr<IUIAnimationStoryboard> pStory;
		m_AnimationManager->CreateStoryboard(&pStory);
		STXTREENODELIST::iterator it = m_arrRootNodes.begin();
		for(;it!=m_arrRootNodes.end();it++)
		{
			std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> spItemToDelete = (*it);

			//Opacity
			ApplySmoothStopTransition(spItemToDelete->m_pAVOpacity, m_nDefaultAnimationDuration / 2, 0.0, pStory);

			std::vector<HSTXTREENODE> arrNodes;
			GetAllVisibleItem(spItemToDelete.get(), &arrNodes);
			std::vector<HSTXTREENODE>::iterator itVisible = arrNodes.begin();
			for(;itVisible!=arrNodes.end();itVisible++)
			{
				ApplySmoothStopTransition((*itVisible)->m_pAVLeftOffset, m_nDefaultAnimationDuration / 2, (*itVisible)->m_nLeftOffsetFix + STXATC_ITEM_DELETE_ANIMATE_OFFSET, pStory);
			}


// 			CComPtr<IUIAnimationTransition> pTrans;
// 			m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 2, 0.0, &pTrans);
// 			pStory->AddTransition(spItemToDelete->m_pAVOpacity, pTrans);

			//Left Offset
// 			CComPtr<IUIAnimationTransition> pTransLeftOffset;
// 			m_AnimationTransitionLibrary->CreateSmoothStopTransition(m_nDefaultAnimationDuration / 2, spItemToDelete->m_nLeftOffsetFix + STXATC_ITEM_DELETE_ANIMATE_OFFSET, &pTransLeftOffset);
// 			pStory->AddTransition(spItemToDelete->m_pAVLeftOffset, pTransLeftOffset);

			if(spItemToDelete->m_bSelected && m_hSelectedNode)
			{
				m_hSelectedNode = NULL;
				//No need to set spItemToDelete->m_bSelected to FALSE.
			}
			m_queDeletedNodes.push(spItemToDelete);

		}
		m_arrRootNodes.clear();
		pStory->Schedule(GetCurrentTime(), NULL);
		ResetScrollBars();
		m_hSelectedNode = NULL;

		if(GetSafeHwnd() && !IsAnimationBusy())
			InvalidateRect(m_hwndControl, NULL, TRUE);
	}
	else
	{
		//Check Validation
		if(!IsValidItem(hItem))
			return FALSE;

		//Trigger PreDelete Event
		std::vector<HSTXTREENODE> arrItemsToBeDelete;
		GetDescendantItems(hItem, &arrItemsToBeDelete);
		std::vector<HSTXTREENODE>::iterator itPreDel = arrItemsToBeDelete.begin();
		for(;itPreDel!=arrItemsToBeDelete.end();itPreDel++)
		{
			(*itPreDel)->TriggerItemPreDeleteEvent();
		}
		hItem->TriggerItemPreDeleteEvent();

		DOUBLE fItemHeight = hItem->GetCurrentHeight();
		if(hItem->m_pParentNode)
		{
			pListToDeleteFrom = &hItem->m_pParentNode->m_arrChildNodes;
			pDeleteQueue = &hItem->m_pParentNode->m_queDeletedNodes;
		}

		ApplyOffsetAfter(pListToDeleteFrom, hItem, -fItemHeight, hItem, TRUE);

		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> spItemToDelete = (*hItem->m_itPositionInParent);

		CComPtr<IUIAnimationStoryboard> pStory;
		m_AnimationManager->CreateStoryboard(&pStory);

		ApplySmoothStopTransition(spItemToDelete->m_pAVOpacity, m_nDefaultAnimationDuration / 2, 0.0, pStory);

		std::vector<HSTXTREENODE> arrNodes;
		GetAllVisibleItem(spItemToDelete.get(), &arrNodes);
		std::vector<HSTXTREENODE>::iterator itVisible = arrNodes.begin();
		for(;itVisible!=arrNodes.end();itVisible++)
		{
			ApplySmoothStopTransition((*itVisible)->m_pAVLeftOffset, m_nDefaultAnimationDuration / 2, (*itVisible)->m_nLeftOffsetFix + STXATC_ITEM_DELETE_ANIMATE_OFFSET, pStory);
		}

		pListToDeleteFrom->erase(hItem->m_itPositionInParent);

		if(spItemToDelete->m_bSelected && m_hSelectedNode)
		{
			m_hSelectedNode = NULL;
			//No need to set spItemToDelete->m_bSelected to FALSE.
		}
		pDeleteQueue->push(spItemToDelete);

		pStory->Schedule(GetCurrentTime(), NULL);

		ResetScrollBars();
		InvalidateRect(m_hwndControl, NULL, TRUE);

	}

	if(originalSelectedNode != m_hSelectedNode)
		SendCommonNotifyMessage(STXATVN_SELECTEDITEMCHANGED, m_hSelectedNode, 0);

	return TRUE;
}

void CSTXAnimatedTreeCtrlNS::ApplySmoothStopTransition(IUIAnimationVariable *pVar, UI_ANIMATION_SECONDS duration, DOUBLE fTargetValue, IUIAnimationStoryboard *pStoryboard, BOOL bResetVelocity)
{
	CComPtr<IUIAnimationStoryboard> pStory;
	if(pStoryboard)
		pStory = pStoryboard;
	else
		m_AnimationManager->CreateStoryboard(&pStory);

	CComPtr<IUIAnimationTransition> pTrans;
	m_AnimationTransitionLibrary->CreateSmoothStopTransition(duration, fTargetValue, &pTrans);
	if(bResetVelocity)
		pTrans->SetInitialVelocity(0.0);
	pStory->AddTransition(pVar, pTrans);

	if(pStoryboard == NULL)
	{
		pStory->Schedule(GetCurrentTime(), NULL);
	}
}

void CSTXAnimatedTreeCtrlNS::CheckDrag(int x, int y)
{
	if(!m_bAllowDragDrop)
		return;

	if(m_bDragging)
	{
		UINT nHitFlags = 0;
		POINT pt = {x, y};
		HSTXTREENODE hNode = Internal_NodeHitTest(pt, &nHitFlags);
		BOOL bInBottomSpace = IsMouseInButtomSpace(x, y);
		if(hNode)
		{
			int iNewHighLightValue = 1;
			if(nHitFlags != STXATHT_ONITEMBOTTOM)
				iNewHighLightValue = 1;		//Before
			else
				iNewHighLightValue = 2;		//After

			if(m_hDragOverNode == NULL || hNode->m_iDropHighlight != iNewHighLightValue)
			{
				if(m_hDragOverNode)
					m_hDragOverNode->m_iDropHighlight = 0;

				m_hDragOverNode = hNode;

				hNode->m_iDropHighlight = iNewHighLightValue;
				InvalidateRect(m_hwndControl, NULL, TRUE);
				m_hDragOverNode = hNode;
			}
		}
		else
		{
			//Check whether the mouse is at the bottom
			if(bInBottomSpace)
			{
				std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pLastRootNode = m_arrRootNodes.back();
				HSTXTREENODE hNodeBottomNode = GetFurthestVisibleItem(pLastRootNode.get());

				if(m_hDragOverNode && m_hDragOverNode->m_iDropHighlight != 2)
				{
					hNodeBottomNode->m_iDropHighlight = 2;	//After
					m_hDragOverNode = hNodeBottomNode;
					InvalidateRect(m_hwndControl, NULL, TRUE);
				}
			}
			else if(m_hDragOverNode)
			{
				m_hDragOverNode->m_iDropHighlight = 0;
				InvalidateRect(m_hwndControl, NULL, TRUE);
				m_hDragOverNode = NULL;
			}
		}
	}
	else
	{
		if(!m_bLButtonDown)
			return;

		if(abs(m_ptLButtonDown.x - x) > 3 || abs(m_ptLButtonDown.y - y) > 3)
		{
			OnBeginDrag();
		}
	}
}

void CSTXAnimatedTreeCtrlNS::OnBeginDrag()
{
	m_bDragging = TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::IsMouseInButtomSpace(int x, int y)
{
	if(m_arrRootNodes.size() == 0)
		return TRUE;

	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);
	y += iVScrollPos;

	if(y > 20000)		// < 0
		return FALSE;

	std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pLastRootNode = m_arrRootNodes.back();
	DOUBLE fLastNodeTop = 0;
	pLastRootNode->m_pAVTopOffset->GetValue(&fLastNodeTop);

	if(y > fLastNodeTop + pLastRootNode->GetCurrentHeight() - 6)
		return TRUE;

	return FALSE;
}

void CSTXAnimatedTreeCtrlNS::OnDrop(HSTXTREENODE hNodeDropTo)
{
	//std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pItemToMove = *m_hSelectedNode->m_itPositionInParent;

	//if(hNodeDropTo->m_iDropHighlight == 1)
	//{
	//	Internal_DeleteItem(m_hSelectedNode);
	//	
	//	Internal_InsertItem(pItemToMove, hNodeDropTo->m_pParentNode ? hNodeDropTo->m_pParentNode : STXTVI_ROOT, m_hDragOverNode);
	//}
	//else if(hNodeDropTo->m_iDropHighlight == 2)
	//{
	//	HSTXTREENODE hNodeNext = Internal_GetNextItem(m_hDragOverNode, STXATVGN_NEXT);
	//	if(hNodeNext)
	//		Internal_InsertItem(_T("Drop"), hNodeDropTo->m_pParentNode ? hNodeDropTo->m_pParentNode : STXTVI_ROOT, hNodeNext);
	//	else
	//		Internal_InsertItem(_T("Drop"), hNodeDropTo->m_pParentNode ? hNodeDropTo->m_pParentNode : STXTVI_ROOT, STXTVI_LAST);
	//}
}

int CSTXAnimatedTreeCtrlNS::GetAllVisibleItem( HSTXTREENODE hSearchRoot /*= STXTVI_ROOT*/, std::vector<HSTXTREENODE> *pArrItems )
{
	if(pArrItems == NULL)
		return 0;

	if(hSearchRoot == STXTVI_ROOT)
	{
		STXTREENODELIST::iterator it = m_arrRootNodes.begin();
		for(;it!=m_arrRootNodes.end();it++)
		{
			std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> spItem = (*it);
			GetAllVisibleItem(spItem.get(), pArrItems);
		}
	}
	else
	{
		pArrItems->push_back(hSearchRoot);
		if(IsItemVisible(hSearchRoot))
		{
			STXTREENODELIST::iterator it = hSearchRoot->m_arrChildNodes.begin();
			for(;it!=hSearchRoot->m_arrChildNodes.end();it++)
			{
				std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> spItem = (*it);
				GetAllVisibleItem(spItem.get(), pArrItems);
			}
		}
	}

	return (int)pArrItems->size();
}

int CSTXAnimatedTreeCtrlNS::GetAllItem( HSTXTREENODE hSearchRoot /*= STXTVI_ROOT*/, std::vector<HSTXTREENODE> *pArrItems )
{
	if(pArrItems == NULL)
		return 0;

	if(hSearchRoot == STXTVI_ROOT)
	{
		STXTREENODELIST::iterator it = m_arrRootNodes.begin();
		for(;it!=m_arrRootNodes.end();it++)
		{
			std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> spItem = (*it);
			GetAllItem(spItem.get(), pArrItems);
		}
	}
	else
	{
		pArrItems->push_back(hSearchRoot);
		STXTREENODELIST::iterator it = hSearchRoot->m_arrChildNodes.begin();
		for(;it!=hSearchRoot->m_arrChildNodes.end();it++)
		{
			std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> spItem = (*it);
			GetAllItem(spItem.get(), pArrItems);
		}
	}

	return (int)pArrItems->size();
}

int CSTXAnimatedTreeCtrlNS::GetItemCount( HSTXTREENODE hSearchRoot /*= STXTVI_ROOT*/)
{
	int nCount = 0;
	if(hSearchRoot == STXTVI_ROOT)
	{
		STXTREENODELIST::iterator it = m_arrRootNodes.begin();
		for(;it!=m_arrRootNodes.end();it++)
		{
			std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> spItem = (*it);
			nCount++;
			nCount += GetItemCount(spItem.get());
		}
	}
	else
	{
		STXTREENODELIST::iterator it = hSearchRoot->m_arrChildNodes.begin();
		for(;it!=hSearchRoot->m_arrChildNodes.end();it++)
		{
			std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> spItem = (*it);
			nCount++;
			nCount += GetItemCount(spItem.get());
		}
	}

	return nCount;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SetItemHeight( HSTXTREENODE hItem, int iHeight )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return FALSE;

	DOUBLE fItemHeightFinal = 0.0;
	hItem->m_pAVItemHeight->GetFinalValue(&fItemHeightFinal);

	ApplySmoothStopTransition(hItem->m_pAVItemHeight, m_nDefaultAnimationDuration / 2, iHeight);
	DOUBLE fDelta = iHeight - fItemHeightFinal;

	if(hItem->m_pImgImage)
		ApplySmoothStopTransition(hItem->m_pAVImageOccupy, m_nDefaultAnimationDuration / 2, iHeight);

	if(hItem->m_pParentNode)
	{
		ApplyOffsetAfter(&hItem->m_pParentNode->m_arrChildNodes, hItem, fDelta, hItem, TRUE);
	}
	else
	{
		ApplyOffsetAfter(&m_arrRootNodes, hItem, fDelta, hItem, TRUE);
	}

	ResetScrollBars();

	return TRUE;
}

bool CSTXAnimatedTreeCtrlNS::STXAnimatedTreeDefaultSortFuncWrapper(std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pNode1, std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pNode2)
{
	if(s_sortStructure.nSortType == 1)
		return s_sortStructure.pfnSort(pNode1->m_dwNodeData, pNode2->m_dwNodeData, s_sortStructure.lParamSort) < 0;
	else
		return s_sortStructure.pfnSortItem(pNode1.get(), pNode2.get(), s_sortStructure.lParamSort) < 0;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SortChildren( HSTXTREENODE hItem, std::function<int(LPARAM, LPARAM, LPARAM)> pfnSortFunc, LPARAM lParamSort )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return FALSE;

	STXTREENODELIST *pListToAddIn = &m_arrRootNodes;
	if(hItem != STXTVI_ROOT)
		pListToAddIn = &hItem->m_arrChildNodes;

	STXTREENODELIST::iterator it = pListToAddIn->begin();
	INT_PTR iIndexBase = 0;
	for(;it!=pListToAddIn->end();it++)
	{
		(*it)->m_iIndexBeforeSort = iIndexBase++;
	}

	s_sortStructure.nSortType = 1;	//lParam(ItemData)
	s_sortStructure.lParamSort = lParamSort;
	s_sortStructure.pfnSort = pfnSortFunc;
	pListToAddIn->sort(STXAnimatedTreeDefaultSortFuncWrapper);
	RecalculateItemOffset(pListToAddIn);

	return TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SortChildrenByItem( HSTXTREENODE hItem, std::function<int(HSTXTREENODE, HSTXTREENODE, LPARAM)> pfnSortFunc, LPARAM lParamSort )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST)
		return FALSE;

	STXTREENODELIST *pListToAddIn = &m_arrRootNodes;
	if(hItem != STXTVI_ROOT)
		pListToAddIn = &hItem->m_arrChildNodes;

	STXTREENODELIST::iterator it = pListToAddIn->begin();
	INT_PTR iIndexBase = 0;
	for(;it!=pListToAddIn->end();it++)
	{
		(*it)->m_iIndexBeforeSort = iIndexBase++;
	}

	s_sortStructure.nSortType = 2;	//Item
	s_sortStructure.lParamSort = lParamSort;
	s_sortStructure.pfnSortItem = pfnSortFunc;
	pListToAddIn->sort(STXAnimatedTreeDefaultSortFuncWrapper);

	RecalculateItemOffset(pListToAddIn);

	return TRUE;
}

int CSTXAnimatedTreeCtrlNS::Internal_GetItemText( HSTXTREENODE hItem, LPTSTR pszTextBuffer, int cchBufferLen )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_SORT)
		return FALSE;

	if(pszTextBuffer == NULL)
		return hItem->m_strText.size();

	_tcscpy_s(pszTextBuffer, cchBufferLen, hItem->m_strText.c_str());
	return min(cchBufferLen, (int)hItem->m_strText.size());
}

DWORD_PTR CSTXAnimatedTreeCtrlNS::Internal_GetItemData( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_SORT)
		return 0;

	return hItem->m_dwNodeData;
}

void CSTXAnimatedTreeCtrlNS::RecalculateItemOffset( std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> > * pList )
{
	STXTREENODELIST::iterator it = pList->begin();
	INT_PTR iIndexBase = 0;
	int iOffset = 0;
	CComPtr<IUIAnimationStoryboard> pStory;
	m_AnimationManager->CreateStoryboard(&pStory);

	for(;it!=pList->end();it++)
	{
		std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> pItem = (*it);
		ApplySmoothStopTransition(pItem->m_pAVTopOffset, m_nDefaultAnimationDuration / 2.0, iOffset, pStory);
		iOffset += static_cast<int>(pItem->GetFinalHeight());
	}
	pStory->Schedule(GetCurrentTime(), NULL);
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_FindItem( HSTXTREENODE hItem, STXAnimatedTreeSearchCompareFuncType pfnSearchFunc, LPARAM lParamSearch )
{
	HSTXTREENODE hCurrent = Internal_GetNextItem(hItem, STXATVGN_CHILD);
	if(pfnSearchFunc == NULL)
		return hCurrent;

	while (hCurrent != NULL) 
	{
		DWORD_PTR dwItemData = Internal_GetItemData(hCurrent);
		if(pfnSearchFunc(dwItemData, lParamSearch) == 0)
			return hCurrent;

		if(/*ItemHasChildren(hCurrent)*/ hCurrent->m_arrChildNodes.size() > 0)
		{
			HSTXTREENODE hChildrenFound = Internal_FindItem(hCurrent, pfnSearchFunc, lParamSearch);
			if(hChildrenFound)
				return hChildrenFound;
		}

		// Try to get the next item
		hCurrent = Internal_GetNextSiblingItem(hCurrent);
	}
	return NULL;
}

int CSTXAnimatedTreeCtrlNS::Internal_FindItems( HSTXTREENODE hItem, STXAnimatedTreeSearchCompareFuncType pfnSearchFunc, LPARAM lParamSearch, HSTXTREENODE *pOutBuffer, int nBufferSize )
{
	int nCountFound = 0;
	HSTXTREENODE hCurrent = Internal_GetNextItem(hItem, STXATVGN_CHILD);

	while (hCurrent != NULL && (pOutBuffer == NULL || nBufferSize > 0)) 
	{
		DWORD_PTR dwItemData = Internal_GetItemData(hCurrent);
		if(pfnSearchFunc == NULL || pfnSearchFunc(dwItemData, lParamSearch) == 0)
		{
			if(pOutBuffer)
			{
				*pOutBuffer = hCurrent;
				pOutBuffer++;
			}
			nBufferSize--;
			nCountFound++;
		}

		if(/*ItemHasChildren(hCurrent)*/ hCurrent->m_arrChildNodes.size() > 0)
		{
			int nFoundInChildren = Internal_FindItems(hCurrent, pfnSearchFunc, lParamSearch, pOutBuffer, nBufferSize);
			nCountFound += nFoundInChildren;
			nBufferSize -= nFoundInChildren;

			if(pOutBuffer)
			{
				pOutBuffer += nFoundInChildren;
			}
		}

		// Try to get the next item
		hCurrent = Internal_GetNextSiblingItem(hCurrent);
	}
	return nCountFound;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SetItemData( HSTXTREENODE hItem, DWORD_PTR dwItemData )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_SORT)
		return FALSE;

	hItem->m_dwNodeData = dwItemData;
	return TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_ItemHasChildren( HSTXTREENODE hItem )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_SORT)
		return FALSE;

	if(hItem == STXTVI_ROOT)
		return m_arrRootNodes.size() > 0;

	return hItem->m_arrChildNodes.size() > 0;
}

void CSTXAnimatedTreeCtrlNS::OnInternalItemPreDelete( CSTXAnimatedTreeNodeNS* pItem )
{
	SendCommonNotifyMessage(STXATVN_PREDELETEITEM, pItem, 0);

	m_setValidItemPtr.erase(pItem);
}

void CSTXAnimatedTreeCtrlNS::OnInternalItemPostDelete( CSTXAnimatedTreeNodeNS* pItem )
{
	SendCommonNotifyMessage(STXATVN_POSTDELETEITEM, pItem, 0);

	if(pItem->m_bInitialized)
		m_mapAccIdToItem.erase(pItem->m_nAccID);

	//m_setValidItemPtr.erase(pItem);
}

int CSTXAnimatedTreeCtrlNS::GetDescendantItems( HSTXTREENODE hItem, std::vector<HSTXTREENODE> *pArrItems )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_SORT)
		return 0;

	STXTREENODELIST *pListToAddIn = &m_arrRootNodes;
	if(hItem != STXTVI_ROOT)
		pListToAddIn = &hItem->m_arrChildNodes;

	if(pListToAddIn->size() == 0)
		return 0;

	std::list<std::tr1::shared_ptr<CSTXAnimatedTreeNodeNS> >::iterator it = pListToAddIn->begin();
	for(;it!=pListToAddIn->end();it++)
	{
		pArrItems->push_back((*it).get());
		GetDescendantItems((*it).get(), pArrItems);
	}
	return pArrItems->size();
}

BOOL CSTXAnimatedTreeCtrlNS::IsValidItem( HSTXTREENODE hItem )
{
	//Check Validation
	if(m_setValidItemPtr.find(hItem) == m_setValidItemPtr.end())
		return FALSE;

	return TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SetBackgroundImage( IStream *pImgStream )
{
	m_pImgBackgroundCached = std::tr1::shared_ptr<Gdiplus::CachedBitmap>(); //Clear cache;
	std::tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(pImgStream));
	m_pImgBackground = img;
	if(GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);

	return TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SetWatermarkImage( IStream *pImgStream )
{
	m_pImgWatermarkCached = std::tr1::shared_ptr<Gdiplus::CachedBitmap>(); //Clear cache;
	std::tr1::shared_ptr<Gdiplus::Image> img(new Gdiplus::Image(pImgStream));
	m_pImgWatermark = img;
	if(GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);

	return TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::IsAnimationBusy()
{
	UI_ANIMATION_MANAGER_STATUS st;
	m_AnimationManager->GetStatus(&st);
	return st == UI_ANIMATION_MANAGER_BUSY;
}

DWORD CSTXAnimatedTreeCtrlNS::GetStyle()
{
	return (DWORD)GetWindowLong(m_hwndControl, GWL_STYLE);
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SetExpanderImage( IStream *pImgStreamExpanded, IStream *pImgStreamCollapsed )
{
	std::tr1::shared_ptr<Gdiplus::Image> imgExpanded(new Gdiplus::Image(pImgStreamExpanded));
	m_pImgExpanderExpanded = imgExpanded;

	std::tr1::shared_ptr<Gdiplus::Image> imgCollapsed(new Gdiplus::Image(pImgStreamCollapsed));
	m_pImgExpanderCollapsed = imgCollapsed;

	if(GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);

	return TRUE;
}

void CSTXAnimatedTreeCtrlNS::Internal_SetDefaultSortFunction( LPSTXTVSORTFUNC lpSortFunc )
{
	m_sortFuncDefault.pfnSortFunc = lpSortFunc->pfnSortFunc;
	m_sortFuncDefault.lParamSort = lpSortFunc->lParamSort;
}

HWND CSTXAnimatedTreeCtrlNS::v_GetWindow()
{
	return GetSafeHwnd();
}

long CSTXAnimatedTreeCtrlNS::_GetChildCount()
{
	return (long)GetItemCount(STXTVI_ROOT);
}

STDMETHODIMP CSTXAnimatedTreeCtrlNS::accHitTest(long xLeft, long yTop, __out VARIANT *pvarChildAtPoint)
{
	HRESULT hr = S_FALSE;
	VariantInit(pvarChildAtPoint);
	POINT ptLocal = {xLeft, yTop};
	ScreenToClient(m_hwndControl, &ptLocal);
	HSTXTREENODE hNode = Internal_NodeHitTest(ptLocal, NULL);
	if(hNode)
	{
		hr = InitVariantFromInt32(hNode->m_nAccID, pvarChildAtPoint);
		return hr;
	}

	HWND hwnd = v_GetWindow();
	if (IsWindow(hwnd))
	{
		RECT rc;
		GetWindowRect(hwnd, &rc);
		POINT pt = {xLeft, yTop};
		if (PtInRect(&rc, pt))
		{
			hr = InitVariantFromInt32(CHILDID_SELF, pvarChildAtPoint);
		}
	}
	return hr;
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::accLocation( __out long *pxLeft, __out long *pyTop, __out long *pcxWidth, __out long *pcyHeight, VARIANT varChild )
{
	if(varChild.lVal == CHILDID_SELF)
		return __super::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);

	std::map<long, HSTXTREENODE>::iterator it = m_mapAccIdToItem.find(varChild.lVal);
	if(it == m_mapAccIdToItem.end())
		return S_FALSE;

	RECT rcItem;
	it->second->GetItemScreenRect(&rcItem);

	*pxLeft = rcItem.left;
	*pyTop = rcItem.top;
	*pcxWidth = rcItem.right - rcItem.left;
	*pcyHeight = rcItem.bottom - rcItem.top;

	return S_OK;
}

// IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::get_accChildCount( __out long* pChildCount )
// {
// 	*pChildCount = (long)GetItemCount(STXTVI_ROOT);
// 	return S_OK;
// }

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::Reset( void )
{
	m_arrEnumItems.clear();
	GetAllItem(STXTVI_ROOT, &m_arrEnumItems);
	_lChildCur = 0;
	return S_OK;
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::Next( ULONG celt, __out_ecount_part(celt, *pceltFetched) VARIANT* rgvar, __out_opt ULONG * pceltFetched )
{
	VARIANT *pvar = rgvar;
	long cChildren = GetItemCount(STXTVI_ROOT);
	long cFetched = 0;
	long lCur = _lChildCur;

	while ((cFetched < (long)celt) && (lCur < cChildren))
	{

		// Note this gives us (index)+1 because we incremented iCur
		InitVariantFromInt32(m_arrEnumItems[lCur]->m_nAccID, pvar);
		++pvar;

		cFetched++;
		lCur++;
	}

	_lChildCur = lCur;

	if (pceltFetched)
	{
		*pceltFetched = cFetched;
	}

	return (cFetched < (long)celt) ? S_FALSE : S_OK;
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::get_accName( VARIANT varChild, __out BSTR *pbstrName )
{
	if(varChild.lVal == CHILDID_SELF)
	{
		*pbstrName = SysAllocString(L"STXTreeControl");
		return S_OK;
	}

	std::map<long, HSTXTREENODE>::iterator it = m_mapAccIdToItem.find(varChild.lVal);
	if(it == m_mapAccIdToItem.end())
		return S_FALSE;

	*pbstrName = SysAllocString(it->second->m_strText.c_str());
	return S_OK;
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::get_accDefaultAction( VARIANT varChild, __out BSTR* pszDefaultAction )
{
	if(varChild.lVal == CHILDID_SELF)
	{
		return DISP_E_MEMBERNOTFOUND;
	}
	std::map<long, HSTXTREENODE>::iterator it = m_mapAccIdToItem.find(varChild.lVal);
	if(it == m_mapAccIdToItem.end())
		return DISP_E_MEMBERNOTFOUND;

	if(it->second->m_bExpanded)
		*pszDefaultAction = SysAllocString(L"Collapse");
	else
		*pszDefaultAction = SysAllocString(L"Expand");

	return S_OK;
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::accDoDefaultAction( VARIANT varChild )
{
	if(varChild.lVal == CHILDID_SELF)
	{
		return DISP_E_MEMBERNOTFOUND;
	}
	std::map<long, HSTXTREENODE>::iterator it = m_mapAccIdToItem.find(varChild.lVal);
	if(it == m_mapAccIdToItem.end())
		return DISP_E_MEMBERNOTFOUND;

	Internal_Expand(it->second, STXATVE_TOGGLE);

	return S_OK;
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::accNavigate( long navDir, VARIANT varStart, __out VARIANT * pvarEndUpAt )
{
	switch(navDir)
	{
	case NAVDIR_DOWN:
		{
			SelectNextVisibleItem();
			HSTXTREENODE hNode = GetSelectedItem();
			InitVariantFromInt32(hNode->m_nAccID, pvarEndUpAt);
			break;
		}
	case NAVDIR_UP:
		{
			SelectPrevVisibleItem();
			HSTXTREENODE hNode = GetSelectedItem();
			InitVariantFromInt32(hNode->m_nAccID, pvarEndUpAt);
			break;
		}
	case NAVDIR_LEFT:
		{
			HSTXTREENODE hNode = GetSelectedItem();
			if(hNode == NULL)
				return DISP_E_MEMBERNOTFOUND;

			OnKeyDown_Left();
			hNode = GetSelectedItem();
			InitVariantFromInt32(hNode->m_nAccID, pvarEndUpAt);
			break;
		}
	case NAVDIR_RIGHT:
		{
			HSTXTREENODE hNode = GetSelectedItem();
			if(hNode == NULL)
				return DISP_E_MEMBERNOTFOUND;

			OnKeyDown_Right();
			hNode = GetSelectedItem();
			InitVariantFromInt32(hNode->m_nAccID, pvarEndUpAt);
			break;
		}
	}

	return S_OK;
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::get_accSelection( __out VARIANT * pvarSelectedChildren )
{
	HSTXTREENODE hNode = GetSelectedItem();
	if(hNode == NULL)
		return DISP_E_MEMBERNOTFOUND;

	return InitVariantFromInt32(hNode->m_nAccID, pvarSelectedChildren);
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::get_accState( VARIANT varChild, __out VARIANT *pvarState )
{
	if(varChild.lVal == CHILDID_SELF)
	{
		return DISP_E_MEMBERNOTFOUND;
	}
	std::map<long, HSTXTREENODE>::iterator it = m_mapAccIdToItem.find(varChild.lVal);
	if(it == m_mapAccIdToItem.end())
		return DISP_E_MEMBERNOTFOUND;

	long nState = STATE_SYSTEM_SELECTABLE;

	if(!IsItemVisible(it->second))
		nState |= STATE_SYSTEM_INVISIBLE;

	if(m_hSelectedNode == it->second)
		nState |= STATE_SYSTEM_SELECTED;

	if(m_hLastHoverNode == it->second)
		nState |= STATE_SYSTEM_HOTTRACKED;

	return InitVariantFromInt32(nState, pvarState);
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::accSelect( long flagsSel, VARIANT varChild )
{
	if(varChild.lVal == CHILDID_SELF)
	{
		SetFocus(m_hwndControl);
		return S_OK;
	}
	std::map<long, HSTXTREENODE>::iterator it = m_mapAccIdToItem.find(varChild.lVal);
	if(it == m_mapAccIdToItem.end())
		return DISP_E_MEMBERNOTFOUND;

	SelectNode(it->second);
	return S_OK;
}

IFACEMETHODIMP CSTXAnimatedTreeCtrlNS::get_accRole( VARIANT varChild, __out VARIANT *pvarRole )
{
	if(varChild.lVal == CHILDID_SELF)
	{
		return DISP_E_MEMBERNOTFOUND;
	}
	std::map<long, HSTXTREENODE>::iterator it = m_mapAccIdToItem.find(varChild.lVal);
	if(it == m_mapAccIdToItem.end())
		return DISP_E_MEMBERNOTFOUND;

	return InitVariantFromInt32(ROLE_SYSTEM_OUTLINEITEM, pvarRole);
}

void CSTXAnimatedTreeCtrlNS::Internal_ModifyItemStyle( HSTXTREENODE hItem, DWORD dwAdd, DWORD dwRemove )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_ROOT || hItem == STXTVI_SORT)
		return;

	hItem->m_dwItemStyle |= dwAdd;
	hItem->m_dwItemStyle &= (~dwRemove);

	if(GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);
}

void CSTXAnimatedTreeCtrlNS::Internal_SetItemDrawFunction( STXAnimatedTreeItemDrawFuncType lpfnItemDrawFunc )
{
	m_pfnItemDrawFunc = lpfnItemDrawFunc;
	if(GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);
}

DWORD CSTXAnimatedTreeCtrlNS::Internal_GetItemState( HSTXTREENODE hItem )
{
	DWORD dwState = hItem->m_dwItemStyle;
	if(hItem->m_bSelected)
		dwState |= STXTVIS_SELECTED;
	if(hItem->m_bHover)
		dwState |= STXTVIS_HOVER;

	return dwState;
}

LRESULT CSTXAnimatedTreeCtrlNS::SendCommonNotifyMessage( UINT nNotifyCode, HSTXTREENODE hNode, DWORD_PTR dwNotifySpec )
{
	STXATVNITEM nm;
	nm.hdr.hwndFrom = GetSafeHwnd();
	nm.hdr.idFrom = GetDlgCtrlID(m_hwndControl);
	nm.hdr.code = nNotifyCode;

	if(hNode)
		nm.dwItemData = hNode->m_dwNodeData;
	else
		nm.dwItemData = 0;

	nm.dwNotifySpec = dwNotifySpec;
	nm.hNode = hNode;
	if(PreSendNotifyMessage(nNotifyCode, (LPNMHDR)&nm))
		return ::SendMessage(GetParent(m_hwndControl), WM_NOTIFY, GetDlgCtrlID(m_hwndControl), (LPARAM)&nm);

	return 0;
}

LRESULT CSTXAnimatedTreeCtrlNS::SendSimpleNotifyMessage(UINT nNotifyCode)
{
	NMHDR hdr;
	hdr.hwndFrom = GetSafeHwnd();
	hdr.idFrom = GetDlgCtrlID(m_hwndControl);
	hdr.code = nNotifyCode;
	if(PreSendNotifyMessage(nNotifyCode, &hdr))
		return ::SendMessage(GetParent(m_hwndControl), WM_NOTIFY, GetDlgCtrlID(m_hwndControl), (LPARAM)&hdr);

	return 0;
}

void CSTXAnimatedTreeCtrlNS::OnEndEdit()
{
	SendSimpleNotifyMessage(STXATVN_ENDEDIT);
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_GetItemRect( HSTXTREENODE hItem, LPRECT lprcItemRect )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_ROOT || hItem == STXTVI_SORT)
		return FALSE;

	if(lprcItemRect == NULL)
		return FALSE;

	hItem->GetItemScreenRect(lprcItemRect);
	ScreenToClient(m_hwndControl, (LPPOINT)lprcItemRect);
	ScreenToClient(m_hwndControl, ((LPPOINT)lprcItemRect) + 1);

	int iVScrollPos = GetScrollPos(m_hwndControl, SB_VERT);

	OffsetRect(lprcItemRect, 0, -iVScrollPos);
	return TRUE;
}

void CSTXAnimatedTreeCtrlNS::Internal_SetAnimationDuration( DWORD dwMilliseconds )
{
	m_nDefaultAnimationDuration = (dwMilliseconds / 1000.0);
}

void CSTXAnimatedTreeCtrlNS::Internal_SetBackgroundColor( COLORREF clrBackground )
{
	m_clrBackground = Gdiplus::Color(255, GetRValue(clrBackground), GetGValue(clrBackground), GetBValue(clrBackground));
	if(GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);
}

HSTXTREENODE CSTXAnimatedTreeCtrlNS::Internal_GetParentItem( HSTXTREENODE hItem )
{
	return Internal_GetNextItem(hItem, STXATVGN_PARENT);
}

BOOL CSTXAnimatedTreeCtrlNS::PreSendNotifyMessage( UINT nCode, LPNMHDR pNMHDR )
{
	return TRUE;
}

std::tr1::shared_ptr<Gdiplus::Image> CSTXAnimatedTreeCtrlNS::OnItemImageCallback(HSTXTREENODE hItem)
{
	return NULL;
}

void CSTXAnimatedTreeCtrlNS::OnInternalHoverItemChanged( HSTXTREENODE hItem )
{
	if(hItem)
	{
		SetTimer(m_hwndControl, STXTC_TIMER_ID_ITEM_FLOAT_CHECK, STXTC_TIMER_INTERVAL_ITEM_FLOAT_CHECK, NULL);
	}
	else
	{
		KillTimer(m_hwndControl, STXTC_TIMER_ID_ITEM_FLOAT_CHECK);
	}
}

void CSTXAnimatedTreeCtrlNS::OnItemFloatCheckTriggered()
{
	KillTimer(m_hwndControl, STXTC_TIMER_ID_ITEM_FLOAT_CHECK);
	if(m_hLastHoverNode)
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(m_hwndControl, &point);
		UINT nHitFlags = 0;
		HSTXTREENODE hNode = Internal_NodeHitTest(point, &nHitFlags);

		m_hLastFloatTriggeredNode = m_hLastHoverNode;
		SendCommonNotifyMessage(STXATVN_ITEMMOUSEFLOAT, m_hLastHoverNode, nHitFlags);
		OutputDebugString(_T("OnItemFloatCheckTriggered\n"));
	}
}

int CSTXAnimatedTreeCtrlNS::GetItemIndent()
{
	return m_nItemIndent;
}

BOOL CSTXAnimatedTreeCtrlNS::SetItemIndent( int nItemIndent )
{
	if(nItemIndent < 0)
		return FALSE;

	if(GetItemCount(STXTVI_ROOT))
		return FALSE;

	m_nItemIndent = nItemIndent;
	return TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SetItemText( HSTXTREENODE hItem, LPCTSTR pszText )
{
	if(hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_ROOT || hItem == STXTVI_SORT)
		return FALSE;

	hItem->m_strText = pszText;
	hItem->m_nBoundsMaxX = -1;

	ResetHorizontalScrollBar();

	if(GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);

	return TRUE;
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SetItemSubText(HSTXTREENODE hItem, LPCTSTR pszSubText)
{
	if (hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_ROOT || hItem == STXTVI_SORT)
		return FALSE;

	if (pszSubText)
	{
		hItem->m_strSubText = pszSubText;
		hItem->m_dwItemStyle |= STXTVIS_SUB_TEXT;
	}
	else
	{
		hItem->m_strSubText.clear();
		hItem->m_dwItemStyle &= (~STXTVIS_SUB_TEXT);
	}

	if (GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);

	return TRUE;
}

int CSTXAnimatedTreeCtrlNS::GetCurrentContentWidth()
{
	std::vector<HSTXTREENODE> items;
	GetAllVisibleItem(STXTVI_ROOT, &items);
	int nMax = 0;

	std::vector<HSTXTREENODE>::iterator it = items.begin();
	for (; it != items.end(); it++)
	{
		int nCurrentItemMaxX = GetItemMaxBoundsX(*it);
		if (nCurrentItemMaxX > nMax)
			nMax = nCurrentItemMaxX;
	}

	return nMax;
}

int CSTXAnimatedTreeCtrlNS::GetItemMaxBoundsX(HSTXTREENODE hItem)
{
	if (hItem->m_nBoundsMaxX >= 0)
		return hItem->m_nBoundsMaxX;


	HDC hDC = ::GetDC(m_hwndControl);

	Gdiplus::Graphics g(hDC);

	int nLeft = hItem->m_nLeftOffsetFix;

	DOUBLE fItemHeight = 0;
	hItem->m_pAVItemHeight->GetValue(&fItemHeight);

	Gdiplus::REAL rItemHeight = static_cast<Gdiplus::REAL>(fItemHeight);

	do
	{
		if (hItem->m_pImgImage == NULL && (hItem->m_dwItemStyle & STXTVIS_IMAGE_CALLBACK) == 0)
			break;

		std::tr1::shared_ptr<Gdiplus::Image> imgUse = hItem->m_pImgImage;
		if (hItem->m_dwItemStyle & STXTVIS_IMAGE_CALLBACK)
			imgUse = OnItemImageCallback(hItem);

		if (imgUse == NULL)
			break;

		nLeft += (int)rItemHeight;

	} while (FALSE);

	//Gdiplus::FontFamily  fontFamily(&m_lfDefaultFont);
	//Gdiplus::Font drawFont(&fontFamily, STXAT_FONT_SIZE, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	//Gdiplus::Font drawFont(hDC, &m_lfDefaultFont);

	Gdiplus::RectF rectTextMain(0, 0, 1000, rItemHeight);
	Gdiplus::StringFormat strFormat;

	strFormat.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
	//strFormat.SetTrimming(Gdiplus::StringTrimmingEllipsisWord);
	Gdiplus::RectF rectTextMeasured;
	g.MeasureString(hItem->m_strText.c_str(), -1, m_pDefaultFont, rectTextMain, &strFormat, &rectTextMeasured);

	nLeft += (int)rectTextMeasured.Width;

	::ReleaseDC(m_hwndControl, hDC);

	hItem->m_nBoundsMaxX = nLeft;
	
	return nLeft;
}

void CSTXAnimatedTreeCtrlNS::GetDefaultFontInfo()
{
	HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
	if (hFont != NULL)
	{
		::GetObject(hFont, sizeof(LOGFONT), &m_lfDefaultFont);
		::DeleteObject(hFont);
		HDC hDC = ::GetDC(m_hwndControl);
		m_pDefaultFont = new Gdiplus::Font(hDC, &m_lfDefaultFont);

		memcpy(&m_lfDefaultSubTextFont, &m_lfDefaultFont, sizeof(m_lfDefaultSubTextFont));
		//m_lfDefaultSubTextFont.lfWidth /= 2;
		m_lfDefaultSubTextFont.lfHeight = (LONG)(m_lfDefaultSubTextFont.lfHeight / 1.3);
		m_pDefaultSubTextFont = new Gdiplus::Font(hDC, &m_lfDefaultSubTextFont);
		::ReleaseDC(m_hwndControl, hDC);
	}
}

Gdiplus::Font * CSTXAnimatedTreeCtrlNS::GetDefaultFont()
{
	return m_pDefaultFont;
}

Gdiplus::Font * CSTXAnimatedTreeCtrlNS::GetDefaultSubTextFont()
{
	return m_pDefaultSubTextFont;
}

BOOL CSTXAnimatedTreeCtrlNS::SetWatermarkLocation(UINT nLocation)
{
	if (nLocation > 0x03)
		return FALSE;

	m_nWatermarkLocation = nLocation;

	if (GetSafeHwnd())
		InvalidateRect(m_hwndControl, NULL, TRUE);

	return TRUE;
}

void CSTXAnimatedTreeCtrlNS::SetWatermarkOpacity(double fOpacity)
{
	m_fWatermarkOpacity = fOpacity;

	if (GetSafeHwnd() && m_pImgWatermark)
		InvalidateRect(m_hwndControl, NULL, TRUE);
}

BOOL CSTXAnimatedTreeCtrlNS::Internal_SelectItem(HSTXTREENODE hItem)
{
	if (hItem == m_hSelectedNode)
		return FALSE;

	if (hItem == NULL || hItem == STXTVI_FIRST || hItem == STXTVI_LAST || hItem == STXTVI_ROOT || hItem == STXTVI_SORT)
		return FALSE;

	SelectNode(hItem);
	return TRUE;
}

