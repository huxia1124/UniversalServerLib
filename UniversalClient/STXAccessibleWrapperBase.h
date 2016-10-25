#pragma once

#include <oleacc.h>
#include <CommCtrl.h>

// Generic IAccessible wrapper class - just calls through on all methods.
// Add overriding behavior in classes derived from this.
class CAccessibleWrapperBase : public IAccessible,
	public IOleWindow,
	public IEnumVARIANT
{
private:
	// We need to do our own refcounting for this wrapper object
	LONG            _cRef;

	// Need ptr to the IAccessible - also keep around ptrs to EnumVar and
	// OleWindow as part of this object, so we can filter those interfaces
	// and trap their QI's...
	// (We leave pEnumVar and OleWin as NULL until we need them)
	IAccessible    *_pAcc;
	IEnumVARIANT   *_pEnumVar;
	IOleWindow     *_pOleWin;

public:
	CAccessibleWrapperBase(__in IAccessible *pAcc);
	virtual ~CAccessibleWrapperBase();

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, __out void** ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IDispatch
	IFACEMETHODIMP GetTypeInfoCount(__out UINT* pctinfo);
	IFACEMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, __out ITypeInfo** pptinfo);
	IFACEMETHODIMP GetIDsOfNames(REFIID riid, __in_ecount(cNames) OLECHAR** rgszNames, UINT cNames,
		LCID lcid, __out_ecount(cNames) DISPID* rgdispid);
	IFACEMETHODIMP Invoke(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags,
		__inout DISPPARAMS* pdp, __out VARIANT* pvarResult,
		__out EXCEPINFO* pxi, __out UINT* puArgErr);

	// IAccessible
	IFACEMETHODIMP get_accParent(__out IDispatch ** ppdispParent);
	IFACEMETHODIMP get_accChildCount(__out long* pChildCount);
	IFACEMETHODIMP get_accChild(VARIANT varChild, __out IDispatch ** ppdispChild);

	IFACEMETHODIMP get_accName(VARIANT varChild, __out BSTR* pszName);
	IFACEMETHODIMP get_accValue(VARIANT varChild, __out BSTR* pszValue);
	IFACEMETHODIMP get_accDescription(VARIANT varChild, __out BSTR* pszDescription);
	IFACEMETHODIMP get_accRole(VARIANT varChild, __out VARIANT *pvarRole);
	IFACEMETHODIMP get_accState(VARIANT varChild, __out VARIANT *pvarState);
	IFACEMETHODIMP get_accHelp(VARIANT varChild, __out BSTR* pszHelp);
	IFACEMETHODIMP get_accHelpTopic(__out BSTR* pszHelpFile, VARIANT varChild, __out long* pidTopic);
	IFACEMETHODIMP get_accKeyboardShortcut(VARIANT varChild, __out BSTR* pszKeyboardShortcut);
	IFACEMETHODIMP get_accFocus(__out VARIANT * pvarFocusChild);
	IFACEMETHODIMP get_accSelection(__out VARIANT * pvarSelectedChildren);
	IFACEMETHODIMP get_accDefaultAction(VARIANT varChild, __out BSTR* pszDefaultAction);

	IFACEMETHODIMP accSelect(long flagsSel, VARIANT varChild);
	IFACEMETHODIMP accLocation(__out long* pxLeft, __out long* pyTop, __out long* pcxWidth, __out long* pcyHeight, VARIANT varChild);
	IFACEMETHODIMP accNavigate(long navDir, VARIANT varStart, __out VARIANT * pvarEndUpAt);
	IFACEMETHODIMP accHitTest(long xLeft, long yTop, __out VARIANT * pvarChildAtPoint);
	IFACEMETHODIMP accDoDefaultAction(VARIANT varChild);

	IFACEMETHODIMP put_accName(VARIANT varChild, __in BSTR szName);
	IFACEMETHODIMP put_accValue(VARIANT varChild, __in BSTR pszValue);

	// IEnumVARIANT
	IFACEMETHODIMP Next(ULONG celt, __out_ecount_part(celt, *pceltFetched) VARIANT* rgvar, __out_opt ULONG * pceltFetched);
	IFACEMETHODIMP Skip(ULONG celt);
	IFACEMETHODIMP Reset(void);
	IFACEMETHODIMP Clone(__out IEnumVARIANT ** ppenum);

	// IOleWindow
	IFACEMETHODIMP GetWindow(__out HWND* phwnd);
	IFACEMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
};

