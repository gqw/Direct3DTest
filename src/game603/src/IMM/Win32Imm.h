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

	std::wstring reading() { return L""; }

	UINT candidate_count() { return m_ulCandidateCount; }
	UINT candidate_page_count() { return m_ulCandidatePageCount; }
	UINT candidate_page_index() { return m_ulCandidatePageIndex; }
	UINT candidate_page_size() { return m_ulCandidatePageSize > 0 ? m_ulCandidatePageSize : 1; }
	UINT candidate_page_max_size() { return m_ulCandidatePageMaxSize; }
	UINT candidate_select_index() { return m_ulCandidateSelect; }

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
	UINT m_ulCandidateCount = 0;
	UINT m_ulCandidatePageCount = 0;
	UINT m_ulCandidatePageIndex = 0;
	UINT m_ulCandidatePageStart = 0;
	UINT m_ulCandidatePageSize = 0;
	UINT m_ulCandidatePageMaxSize = 0;
	UINT m_ulCandidateSelect = 0;
};