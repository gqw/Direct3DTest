#include "TsfInputMethodStore.h"


#ifndef NOMINMAX
#undef min
#undef max
#endif

using Microsoft::WRL::ComPtr;

// We support only one view.
const TsViewCookie kViewCookie = 1;

bool TsfInputMethodStore::OnInit(HWND hWnd, wchar_t* buffer, std::size_t bufferSize) {
	TRACE_FUNC();
	ASSERT_THROW(IsWindow(hWnd), "imm init failed.");
	m_hWnd = hWnd;
	m_dwRefernce = 1;
	m_strContentBuffer = buffer;
	m_strContentBufferSize = bufferSize;
	if (buffer == nullptr || bufferSize == 0) {
		m_strContentBufferSize = 1024;
		m_strDocContent.resize(m_strContentBufferSize);
		m_strContentBuffer = m_strDocContent.data();
	}

	if (FAILED(CoInitialize(NULL))) return false;
	ComPtr<ITfThreadMgr> threadmgr;
	THROW_IF_FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, &threadmgr));
	THROW_IF_FAILED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles, &m_itfProfiles));

	// Active
	THROW_IF_FAILED(threadmgr->QueryInterface(IID_ITfThreadMgrEx, (LPVOID*)&m_itfThreadMgr));
	THROW_IF_FAILED(m_itfThreadMgr->ActivateEx(&m_dwClientId, TF_TMAE_UIELEMENTENABLEDONLY));

	// ITfUIElementSink
	THROW_IF_FAILED(threadmgr->QueryInterface(IID_ITfUIElementMgr, &m_itfUIElementMgr));
	ComPtr<ITfSource> source;
	THROW_IF_FAILED(m_itfUIElementMgr->QueryInterface(IID_ITfSource, &source));
	THROW_IF_FAILED(source->AdviseSink(IID_ITfUIElementSink, (ITfUIElementSink*)this, &m_dwUIElementSinkCookie));
	THROW_IF_FAILED(source->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink*)this, &m_dwThreadMgrEventSinkCookie));
	
	
	THROW_IF_FAILED(threadmgr->CreateDocumentMgr(&m_itfDocumentMgr));

	ComPtr<ITfSource> source2;
	THROW_IF_FAILED(m_itfThreadMgr->QueryInterface(IID_ITfSource, &source2));
	THROW_IF_FAILED(source2->AdviseSink(IID_ITfActiveLanguageProfileNotifySink, (ITfActiveLanguageProfileNotifySink*)this, &m_dwLanguageProfileNotifySinkCookie));

	ComPtr<ITfContext> context;
	THROW_IF_FAILED(m_itfDocumentMgr->CreateContext(m_dwClientId, 0, (ITextStoreACP*)this, &context, &m_dwContextReadWriteCookie));
	THROW_IF_FAILED(m_itfDocumentMgr->Push(context.Get()));

	m_itfThreadMgr->AssociateFocus(hWnd, m_itfDocumentMgr.Get(), nullptr);
	m_itfThreadMgr->SetFocus(m_itfDocumentMgr.Get());

	ComPtr<ITfSource> source3;
	THROW_IF_FAILED(m_itfProfiles->QueryInterface(IID_ITfSource, &source3));
	THROW_IF_FAILED(source3->AdviseSink(IID_ITfLanguageProfileNotifySink, (ITfLanguageProfileNotifySink*)this, &m_dwLanguageProfileNotifySinkCookie2));

	do 
	{
		CLSID clsId;
		GUID guild;
		BSTR desc;
		m_strInputDescW.clear();
		m_strInputDescA.clear();
		if (FAILED(m_itfProfiles->GetCurrentLanguage(&m_langId))) break;
		if (FAILED(m_itfProfiles->GetDefaultLanguageProfile(m_langId, GUID_TFCAT_TIP_KEYBOARD, &clsId, &guild))) break;
		if (FAILED(m_itfProfiles->GetActiveLanguageProfile(clsId, &m_langId, &guild))) break;
		if (FAILED(m_itfProfiles->GetLanguageProfileDescription(clsId, m_langId, guild, &desc))) break;
		m_strInputDescW = desc;
		m_strInputDescA = wstring_to_utf8(desc);
		SysFreeString(desc);
	} while (false);
	
	m_himcOrg = ImmGetContext(hWnd);
	ImmReleaseContext(hWnd, m_himcOrg);

	if (::SetWindowSubclass(hWnd, TsfInputMethodStore::ImmSubClassProc,
		reinterpret_cast<UINT_PTR>(TsfInputMethodStore::ImmSubClassProc),
		reinterpret_cast<DWORD_PTR>(this))) {

	}
	return true;
}

void TsfInputMethodStore::SwitchImeConvert() {
	HRESULT hr;
	ComPtr<ITfCompartmentMgr> compartMgr;
	ComPtr<ITfCompartment> openMode, convMode, keyMode;
	if (FAILED(m_itfThreadMgr->QueryInterface(IID_ITfCompartmentMgr, &compartMgr))) return;
	if (FAILED(compartMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &openMode))) return;
	if (FAILED(compartMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &convMode))) return;

	ComPtr<ITfClientId> clientId;
	m_itfThreadMgr->QueryInterface(IID_ITfClientId, &clientId);
	TfClientId tid;
	clientId->GetClientId(IID_ITfClientId, &tid);

	if (openMode && convMode)
	{
		VARIANT valOpenMode;
		VariantInit(&valOpenMode);
		hr = openMode->GetValue(&valOpenMode);

		VARIANT valConvMode;
		VariantInit(&valConvMode);
		hr = convMode->GetValue(&valConvMode);
		
		valConvMode.lVal = ((valConvMode.lVal & TF_CONVERSIONMODE_NATIVE) == 0) ? TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_SYMBOL : 0;
		convMode->SetValue(m_dwClientId, &valConvMode);
	}
}

LRESULT WINAPI TsfInputMethodStore::ImmSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
	switch (uMsg)
	{
	case WM_KEYDOWN:

		break;
	case WM_SETFOCUS: {
		TsfInputMethodStore::get().FocusDocument();
		break;
	}
	default:
		break;
	}
	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

#define GETLANG()		LOWORD(g_hklCurrent)
#define GETPRIMLANG()	((WORD)PRIMARYLANGID(GETLANG()))
#define GETSUBLANG()	SUBLANGID(GETLANG())