template <class T>
static LRESULT CALLBACK AccessibleSubWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, WPARAM uID, ULONG_PTR dwRefData)
{
	switch (uMsg)
	{
	case WM_GETOBJECT:
		if ((DWORD)lParam == OBJID_CLIENT)
		{
			CAccessibleWrapperBase *pWrapAcc = NULL;
			IAccessible *pAcc = NULL;
			HRESULT hr = CreateStdAccessibleObject(hWnd, OBJID_CLIENT, IID_PPV_ARGS(&pAcc));
			if (SUCCEEDED(hr) && pAcc)
			{
				pWrapAcc = new T(pAcc, dwRefData);
				pAcc->Release();

				if (pWrapAcc != NULL)
				{
					LRESULT lres = LresultFromObject(IID_IAccessible, wParam, SAFECAST(pWrapAcc, IAccessible*));
					pWrapAcc->Release();
					return lres;
				}
			}
		}
		break;
	case WM_DESTROY:
		RemoveWindowSubclass(hWnd, AccessibleSubWndProc<T>, uID);
		break;    
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

template <class T>
static STDMETHODIMP WrapAccessibleControl(HWND hWnd, ULONG_PTR dwRefData=0)
{
	if (SetWindowSubclass(hWnd, AccessibleSubWndProc<T>, 0, dwRefData))
	{
		return S_OK;
	}
	return E_FAIL;
}

// Helper class for implementing IAccessible from scratch
// Derive your class from this one, implement the protected virtual functions, and override any 
// behaviors you want to override.
class CSHAccessibleBase : public IAccessible, 
	public IOleWindow,
	public IEnumVARIANT
{
protected:
	LONG            _cRef;
public:
	CSHAccessibleBase();
	virtual ~CSHAccessibleBase();

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, __out void **ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IDispatch
	IFACEMETHODIMP GetTypeInfoCount(__out UINT *pctinfo);
	IFACEMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, __out ITypeInfo **pptinfo);
	IFACEMETHODIMP GetIDsOfNames(REFIID riid, __in_ecount(cNames) OLECHAR **rgszNames, UINT cNames,
		LCID lcid, __out_ecount(cNames) DISPID *rgdispid);
	IFACEMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
		__inout DISPPARAMS *pdispparams, __out VARIANT *pvarResult, 
		__out EXCEPINFO *pexcepinfo, __out UINT *puArgErr);

	//  IAccessible
	IFACEMETHODIMP get_accParent(__out IDispatch **ppdispParent); // Must override this to provide valid parent based on your usage.
	IFACEMETHODIMP get_accChildCount(__out long *pcChildren);
	IFACEMETHODIMP get_accChild(VARIANT varChild, __out IDispatch **ppdispChild);

	IFACEMETHODIMP get_accValue(VARIANT varChild, __out BSTR *pbstrValue);
	IFACEMETHODIMP get_accName(VARIANT varChild, __out BSTR *pbstrName);
	IFACEMETHODIMP get_accDescription(VARIANT varChild, __out BSTR *pbstrDescription);
	IFACEMETHODIMP get_accRole(VARIANT varChild, __out VARIANT *pvarRole);
	IFACEMETHODIMP get_accState(VARIANT varChild, __out VARIANT *pvarState);
	IFACEMETHODIMP get_accHelp(VARIANT varChild, __out BSTR *pbstrHelp);
	IFACEMETHODIMP get_accHelpTopic(__out BSTR *pbstrHelpFile, VARIANT varChild, __out long *pidTopic);
	IFACEMETHODIMP get_accKeyboardShortcut(VARIANT varChild, __out BSTR *pbstrKeyboardShortcut);
	IFACEMETHODIMP get_accFocus(__out VARIANT *pvarFocusChild);
	IFACEMETHODIMP get_accSelection(__out VARIANT *pvarSelectedChildren);
	IFACEMETHODIMP get_accDefaultAction(VARIANT varChild, __out BSTR *pbstrDefaultAction);

	IFACEMETHODIMP accSelect(long flagsSelect, VARIANT varChild);
	IFACEMETHODIMP accLocation(__out long *pxLeft, __out long *pyTop, __out long *pcxWidth, __out long *pcyHeight, VARIANT varChild);
	IFACEMETHODIMP accNavigate(long navDir, VARIANT varStart, __out VARIANT *pvarEndUpAt);
	IFACEMETHODIMP accHitTest(long xLeft, long yTop, __out VARIANT *pvarChildAtPoint);
	IFACEMETHODIMP accDoDefaultAction(VARIANT varChild);

	IFACEMETHODIMP put_accName(VARIANT /*varChild*/, __in BSTR /*bstrName*/)   { return E_NOTIMPL; }
	IFACEMETHODIMP put_accValue(VARIANT /*varChild*/, __in BSTR /*bstrValue*/) { return E_NOTIMPL; }

	//  IOleWindow
	IFACEMETHODIMP GetWindow(__out HWND *phwnd);
	IFACEMETHODIMP ContextSensitiveHelp(BOOL /*fEnterMode*/) { return E_NOTIMPL; }

	// IEnumVARIANT
	IFACEMETHODIMP Next(ULONG celt, __out_ecount_part(celt, *pceltFetched) VARIANT* rgvar, __out_opt ULONG * pceltFetched);
	IFACEMETHODIMP Skip(ULONG celt);
	IFACEMETHODIMP Reset();
	IFACEMETHODIMP Clone(__out IEnumVARIANT ** ppenum);

protected:

	// must override this to provide the window
	virtual HWND v_GetWindow() PURE;
	virtual long _GetChildCount() { return 0; }

	// Override these methods to provide common accessibility values
	virtual HRESULT v_OnGetDescription(__out BSTR * /*pbstrDescription*/)    { return S_FALSE; }
	virtual HRESULT v_OnGetHelp(__out BSTR * /*pbstrHelp*/)                  { return S_FALSE; }
	virtual HRESULT v_OnGetName(__out BSTR * /*pbstrName*/)                  { return S_FALSE; }
	virtual long    v_OnGetRole()                                            { return ROLE_SYSTEM_CLIENT; }
	virtual HRESULT v_OnGetValue(__out BSTR * /*pbstrValue*/)                { return S_FALSE; }

	HRESULT _ValidateAccChild(__inout VARIANT *pvar);

	long _lChildCur;

private:
	HRESULT _EnsureTypeInfo();
	static HRESULT s_accLoadTypeInfo(__out ITypeInfo **ppti);

	ITypeInfo  *_ptiAcc;
};


