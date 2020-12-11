
#include "meow_textapp.h"


MeowTextApp::MeowTextApp(HWND _hwnd)
{
	hwnd = _hwnd;
	reference = 1;

	CoInitialize(NULL);
	HRESULT hr;
	
	hr= CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr, (void**)&threadmgr);

	// Active
	hr = threadmgr->QueryInterface(IID_ITfThreadMgrEx, (LPVOID*)&threadmgrex);
	hr = threadmgrex->ActivateEx(&clientid, TF_TMAE_UIELEMENTENABLEDONLY);

	// ITfUIElementSink
	hr = threadmgr->QueryInterface(IID_ITfUIElementMgr, (LPVOID*)&uielementmgr);
	CComPtr<ITfSource> source;
	hr = uielementmgr->QueryInterface(IID_ITfSource, (LPVOID*)&source);
	hr = source->AdviseSink(IID_ITfUIElementSink, (ITfUIElementSink*)this, &uielementsinkcookie);
	// hr = source->AdviseSink(IID_ITfContextOwnerCompositionSink, (ITfContextOwnerCompositionSink*)this, &_contextCookie);

	hr = source->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink*)this, &threadMgrEventSinkCookie);
	if (hr != S_OK) {
		return;
	}
	hr = threadmgr->CreateDocumentMgr(&documentmgr);
	
	CComPtr<ITfContext> context;
	hr = documentmgr->CreateContext(clientid, 0, (ITfContextOwnerCompositionSink*)this, &context, &editcookie);
	hr = threadmgr->SetFocus(documentmgr.Get());
	hr = documentmgr->Push(context);

	//CComPtr<ITfDocumentMgr> documentmgr;
	

	/*
	ITfContext * contentx;
	documentmgr->GetTop(&contentx);

	contentx->AddRef();
	ULONG x = contentx->Release();


	documentmgr->GetTop(&contentx);
	documentmgr->GetTop(&contentx);
	documentmgr->GetTop(&contentx);
	documentmgr->GetTop(&contentx);


	contentx->AddRef();
	x = contentx->Release();
	*/
}

VOID MeowTextApp::SetFocus() {

}

HRESULT STDMETHODCALLTYPE MeowTextApp::OnSetFocus(
	/* [in] */ __RPC__in_opt ITfDocumentMgr* pdimFocus,
	/* [in] */ __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) {
	if (_textEditSinkCookie != -1) {
		Microsoft::WRL::ComPtr<ITfSource> source;
		_editSinkContext.As(&source);
		_editSinkContext.Reset();
		_textEditSinkCookie = -1;
	}
	if (pdimPrevFocus == nullptr) {
		return 0;
	}
	
	pdimPrevFocus->GetTop(&_editSinkContext);
	if (_editSinkContext != nullptr) {
		Microsoft::WRL::ComPtr<ITfSource> source;
		_editSinkContext.As(&source);
		source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &_textEditSinkCookie);
	}
	return 0;
}


MeowTextApp::~MeowTextApp()
{
	HRESULT hr;
	CComPtr<ITfSource> source;
	hr = uielementmgr->QueryInterface(IID_ITfSource, (LPVOID*)&source);
	source->UnadviseSink(uielementsinkcookie);

	uielementmgr->Release();

	threadmgrex->Release();
	threadmgr->Release();
}


HRESULT STDMETHODCALLTYPE MeowTextApp::QueryInterface(REFIID riid, void **ppvObj) {
	if (ppvObj == NULL) return E_INVALIDARG;
	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfUIElementSink))
	{
		*ppvObj = (ITfUIElementSink *)this;
	}
	else if (IsEqualIID(riid, IID_ITfContextOwnerCompositionSink))
	{
		AddRef();

		*ppvObj = (ITfContextOwnerCompositionSink*)this;

		return S_OK;
	}
	else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
	{
		AddRef();
		*ppvObj = (ITfThreadMgrEventSink*)this;
		return S_OK;
	}
	
	else if (IsEqualIID(riid, IID_ITfTextEditSink))
	{
		*ppvObj = (ITfTextEditSink*)this;
	}
	else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
	{
		*ppvObj = (ITfThreadFocusSink*)this;
	}

	if (*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}
ULONG STDMETHODCALLTYPE MeowTextApp::AddRef() {
	return ++reference;
}
ULONG STDMETHODCALLTYPE MeowTextApp::Release() {
	if (--reference <= 0) delete this;
	return reference;
}


HRESULT MeowTextApp::BeginUIElement(DWORD dwUIElementId, BOOL *pbShow)
{
 	if (pbShow)
	{
		*pbShow = FALSE;

		//CComPtr<ITfContext> context;
		//HRESULT hr;
		// hr = documentmgr->CreateContext(clientid, 0, (ITfContextOwnerCompositionSink*)this, &context, &editcookie);
		// hr = threadmgr->SetFocus(documentmgr.Get());
		// hr = documentmgr->Push(context);

		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
}

HRESULT MeowTextApp::UpdateUIElement(DWORD dwUIElementId)
{
	CComPtr<ITfUIElement> uiElement;
	if (SUCCEEDED(uielementmgr->GetUIElement(dwUIElementId, &uiElement)))
	{
		CComPtr<ITfCandidateListUIElement> candidatelistuielement;
		if (SUCCEEDED(uiElement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidatelistuielement)))
		{
			candidatelist.clear();

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
				for (UINT i = start; i<end; ++i)
				{
					CComBSTR candidate;
					if (SUCCEEDED(candidatelistuielement->GetString(i, &candidate)))
					{
						LPWSTR text = candidate;

						candidatelist.push_back(text);
					}
				}
			}

			

		

			InvalidateRect(hwnd, NULL, NULL);
			candidatelistuielement->GetSelection(&m_candidateSelection);
		}
	}

	return S_OK;
}

HRESULT MeowTextApp::EndUIElement(DWORD dwUIElementId)
{
	CComPtr<ITfUIElement> uielement;
	if (SUCCEEDED(uielementmgr->GetUIElement(dwUIElementId, &uielement)))
	{
		CComPtr<ITfCandidateListUIElement> candidatelistuielement;
		if (SUCCEEDED(uielement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidatelistuielement)))
		{
			candidatelist.clear();
		}
	}
	auto hr = threadmgr->SetFocus(documentmgr.Get());
	return S_OK;
}

HRESULT MeowTextApp::OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk)
{
	*pfOk = true;
	return S_OK;
}

HRESULT MeowTextApp::OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew)
{
	return S_OK;
}

HRESULT MeowTextApp::OnEndComposition(ITfCompositionView *pComposition)
{
	return S_OK;
}