void TsfInputMethodStore::CheckInputLocale()
{
	//static HKL hklPrev = 0;
	//g_hklCurrent = GetKeyboardLayout(0);
	//if (hklPrev == g_hklCurrent)
	//{
	//	return;
	//}
	//hklPrev = g_hklCurrent;
	//switch (GETPRIMLANG())
	//{
	//	// Simplified Chinese
	//case LANG_CHINESE:
	//	g_bVerticalCand = true;
	//	switch (GETSUBLANG())
	//	{
	//	case SUBLANG_CHINESE_SIMPLIFIED:
	//		g_pszIndicatior = g_aszIndicator[INDICATOR_CHS];
	//		//g_bVerticalCand = GetImeId() == 0;
	//		g_bVerticalCand = false;
	//		break;
	//	case SUBLANG_CHINESE_TRADITIONAL:
	//		g_pszIndicatior = g_aszIndicator[INDICATOR_CHT];
	//		break;
	//	default:	// unsupported sub-language
	//		g_pszIndicatior = g_aszIndicator[INDICATOR_NON_IME];
	//		break;
	//	}
	//	break;
	//	// Korean
	//case LANG_KOREAN:
	//	g_pszIndicatior = g_aszIndicator[INDICATOR_KOREAN];
	//	g_bVerticalCand = false;
	//	break;
	//	// Japanese
	//case LANG_JAPANESE:
	//	g_pszIndicatior = g_aszIndicator[INDICATOR_JAPANESE];
	//	g_bVerticalCand = true;
	//	break;
	//default:
	//	g_pszIndicatior = g_aszIndicator[INDICATOR_NON_IME];
	//}
	//char szCodePage[8];
	//int iRc = GetLocaleInfoA(MAKELCID(GETLANG(), SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE, szCodePage, COUNTOF(szCodePage)); iRc;
	//g_uCodePage = _strtoul(szCodePage, NULL, 0);
	//for (int i = 0; i < 256; i++)
	//{
	//	LeadByteTable[i] = (BYTE)IsDBCSLeadByteEx(g_uCodePage, (BYTE)i);
	//}
}

void TsfInputMethodStore::EnableIme(bool bEnable) {
	HIMC himcDbg;
	himcDbg = ImmAssociateContext(m_hWnd, bEnable ? m_himcOrg : NULL);
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::QueryInterface(REFIID riid, void** ppvObj) {
	if (ppvObj == NULL) return E_INVALIDARG;
	*ppvObj = NULL;

	if (ppvObj == NULL) return E_INVALIDARG;
	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfUIElementSink))
	{
		*ppvObj = (ITfUIElementSink*)this;
	}
	else if (IsEqualIID(riid, IID_ITfContextOwnerCompositionSink))
	{
		*ppvObj = (ITfContextOwnerCompositionSink*)this;
	}
	else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
	{
		*ppvObj = (ITfThreadMgrEventSink*)this;
	}

	else if (IsEqualIID(riid, IID_ITfTextEditSink))
	{
		*ppvObj = (ITfTextEditSink*)this;
	}
	//else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
	//{
	//	*ppvObj = (ITfThreadFocusSink*)this;
	//}
	else if (IsEqualIID(riid, IID_ITfReadingInformationUIElement))
	{
		*ppvObj = (ITfReadingInformationUIElement*)this;
	}
	else if (IsEqualIID(riid, IID_ITfActiveLanguageProfileNotifySink))
	{
		*ppvObj = (ITfActiveLanguageProfileNotifySink*)this;
	}
	else if (IsEqualIID(riid, IID_ITfLanguageProfileNotifySink))
	{
		*ppvObj = (ITfLanguageProfileNotifySink*)this;
	}
	else if (IsEqualIID(riid, IID_ITextStoreACP))
	{
		*ppvObj = (ITextStoreACP*)this;
	}



	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE TsfInputMethodStore::AddRef() {
	return ++m_dwRefernce;
}
ULONG STDMETHODCALLTYPE TsfInputMethodStore::Release() {
	if (--m_dwRefernce <= 0) {
		delete this;
		return 0;
	}
	else {
		return m_dwRefernce;
	}
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::BeginUIElement(
	/* [in] */ DWORD dwUIElementId,
	/* [out][in] */ BOOL* pbShow) {
	TRACE_FUNC();
	LOG_DEBUG("ITfUIElementSink");
	m_ulCandidatePageMaxSize = 0;
	FocusDocument();
	if (pbShow)
	{
		
		/*m_imeDocumentMgr->Pop(0);
		ComPtr<ITfContext> context;
		THROW_IF_FAILED(m_imeDocumentMgr->CreateContext(m_dwClientId, 0, (ITextStoreACP*)this, &context, &m_dwContextOwnerCompositionSinkCookie));
		THROW_IF_FAILED(m_imeDocumentMgr->Push(context.Get()));
		m_imeThreadMgr->AssociateFocus(m_hWnd, m_imeDocumentMgr.Get(), nullptr);
		m_imeThreadMgr->SetFocus(m_imeDocumentMgr.Get());*/
		*pbShow = FALSE;
		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
};

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::UpdateUIElement(
	/* [in] */ DWORD dwUIElementId) {
	TRACE_FUNC();
	LOG_DEBUG("ITfUIElementSink");
	//if (!m_bIsFocused) {
	//	return S_OK;
	//}

	m_vCandidates.clear();

	ComPtr<ITfUIElement> uiElement;
	if (FAILED(m_itfUIElementMgr->GetUIElement(dwUIElementId, &uiElement))) return S_OK;

	//ComPtr<ITfReadingInformationUIElement> uiReading;
	//if (SUCCEEDED(uiElement->QueryInterface(IID_ITfReadingInformationUIElement, &uiReading))) {
	//	BSTR reading = nullptr;
	//	if (SUCCEEDED(uiReading->GetString(&reading))) {
	//		m_strReading = reading;
	//		::SysFreeString(reading);
	//	}
	//}

	ComPtr<ITfCandidateListUIElement> candidatelistuielement;
	if (FAILED(uiElement->QueryInterface(IID_ITfCandidateListUIElement, &candidatelistuielement))) return S_OK;

	candidatelistuielement->GetCount(&m_ulCandidateCount);
	candidatelistuielement->GetCurrentPage(&m_ulCandidatePageIndex);
	candidatelistuielement->GetSelection(&m_ulCandidateSelect);
	candidatelistuielement->GetPageIndex(nullptr, 0, &m_ulCandidatePageCount);
	
	if (m_ulCandidatePageCount <= 0) return S_OK;

	std::vector<UINT> pages;
	pages.resize(m_ulCandidatePageCount);
	candidatelistuielement->GetPageIndex(pages.data(), (UINT)pages.size(), &m_ulCandidatePageCount);
	m_ulCandidatePageStart = pages[m_ulCandidatePageIndex];
	m_ulCandidatePageSize = (m_ulCandidatePageIndex < m_ulCandidatePageCount - 1) ?
		std::min(m_ulCandidateCount, pages[m_ulCandidatePageIndex + 1]) - m_ulCandidatePageStart :
		m_ulCandidateCount - m_ulCandidatePageStart;

	m_ulCandidatePageMaxSize = std::max(m_ulCandidatePageMaxSize, m_ulCandidatePageSize);

	UINT end = m_ulCandidateCount;
	UINT start = 0;
	if (m_ulCandidatePageIndex != m_ulCandidatePageCount - 1) {
		end = pages[m_ulCandidatePageIndex + 1];
	}
	start = pages[m_ulCandidatePageIndex];
	if (m_ulCandidatePageCount == 0) {
		end = start = 0;
	}

	for (UINT i = start; i < end; ++i)
	{
		BSTR candidate = nullptr;
		if (SUCCEEDED(candidatelistuielement->GetString(i, &candidate)))
		{
			m_vCandidates.push_back(candidate);
			::SysFreeString(candidate);
		}
	}
	return S_OK;
};

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::EndUIElement(
	/* [in] */ DWORD dwUIElementId) {
	TRACE_FUNC();
	LOG_DEBUG("ITfUIElementSink");

	ComPtr<ITfUIElement> uielement;
	if (SUCCEEDED(m_itfUIElementMgr->GetUIElement(dwUIElementId, &uielement)))
	{
		//ComPtr<ITfReadingInformationUIElement> preadingui;
		//if (SUCCEEDED(uielement->QueryInterface(IID_ITfReadingInformationUIElement, &preadingui)))
		//{
		//	m_strReading.clear();
		//}

		ComPtr<ITfCandidateListUIElement> candidatelistuielement;
		if (SUCCEEDED(uielement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidatelistuielement)))
		{
			m_vCandidates.clear();
		}
	}
	return S_OK;
};


HRESULT TsfInputMethodStore::OnEndEdit(ITfContext* pic, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord) {
	TRACE_FUNC();
	LOG_DEBUG("ITfTextEditSink");
	//HRESULT hr = S_OK;
	//ULONG readSize = 0;

	//if (m_itfEndRange) {
	//	m_strReading.clear();
	//	m_strReading.resize(1024);
	//	m_itfEndRange->GetText(ecReadOnly, TF_TF_IGNOREEND, m_strReading.data(), (ULONG)m_strReading.length(), &readSize);
	//	m_strReading.resize(readSize);

	//	m_itfEndRange.Reset();

	//	ComPtr<ITfInsertAtSelection > acp;
	//	hr = pic->QueryInterface(IID_ITfInsertAtSelection, &acp);
	//	ComPtr<ITfRange> tmpEndRange;
	//	hr = acp->InsertTextAtSelection(ecReadOnly, TF_IAS_QUERYONLY, m_strReading.c_str(), (ULONG)m_strReading.length(), &tmpEndRange);

	//	for (const auto& c : m_strReading)
	//	{
	//		PostMessage(m_hWnd, WM_CHAR, c, NULL);
	//	}
	//	return hr;
	//}
	//// get pointer to ITfContextComposition interface
	//ComPtr<ITfContextComposition> pContextComposition;
	//if (FAILED(pic->QueryInterface(IID_ITfContextComposition, &pContextComposition))) return S_OK;

	//// creates enumerator object for all compositions in context
	//ComPtr<IEnumITfCompositionView> pEnumComposition;
	//if (FAILED(pContextComposition->EnumCompositions(&pEnumComposition))) {
	//	return S_OK;
	//};

	//// obtain pointer to the composition view from the current 
	//// position to the end

	//ULONG compositionViewCount = 0;
	//ComPtr<ITfCompositionView> pCompositionView;
	//ComPtr<ITfRange> pvRangeColone;
	//while (pEnumComposition->Next(1, &pCompositionView, NULL) == S_OK) {
	//	compositionViewCount++;

	//	ComPtr<ITfRange> pvRange;

	//	// get range object and receive the text into buffer (wStr)
	//	if (FAILED(pCompositionView->GetRange(&pvRange))) {
	//		continue;
	//	}
	//	if (!pvRangeColone)
	//	{
	//		hr = pvRange->Clone(&pvRangeColone);
	//	}
	//	else
	//	{
	//		hr = pvRangeColone->ShiftEndToRange(ecReadOnly, pvRange.Get(), TF_ANCHOR_END);
	//	}

	//}
	//if (pvRangeColone) {
	//	m_strReading.clear();
	//	m_strReading.resize(1024);
	//	pvRangeColone->GetText(ecReadOnly, TF_TF_IGNOREEND, m_strReading.data(), (ULONG)m_strReading.length(), &readSize);
	//	m_strReading.resize(readSize);
	//}


	//if (compositionViewCount == 0) {
	//	do
	//	{
	//		ComPtr<ITfInsertAtSelection > acp;
	//		hr = pic->QueryInterface(IID_ITfInsertAtSelection, &acp);
	//		// acp->InsertTextAtSelection()

	//		ULONG writeSize = 0;
	//		ComPtr<ITfRange> pvRange;
	//		pic->GetStart(ecReadOnly, &pvRange);
	//		m_strReading.clear();
	//		m_strReading.resize(1024);
	//		pvRange->GetText(ecReadOnly, TF_TF_IGNOREEND, m_strReading.data(), (ULONG)m_strReading.length(), &writeSize);
	//		m_strReading.resize(writeSize);
	//		LONG shiftLen = 0;
	//		hr = pvRange->ShiftStart(ecReadOnly, writeSize, &shiftLen, nullptr);

	//	} while (false);

	//	/*for (const auto& c : m_strReading)
	//	{
	//		PostMessage(m_hWnd, WM_CHAR, c, NULL);
	//	}*/
	//	m_strReading.clear();
	//}
	return S_OK;
}


HRESULT TsfInputMethodStore::OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)
{
	TRACE_FUNC();
	LOG_DEBUG("ITfContextOwnerCompositionSink");
	*pfOk = TRUE;
	return S_OK;
}