// Helper class for implementing IAccessible for HWND based objects.
// The assumption is that the hwnd used in the constructor is the
// actual window this IAccessible object represents.
class CSHAccessibleHWNDBase : public CSHAccessibleBase
{
public:
	CSHAccessibleHWNDBase() : CSHAccessibleBase() {}
	virtual ~CSHAccessibleHWNDBase() {}

	// IAccessible
	IFACEMETHODIMP get_accParent(__out IDispatch **ppdispParent);
	IFACEMETHODIMP get_accState(VARIANT varChild, __out VARIANT *pvarState);
	IFACEMETHODIMP get_accFocus(__out VARIANT *pvarFocusChild);
	IFACEMETHODIMP get_accSelection(__out VARIANT *pvarSelectedChildren);
	IFACEMETHODIMP accSelect(long flagsSelect, VARIANT varChild);
	IFACEMETHODIMP accLocation(__out long *pxLeft, __out long *pyTop, __out long *pcxWidth, __out long *pcyHeight, VARIANT varChild);
	IFACEMETHODIMP accHitTest(long xLeft, long yTop, __out VARIANT *pvarChildAtPoint);
};

HRESULT InitVariantFromInt32(__in LONG lVal, __out VARIANT *pvar);
HRESULT InitVariantFromDispatch(__in_opt IDispatch* pdisp, __out VARIANT *pvar);
