#pragma once

#include "../pch.h"
#include <deque>


class TsfInputMethodStore :
	public ITextStoreACP,
	public ITfUIElementSink,
	public ITfTextEditSink,
	public ITfThreadMgrEventSink,
	public ITfContextOwnerCompositionSink,
	public ITfActiveLanguageProfileNotifySink,
	public ITfLanguageProfileNotifySink {
public:
	static TsfInputMethodStore& get() {
		static TsfInputMethodStore tsf;
		return tsf;
	}

	bool OnInit(HWND hWnd, wchar_t* buffer, std::size_t bufferSize);
	bool OnDestroy();
	void SetFocus(bool isFocus);
	void SetBufferLength(std::size_t buffer_len);
	void SetCorsorPos(std::size_t insert_pos);
	void SwitchImeConvert();

	HWND window_handle() { return m_hWnd; }
	std::vector<std::wstring> candidates() { return m_vCandidates; }
	const wchar_t* reading() { return m_strContentBuffer; }
	std::size_t reading_len() { return m_strContentBufferLength; }
	bool is_reading_upate();

	UINT candidate_count() { return m_ulCandidateCount; }
	UINT candidate_page_count() { return m_ulCandidatePageCount; }
	UINT candidate_page_index() { return m_ulCandidatePageIndex; }
	UINT candidate_page_size() { return m_ulCandidatePageSize > 0 ? m_ulCandidatePageSize : 1; }
	UINT candidate_page_max_size() { return m_ulCandidatePageMaxSize; }
	UINT candidate_select_index() { return m_ulCandidateSelect; }
	UINT candidate_select_start() { return m_acpContentSelection.acpStart; }
	UINT candidate_select_end() { return m_acpContentSelection.acpEnd; }

	bool enable_native() { return m_bImeEnableNative; }
	LANGID lang_id() { return m_langId; }
	std::wstring& ime_desc_w() { return m_strInputDescW; }
	std::string& ime_desc_a() { return m_strInputDescA; }

private:
	HRESULT	STDMETHODCALLTYPE	QueryInterface(REFIID, LPVOID*);
	DWORD	STDMETHODCALLTYPE	AddRef();
	DWORD	STDMETHODCALLTYPE	Release();

	/* ITextStoreACP */
	HRESULT STDMETHODCALLTYPE AdviseSink(REFIID riid, IUnknown* punk, DWORD dwMask);
	HRESULT STDMETHODCALLTYPE UnadviseSink(IUnknown* punk);
	HRESULT STDMETHODCALLTYPE RequestLock(DWORD dwLockFlags, HRESULT* phrSession) override;
	HRESULT STDMETHODCALLTYPE GetStatus(TS_STATUS* pdcs) override;
	HRESULT STDMETHODCALLTYPE QueryInsert(LONG acpTestStart, LONG acpTestEnd, ULONG cch, LONG* pacpResultStart, LONG* pacpResultEnd) override;
	HRESULT STDMETHODCALLTYPE GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP* pSelection, ULONG* pcFetched)  override;
	HRESULT STDMETHODCALLTYPE SetSelection(ULONG ulCount, const TS_SELECTION_ACP* pSelection)  override;
	HRESULT STDMETHODCALLTYPE GetText(LONG acpStart, LONG acpEnd, WCHAR* pchPlain, ULONG cchPlainReq, ULONG* pcchPlainRet,
		TS_RUNINFO* prgRunInfo, ULONG cRunInfoReq, ULONG* pcRunInfoRet, LONG* pacpNext) override;
	HRESULT STDMETHODCALLTYPE SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, const WCHAR* pchText, ULONG cch, TS_TEXTCHANGE* pChange) override;
	HRESULT STDMETHODCALLTYPE GetFormattedText(LONG acpStart, LONG acpEnd, IDataObject** ppDataObject)  override;
	HRESULT STDMETHODCALLTYPE GetEmbedded(LONG acpPos, REFGUID rguidService, REFIID riid, IUnknown** ppunk)  override;
	HRESULT STDMETHODCALLTYPE QueryInsertEmbedded(const GUID* pguidService, const FORMATETC* pFormatEtc, BOOL* pfInsertable)  override;
	HRESULT STDMETHODCALLTYPE InsertEmbedded(DWORD dwFlags, LONG acpStart, LONG acpEnd, IDataObject* pDataObject, TS_TEXTCHANGE* pChange)  override;
	HRESULT STDMETHODCALLTYPE InsertTextAtSelection(DWORD dwFlags, const WCHAR* pchText, ULONG cch, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange)  override;
	HRESULT STDMETHODCALLTYPE InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject* pDataObject, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange) override;
	HRESULT STDMETHODCALLTYPE RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs) override;
	HRESULT STDMETHODCALLTYPE RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags) override;
	HRESULT STDMETHODCALLTYPE RequestAttrsTransitioningAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags) override;
	HRESULT STDMETHODCALLTYPE FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags,
		LONG* pacpNext, BOOL* pfFound, LONG* plFoundOffset) override;
	HRESULT STDMETHODCALLTYPE RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL* paAttrVals, ULONG* pcFetched) override;
	HRESULT STDMETHODCALLTYPE GetEndACP(LONG* pacp) override;
	HRESULT STDMETHODCALLTYPE GetActiveView(TsViewCookie* pvcView) override;
	HRESULT STDMETHODCALLTYPE GetACPFromPoint(TsViewCookie vcView, const POINT* ptScreen, DWORD dwFlags, LONG* pacp) override;
	HRESULT STDMETHODCALLTYPE GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override;
	HRESULT STDMETHODCALLTYPE GetScreenExt(TsViewCookie vcView, RECT* prc) override;
	HRESULT STDMETHODCALLTYPE GetWnd(TsViewCookie vcView, HWND* phwnd) override;

	// HRESULT STDMETHODCALLTYPE OnEndEditTransaction(void) override;

	/* ITfContextOwnerCompositionSink */
	HRESULT STDMETHODCALLTYPE	OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)override;
	HRESULT STDMETHODCALLTYPE	OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew) override;
	HRESULT STDMETHODCALLTYPE	OnEndComposition(ITfCompositionView* pComposition) override;

	/* ITfUIElementSink */
	HRESULT	STDMETHODCALLTYPE	BeginUIElement(DWORD dwUIElementId, BOOL* pbShow) override;
	HRESULT	STDMETHODCALLTYPE	UpdateUIElement(DWORD dwUIElementId) override;
	HRESULT	STDMETHODCALLTYPE	EndUIElement(DWORD dwUIElementId) override;

	/*ITfThreadMgrEventSink*/
	HRESULT STDMETHODCALLTYPE OnInitDocumentMgr(ITfDocumentMgr* pdim) override;
	HRESULT STDMETHODCALLTYPE OnUninitDocumentMgr(ITfDocumentMgr* pdim) override;
	HRESULT STDMETHODCALLTYPE OnSetFocus(ITfDocumentMgr* pdimFocus, ITfDocumentMgr* pdimPrevFocus) override;
	HRESULT STDMETHODCALLTYPE OnPushContext(ITfContext* pic) override;
	HRESULT STDMETHODCALLTYPE OnPopContext(ITfContext* pic) override;

	/*ITfTextEditSink*/
	HRESULT STDMETHODCALLTYPE OnEndEdit(ITfContext* pic, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord) override;

	/*ITfActiveLanguageProfileNotifySink*/
	HRESULT STDMETHODCALLTYPE OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL fActivated) override;
	/*ITfLanguageProfileNotifySink*/
	HRESULT STDMETHODCALLTYPE OnLanguageChange(LANGID langid, BOOL* pfAccept)  override;
	HRESULT STDMETHODCALLTYPE OnLanguageChanged(void) override;