HRESULT TsfInputMethodStore::OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew)
{
	TRACE_FUNC();
	LOG_DEBUG("ITfContextOwnerCompositionSink");
	//ComPtr<ITfRange> range;
	//ITfRange* pRange = pRangeNew;
	//if (pRangeNew == nullptr)
	//{
	//	if (FAILED(pComposition->GetRange(&range))) return S_OK;

	//	pRange = range.Get();
	//}
	//BOOL isEmpty = FALSE;
	//if (FAILED(pRange->IsEmpty(m_dwContextOwnerCompositionSinkCookie, &isEmpty)) || isEmpty) { return S_OK; };

	//// if (pRangeNew == nullptr) return S_OK;
	//ULONG retLen = 0;
	//m_strReading.resize(1024);
	//pRange->GetText(m_dwContextOwnerCompositionSinkCookie, TF_TF_MOVESTART, m_strReading.data(), (ULONG)m_strReading.length(), &retLen);
	//m_strReading.resize(retLen);
	return S_OK;
}

HRESULT TsfInputMethodStore::OnEndComposition(ITfCompositionView* pComposition)
{
	TRACE_FUNC();
	// pComposition->GetRange(&m_imeEndRange);
	// logger::get().log({}, spdlog::level::level_enum::debug, L"***** OnEndComposition: {}", string_buffer_.c_str());
	LOG_DEBUG("***** ITfContextOwnerCompositionSink: {}", logger::wstr_to_utf8(m_strContentBuffer));
	//
	//for (const auto& wc : string_buffer_)
	//{
	//	PostMessageW(m_hWnd, WM_CHAR, wc, 0);
	//}
	// Clear();
	return S_OK;
}


bool TsfInputMethodStore::OnDestroy() {
	TRACE_FUNC();
	HRESULT hr;
	ComPtr<ITfSource> source;
	hr = m_itfUIElementMgr->QueryInterface(IID_ITfSource, &source);
	source->UnadviseSink(m_dwUIElementSinkCookie);
	m_itfThreadMgr->Deactivate();

	m_itfUIElementMgr->Release();
	m_itfUIElementMgr->Release();
	return true;
}

void TsfInputMethodStore::FocusDocument() {
	if (m_itfThreadMgr == nullptr || m_itfDocumentMgr == nullptr) {
		LOG_WARN("threadmgr is null or docment mgr is null");
		return;
	}

	ComPtr<ITfDocumentMgr> imeDocumentMgr;
	if (FAILED(m_itfThreadMgr->GetFocus(&imeDocumentMgr))) return;
	if (imeDocumentMgr != m_itfDocumentMgr)
	{
		LOG_DEBUG("doc focused, {}", (std::size_t(m_itfDocumentMgr.Get())));
		m_itfThreadMgr->SetFocus(m_itfDocumentMgr.Get());
	}
}

