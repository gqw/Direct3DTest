#include "ViewManager.h"

#include "Views/ViewChat.h"
#include "Views/ViewHello.h"

bool ViewManager::RegistView(const std::shared_ptr<ViewInterface>& view) {
    ASSERT_THROW(view, "Regist view cannot nullptr.");
    auto iter = m_viewMap.find(view->Name());
    ASSERT_THROW(iter == m_viewMap.end(), "view already exists, name: " + view->Name());
    ASSERT_THROW(view->OnInit(), view->Name() + " view init failed.");
    auto iter_insert = m_viewMap.emplace(view->Name(), view);
    ASSERT_THROW(iter_insert.second, "Insert vew to map failed, " + view->Name());
    ASSERT_THROW(m_viewLists.emplace_back(iter_insert.first->second), "Insert vew to list failed, " + view->Name());
    return true;
}

bool ViewManager::OnInit() {
    RegistView(std::make_shared<ViewHello>("Hello"));
    RegistView(std::make_shared<ViewChat>("Chat"));
    return true;
}

void ViewManager::OnRender() {
	for (const auto& view : m_viewLists)
	{
		view->OnRender();
	}
}

void ViewManager::OnDestory() {
    for (const auto& view : m_viewLists)
    {
        view->OnDestroy();
    }
    m_viewLists.clear();
    m_viewMap.clear();
}