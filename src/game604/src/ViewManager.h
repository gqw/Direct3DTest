#pragma once
#include "pch.h"

#include "ViewInterface.h"

class ViewManager {
public:
    static ViewManager& Inst() { static ViewManager mgr; return mgr; }

    bool RegistView(const std::shared_ptr<ViewInterface>& view);

    bool OnInit(HWND hwnd);
    void OnRender();
    void OnDestory();

private:
    std::list<std::shared_ptr<ViewInterface>> m_viewLists;
    std::map<std::string, std::shared_ptr<ViewInterface>> m_viewMap;
};