private:
	static LRESULT WINAPI ImmSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
			UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	void EnableIme(bool bEnable);

private:
	// Checks if the document has a read-only lock.
	bool HasReadLock() const;
	// Checks if the document has a read and write lock.
	bool HasReadWriteLock() const;
	void CheckInputLocale();
	
	void FocusDocument();

	// 清除内容
	void Clear();

private:
	bool m_isOpenImm = false;
	bool m_isShowCanditate = false;

	UINT m_ulCandidateCount = 0;
	UINT m_ulCandidatePageCount = 0;
	UINT m_ulCandidatePageIndex = 0;
	UINT m_ulCandidatePageStart = 0;
	UINT m_ulCandidatePageSize = 0;
	UINT m_ulCandidatePageMaxSize = 0;
	UINT m_ulCandidateSelect = 0;

	DWORD m_dwRefernce = 0;
	DWORD m_dwUIElementSinkCookie = TF_INVALID_COOKIE;
	DWORD m_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
	DWORD m_dwLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;
	DWORD m_dwLanguageProfileNotifySinkCookie2 = TF_INVALID_COOKIE;
	DWORD m_dwTextEditSinkCookie = TF_INVALID_COOKIE;
	DWORD m_dwOwnerCompositionCookie = TF_INVALID_COOKIE;

	TfClientId m_dwClientId = 0;
	TfEditCookie m_dwContextReadWriteCookie = TF_INVALID_COOKIE;
	LANGID m_langId = 0;
	std::wstring m_strInputDescW;
	std::string m_strInputDescA;

	Microsoft::WRL::ComPtr<ITfThreadMgrEx> m_itfThreadMgr;
	Microsoft::WRL::ComPtr<ITfUIElementMgr> m_itfUIElementMgr;
	Microsoft::WRL::ComPtr<ITfDocumentMgr> m_itfDocumentMgr;
	Microsoft::WRL::ComPtr<ITfContext> m_itfEditSinkContext;
	Microsoft::WRL::ComPtr<ITfInputProcessorProfiles> m_itfProfiles;

	Microsoft::WRL::ComPtr<ITfRange> m_itfEndRange;

	Microsoft::WRL::ComPtr<ITextStoreACPSink> m_itfTextStoreACPSink_;
	DWORD text_store_acp_sink_mask_;

	std::vector<std::wstring> m_vCandidates;
	/*std::wstring m_strReading;
	std::wstring m_strSelectedStr;*/
	std::wstring m_strDocContent;
	TS_SELECTION_ACP m_docSelection{};

	HWND m_hWnd = nullptr;


	// |edit_flag_| indicates that the status is edited during
