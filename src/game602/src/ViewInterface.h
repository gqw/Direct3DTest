#pragma once

class ViewInterface {
public:
    ViewInterface(std::string_view name) : m_name(name.data()) {}

    virtual std::string Name() { return m_name; };
    virtual bool OnInit() { return true; };
    virtual void OnRender() = 0;
    virtual void OnDestroy() {};

private:
    std::string m_name;
};