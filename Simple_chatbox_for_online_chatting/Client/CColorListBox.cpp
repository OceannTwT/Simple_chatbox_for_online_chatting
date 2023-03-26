#include "pch.h"
#include "CColorListBox.h"
CColorListBox::CColorListBox() 
{
}
CColorListBox::~CColorListBox()
{
}

void CColorListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// Losing focus ?
	if (lpDrawItemStruct->itemID == -1)
	{
		DrawFocusRect(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem);
		return;
	}

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	COLORREF clrOld;
	CString sText;

	// get color info from item data
	COLORREF clrNew = (COLORREF)(lpDrawItemStruct->itemData);

	// item selected ?
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) &&
		(lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		CBrush brush(::GetSysColor(COLOR_HIGHLIGHT));
		pDC->FillRect(&lpDrawItemStruct->rcItem, &brush);
	}

	// item deselected ?
	if (!(lpDrawItemStruct->itemState & ODS_SELECTED) &&
		(lpDrawItemStruct->itemAction & ODA_SELECT))
	{
		CBrush brush(::GetSysColor(COLOR_WINDOW));
		pDC->FillRect(&lpDrawItemStruct->rcItem, &brush);
	}

	// item has focus ?
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) &&
		(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		pDC->DrawFocusRect(&lpDrawItemStruct->rcItem);
	}

	// lost focus ?
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) &&
		!(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		pDC->DrawFocusRect(&lpDrawItemStruct->rcItem);
	}

	// set the background mode to TRANSPARENT
	int nBkMode = pDC->SetBkMode(TRANSPARENT);

	if (lpDrawItemStruct->itemState & ODS_SELECTED)
		clrOld = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	else
		if (lpDrawItemStruct->itemState & ODS_DISABLED)
			clrOld = pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
		else
			clrOld = pDC->SetTextColor(clrNew);

	// get item text
	GetText(lpDrawItemStruct->itemID, sText);
	CRect rect = lpDrawItemStruct->rcItem;

	// text format
	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	if (GetStyle() & LBS_USETABSTOPS)
		nFormat |= DT_EXPANDTABS;

	// draw the text
	pDC->DrawText(sText, -1, &rect, nFormat);

	// restore old values
	pDC->SetTextColor(clrOld);
	pDC->SetBkMode(nBkMode);
}

void CColorListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = ::GetSystemMetrics(SM_CYMENUCHECK);
}

//int CColorListBox::InsertString(int n, LPCTSTR lpszItem, COLORREF itemColor)
//{
//	int nIndex = CListBox::InsertString(n, lpszItem);
//
//	// save color data
//	if (nIndex >= 0)
//		SetItemData(nIndex, itemColor);
//
//	return nIndex;
//}

void CColorListBox::SetColour(int n, COLORREF itemColor)
{
	if (n >= 0)
		SetItemData(n, itemColor);

}

