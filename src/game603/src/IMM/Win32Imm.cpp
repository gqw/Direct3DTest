#include "Win32Imm.h"


using Microsoft::WRL::ComPtr;

bool Win32Imm::OnInit(HWND hWnd) {
	ASSERT_THROW(IsWindow(hWnd), "imm init failed.");

	// CoInitialize(NULL);
	//hr = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&threadmgr);

	//// Active
	//hr = threadmgr->QueryInterface(IID_ITfThreadMgrEx, (LPVOID*)&threadmgrex);
	//hr = threadmgrex->ActivateEx(&clientid, TF_TMAE_UIELEMENTENABLEDONLY);

	// THROW_IF_FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
	Microsoft::WRL::ComPtr<ITfThreadMgr> imeThreadMgr;
	THROW_IF_FAILED(CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, &imeThreadMgr));
	imeThreadMgr->QueryInterface(IID_ITfThreadMgrEx, (LPVOID*)&m_imeThreadMgr);
	THROW_IF_FAILED(m_imeThreadMgr->ActivateEx(&m_dwClientId, TF_TMAE_UIELEMENTENABLEDONLY));


	THROW_IF_FAILED(m_imeThreadMgr->QueryInterface(IID_ITfUIElementMgr, &m_imeUIElementMgr));
	ComPtr<ITfSource> source;
	THROW_IF_FAILED(m_imeUIElementMgr->QueryInterface(IID_ITfSource, (LPVOID*)&source));
	THROW_IF_FAILED(source->AdviseSink(IID_ITfUIElementSink, (ITfUIElementSink*)this, &m_dwUIElementSinkCookie));


	ComPtr<ITfContext> context;
	THROW_IF_FAILED(m_imeThreadMgr->CreateDocumentMgr(&documentmgr));
	TfEditCookie editcookie;
	THROW_IF_FAILED(documentmgr->CreateContext(m_dwClientId, 0, (ITfContextOwnerCompositionSink*)this, &context, &editcookie));
	THROW_IF_FAILED(m_imeThreadMgr->SetFocus(documentmgr.Get()));
	THROW_IF_FAILED(documentmgr->Push(context.Get()));
	return true;
}

HRESULT STDMETHODCALLTYPE Win32Imm::QueryInterface(REFIID riid, void** ppvObj) {
	if (ppvObj == NULL) return E_INVALIDARG;
	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfUIElementSink))
	{
		*ppvObj = (ITfUIElementSink*)this;
	}
	else if (IsEqualIID(riid, IID_ITfContextOwnerCompositionSink))
	{
		AddRef();

		*ppvObj = (ITfContextOwnerCompositionSink*)this;

		return S_OK;
	}

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE Win32Imm::AddRef() {
	return ++m_dwRefernce;
}
ULONG STDMETHODCALLTYPE Win32Imm::Release() {
	if (--m_dwRefernce <= 0) {
		delete this;
		return 0;
	}
	else {
		return m_dwRefernce;
	}
}

HRESULT STDMETHODCALLTYPE Win32Imm::BeginUIElement(
	/* [in] */ DWORD dwUIElementId,
	/* [out][in] */ BOOL* pbShow) {
	if (pbShow)
	{
		*pbShow = FALSE;
		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
};

HRESULT STDMETHODCALLTYPE Win32Imm::UpdateUIElement(
	/* [in] */ DWORD dwUIElementId) {
	ComPtr<ITfUIElement> uiElement;
	if (SUCCEEDED(m_imeUIElementMgr->GetUIElement(dwUIElementId, &uiElement)))
	{
		ComPtr<ITfCandidateListUIElement> candidatelistuielement;
		if (SUCCEEDED(uiElement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidatelistuielement)))
		{
			m_vCandidates.clear();

			UINT count;
			candidatelistuielement->GetCount(&count);
			UINT pcount;
			UINT pages[10];
			candidatelistuielement->GetPageIndex(pages, 10, &pcount);

			UINT cpage;
			candidatelistuielement->GetCurrentPage(&cpage);

			UINT end = count;
			UINT start = 0;
			if (cpage != pcount - 1) {
				end = pages[cpage + 1];
			}
			start = pages[cpage];
			if (pcount == 0) {
				end = start = 0;
			}

			{
				for (UINT i = start; i < end; ++i)
				{
					BSTR candidate;
					if (SUCCEEDED(candidatelistuielement->GetString(i, &candidate)))
					{
						LPWSTR text = candidate;
						
						m_vCandidates.push_back(text);
					}
				}
			}
			//InvalidateRect(hwnd, NULL, NULL);
			//candidatelistuielement->GetSelection(&m_candidateSelection);
		}
	}

	return S_OK;
};

