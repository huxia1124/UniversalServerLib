#include "StdAfx.h"
#include <Shlwapi.h>

#include "STXAccessibleWrapperBase.h"

#ifdef __cplusplus
extern "C++" {
	template <typename T>
	void IUnknown_SafeReleaseAndNullPtr(T *& p)
	{
		if (p)
		{
			T *pTemp = p;
			p = NULL;
			pTemp->Release();
		}
	}
}
#endif  // __cplusplus

// shorthand
#ifndef ATOMICRELEASE
#ifdef __cplusplus
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=(p); (p)=NULL; punkT->Release();} }
#else
#define ATOMICRELEASET(p, type) { if(p) { type* punkT=(p); (p)=NULL; punkT->lpVtbl->Release(punkT);} }
#endif

// doing this as a function instead of inline seems to be a size win.
//
#ifdef NOATOMICRELESEFUNC
#define ATOMICRELEASE(p) ATOMICRELEASET(p, IUnknown)
#else
#   ifdef __cplusplus
#       define ATOMICRELEASE(p) IUnknown_SafeReleaseAndNullPtr(p)
#   else
#       define ATOMICRELEASE(p) IUnknown_AtomicRelease((LPVOID*)&p)
#   endif
#endif
#endif //ATOMICRELEASE

#define SAFECAST(_obj, _type) (((_type)(_obj)==(_obj)?0:0), (_type)(_obj))

#ifndef RECTWIDTH
#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#endif

#ifndef RECTHEIGHT
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)
#endif

inline HRESULT InitVariantFromInt32(__in LONG lVal, __out VARIANT *pvar)
{
	pvar->vt = VT_I4;
	pvar->lVal = lVal;
	return S_OK;
}

HRESULT InitVariantFromDispatch(__in_opt IDispatch* pdisp, __out VARIANT *pvar)
{
	pvar->vt = VT_DISPATCH;
	pvar->pdispVal = pdisp;
	if (pvar->pdispVal)
	{
		(pvar->pdispVal)->AddRef();
	}
	return S_OK;
}

//#include "stock.h"
//#include "varutil.h"
#pragma hdrstop

//#include "accessible.h"

#pragma comment(lib, "Oleacc.lib")

// !!! README FIRST !!!
//   Functions in this file do not depend on propsys.dll

CAccessibleWrapperBase::CAccessibleWrapperBase(__in IAccessible *pAcc)
: _cRef(1), _pAcc(pAcc), _pEnumVar(NULL), _pOleWin(NULL)
{
	_pAcc->AddRef();
}

CAccessibleWrapperBase::~CAccessibleWrapperBase()
{
	ATOMICRELEASE(_pEnumVar);
	ATOMICRELEASE(_pOleWin);
	_pAcc->Release();
}

