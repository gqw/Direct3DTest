#pragma once

#include "../pch.h"


class Win32Imm :
	public ITfContextOwnerCompositionSink,
	public ITfUIElementSink {
public:
	// HWND SubclassifyWindow(HWND hWnd);

	bool OnInit(HWND hWnd);
	bool OnDestroy();
	
	LRESULT IMMSubclassProc(
		HWND hWnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
	);

	std::vector<std::wstring> candidates() { return m_vCandidates; }

private:
	HRESULT	STDMETHODCALLTYPE	QueryInterface(REFIID, LPVOID*);
	DWORD	STDMETHODCALLTYPE	AddRef();
	DWORD	STDMETHODCALLTYPE	Release();

	/* ITfContextOwnerCompositionSink */
	HRESULT STDMETHODCALLTYPE	OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk);
	HRESULT STDMETHODCALLTYPE	OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew);
	HRESULT STDMETHODCALLTYPE	OnEndComposition(ITfCompositionView* pComposition);

	/* ITfUIElementSink */
	void OnImeComposition(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnImeGetCompStr(HWND hWnd, HIMC pImc, WPARAM wParam, LPARAM lParam);
	void OnImeGetCompAttr(HWND hWnd, HIMC pImc, WPARAM wParam, LPARAM lParam);

	virtual HRESULT STDMETHODCALLTYPE BeginUIElement(
		/* [in] */ DWORD dwUIElementId,
		/* [out][in] */ BOOL* pbShow);

	virtual HRESULT STDMETHODCALLTYPE UpdateUIElement(
		/* [in] */ DWORD dwUIElementId);

	virtual HRESULT STDMETHODCALLTYPE EndUIElement(
		/* [in] */ DWORD dwUIElementId);
private:
	bool m_isOpenImm = false;
	bool m_isShowCanditate = false;

	Microsoft::WRL::ComPtr<ITfThreadMgrEx> m_imeThreadMgr;
	Microsoft::WRL::ComPtr<ITfUIElementMgr> m_imeUIElementMgr;
	Microsoft::WRL::ComPtr<ITfDocumentMgr> documentmgr;
	TfClientId m_dwClientId;
	DWORD m_dwUIElementSinkCookie = 0;
	DWORD m_dwRefernce = 0;

	std::vector<std::wstring> m_vCandidates;
};