HRESULT STDMETHODCALLTYPE Win32Imm::EndUIElement(
	/* [in] */ DWORD dwUIElementId) {
	ComPtr<ITfUIElement> uielement;
	if (SUCCEEDED(m_imeUIElementMgr->GetUIElement(dwUIElementId, &uielement)))
	{
		ComPtr<ITfCandidateListUIElement> candidatelistuielement;
		if (SUCCEEDED(uielement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidatelistuielement)))
		{
			m_vCandidates.clear();
		}
	}
	return S_OK;
};

HRESULT Win32Imm::OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)
{
	*pfOk = TRUE;
	return S_OK;
}

HRESULT Win32Imm::OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew)
{
	return S_OK;
}

HRESULT Win32Imm::OnEndComposition(ITfCompositionView* pComposition)
{
	return S_OK;
}


bool Win32Imm::OnDestroy() {
	HRESULT hr;
	ComPtr<ITfSource> source;
	hr = m_imeUIElementMgr->QueryInterface(IID_ITfSource, &source);
	source->UnadviseSink(m_dwUIElementSinkCookie);

	m_imeUIElementMgr->Release();
	m_imeUIElementMgr->Release();
	return true;
}

LRESULT Win32Imm::IMMSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_DESTROY:
	{
		if (!ImmAssociateContextEx(hWnd, HIMC(nullptr), IACE_DEFAULT)) break;
	}
	break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (m_isOpenImm) {
			return 0;
		}
		break;
	
	case WM_IME_STARTCOMPOSITION: {
		m_isOpenImm = true;
		break;
	}
	case WM_IME_ENDCOMPOSITION: {
		m_isOpenImm = false;
		return 0;
		break;
	}
	case WM_IME_SETCONTEXT: {
		lParam = 0;
		return DefWindowProc(hWnd, uMsg, wParam, 0);
	}


	case WM_IME_COMPOSITION: {
		// OnImeComposition(hWnd, wParam, lParam);
		break;
	}

	case WM_IME_REQUEST:
		return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);

	case WM_INPUTLANGCHANGE:
		return ::DefSubclassProc(hWnd, uMsg, wParam, lParam);


	case WM_IME_NOTIFY: {
		switch (wParam)
		{
		case IMN_OPENCANDIDATE: {
			m_isShowCanditate = true;
			break;
		}
		case IMN_CHANGECANDIDATE: {
			m_isShowCanditate = true;


			break;
		}
		case IMN_PRIVATE: {
			return 0;
			break;
		}
		default:
			return 0;
		}
	}
	}
	return 1;
}

void Win32Imm::OnImeComposition(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	auto pImc = ImmGetContext(hWnd);
	if (!pImc) return;

	if (lParam & GCS_RESULTSTR) {
		m_isShowCanditate = false;
	}

	if (lParam & GCS_COMPSTR) {
		OnImeGetCompStr(hWnd, pImc, wParam, lParam);
	}
	if (lParam & GCS_COMPATTR) {
		OnImeGetCompAttr(hWnd, pImc, wParam, lParam);
	}
	ImmReleaseContext(hWnd, pImc);
}

void Win32Imm::OnImeGetCompStr(HWND hWnd, HIMC pImc, WPARAM wParam, LPARAM lParam) {
	// 查询组合词
	auto compStrLen = ImmGetCompositionStringW(pImc, GCS_COMPSTR, nullptr, 0);
	if (compStrLen < 0) return;
	auto compStrCount = compStrLen + 1;
	std::wstring strbuf(compStrCount, 0);
	auto ret = ImmGetCompositionStringW(pImc, GCS_COMPSTR, strbuf.data(), (DWORD)strbuf.length());
	if (ret == IMM_ERROR_NODATA) return;
	if (ret == IMM_ERROR_GENERAL) {
		THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
		return;
	}
	OutputDebugString(strbuf .c_str());
	OutputDebugString(L"\r\n");
}

void Win32Imm::OnImeGetCompAttr(HWND hWnd, HIMC pImc, WPARAM wParam, LPARAM lParam) {
	// 查询组合词属性
	auto compAttrLen = ImmGetCompositionStringW(pImc, GCS_COMPATTR, nullptr, 0);
	if (compAttrLen < 0) return;
	std::string attrbuf(compAttrLen, 0);
	auto ret = ImmGetCompositionStringW(pImc, GCS_COMPATTR, attrbuf.data(), (DWORD)attrbuf.length());
	if (ret == IMM_ERROR_NODATA) return;
	if (ret == IMM_ERROR_GENERAL) {
		THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
		return;
	}
	OutputDebugString(L"attr: ");
	OutputDebugStringA(std::to_string(ret).c_str());
	OutputDebugStringA(attrbuf.c_str());
	OutputDebugString(L"\r\n");
}