void TsfInputMethodStore::Clear() {
	// string_buffer_.clear();
	// edit_flag_ = 0;
	m_dwContentLockType = 0;
	m_dqLockQueue.clear();
	m_iComposeCommittedSize = 0;
	m_acpContentSelection.acpStart = 0;
	m_acpContentSelection.acpEnd = 0;

	m_ulCandidateCount = 0;
	m_ulCandidatePageCount = 0;
	m_ulCandidatePageIndex = 0;
	m_ulCandidatePageStart = 0;
	m_ulCandidatePageSize = 0;
	m_ulCandidatePageMaxSize = 0;
	m_ulCandidateSelect = 0;

}

void TsfInputMethodStore::SetBufferLength(std::size_t buffer_len) {
	auto old = m_acpContentSelection.acpEnd;
	m_acpContentSelection.acpStart = buffer_len;
	m_acpContentSelection.acpEnd = buffer_len;

	TS_TEXTCHANGE txt;
	txt.acpStart = buffer_len;
	txt.acpOldEnd = buffer_len;
	txt.acpNewEnd = buffer_len;
	m_itfTextStoreACPSink_->OnTextChange(0, &txt);

	m_strContentBufferLength = buffer_len;
	m_bIsContentUpdate = true;

	
	LOG_DEBUG("SetBufferLength acpStart： {}, acpNewEnd: {}", m_acpContentSelection.acpStart, m_acpContentSelection.acpEnd);
}

void TsfInputMethodStore::SetCorsorPos(std::size_t insert_pos) {
	insert_pos = std::min(insert_pos, m_strContentBufferLength);
	m_acpContentSelection.acpStart = (LONG)insert_pos;
	m_acpContentSelection.acpEnd = (LONG)insert_pos;

	TS_TEXTCHANGE txt;
	txt.acpStart = insert_pos;
	txt.acpOldEnd = insert_pos;
	txt.acpNewEnd = insert_pos;
	m_itfTextStoreACPSink_->OnTextChange(0, &txt);
}

void TsfInputMethodStore::SetFocus(bool isFocus) {
	// if (isFocus == m_bIsFocused) return;
	m_bIsFocused = isFocus;

	// ImmAssociateContext(m_hWnd, m_bIsFocused ? m_himcOrg : nullptr);

	TRACE_FUNC_EXT("@@@@@@@@@is focus? {}", isFocus);
	LOG_DEBUG("@@@@@@@@@is focus? {}", isFocus);

	FocusDocument();

	::SetFocus(isFocus ? m_hWnd : nullptr);

	// if (!isFocus) Clear();

	ComPtr<ITfSource> source;
	if (FAILED(m_itfUIElementMgr->QueryInterface(IID_ITfSource, &source))) {
		LOG_WARN("unsink query tfsource failed");
		return;
	}
	if (!isFocus) {
		// m_himcOrg = ImmAssociateContext(m_hWnd, NULL);
		if (FAILED(source->UnadviseSink(m_dwUIElementSinkCookie))) {
			LOG_WARN("unsink UIElementSink failed, {}", m_dwUIElementSinkCookie);
		} else {
			LOG_INFO("unsink UIElementSink suc, {}", m_dwUIElementSinkCookie);
		}
		if (FAILED(source->UnadviseSink(m_dwThreadMgrEventSinkCookie))) {
			LOG_WARN("unsink ThreadMgrEventSink failed, {}", m_dwThreadMgrEventSinkCookie);
		}
		else {
			LOG_INFO("unsink ThreadMgrEventSink suc, {}", m_dwThreadMgrEventSinkCookie);
		}
		m_itfDocumentMgr->GetTop(&m_itfEditSinkContext);
		if (m_dwTextEditSinkCookie != TF_INVALID_COOKIE && m_itfEditSinkContext != nullptr) {
			ComPtr<ITfSource> source;
			m_itfEditSinkContext.As(&source);
			if (source) {
				if (FAILED(source->UnadviseSink(m_dwTextEditSinkCookie))) {
					LOG_WARN("unsink EditSink failed, {}", m_dwTextEditSinkCookie);
				}
				else {
					LOG_INFO("unsink EditSink suc, {}", m_dwTextEditSinkCookie);
					m_dwTextEditSinkCookie = TF_INVALID_COOKIE;
				}
			}
		}
	}
	else {
		// ImmAssociateContext(m_hWnd, m_himcOrg);
		source->UnadviseSink(m_dwUIElementSinkCookie);
		source->AdviseSink(IID_ITfUIElementSink, (ITfUIElementSink*)this, &m_dwUIElementSinkCookie);
		LOG_INFO("sink ITfUIElementSink suc, {}", m_dwUIElementSinkCookie);
		source->UnadviseSink(m_dwThreadMgrEventSinkCookie);
		source->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink*)this, &m_dwThreadMgrEventSinkCookie);
		LOG_INFO("sink ITfThreadMgrEventSink suc, {}", m_dwThreadMgrEventSinkCookie);
		//OnSetFocus(m_itfDocumentMgr.Get(), nullptr);
		//m_itfThreadMgr->SetFocus(m_itfDocumentMgr.Get());
		m_itfDocumentMgr->GetTop(&m_itfEditSinkContext);
		if (m_itfEditSinkContext != nullptr) {
			ComPtr<ITfSource> source;
			m_itfEditSinkContext.As(&source);
			if (source) {
				source->UnadviseSink(m_dwTextEditSinkCookie);
				source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &m_dwTextEditSinkCookie);
				LOG_INFO("sink ITfTextEditSink suc, {}", m_dwTextEditSinkCookie);
			}
		}
	}
}


/*ITfThreadMgrEventSink*/
HRESULT TsfInputMethodStore::OnInitDocumentMgr(ITfDocumentMgr* pdim) {
	TRACE_FUNC();
	return S_OK;
}

HRESULT TsfInputMethodStore::OnUninitDocumentMgr(ITfDocumentMgr* pdim) {
	TRACE_FUNC();
	return S_OK;
}

HRESULT TsfInputMethodStore::OnSetFocus(ITfDocumentMgr* pdimFocus, ITfDocumentMgr* pdimPrevFocus) {
	TRACE_FUNC();
	if (m_dwTextEditSinkCookie != TF_INVALID_COOKIE) {
		ComPtr<ITfSource> source;
		m_itfEditSinkContext.As(&source);
		if (source) {
			source->UnadviseSink(m_dwTextEditSinkCookie);
			LOG_INFO("unsink ITfTextEditSink, {}", m_dwTextEditSinkCookie);
		}
		m_dwTextEditSinkCookie = TF_INVALID_COOKIE;
		m_itfEditSinkContext.Reset();
	}
	if (pdimFocus == nullptr) {
		return 0;
	}

	pdimFocus->GetTop(&m_itfEditSinkContext);
	if (m_itfEditSinkContext != nullptr) {
		ComPtr<ITfSource> source;
		m_itfEditSinkContext.As(&source);
		if (source) {
			if (m_dwTextEditSinkCookie != TF_INVALID_COOKIE) {
				LOG_INFO("unsink ITfTextEditSink, {}", m_dwTextEditSinkCookie);
				source->UnadviseSink(m_dwTextEditSinkCookie);
			}

			source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &m_dwTextEditSinkCookie);
			LOG_INFO("sink ITfTextEditSink, {}", m_dwTextEditSinkCookie);
		}
	}
	return S_OK;
}


