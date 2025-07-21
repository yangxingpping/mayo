/****************************************************************************
** Copyright (c) 2022, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "app_context_new.h"

#include "ribbon_main_window.h"
#include "ui_ribbon_main_window.h"
#include "widget_gui_document.h"
#include "widget_main_control.h"
#include "widget_main_home.h"

#include <cassert>

namespace Mayo {

AppContextNew::AppContextNew(RibbonMainWindow* wnd)
    : IAppContext(wnd),
      m_wnd(wnd)
{
    assert(m_wnd != nullptr);
    assert(m_wnd->widgetPageDocuments() != nullptr);

    QObject::connect(
        m_wnd->widgetPageDocuments(), &WidgetMainControl::currentDocumentIndexChanged,
        this, &AppContextNew::onCurrentDocumentIndexChanged
    );
}

GuiApplication* AppContextNew::guiApp() const
{
    return m_wnd->m_guiApp;
}

TaskManager* AppContextNew::taskMgr() const
{
    return &m_wnd->m_taskMgr;
}

QWidget* AppContextNew::pageDocuments_widgetLeftSideBar() const
{
    const WidgetMainControl* pageDocs = m_wnd->widgetPageDocuments();
    return pageDocs ? pageDocs->widgetLeftSideBar() : nullptr;
}

QWidget* AppContextNew::widgetMain() const
{
    return m_wnd;
}

QWidget* AppContextNew::widgetPage(Page page) const
{
    if (page == Page::Home)
        return m_wnd->widgetPageHome();
    else if (page == Page::Documents)
        return m_wnd->widgetPageDocuments();
    else
        return nullptr;
}

IAppContext::Page AppContextNew::currentPage() const
{
    auto widget = m_wnd->m_ui->stack_Main->currentWidget();
    if (widget == m_wnd->widgetPageHome())
        return Page::Home;
    else if (widget == m_wnd->widgetPageDocuments())
        return Page::Documents;

    return Page::Unknown;
}

void AppContextNew::setCurrentPage(Page page)
{
    QWidget* widgetPage = this->widgetPage(page);
    assert(widgetPage);
    m_wnd->m_ui->stack_Main->setCurrentWidget(widgetPage);
}

V3dViewController* AppContextNew::v3dViewController(const GuiDocument* guiDoc) const
{
    auto widgetDoc = this->findWidgetGuiDocument([=](WidgetGuiDocument* candidate) {
        return candidate->guiDocument() == guiDoc;
    });
    return widgetDoc ? widgetDoc->controller() : nullptr;
}

int AppContextNew::findDocumentIndex(Document::Identifier docId) const
{
    int index = -1;
    auto widgetDoc = this->findWidgetGuiDocument([&](WidgetGuiDocument* candidate) {
        ++index;
        return candidate->documentIdentifier() == docId;
    });
    return widgetDoc ? index : -1;
}

Document::Identifier AppContextNew::findDocumentFromIndex(int index) const
{
    auto widgetDoc = this->widgetGuiDocument(index);
    return widgetDoc ? widgetDoc->documentIdentifier() : -1;
}

Document::Identifier AppContextNew::currentDocument() const
{
    const int index = m_wnd->widgetPageDocuments()->currentDocumentIndex();
    auto widgetDoc = this->widgetGuiDocument(index);
    return widgetDoc ? widgetDoc->documentIdentifier() : -1;
}

void AppContextNew::setCurrentDocument(Document::Identifier docId)
{
    auto widgetDoc = this->findWidgetGuiDocument([=](WidgetGuiDocument* widgetDoc) {
        return widgetDoc->documentIdentifier() == docId;
    });
    const int docIndex = m_wnd->widgetPageDocuments()->indexOfWidgetGuiDocument(widgetDoc);
    m_wnd->widgetPageDocuments()->setCurrentDocumentIndex(docIndex);
}

void AppContextNew::updateControlsEnabledStatus()
{
    m_wnd->updateControlsActivation();
}

WidgetGuiDocument* AppContextNew::widgetGuiDocument(int idx) const
{
    return m_wnd->widgetPageDocuments()->widgetGuiDocument(idx);
}

WidgetGuiDocument* AppContextNew::findWidgetGuiDocument(std::function<bool(WidgetGuiDocument*)> fn) const
{
    const int widgetCount = m_wnd->widgetPageDocuments()->widgetGuiDocumentCount();
    for (int i = 0; i < widgetCount; ++i) {
        auto candidate = this->widgetGuiDocument(i);
        if (candidate && fn(candidate))
            return candidate;
    }

    return nullptr;
}

void AppContextNew::onCurrentDocumentIndexChanged(int docIndex)
{
    auto widgetDoc = this->widgetGuiDocument(docIndex);
    emit this->currentDocumentChanged(widgetDoc ? widgetDoc->documentIdentifier() : -1);
}

} // namespace Mayo