// IUnknown
// Implement refcounting ourselves
// Also implement QI ourselves, so that we return a ptr back to the wrapper.
STDMETHODIMP CAccessibleWrapperBase::QueryInterface(REFIID riid, __out void** ppv)
{
	HRESULT hr;
	*ppv = NULL;

	if ((riid == IID_IUnknown)  ||
		(riid == IID_IDispatch) ||
		(riid == IID_IAccessible))
	{
		*ppv = SAFECAST(this, IAccessible*);
	}
	else if (riid == IID_IEnumVARIANT)
	{
		// Get the IEnumVariant from the object we are sub-classing so we can delegate
		// calls.
		if (!_pEnumVar)
		{
			hr = _pAcc->QueryInterface(IID_PPV_ARGS(&_pEnumVar));
			if (FAILED(hr))
			{
				_pEnumVar = NULL;
				return hr;
			}
			// Paranoia (in case QI returns S_OK with NULL...)
			if (!_pEnumVar)
				return E_NOINTERFACE;
		}

		*ppv = SAFECAST(this, IEnumVARIANT*);
	}
	else if (riid == IID_IOleWindow)
	{
		// Get the IOleWindow from the object we are sub-classing so we can delegate
		// calls.
		if (!_pOleWin)
		{
			hr = _pAcc->QueryInterface(IID_PPV_ARGS(&_pOleWin));
			if(FAILED(hr))
			{
				_pOleWin = NULL;
				return hr;
			}
			// Paranoia (in case QI returns S_OK with NULL...)
			if (!_pOleWin)
				return E_NOINTERFACE;
		}

		*ppv = SAFECAST(this, IOleWindow*);
	}
	else
		return E_NOINTERFACE;

	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CAccessibleWrapperBase::AddRef()
{
	return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CAccessibleWrapperBase::Release()
{
	ULONG cRef = InterlockedDecrement(&_cRef);
	if (!cRef)
		delete this;

	return cRef;
}

// IDispatch
// - pass all through _pAcc

STDMETHODIMP CAccessibleWrapperBase::GetTypeInfoCount(__out UINT* pctinfo)
{
	return _pAcc->GetTypeInfoCount(pctinfo);
}

STDMETHODIMP CAccessibleWrapperBase::GetTypeInfo(UINT itinfo, LCID lcid, __out ITypeInfo** pptinfo)
{
	return _pAcc->GetTypeInfo(itinfo, lcid, pptinfo);
}

STDMETHODIMP CAccessibleWrapperBase::GetIDsOfNames(REFIID riid, __in_ecount(cNames) OLECHAR** rgszNames, UINT cNames,
												   LCID lcid, __out_ecount(cNames) DISPID* rgdispid)
{
	return _pAcc->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

STDMETHODIMP CAccessibleWrapperBase::Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags,
											__inout DISPPARAMS* pdp, __out VARIANT* pvarResult,
											__out EXCEPINFO* pxi, __out UINT* puArgErr)
{
	return _pAcc->Invoke(dispid, riid, lcid, wFlags, pdp, pvarResult, pxi, puArgErr);
}

// IAccessible
// - pass all through _pAcc

STDMETHODIMP CAccessibleWrapperBase::get_accParent(__out IDispatch ** ppdispParent)
{
	return _pAcc->get_accParent(ppdispParent);
}

STDMETHODIMP CAccessibleWrapperBase::get_accChildCount(__out long* pChildCount)
{
	return _pAcc->get_accChildCount(pChildCount);
}

STDMETHODIMP CAccessibleWrapperBase::get_accChild(VARIANT varChild, __out IDispatch ** ppdispChild)
{
	return _pAcc->get_accChild(varChild, ppdispChild);
}

STDMETHODIMP CAccessibleWrapperBase::get_accName(VARIANT varChild, __out BSTR* pszName)
{
	return _pAcc->get_accName(varChild, pszName);
}

STDMETHODIMP CAccessibleWrapperBase::get_accValue(VARIANT varChild, __out BSTR* pszValue)
{
	return _pAcc->get_accValue(varChild, pszValue);
}

STDMETHODIMP CAccessibleWrapperBase::get_accDescription(VARIANT varChild, __out BSTR* pszDescription)
{
	return _pAcc->get_accDescription(varChild, pszDescription);
}

STDMETHODIMP CAccessibleWrapperBase::get_accRole(VARIANT varChild, __out VARIANT *pvarRole)
{
	return _pAcc->get_accRole(varChild, pvarRole);
}

STDMETHODIMP CAccessibleWrapperBase::get_accState(VARIANT varChild, __out VARIANT *pvarState)
{
	return _pAcc->get_accState(varChild, pvarState);
}

STDMETHODIMP CAccessibleWrapperBase::get_accHelp(VARIANT varChild, __out BSTR* pszHelp)
{
	return _pAcc->get_accHelp(varChild, pszHelp);
}

STDMETHODIMP CAccessibleWrapperBase::get_accHelpTopic(__out BSTR* pszHelpFile, VARIANT varChild, __out long* pidTopic)
{
	return _pAcc->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
}

STDMETHODIMP CAccessibleWrapperBase::get_accKeyboardShortcut(VARIANT varChild, __out BSTR* pszKeyboardShortcut)
{
	return _pAcc->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}

STDMETHODIMP CAccessibleWrapperBase::get_accFocus(__out VARIANT * pvarFocusChild)
{
	return _pAcc->get_accFocus(pvarFocusChild);
}

STDMETHODIMP CAccessibleWrapperBase::get_accSelection(__out VARIANT * pvarSelectedChildren)
{
	return _pAcc->get_accSelection(pvarSelectedChildren);
}

STDMETHODIMP CAccessibleWrapperBase::get_accDefaultAction(VARIANT varChild, __out BSTR* pszDefaultAction)
{
	return _pAcc->get_accDefaultAction(varChild, pszDefaultAction);
}

STDMETHODIMP CAccessibleWrapperBase::accSelect(long flagsSel, VARIANT varChild)
{
	return _pAcc->accSelect(flagsSel, varChild);
}

STDMETHODIMP CAccessibleWrapperBase::accLocation(__out long* pxLeft, __out long* pyTop, __out long* pcxWidth, __out long* pcyHeight, VARIANT varChild)
{
	return _pAcc->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
}

STDMETHODIMP CAccessibleWrapperBase::accNavigate(long navDir, VARIANT varStart, __out VARIANT * pvarEndUpAt)
{
	return _pAcc->accNavigate(navDir, varStart, pvarEndUpAt);
}

STDMETHODIMP CAccessibleWrapperBase::accHitTest(long xLeft, long yTop, __out VARIANT * pvarChildAtPoint)
{
	return _pAcc->accHitTest(xLeft, yTop, pvarChildAtPoint);
}

STDMETHODIMP CAccessibleWrapperBase::accDoDefaultAction(VARIANT varChild)
{
	return _pAcc->accDoDefaultAction(varChild);
}

STDMETHODIMP CAccessibleWrapperBase::put_accName(VARIANT varChild, __in BSTR szName)
{
	return _pAcc->put_accName(varChild, szName);
}

STDMETHODIMP CAccessibleWrapperBase::put_accValue(VARIANT varChild, __in BSTR pszValue)
{
	return _pAcc->put_accValue(varChild, pszValue);
}

// IEnumVARIANT
// - pass all through _pEnumVar

STDMETHODIMP CAccessibleWrapperBase::Next(ULONG celt, __out_ecount_part(celt, *pceltFetched) VARIANT* rgvar, __out_opt ULONG * pceltFetched)
{
	return _pEnumVar->Next(celt, rgvar, pceltFetched);
}

STDMETHODIMP CAccessibleWrapperBase::Skip(ULONG celt)
{
	return _pEnumVar->Skip(celt);
}

STDMETHODIMP CAccessibleWrapperBase::Reset()
{
	return _pEnumVar->Reset();
}

STDMETHODIMP CAccessibleWrapperBase::Clone(__out IEnumVARIANT ** ppenum)
{
	return _pEnumVar->Clone(ppenum);
}

// IOleWindow
// - pass all through _pOleWin

STDMETHODIMP CAccessibleWrapperBase::GetWindow(__out HWND* phwnd)
{
	return _pOleWin->GetWindow(phwnd);
}

STDMETHODIMP CAccessibleWrapperBase::ContextSensitiveHelp(BOOL fEnterMode)
{
	return _pOleWin->ContextSensitiveHelp(fEnterMode);
}


//-----------------------
CSHAccessibleBase::CSHAccessibleBase() : _ptiAcc(NULL)
{ 
	//DllAddRef();
	_cRef = 1;
}

CSHAccessibleBase::~CSHAccessibleBase()
{ 
	ATOMICRELEASE(_ptiAcc);
	//DllRelease();
}

// IUnknown
STDMETHODIMP CSHAccessibleBase::QueryInterface(REFIID riid, __out void **ppv)
{
	static const QITAB qit[] = 
	{
		QITABENT(CSHAccessibleBase, IDispatch),
		QITABENT(CSHAccessibleBase, IAccessible),
		QITABENT(CSHAccessibleBase, IOleWindow),
		QITABENT(CSHAccessibleBase, IEnumVARIANT),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSHAccessibleBase::AddRef()
{
	return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CSHAccessibleBase::Release()
{
	ULONG cRef = InterlockedDecrement(&_cRef);
	if (!cRef)
		delete this;

	return cRef;
}

// private IDispatch Helpers
HRESULT CSHAccessibleBase::s_accLoadTypeInfo(__out ITypeInfo **ppti)
{
	*ppti = NULL;
	ITypeLib *ptl;
	HRESULT hr = LoadTypeLib(L"oleacc.dll", &ptl);
	if (SUCCEEDED(hr))
	{
		hr = ptl->GetTypeInfoOfGuid(IID_IAccessible, ppti);
		ptl->Release();
	}

	return hr;
}

HRESULT CSHAccessibleBase::_EnsureTypeInfo()
{
	HRESULT hr = S_OK;
	if (!_ptiAcc)
	{
		hr = s_accLoadTypeInfo(&_ptiAcc);
	}
	return hr;
}

// IDispatch
STDMETHODIMP CSHAccessibleBase::GetTypeInfoCount(__out UINT *pctinfo) 
{ 
	HRESULT hr = _EnsureTypeInfo();
	if (SUCCEEDED(hr))
	{
		*pctinfo = 1;
	}
	else
	{
		*pctinfo = 0;
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::GetTypeInfo(UINT iTInfo, LCID lcid, __out ITypeInfo **ppTInfo)
{ 
	if (!ppTInfo)
	{
		return E_POINTER;
	}

	if (iTInfo != 0)
	{
		return DISP_E_BADINDEX;
	}

	*ppTInfo = NULL;
	HRESULT hr = _EnsureTypeInfo();
	if (SUCCEEDED(hr))
	{
		*ppTInfo = _ptiAcc;
		(*ppTInfo)->AddRef();
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::GetIDsOfNames(
	REFIID riid, 
	__in_ecount(cNames) OLECHAR **rgszNames, 
	UINT cNames,
	LCID lcid, 
	__out_ecount(cNames) DISPID *rgdispid)
{
	ZeroMemory(rgdispid, sizeof(DISPID) * cNames);
	HRESULT hr = DISP_E_UNKNOWNINTERFACE;
	if (IsEqualIID(IID_NULL, riid) ||
		IsEqualIID(IID_IAccessible, riid))
	{
		hr = _EnsureTypeInfo();
		if (SUCCEEDED(hr))
		{
			hr = _ptiAcc->GetIDsOfNames(rgszNames, cNames, rgdispid);
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::Invoke(
									   DISPID  dispidMember, 
									   REFIID  riid, 
									   LCID    lcid, 
									   WORD    wFlags,
									   __inout DISPPARAMS *pdispparams, 
									   __out VARIANT *pvarResult, 
									   __out EXCEPINFO *pexcepinfo, 
									   __out UINT *puArgErr)
{
	HRESULT hr = _EnsureTypeInfo();
	if (SUCCEEDED(hr))
	{
		hr = _ptiAcc->Invoke(this, dispidMember, wFlags, pdispparams, 
			pvarResult, pexcepinfo, puArgErr);
	}
	return hr;
}

//  IAccessible
STDMETHODIMP CSHAccessibleBase::get_accParent(__out IDispatch **ppdispParent)
{
	*ppdispParent = NULL;
	return S_FALSE;
}

STDMETHODIMP CSHAccessibleBase::get_accChildCount(__out long *pcChildren)
{
	*pcChildren = _GetChildCount();
	return S_OK;
}

STDMETHODIMP CSHAccessibleBase::get_accChild(VARIANT varChild, __out IDispatch **ppdispChild)
{
	*ppdispChild = NULL;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		hr = varChild.lVal == CHILDID_SELF ? E_INVALIDARG : S_FALSE;
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accValue(VARIANT varChild, __out BSTR *pbstrValue)
{
	*pbstrValue = NULL;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		if (varChild.lVal == CHILDID_SELF)
		{
			hr = v_OnGetValue(pbstrValue);
		}
		else
		{
			hr = DISP_E_MEMBERNOTFOUND;
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accName(VARIANT varChild, __out BSTR *pbstrName)
{
	*pbstrName = NULL;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		if (varChild.lVal == CHILDID_SELF)
		{
			hr = v_OnGetName(pbstrName);
		}
		else
		{
			hr = S_FALSE;
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accDescription(VARIANT varChild, __out BSTR *pbstrDescription)
{
	*pbstrDescription = NULL;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		if (varChild.lVal == CHILDID_SELF)
		{
			hr = v_OnGetDescription(pbstrDescription);
		}
		else
		{
			hr = DISP_E_MEMBERNOTFOUND;
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accRole(VARIANT varChild, __out VARIANT *pvarRole)
{
	VariantInit(pvarRole);
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		if (varChild.lVal == CHILDID_SELF)
		{
			pvarRole->vt = VT_I4;
			pvarRole->intVal = v_OnGetRole();
			hr = S_OK;
		}
		else
		{
			hr = DISP_E_MEMBERNOTFOUND;
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accState(VARIANT varChild, __out VARIANT *pvarState)
{
	VariantInit(pvarState);
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		if (varChild.lVal == CHILDID_SELF)
		{
			hr = InitVariantFromInt32(STATE_SYSTEM_DEFAULT, pvarState);
		}
		else
		{
			hr = DISP_E_MEMBERNOTFOUND;
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accHelp(VARIANT varChild, __out BSTR *pbstrHelp)
{
	*pbstrHelp = NULL;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		if (varChild.lVal == CHILDID_SELF)
		{
			hr = v_OnGetHelp(pbstrHelp);
		}
		else
		{
			hr = DISP_E_MEMBERNOTFOUND;
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accHelpTopic(__out BSTR *pbstrHelpFile, VARIANT varChild, __out long *pidTopic)
{
	*pbstrHelpFile = NULL;
	*pidTopic    = -1;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		hr = DISP_E_MEMBERNOTFOUND;
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accKeyboardShortcut(VARIANT varChild, __out BSTR *pbstrKeyboardShortcut)
{
	*pbstrKeyboardShortcut = NULL;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		hr = DISP_E_MEMBERNOTFOUND;
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::get_accFocus(__out VARIANT *pvarFocusChild)
{
	VariantInit(pvarFocusChild);
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP CSHAccessibleBase::get_accSelection(__out VARIANT *pvarSelectedChildren)
{
	VariantInit(pvarSelectedChildren);
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP CSHAccessibleBase::get_accDefaultAction(VARIANT varChild, __out BSTR *pbstrDefaultAction)
{
	*pbstrDefaultAction = NULL;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		hr = DISP_E_MEMBERNOTFOUND;
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::accSelect(long flagsSelect, VARIANT varChild)
{
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		hr = DISP_E_MEMBERNOTFOUND;
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::accLocation(__out long *pxLeft, __out long *pyTop, __out long *pcxWidth, __out long *pcyHeight, VARIANT varChild)
{
	*pxLeft = 0;
	*pyTop = 0;
	*pcxWidth = 0;
	*pcyHeight = 0;
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		hr = DISP_E_MEMBERNOTFOUND;
	}
	return hr;
}

STDMETHODIMP CSHAccessibleBase::accNavigate(long navDir, VARIANT varStart, __out VARIANT *pvarEndUpAt)
{
	VariantInit(pvarEndUpAt);
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP CSHAccessibleBase::accHitTest(long xLeft, long yTop, __out VARIANT *pvarChildAtPoint)
{
	VariantInit(pvarChildAtPoint);
	return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP CSHAccessibleBase::accDoDefaultAction(VARIANT varChild)
{
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		hr = DISP_E_MEMBERNOTFOUND;
	}
	return hr;
}

//  IOleWindow
STDMETHODIMP CSHAccessibleBase::GetWindow(__out HWND *phwnd)
{
	*phwnd = v_GetWindow();
	return IsWindow(*phwnd) ? S_OK : S_FALSE;
}

// IEnumVARIANT
STDMETHODIMP CSHAccessibleBase::Next(ULONG celt, __out_ecount_part(celt, *pceltFetched) VARIANT* rgvar, __out_opt ULONG * pceltFetched)
{
	VARIANT *pvar = rgvar;
	long cChildren = _GetChildCount();
	long cFetched = 0;
	long lCur = _lChildCur;

	while ((cFetched < (long)celt) && (lCur < cChildren))
	{
		cFetched++;
		lCur++;

		// Note this gives us (index)+1 because we incremented iCur
		InitVariantFromInt32(lCur, pvar);
		++pvar;
	}

	_lChildCur = lCur;

	if (pceltFetched)
	{
		*pceltFetched = cFetched;
	}

	return (cFetched < (long)celt) ? S_FALSE : S_OK;
}

STDMETHODIMP CSHAccessibleBase::Skip(ULONG celt)
{
	long cChildren = _GetChildCount();

	_lChildCur += celt;
	if (_lChildCur > cChildren)
	{
		_lChildCur = cChildren;
	}

	//
	// We return S_FALSE if at the end
	//
	return (_lChildCur >= cChildren) ? S_FALSE : S_OK;
}

STDMETHODIMP CSHAccessibleBase::Reset()
{
	_lChildCur = 0;
	return S_OK;
}

STDMETHODIMP CSHAccessibleBase::Clone(__out IEnumVARIANT ** ppenum)
{
	return E_NOTIMPL;
}

// protected helpers
HRESULT CSHAccessibleBase::_ValidateAccChild(__inout VARIANT *pvar)
{
	HRESULT hr = S_OK;
	while ((pvar->vt == (VT_VARIANT | VT_BYREF)) && SUCCEEDED(hr))
	{
		hr = VariantCopy(pvar, pvar->pvarVal);
	}

	if (SUCCEEDED(hr))
	{
		if (pvar->vt == VT_I4 &&
			(pvar->lVal >= 0) && (pvar->lVal < (1 + _GetChildCount())))
		{
			hr = S_OK;
		}
		else
		{
			hr = E_INVALIDARG;
		}
	}

	return hr;
}

// CSHAccessibleHWNDBase
//  IAccessible
STDMETHODIMP CSHAccessibleHWNDBase::get_accParent(__out IDispatch **ppdispParent)
{
	*ppdispParent = NULL;
	HRESULT hr = S_FALSE;
	HWND hwnd = v_GetWindow();
	if (IsWindow(hwnd))
	{
		hr = AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_PPV_ARGS(ppdispParent));
	}
	return hr;
}

STDMETHODIMP CSHAccessibleHWNDBase::get_accState(VARIANT varChild, __out VARIANT *pvarState)
{
	VariantInit(pvarState);
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		if (varChild.lVal == CHILDID_SELF)
		{
			long lState = STATE_SYSTEM_DEFAULT;

			HWND hwnd = v_GetWindow();
			if (GetFocus() == hwnd)
			{
				lState |= STATE_SYSTEM_FOCUSED;
			}
			else if (IsWindowEnabled(hwnd))
			{
				lState|= STATE_SYSTEM_FOCUSABLE;
			}

			if (!IsWindowVisible(hwnd))
			{
				lState |= STATE_SYSTEM_INVISIBLE;
			}

			hr = InitVariantFromInt32(lState, pvarState);
		}
		else
		{
			hr = S_FALSE;
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleHWNDBase::get_accFocus(__out VARIANT *pvarFocusChild)
{
	VariantInit(pvarFocusChild);
	HRESULT hr = S_FALSE;
	HWND hwndFocus = GetFocus();
	HWND hwnd = v_GetWindow();
	if (IsWindow(hwnd) && (hwndFocus == hwnd || IsChild(hwnd, hwndFocus)))
	{
		hr = InitVariantFromInt32(CHILDID_SELF, pvarFocusChild);
	}
	return hr;
}

STDMETHODIMP CSHAccessibleHWNDBase::get_accSelection(__out VARIANT *pvarSelectedChildren)
{
	return get_accFocus(pvarSelectedChildren);  // implemented same as focus.
}

STDMETHODIMP CSHAccessibleHWNDBase::accSelect(long flagsSelect, VARIANT varChild)
{
	HRESULT hr = _ValidateAccChild(&varChild);
	if (SUCCEEDED(hr))
	{
		hr = S_FALSE;
		if (varChild.lVal == CHILDID_SELF)
		{
			HWND hwnd = v_GetWindow();
			if (flagsSelect & SELFLAG_TAKEFOCUS && IsWindow(hwnd))
			{
				SetFocus(hwnd);
				hr = S_OK;
			}
		}
	}
	return hr;
}

STDMETHODIMP CSHAccessibleHWNDBase::accLocation(__out long *pxLeft, __out long *pyTop, __out long *pcxWidth, __out long *pcyHeight, VARIANT varChild)
{
	*pxLeft = 0;
	*pyTop = 0;
	*pcxWidth = 0;
	*pcyHeight = 0;
	HRESULT hr = DISP_E_MEMBERNOTFOUND;
	HWND hwnd = v_GetWindow();
	if (IsWindow(hwnd))
	{
		RECT rc;
		GetWindowRect(hwnd, &rc);
		*pxLeft = rc.left;
		*pyTop  = rc.top;
		*pcxWidth  = RECTWIDTH(rc);
		*pcyHeight = RECTHEIGHT(rc);
		hr = S_OK;
	}
	return hr;
}

STDMETHODIMP CSHAccessibleHWNDBase::accHitTest(long xLeft, long yTop, __out VARIANT *pvarChildAtPoint)
{
	VariantInit(pvarChildAtPoint);
	HRESULT hr = S_FALSE;
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

// Some controls (typically the ones that want to act like
// a pop-up menu, like CListViewPopup and autocomplete)
// fire EVENT_OBJECT_FOCUS notifications even though they
// don't have Win32 focus. When these controls are destroyed,
// focus "returns" to the previous Win32 control from an
// accessibility standpoint, but from a window manager standpoint,
// the previous Win32 control never *lost* focus so there
// is no need to say that it *regained* focus (you can't regain
// what you never lost).  Therefore, we have to fire an artificial
// EVENT_OBJECT_FOCUS event to get MSAA and Win32 back in sync.
//
STDAPI_(void) SyncMSAAFocusWithWin32()
{
	HWND hwndFocus = GetFocus();
	// not perfect but it's what USER32 does when menus are dismissed
	if (hwndFocus && IsWinEventHookInstalled(EVENT_OBJECT_FOCUS))
	{
		BOOL fEventFired = FALSE;
		IAccessible *pacc;
		HRESULT hr = AccessibleObjectFromWindow(hwndFocus, (DWORD) OBJID_CLIENT, IID_PPV_ARGS(&pacc));        
		if (SUCCEEDED(hr))
		{
			VARIANT varChild = {VT_I4, CHILDID_SELF};
			hr = pacc->get_accFocus(&varChild);
			if (SUCCEEDED(hr) && varChild.vt == VT_I4)
			{
				NotifyWinEvent(EVENT_OBJECT_FOCUS, hwndFocus, OBJID_CLIENT, varChild.lVal);
				fEventFired = TRUE;
			}
			VariantClear(&varChild);
			pacc->Release();
		}
		if (!fEventFired)
		{
			// Couldn't get the focus sub-object. Just put focus on the window as a fallback.
			NotifyWinEvent(EVENT_OBJECT_FOCUS, hwndFocus, OBJID_CLIENT, 0);
		}
	}
}