HRESULT TsfInputMethodStore::OnPushContext(ITfContext* pic) {
	TRACE_FUNC();
	return S_OK;
};

HRESULT TsfInputMethodStore::OnPopContext(ITfContext* pic) {
	TRACE_FUNC();
	return S_OK;
};

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::AdviseSink(REFIID riid, IUnknown* punk, DWORD dwMask) {
	TRACE_FUNC();

	if (!IsEqualGUID(riid, IID_ITextStoreACPSink))
		return S_OK;

	if (m_itfTextStoreACPSink_) {
		text_store_acp_sink_mask_ = dwMask;
		return S_OK;
	}
	;
	if (FAILED(punk->QueryInterface(IID_ITextStoreACPSink, &m_itfTextStoreACPSink_)))
		return E_UNEXPECTED;
	text_store_acp_sink_mask_ = dwMask;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::UnadviseSink(IUnknown* punk) {
	TRACE_FUNC();
	m_itfTextStoreACPSink_.Reset();
	text_store_acp_sink_mask_ = 0;
	return S_OK;
}
HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetWnd(TsViewCookie view_cookie,
	HWND* window_handle) {
	TRACE_FUNC();
	if (!window_handle)
		return E_INVALIDARG;
	/*if (view_cookie != kViewCookie)
		return E_INVALIDARG;*/
	*window_handle = m_hWnd;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::RequestLock(
	/* [in] */ DWORD dwLockFlags,
	/* [out] */ __RPC__out HRESULT* phrSession) {
	TRACE_FUNC();
	if (!m_itfTextStoreACPSink_)
		return E_FAIL;
	if (!phrSession)
		return E_INVALIDARG;

	//if (!m_bIsFocused) {
	//	*phrSession = m_itfTextStoreACPSink_->OnLockGranted(TS_S_ASYNC);
	//	return S_OK;
	//}

	if (m_dwContentLockType != 0) {
		if (dwLockFlags & TS_LF_SYNC) {
			// Can't lock synchronously.
			*phrSession = TS_E_SYNCHRONOUS;
			return S_OK;
		}
		// Queue the lock request.
		m_dqLockQueue.push_back(dwLockFlags & TS_LF_READWRITE);
		*phrSession = TS_S_ASYNC;
		return S_OK;
	}
	// Lock
	m_dwContentLockType = (dwLockFlags & TS_LF_READWRITE);
	// edit_flag_ = false;
	const size_t last_committed_size = m_iComposeCommittedSize;
	// Grant the lock.
	LOG_DEBUG("grand lock: {}", m_dwContentLockType);
	*phrSession = m_itfTextStoreACPSink_->OnLockGranted(m_dwContentLockType);
	// Unlock
	m_dwContentLockType = 0;
	// Handles the pending lock requests.
	while (!m_dqLockQueue.empty()) {
		m_dwContentLockType = m_dqLockQueue.front();
		m_dqLockQueue.pop_front();
		LOG_DEBUG("grand lock: {}", m_dwContentLockType);
		m_itfTextStoreACPSink_->OnLockGranted(m_dwContentLockType);
		m_dwContentLockType = 0;
	}
	//if (!edit_flag_) {
	//	return S_OK;
	//}
	// If the text store is edited in OnLockGranted(), we may need to call
	// TextInputClient::InsertText() or TextInputClient::SetCompositionText().
	//const size_t new_committed_size = committed_size_;
	//std::wstring new_committed_string =
	//	string_buffer_.substr(last_committed_size,
	//		new_committed_size - last_committed_size).data();
	//const std::wstring& composition_string =
	//	string_buffer_.substr(new_committed_size);
	//// If there is new committed string, calls TextInputClient::InsertText().
	//if ((!new_committed_string.empty()) && text_input_client_) {
	//	text_input_client_->InsertText(new_committed_string);
	//}
	//// Calls TextInputClient::SetCompositionText().
	//CompositionText composition_text;
	//composition_text.text = composition_string;
	//composition_text.underlines = composition_undelines_;
	//// Adjusts the offset.
	//for (size_t i = 0; i < composition_text.underlines.size(); ++i) {
	//	composition_text.underlines[i].start_offset -= new_committed_size;
	//	composition_text.underlines[i].end_offset -= new_committed_size;
	//}
	//if (selection_.start() < new_committed_size) {
	//	composition_text.selection.set_start(0);
	//}
	//else {
	//	composition_text.selection.set_start(
	//		selection_.start() - new_committed_size);
	//}
	//if (selection_.end() < new_committed_size) {
	//	composition_text.selection.set_end(0);
	//}
	//else {
	//	composition_text.selection.set_end(selection_.end() - new_committed_size);
	//}
	//if (text_input_client_)
	//	text_input_client_->SetCompositionText(composition_text);
	//// If there is no composition string, clear the text store status.
	//// And call OnSelectionChange(), OnLayoutChange(), and OnTextChange().
	//if ((composition_string.empty()) && (new_committed_size != 0)) {
	//	string_buffer_.clear();
	//	committed_size_ = 0;
	//	selection_.set_start(0);
	//	selection_.set_end(0);
	//	if (text_store_acp_sink_mask_ & TS_AS_SEL_CHANGE)
	//		text_store_acp_sink_->OnSelectionChange();
	//	if (text_store_acp_sink_mask_ & TS_AS_LAYOUT_CHANGE)
	//		text_store_acp_sink_->OnLayoutChange(TS_LC_CHANGE, 0);
	//	if (text_store_acp_sink_mask_ & TS_AS_TEXT_CHANGE) {
	//		TS_TEXTCHANGE textChange;
	//		textChange.acpStart = 0;
	//		textChange.acpOldEnd = new_committed_size;
	//		textChange.acpNewEnd = 0;
	//		text_store_acp_sink_->OnTextChange(0, &textChange);
	//	}
	//}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetStatus(
	/* [out] */ __RPC__out TS_STATUS* pdcs) {
	TRACE_FUNC();

	if (!pdcs)
		return E_INVALIDARG;
	pdcs->dwDynamicFlags = 0;
	//if (!m_bIsFocused) {
	//	pdcs->dwDynamicFlags = TS_SD_READONLY;
	//}
	// We use transitory contexts and we don't support hidden text.
	pdcs->dwStaticFlags = TS_SS_TRANSITORY | TS_SS_NOHIDDENTEXT;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::QueryInsert(
	/* [in] */ LONG acpTestStart,
	/* [in] */ LONG acpTestEnd,
	/* [in] */ ULONG cchTextSize,
	/* [out] */ __RPC__out LONG* pacpResultStart,
	/* [out] */ __RPC__out LONG* pacpResultEnd) {
	TRACE_FUNC();
	if (!pacpResultStart || !pacpResultEnd) {
		LOG_WARN("parameter is null.");
		return E_INVALIDARG;
	}
	/*if (!((static_cast<LONG>(m_iComposeCommittedSize) <= acpTestStart) &&
		(acpTestStart <= acpTestEnd) &&
		(acpTestEnd <= static_cast<LONG>(m_strContentBufferLength)))) {
		LOG_WARN("size not match.");
		return E_INVALIDARG;
	}*/
	*pacpResultStart = m_acpContentSelection.acpStart;
	*pacpResultEnd = m_acpContentSelection.acpStart + cchTextSize;
	LOG_DEBUG("query insert {}<-{}->{}", m_acpContentSelection.acpStart, cchTextSize, m_acpContentSelection.acpEnd);
	LOG_DEBUG("query insert {}<-{}->{}", *pacpResultStart, cchTextSize, *pacpResultEnd);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetSelection(
	/* [in] */ ULONG ulSelectionIndex,
	/* [in] */ ULONG ulSelectionSize,
	/* [length_is][size_is][out] */ __RPC__out_ecount_part(ulCount, *pcFetched) TS_SELECTION_ACP* pSelection,
	/* [out] */ __RPC__out ULONG* pcFetched) {
	TRACE_FUNC();
	if (!pSelection)
		return E_INVALIDARG;
	if (!pcFetched)
		return E_INVALIDARG;
	if (!HasReadLock())
		return TS_E_NOLOCK;
	*pcFetched = 0;
	if ((ulSelectionSize > 0) &&
		((ulSelectionIndex == 0) || (ulSelectionIndex == TS_DEFAULT_SELECTION))) {
		pSelection[0].acpStart = m_acpContentSelection.acpStart;
		pSelection[0].acpEnd = m_acpContentSelection.acpEnd;
		pSelection[0].style.ase = TS_AE_END;
		pSelection[0].style.fInterimChar = FALSE;
		*pcFetched = 1;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::SetSelection(
	/* [in] */ ULONG ulSelectionSize,
	/* [size_is][in] */ __RPC__in_ecount_full(ulCount) const TS_SELECTION_ACP* pSelection) {
	TRACE_FUNC();
	if (!HasReadWriteLock())
		return TF_E_NOLOCK;
	if (ulSelectionSize > 0) {
		const LONG start_pos = pSelection[0].acpStart;
		const LONG end_pos = pSelection[0].acpEnd;
		if (!((static_cast<LONG>(m_iComposeCommittedSize) <= start_pos) &&
			(start_pos <= end_pos) &&
			(end_pos <= static_cast<LONG>(m_strContentBufferLength)))) {
			return TF_E_INVALIDPOS;
		}
		m_acpContentSelection.acpStart = start_pos;
		m_acpContentSelection.acpEnd = end_pos;
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetText(
	/* [in] */ LONG acpStart,
	/* [in] */ LONG acpEnd,
	/* [length_is][size_is][out] */ __RPC__out_ecount_part(cchTextBufferSize, *pcchTextBuffer) WCHAR* pcchTextBuffer,
	/* [in] */ ULONG cchTextBufferSize,
	/* [out] */ __RPC__out ULONG* cchTextBufferCopied,
	/* [length_is][size_is][out] */ __RPC__out_ecount_part(cchRunInfoBufferSize, *pcchRunInfoBuffer) TS_RUNINFO* pcchRunInfoBuffer,
	/* [in] */ ULONG cchRunInfoBufferSize,
	/* [out] */ __RPC__out ULONG* pRunInfoBufferCopied,
	/* [out] */ __RPC__out LONG* pacpNext) {
	TRACE_FUNC_EXT("req, start:{}, end: {}, size: {}", acpStart, acpEnd, cchTextBufferSize);

	if (!cchTextBufferCopied || !pRunInfoBufferCopied)
		return E_INVALIDARG;
	if (!pcchTextBuffer && cchTextBufferSize != 0)
		return E_INVALIDARG;
	if (!pcchRunInfoBuffer && cchRunInfoBufferSize != 0)
		return E_INVALIDARG;
	if (!pacpNext)
		return E_INVALIDARG;
	if (!HasReadLock())
		return TF_E_NOLOCK;
	const LONG string_buffer_size = (LONG)m_strContentBufferLength;
	if (acpEnd == -1)
		acpEnd = string_buffer_size;
	if (!((0 <= acpStart) &&
		(acpStart <= acpEnd) &&
		(acpEnd <= string_buffer_size))) {
		return TF_E_INVALIDPOS;
	}
	try
	{
		acpEnd = std::min(acpEnd, acpStart + static_cast<LONG>(cchTextBufferSize));
		*cchTextBufferCopied = acpEnd - acpStart;
		wcsncpy_s(pcchTextBuffer, cchTextBufferSize, m_strContentBuffer, cchTextBufferSize-1);
	}
	catch (...)
	{
		STM_DEBUG() << "over buffer";
	}
	
	/*const std::wstring& result =
		string_buffer_.substr(acpStart, *cchTextBufferCopied);
	for (size_t i = 0; i < result.size(); ++i) {
		pcchTextBuffer[i] = result[i];
	}*/

	if (*cchTextBufferCopied > 0) {
		if (cchRunInfoBufferSize) {
			pcchRunInfoBuffer[0].uCount = *cchTextBufferCopied;
			pcchRunInfoBuffer[0].type = TS_RT_PLAIN;
			*pRunInfoBufferCopied = 1;
		}
	}
	else {
		pcchRunInfoBuffer = nullptr;
		*pRunInfoBufferCopied = 0;
	}

	*pacpNext = acpEnd;
	LOG_DEBUG("get text: {}, end: {}", wstring_to_utf8(pcchTextBuffer), *pacpNext);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::SetText(
	/* [in] */ DWORD dwFlags,
	/* [in] */ LONG acpStart,
	/* [in] */ LONG acpEnd,
	/* [size_is][in] */ __RPC__in_ecount_full(cch) const WCHAR* pchText,
	/* [in] */ ULONG cch,
	/* [out] */ __RPC__out TS_TEXTCHANGE* pChange) {
	TRACE_FUNC();
	LOG_DEBUG("@@@@@   start: {}, end: {}, cch_len:  {} cch: {}", acpStart, acpEnd, cch, wstring_to_utf8(pchText));
	if (!HasReadWriteLock())
		return TS_E_NOLOCK;
	//if (!((static_cast<LONG>(m_iComposeCommittedSize) <= acpStart) &&
	//	(acpStart <= acpEnd) &&
	//	(acpEnd <= static_cast<LONG>(m_strContentBufferLength)))) {
	//	return TS_E_INVALIDPOS;
	//}

	LOG_DEBUG("start: {}, end: {}, cch_len:  {}", acpStart, acpEnd, cch);
	HRESULT ret;
	m_acpContentSelection.acpStart = acpStart;
	m_acpContentSelection.acpEnd = acpEnd;

	//TS_SELECTION_ACP selection;
	//selection.acpStart = acpStart;
	//selection.acpEnd = acpEnd;
	//selection.style.ase = TS_AE_NONE;
	//selection.style.fInterimChar = 0;
	//
	//ret = SetSelection(1, &selection);
	//if (ret != S_OK)
	//	return ret;

	//if (!m_bIsFocused)
	//{
	//	string_buffer_.clear();
	//	return S_OK;
	//}
	TS_TEXTCHANGE change;
	ret = InsertTextAtSelection(0, pchText, cch,
		&acpStart, &acpEnd, &change);
	if (ret != S_OK)
		return ret;
	if (pChange)
		*pChange = change;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetFormattedText(
	/* [in] */ LONG acpStart,
	/* [in] */ LONG acpEnd,
	/* [out] */ __RPC__deref_out_opt IDataObject** ppDataObject) {
	TRACE_FUNC();
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetEmbedded(
	/* [in] */ LONG acpPos,
	/* [in] */ __RPC__in REFGUID rguidService,
	/* [in] */ __RPC__in REFIID riid,
	/* [iid_is][out] */ __RPC__deref_out_opt IUnknown** ppunk) {
	TRACE_FUNC();
	if (!ppunk)
		return E_INVALIDARG;
	*ppunk = NULL;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::QueryInsertEmbedded(
	/* [in] */ __RPC__in const GUID* pguidService,
	/* [in] */ __RPC__in const FORMATETC* pFormatEtc,
	/* [out] */ __RPC__out BOOL* pfInsertable) {
	TRACE_FUNC();
	if (!pFormatEtc)
		return E_INVALIDARG;
	// We don't support any embedded objects.
	if (pfInsertable)
		*pfInsertable = FALSE;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::InsertEmbedded(
	/* [in] */ DWORD dwFlags,
	/* [in] */ LONG acpStart,
	/* [in] */ LONG acpEnd,
	/* [in] */ __RPC__in_opt IDataObject* pDataObject,
	/* [out] */ __RPC__out TS_TEXTCHANGE* pChange) {
	TRACE_FUNC();
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::InsertTextAtSelection(
	/* [in] */ DWORD dwFlags,
	/* [size_is][in] */ __RPC__in_ecount_full(cch) const WCHAR* pchText,
	/* [in] */ ULONG cch,
	/* [out] */ __RPC__out LONG* pacpStart,
	/* [out] */ __RPC__out LONG* pacpEnd,
	/* [out] */ __RPC__out TS_TEXTCHANGE* pChange) {
	try
	{
		const LONG start_pos = m_acpContentSelection.acpStart;
		const LONG end_pos = m_acpContentSelection.acpEnd;
		const LONG new_end_pos = start_pos + cch;
		if (dwFlags & TS_IAS_QUERYONLY) {
			if (!HasReadLock())
				return TS_E_NOLOCK;
			if (pacpStart)
				*pacpStart = start_pos;
			if (pacpEnd) {
				*pacpEnd = end_pos;
			}
			return S_OK;
		}
		if (!HasReadWriteLock())
			return TS_E_NOLOCK;
		if (!pchText)
			return E_INVALIDARG;
		// DCHECK_LE(start_pos, end_pos);
		std::wstring prefix(m_strContentBuffer, start_pos);
		std::wstring insert(pchText, cch);
		std::wstring suffix(m_strContentBuffer + end_pos);
		std::wstring newcontent = prefix + insert + suffix;
		LOG_DEBUG("****** insert, pre: {}, insert: {}, suffix: {}, new: {}",
			wstring_to_utf8(prefix), wstring_to_utf8(insert), wstring_to_utf8(suffix), wstring_to_utf8(newcontent));

		m_strContentBufferLength = std::min(m_strContentBufferSize - 1, newcontent.length());
		newcontent = newcontent.substr(0, m_strContentBufferLength);
		wcsncpy_s(m_strContentBuffer, m_strContentBufferSize, newcontent.c_str(), m_strContentBufferSize -1);
		m_strContentBuffer[m_strContentBufferLength] = L'\0';
		LOG_DEBUG("****** insert, newcontent: {}, buff len: {}",
			wstring_to_utf8(m_strContentBuffer), m_strContentBufferLength);

		m_bIsContentUpdate = true;
		//string_buffer_.substr(0, start_pos) +
		//	std::wstring(pchText, pchText + cch) +
		//	string_buffer_.substr(end_pos);
		if (pacpStart)
			*pacpStart = start_pos;
		if (pacpEnd)
			*pacpEnd = new_end_pos;
		if (pChange) {
			pChange->acpStart = start_pos;
			pChange->acpOldEnd = end_pos;
			pChange->acpNewEnd = new_end_pos;
		}
		m_acpContentSelection.acpStart = start_pos;
		m_acpContentSelection.acpEnd = new_end_pos;

		TRACE_FUNC_EXT("****** insert: {}, start: {}, end: {}, content: {}", cch, *pacpStart, *pacpEnd, wstring_to_utf8(m_strContentBuffer));
	}
	catch (...)
	{
		STM_DEBUG() << "ex#############";
	}
	
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::InsertEmbeddedAtSelection(
	/* [in] */ DWORD dwFlags,
	/* [in] */ __RPC__in_opt IDataObject* pDataObject,
	/* [out] */ __RPC__out LONG* pacpStart,
	/* [out] */ __RPC__out LONG* pacpEnd,
	/* [out] */ __RPC__out TS_TEXTCHANGE* pChange) {
	TRACE_FUNC();
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::RequestSupportedAttrs(
	/* [in] */ DWORD dwFlags,
	/* [in] */ ULONG cFilterAttrs,
	/* [unique][size_is][in] */ __RPC__in_ecount_full_opt(cFilterAttrs) const TS_ATTRID* paFilterAttrs) {
	TRACE_FUNC();
	if (!paFilterAttrs)
		return E_INVALIDARG;
	// We support only input scope attribute.
	/*for (size_t i = 0; i < cFilterAttrs; ++i) {
		if (IsEqualGUID(GUID_PROP_INPUTSCOPE, paFilterAttrs[i]))
			return S_OK;
	}*/
	return E_FAIL;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::RequestAttrsAtPosition(
	/* [in] */ LONG acpPos,
	/* [in] */ ULONG cFilterAttrs,
	/* [unique][size_is][in] */ __RPC__in_ecount_full_opt(cFilterAttrs) const TS_ATTRID* paFilterAttrs,
	/* [in] */ DWORD dwFlags) {
	TRACE_FUNC();
	// We don't support any document attributes.
  // This method just returns S_OK, and the subsequently called
  // RetrieveRequestedAttrs() returns 0 as the number of supported attributes.
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::RequestAttrsTransitioningAtPosition(
	/* [in] */ LONG acpPos,
	/* [in] */ ULONG cFilterAttrs,
	/* [unique][size_is][in] */ __RPC__in_ecount_full_opt(cFilterAttrs) const TS_ATTRID* paFilterAttrs,
	/* [in] */ DWORD dwFlags) {
	TRACE_FUNC();
	// We don't support any document attributes.
  // This method just returns S_OK, and the subsequently called
  // RetrieveRequestedAttrs() returns 0 as the number of supported attributes.
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::FindNextAttrTransition(
	/* [in] */ LONG acpStart,
	/* [in] */ LONG acpHalt,
	/* [in] */ ULONG cFilterAttrs,
	/* [unique][size_is][in] */ __RPC__in_ecount_full_opt(cFilterAttrs) const TS_ATTRID* paFilterAttrs,
	/* [in] */ DWORD dwFlags,
	/* [out] */ __RPC__out LONG* pacpNext,
	/* [out] */ __RPC__out BOOL* pfFound,
	/* [out] */ __RPC__out LONG* plFoundOffset) {
	TRACE_FUNC();
	if (!pacpNext || !pfFound || !plFoundOffset)
		return E_INVALIDARG;
	// We don't support any attributes.
	// So we always return "not found".
	*pacpNext = 0;
	*pfFound = FALSE;
	*plFoundOffset = 0;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::RetrieveRequestedAttrs(
	/* [in] */ ULONG ulCount,
	/* [length_is][size_is][out] */ __RPC__out_ecount_part(ulCount, *pcFetched) TS_ATTRVAL* paAttrVals,
	/* [out] */ __RPC__out ULONG* pcFetched) {
	TRACE_FUNC();
	if (!pcFetched)
		return E_INVALIDARG;
	if (!paAttrVals)
		return E_INVALIDARG;
	// We support only input scope attribute.
	*pcFetched = 0;
	if (ulCount == 0)
		return S_OK;
	//paAttrVals[0].dwOverlapId = 0;
	//paAttrVals[0].idAttr = GUID_PROP_INPUTSCOPE;
	//paAttrVals[0].varValue.vt = VT_UNKNOWN;
	//paAttrVals[0].varValue.punkVal = tsf_inputscope::CreateInputScope(
	//	text_input_client_->GetTextInputType(), TEXT_INPUT_MODE_DEFAULT);
	//paAttrVals[0].varValue.punkVal->AddRef();
	*pcFetched = 0;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetEndACP(
	/* [out] */ __RPC__out LONG* pacp) {
	TRACE_FUNC();
	if (!pacp)
		return E_INVALIDARG;
	if (!HasReadLock())
		return TS_E_NOLOCK;
	*pacp = (LONG)m_strContentBufferLength;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetActiveView(
	/* [out] */ __RPC__out TsViewCookie* pvcView) {
	TRACE_FUNC();
	if (!pvcView)
		return E_INVALIDARG;
	// We support only one view.
	*pvcView = kViewCookie;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetACPFromPoint(
	/* [in] */ TsViewCookie vcView,
	/* [in] */ __RPC__in const POINT* ptScreen,
	/* [in] */ DWORD dwFlags,
	/* [out] */ __RPC__out LONG* pacp) {
	TRACE_FUNC();
	if (vcView != kViewCookie)
		return E_INVALIDARG;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetTextExt(
	/* [in] */ TsViewCookie vcView,
	/* [in] */ LONG acpStart,
	/* [in] */ LONG acpEnd,
	/* [out] */ __RPC__out RECT* pRect,
	/* [out] */ __RPC__out BOOL* pfClipped) {
	TRACE_FUNC();
	//if (!pRect || !pfClipped)
	//	return E_INVALIDARG;
	//if (!text_input_client_)
	//	return E_UNEXPECTED;
	//if (view_cookie != kViewCookie)
	//	return E_INVALIDARG;
	//if (!HasReadLock())
	//	return TS_E_NOLOCK;
	//if (!((static_cast<LONG>(committed_size_) <= acp_start) &&
	//	(acp_start <= acp_end) &&
	//	(acp_end <= static_cast<LONG>(string_buffer_.size())))) {
	//	return TS_E_INVALIDPOS;
	//}
	//// According to a behavior of notepad.exe and wordpad.exe, top left corner of
	//// rect indicates a first character's one, and bottom right corner of rect
	//// indicates a last character's one.
	//// We use RECT instead of gfx::Rect since left position may be bigger than
	//// right position when composition has multiple lines.
	//RECT result;
	//gfx::Rect tmp_rect;
	//const uint32_t start_pos = acp_start - committed_size_;
	//const uint32_t end_pos = acp_end - committed_size_;
	//if (start_pos == end_pos) {
	//	// According to MSDN document, if |acp_start| and |acp_end| are equal it is
	//	// OK to just return E_INVALIDARG.
	//	// http://msdn.microsoft.com/en-us/library/ms538435
	//	// But when using Pinin IME of Windows 8, this method is called with the
	//	// equal values of |acp_start| and |acp_end|. So we handle this condition.
	//	if (start_pos == 0) {
	//		if (text_input_client_->GetCompositionCharacterBounds(0, &tmp_rect)) {
	//			tmp_rect.set_width(0);
	//			result = tmp_rect.ToRECT();
	//		}
	//		else if (string_buffer_.size() == committed_size_) {
	//			result = text_input_client_->GetCaretBounds().ToRECT();
	//		}
	//		else {
	//			return TS_E_NOLAYOUT;
	//		}
	//	}
	//	else if (text_input_client_->GetCompositionCharacterBounds(start_pos - 1,
	//		&tmp_rect)) {
	//		result.left = tmp_rect.right();
	//		result.right = tmp_rect.right();
	//		result.top = tmp_rect.y();
	//		result.bottom = tmp_rect.bottom();
	//	}
	//	else {
	//		return TS_E_NOLAYOUT;
	//	}
	//}
	//else {
	//	if (text_input_client_->GetCompositionCharacterBounds(start_pos,
	//		&tmp_rect)) {
	//		result.left = tmp_rect.x();
	//		result.top = tmp_rect.y();
	//		result.right = tmp_rect.right();
	//		result.bottom = tmp_rect.bottom();
	//		if (text_input_client_->GetCompositionCharacterBounds(end_pos - 1,
	//			&tmp_rect)) {
	//			result.right = tmp_rect.right();
	//			result.bottom = tmp_rect.bottom();
	//		}
	//		else {
	//			// We may not be able to get the last character bounds, so we use the
	//			// first character bounds instead of returning TS_E_NOLAYOUT.
	//		}
	//	}
	//	else {
	//		// Hack for PPAPI flash. PPAPI flash does not support GetCaretBounds, so
	//		// it's better to return previous caret rectangle instead.
	//		// TODO(nona, kinaba): Remove this hack.
	//		if (start_pos == 0) {
	//			result = text_input_client_->GetCaretBounds().ToRECT();
	//		}
	//		else {
	//			return TS_E_NOLAYOUT;
	//		}
	//	}
	//}
	//*pRect = result;
	//*pfClipped = FALSE;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::GetScreenExt(
	/* [in] */ TsViewCookie vcView,
	/* [out] */ __RPC__out RECT* prc) {
	TRACE_FUNC();
	return E_NOTIMPL;
}

bool TsfInputMethodStore::HasReadLock() const {
	return (m_dwContentLockType & TS_LF_READ) == TS_LF_READ;
}
bool TsfInputMethodStore::HasReadWriteLock() const {
	return (m_dwContentLockType & TS_LF_READWRITE) == TS_LF_READWRITE;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL fActivated) {
	TRACE_FUNC();

	m_itfProfiles->GetCurrentLanguage(&m_langId);
	BSTR langDesc;
	if (FAILED(m_itfProfiles->GetLanguageProfileDescription(clsid, m_langId, guidProfile, &langDesc))) {
		m_strInputDescW.clear();
		m_strInputDescA.clear();
		return S_OK;
	}
	LOG_DEBUG("lang desc: {}, {}, ", wstring_to_utf8(langDesc), fActivated ? "actived" : "deactived");
	m_strInputDescW = langDesc;
	m_strInputDescA = wstring_to_utf8(langDesc);
	SysFreeString(langDesc);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::OnLanguageChange(LANGID langid, BOOL* pfAccept) {
	*pfAccept = true;
	TRACE_FUNC_EXT("++++++++++++++langId: {} changing", langid);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE TsfInputMethodStore::OnLanguageChanged(void) {
	TRACE_FUNC_EXT("+++++++++++++ lang changed");
	return S_OK;
}

bool TsfInputMethodStore::is_reading_upate() {
	bool is_update = m_bIsContentUpdate;
	m_bIsContentUpdate = false;
	return is_update;
}

// HRESULT STDMETHODCALLTYPE Win32Imm::OnEndEditTransaction(void) { return E_NOTIMPL; }