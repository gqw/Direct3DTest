#pragma once
#include "pch.h"
#include <wrl/client.h>

class MeowTextApp :
	public ITfUIElementSink,
	public ITfTextEditSink,
	public ITfThreadMgrEventSink,
	public ITfContextOwnerCompositionSink
{
public:
	MeowTextApp(HWND);
	~MeowTextApp();

	HRESULT	STDMETHODCALLTYPE	QueryInterface(REFIID, LPVOID*);
	DWORD	STDMETHODCALLTYPE	AddRef();
	DWORD	STDMETHODCALLTYPE	Release();

	/* ITfContextOwnerCompositionSink */
	HRESULT STDMETHODCALLTYPE	OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk);
	HRESULT STDMETHODCALLTYPE	OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew);
	HRESULT STDMETHODCALLTYPE	OnEndComposition(ITfCompositionView *pComposition);

	/* ITfUIElementSink */
	HRESULT	STDMETHODCALLTYPE	BeginUIElement(DWORD dwUIElementId, BOOL *pbShow);
	HRESULT	STDMETHODCALLTYPE	UpdateUIElement(DWORD dwUIElementId);
	HRESULT	STDMETHODCALLTYPE	EndUIElement(DWORD dwUIElementId);

	/*ITfThreadMgrEventSink*/
	virtual HRESULT STDMETHODCALLTYPE OnInitDocumentMgr(
		/* [in] */ __RPC__in_opt ITfDocumentMgr* pdim) {
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnUninitDocumentMgr(
		/* [in] */ __RPC__in_opt ITfDocumentMgr* pdim) {
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE OnSetFocus(
		/* [in] */ __RPC__in_opt ITfDocumentMgr* pdimFocus,
		/* [in] */ __RPC__in_opt ITfDocumentMgr* pdimPrevFocus);

	virtual HRESULT STDMETHODCALLTYPE OnPushContext(
		/* [in] */ __RPC__in_opt ITfContext* pic) {
		return S_OK;
	};

	virtual HRESULT STDMETHODCALLTYPE OnPopContext(
		/* [in] */ __RPC__in_opt ITfContext* pic) {
		return S_OK;
	};

	virtual HRESULT STDMETHODCALLTYPE OnEndEdit(
		/* [in] */ __RPC__in_opt ITfContext* pic,
		/* [in] */ TfEditCookie ecReadOnly,
		/* [in] */ __RPC__in_opt ITfEditRecord* pEditRecord) {
		return S_OK;
	}

	std::vector<std::wstring> candidatelist;
	VOID SetFocus();

private:
	HWND hwnd;
	ULONG reference;
	DWORD clientid;
	ITfThreadMgr * threadmgr;
	ITfThreadMgrEx * threadmgrex;
	ITfUIElementMgr * uielementmgr;

	DWORD uielementsinkcookie = -1;
	DWORD threadMgrEventSinkCookie = 0;
	DWORD _textEditSinkCookie = -1;
	DWORD _contextCookie = -1;
	TfEditCookie editcookie;

	Microsoft::WRL::ComPtr<ITfContext> _editSinkContext;
	Microsoft::WRL::ComPtr<ITfDocumentMgr> documentmgr;
	UINT m_candidateSelection;
};