// ITextStoreACPSink::OnLockGranted().
	bool edit_flag_ = 0;
	// The type of current lock.
	//   0: No lock.
	//   TS_LF_READ: read-only lock.
	//   TS_LF_READWRITE: read/write lock.
	DWORD m_dwContentLockType = 0;
	// Queue of the lock request used in RequestLock().
	std::deque<DWORD> m_dqLockQueue;

	//  |m_strContentBuffer| contains committed string and composition string.
	//  Example: "aoi" is committed, and "umi" is under composition.
	//    |m_strContentBuffer|: "aoiumi"
	//    |m_iComposeCommittedSize|: 3
	wchar_t* m_strContentBuffer = nullptr;
	std::size_t m_strContentBufferSize = 0;
	std::size_t m_strContentBufferLength = 0;

	size_t m_iComposeCommittedSize = 0;
	// is content changed
	bool m_bIsContentUpdate = false;

	//  |selection_start_| and |selection_end_| indicates the selection range.
	//  Example: "iue" is selected
	//    |string_buffer_|: "aiueo"
	//    |selection_.start()|: 1
	//    |selection_.end()|: 4
	TS_SELECTION_ACP m_acpContentSelection{};

	bool m_bIsFocused = false;
	bool m_bImeEnableNative = true; // true， 中文； false, 英文
	HIMC m_himcOrg = nullptr;
	HKL g_hklCurrent;
};