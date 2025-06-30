#include "UITour.hpp"

#include "ParamsDialog.hpp"
#include "I18N.hpp"
#include "ParamsPanel.hpp"
#include "GUI_App.hpp"
#include "MainFrame.hpp"
#include "GLCanvas3D.hpp"
#include "Tab.hpp"
#include "libslic3r/Utils.hpp"

namespace Slic3r { namespace GUI {
GuidePanel::GuidePanel(wxWindow* parent) : wxPanel(parent)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&) {});
    Bind(wxEVT_PAINT, &GuidePanel::OnPaint, this);
    SetDoubleBuffered(true);

    wxSizer* verSizer = new wxBoxSizer(wxVERTICAL);
    wxSizer* horSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(horSizer);

    m_MainPanel  = new wxPanel(this);
    bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    m_MainPanel->SetBackgroundColour(is_dark ? wxColor("#373739") : wxColor("#ffffff"));
    m_MainPanel->SetSizer(verSizer);
    horSizer->Add(m_MainPanel, 1, wxEXPAND | wxALL, 0);

    wxSize     imgSize(FromDIP(456), FromDIP(238));
    wxBitmap   imgTip = createScaledBitmap("userGuide_step1", m_MainPanel, 17, wxDefaultSize);
    StaticBox* imgBox = new StaticBox(m_MainPanel);
    imgBox->SetCornerRadius(6);
    imgBox->SetBorderColor(is_dark ? wxColor("#505052") : wxColor("#ffffff"));
    imgBox->SetBackgroundColor(is_dark ? wxColor("#373739") : wxColor("#e1e4e9"));
    imgBox->SetBorderWidth(is_dark ? 2 : 0);
    imgBox->SetSize(imgSize);
    imgBox->SetMinSize(imgSize);
    imgBox->SetMaxSize(imgSize);
    wxBoxSizer* imgSizer = new wxBoxSizer(wxVERTICAL);
    imgBox->SetSizer(imgSizer);

    m_StepBitmap = new wxStaticBitmap(imgBox, wxID_ANY, imgTip);
    imgSizer->AddStretchSpacer();
    imgSizer->Add(m_StepBitmap, 0, wxALL | wxALIGN_CENTRE_HORIZONTAL, 0);
    imgSizer->AddStretchSpacer();
    verSizer->Add(imgBox, 0, wxALL | wxALIGN_CENTRE_HORIZONTAL, FromDIP(12));

    const wxColour font_bg = wxGetApp().dark_mode() ? wxColour("#4B4B4D") : wxColour("#FFFFFF");
    const wxColour font_fg = wxGetApp().dark_mode() ? wxColour("#FFFFFF") : wxColour("#000000");

    std::string frontText = m_TipsFront.ToStdString();

    wxBoxSizer* tipSizerV = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* tipSizerH = new wxBoxSizer(wxHORIZONTAL);
    m_TipContent          = new wxStaticText(m_MainPanel, wxID_ANY, frontText.c_str(), wxDefaultPosition, wxDefaultSize);
    m_TipContent->SetFont(Label::Body_16);
    m_TipContent->SetForegroundColour(font_fg);

    // 小图片
    wxBitmap littleImg = create_scaled_bitmap3("wifi", m_MainPanel, 17, wxSize(24, 24));
    m_TipBitMap        = new wxStaticBitmap(m_MainPanel, wxID_ANY, littleImg);

    std::string backText = m_TipsBack.ToStdString();
    m_TipContent_back    = new wxStaticText(m_MainPanel, wxID_ANY, backText.c_str(), wxDefaultPosition, wxDefaultSize);
    m_TipContent_back->SetFont(Label::Body_16);
    m_TipContent_back->SetForegroundColour(font_fg);

    m_TipContent_back_extension = new wxStaticText(m_MainPanel, wxID_ANY, backText.c_str(), wxDefaultPosition, wxDefaultSize);
    m_TipContent_back_extension->SetFont(Label::Body_16);
    m_TipContent_back_extension->SetForegroundColour(font_fg);

    tipSizerH->Add(m_TipContent, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, FromDIP(12));
    tipSizerH->Add(m_TipBitMap, 0, wxALIGN_CENTRE_VERTICAL, FromDIP(10));
    tipSizerH->Add(m_TipContent_back, 0, wxALIGN_CENTRE_VERTICAL, FromDIP(10));

    m_TipContent_back_extension->SetMinSize(wxSize(-1, FromDIP(24)));
    m_TipContent_back_extension->SetSize(wxSize(-1, FromDIP(24)));

    tipSizerV->Add(tipSizerH, 0, wxALIGN_LEFT, 0);
    tipSizerV->Add(m_TipContent_back_extension, 0, wxEXPAND | wxLEFT, FromDIP(12));

    verSizer->Add(tipSizerV, 1, wxALL | wxEXPAND | wxALIGN_CENTER_HORIZONTAL, FromDIP(5));

    wxSizer* btns = new wxBoxSizer(wxHORIZONTAL);
    verSizer->AddStretchSpacer();
    verSizer->Add(btns, 0, wxALL | wxEXPAND | wxALIGN_CENTER_HORIZONTAL, FromDIP(10));

    wxString curStep = wxString::Format("%d/%d", m_CurStep, m_TotalStep);
    m_CurStepStatic  = new wxStaticText(m_MainPanel, wxID_ANY, curStep, wxDefaultPosition, wxDefaultSize);
    m_CurStepStatic->SetFont(Label::Body_12);
    m_CurStepStatic->SetForegroundColour(font_fg);
    btns->Add(m_CurStepStatic, 0, wxALL | wxEXPAND | wxALIGN_CENTER_HORIZONTAL, FromDIP(10));

    btns->AddStretchSpacer();

    StateColor report_bg(std::pair<wxColour, int>(wxColour(255, 255, 255), StateColor::Disabled),
                         std::pair<wxColour, int>(wxColour(206, 206, 206), StateColor::Pressed),
                         std::pair<wxColour, int>(wxColour(21, 191, 89), StateColor::Hovered),
                         std::pair<wxColour, int>(wxColour(255, 255, 255), StateColor::Enabled),
                         std::pair<wxColour, int>(wxColour(255, 255, 255), StateColor::Normal));

    StateColor report_bd(std::pair<wxColour, int>(wxColour(144, 144, 144), StateColor::Disabled),
                         std::pair<wxColour, int>(wxColour(21, 191, 89), StateColor::Hovered),
                         std::pair<wxColour, int>(wxColour(166, 172, 180), StateColor::Enabled));

    StateColor report_text(std::pair<wxColour, int>(wxColour(144, 144, 144), StateColor::Disabled),
                           std::pair<wxColour, int>(wxColour(38, 46, 48), StateColor::Enabled));

    m_SkipBtn = new Button(m_MainPanel, _L("Skip"));
    m_SkipBtn->SetBackgroundColor(report_bg);
    m_SkipBtn->SetBorderColor(report_bd);
    m_SkipBtn->SetTextColor(report_text);
    m_SkipBtn->SetFont(Label::Body_12);
    m_SkipBtn->SetCornerRadius(FromDIP(4));
    m_SkipBtn->SetMinSize(wxSize(FromDIP(72), FromDIP(32)));
    m_SkipBtn->Bind(wxEVT_BUTTON, ([this](wxCommandEvent e) {
                        UITour* tour = dynamic_cast<UITour*>(GetParent());
                        if (tour) {
                            tour->End();
                        }
                    }));
    m_PreBtn = new Button(m_MainPanel, _L("Previous"));
    m_PreBtn->SetBackgroundColor(report_bg);
    m_PreBtn->SetBorderColor(report_bd);
    m_PreBtn->SetTextColor(report_text);
    m_PreBtn->SetFont(Label::Body_12);
    m_PreBtn->SetCornerRadius(FromDIP(4));
    m_PreBtn->SetMinSize(wxSize(FromDIP(72), FromDIP(32)));
    m_PreBtn->Bind(wxEVT_BUTTON, ([this](wxCommandEvent e) {
                       UITour* tour = dynamic_cast<UITour*>(GetParent());
                       if (tour) {
                           tour->Previous();
                       }
                   }));

    m_NextBtn = new Button(m_MainPanel, _L("Next"));
    m_NextBtn->SetBackgroundColor(report_bg);
    m_NextBtn->SetBorderColor(report_bd);
    m_NextBtn->SetTextColor(report_text);
    m_NextBtn->SetFont(Label::Body_12);
    m_NextBtn->SetCornerRadius(FromDIP(4));
    m_NextBtn->SetMinSize(wxSize(FromDIP(72), FromDIP(32)));
    m_NextBtn->Bind(wxEVT_BUTTON, ([this](wxCommandEvent e) {
                        UITour* tour = dynamic_cast<UITour*>(GetParent());
                        if (tour) {
                            m_CurStep == m_TotalStep - 1 ? tour->End() : tour->Next();
                        }
                    }));

    btns->Add(m_SkipBtn, 0, wxALIGN_CENTRE_VERTICAL | wxRIGHT, FromDIP(16));
    btns->Add(m_PreBtn, 0, wxALIGN_CENTRE_VERTICAL | wxRIGHT, FromDIP(16));
    btns->Add(m_NextBtn, 0, wxALIGN_CENTRE_VERTICAL | wxRIGHT, FromDIP(16));
    

}

GuidePanel::~GuidePanel() {}

void GuidePanel::UpdateUI(wxRect          pos,
                          int             curStep,
                          int             totalSteps,
                          const wxString& tipsFront,
                          const wxString& tipsBack,
                          const wxString& tipsImg,
                          const wxString& littleImg,
                          wxDirection     arrowDir,
                          wxWindow*       item)
{
    m_TotalStep  = totalSteps;
    m_CurStep    = curStep;
    m_TipsFront  = tipsFront;
    m_TipsBack   = tipsBack;
    m_Img        = tipsImg;
    m_Pos        = pos;
    m_ArrowDir   = arrowDir;
    m_TargetItem = item;

    // if (m_CurStep == 0)
    m_PreBtn->Enable(m_CurStep == 0 ? false : true);
    // m_NextBtn->Enable(m_CurStep == m_TotalStep-1 ? false : true);
    m_CurStep == m_TotalStep - 1 ? m_NextBtn->SetLabel(_L("Finish")) : m_NextBtn->SetLabel(_L("Next"));

    wxSize imgSize;
    if (m_CurStep == 2) {
        imgSize = wxSize(FromDIP(328), FromDIP(234));
    } else if (m_CurStep == 3) {
        imgSize = wxSize(FromDIP(390), FromDIP(180));
    } else {
        imgSize = wxSize(FromDIP(448), FromDIP(234));
    }

    wxBitmap bitMap = createScaledBitmap(m_Img.ToStdString(), m_MainPanel, 17, imgSize);

    wxImage scaledImg = bitMap.ConvertToImage();
    scaledImg.Rescale(imgSize.GetWidth(), imgSize.GetHeight(), wxIMAGE_QUALITY_BICUBIC);
    wxBitmap scaledMap(scaledImg);

    // 更新图片
    m_StepBitmap->SetBitmap(scaledMap);
    m_StepBitmap->Refresh();
    m_StepBitmap->Update();
    m_StepBitmap->Layout();
    // 更新步骤
    wxString stepText = wxString::Format("%d/%d", m_CurStep + 1, m_TotalStep);
    m_CurStepStatic->SetLabelText(stepText.ToStdString());
    // 更新提示内容
    if (m_TipsBack.empty()) {
        m_TipBitMap->Hide();
    } else {
        wxBitmap littleBitMap = create_scaled_bitmap3(littleImg.ToStdString(), m_MainPanel, 17, wxSize(24, 24));
        m_TipBitMap->SetBitmap(littleBitMap);
        m_TipBitMap->Refresh();
        m_TipBitMap->Update();
        m_TipBitMap->Show();
    }

    m_TipContent->SetLabelText(m_TipsFront.ToStdString());
    std::string lang = wxGetApp().app_config->get("language");

    if (m_CurStep == 1) {
        wxString firstFive = lang == "zh_CN" ? m_TipsBack : m_TipsBack.SubString(0, m_TipsBack.Length() - 8);
        wxString remaining = lang == "zh_CN" ? "" : m_TipsBack.SubString(m_TipsBack.Length() - 7, m_TipsBack.Length());
        m_TipContent_back->SetLabelText(firstFive.ToStdString());
        m_TipContent_back_extension->SetLabelText(remaining.ToStdString());
    } else if (m_CurStep == 3) {
        wxString firstFive = lang == "zh_CN" ? m_TipsBack : m_TipsBack.SubString(0, 27);
        wxString remaining = lang == "zh_CN" ? "" : m_TipsBack.SubString(28, m_TipsBack.Length());
        m_TipContent_back->SetLabelText(firstFive.ToStdString());
        m_TipContent_back_extension->SetLabelText(remaining.ToStdString());
    } else if (m_CurStep == 5) {
        wxString firstFive = lang == "zh_CN" ? m_TipsFront : m_TipsFront.SubString(0, m_TipsFront.Length() - 15);
        wxString remaining = lang == "zh_CN" ? "" : m_TipsFront.SubString(m_TipsFront.Length() - 14, m_TipsFront.Length());
        m_TipContent->SetLabelText(firstFive.ToStdString());
        m_TipContent_back_extension->SetLabelText(remaining.ToStdString());
    } else {
        m_TipContent_back->SetLabelText(m_TipsBack.ToStdString());
        m_TipContent_back_extension->SetLabelText("");
    }

    int xPos = m_Pos.x + m_Pos.width;
    int yPos = m_Pos.y + m_Pos.height;
    switch (m_ArrowDir) {
    case wxLEFT:
        xPos = m_Pos.x - this->GetSize().GetWidth();
        yPos = m_Pos.y + 40;
        break;
    case wxRIGHT:
        xPos = m_Pos.x + m_Pos.width;
        yPos = m_Pos.y + 40;
        break;
    case wxTOP:
        xPos = m_Pos.x + (m_Pos.width - this->GetSize().GetWidth()) / 2;
        yPos = m_Pos.y - this->GetSize().GetHeight() - FromDIP(20);
        break;
    case wxBOTTOM:
        xPos = m_Pos.x + (m_Pos.width - this->GetSize().GetHeight()) / 2;
        yPos = m_Pos.y + m_Pos.height;
        break;
    default: break;
    }
    SetPosition(wxPoint(xPos, yPos));
    if (m_CurStep > m_TotalStep) {
        m_CurStep = 0;
        return;
    }

    Layout();
    Refresh();
}

void GuidePanel::OnPaint(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    // 绘制背景图片（如果有）

    wxBitmap map = create_scaled_bitmap3("transparent_ams_item", this, 17, wxSize(478, 378));
    if (map.IsOk()) {
        dc.DrawBitmap(map, 0, 0, true);
    } else {
        // 如果没有背景图片，绘制默认背景
        dc.SetBackground(*wxTRANSPARENT_BRUSH);
        dc.Clear();
    }
}

void GuidePanel::DrawArrow(wxGraphicsContext* gc, const wxPoint& start, const wxPoint& end)
{
    // 计算箭头方向向量
    wxPoint dir    = end - start;
    double  length = sqrt(dir.x * dir.x + dir.y * dir.y);
    dir.x          = dir.x / length * 20; // 标准化并缩放
    dir.y          = dir.y / length * 20;

    // 绘制箭头线
    gc->SetPen(wxPen(*wxRED, 3));
    gc->StrokeLine(start.x, start.y, end.x, end.y);

    // 绘制箭头头部（三角形）
    wxGraphicsPath path = gc->CreatePath();
    path.MoveToPoint(end.x, end.y);
    path.AddLineToPoint(end.x - dir.x + dir.y * 0.3, end.y - dir.y - dir.x * 0.3);
    path.AddLineToPoint(end.x - dir.x - dir.y * 0.3, end.y - dir.y + dir.x * 0.3);
    path.CloseSubpath();
    gc->SetBrush(gc->CreateBrush(wxColour(255, 0, 0)));
    gc->FillPath(path);
}

wxBitmap GuidePanel::createScaledBitmap(const std::string& bmp_name_in, wxWindow* win, const int px_cnt, const wxSize imgSize)
{
    bool        is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    std::string lang    = wxGetApp().app_config->get("language");
    wxString    imgPath = wxString::Format("%s_%s_%s", bmp_name_in, lang == "zh_CN" ? "zh" : "en", is_dark ? "dark" : "light");
    return create_scaled_bitmap3(imgPath.ToStdString(), win, 17, imgSize);
}

UITour::UITour(wxWindow* parent) : 
    wxWindow(parent, wxID_ANY)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&) {});
    Bind(wxEVT_PAINT, &UITour::OnPaint, this);
    // Bind(wxEVT_LEFT_DOWN, &UITour::OnClick, this);

    m_GuidePanel = new GuidePanel(this);
    m_GuidePanel->Hide();
}

UITour::~UITour()
{

}

void UITour::AddStep(wxWindow* target, const wxString& text, wxDirection arrowDir)
{
    // steps.insert({wxRect(), target, text, arrowDir});
}

void UITour::AddStep(int             index,
                     wxRect          target,
                     const wxString& frontText,
                     const wxString& backText,
                     const wxString& stepImg,
                     const wxString& littleImg,
                     wxDirection     arrowDir,
                     wxWindow*       item)
{
    auto it = steps.find(index);
    if (it != steps.end())
        return;
    steps.insert(std::make_pair(index, TourStep{target, frontText, backText, stepImg, littleImg, arrowDir, item}));
}

void UITour::Start()
{
    if (steps.empty())
        return;
     #ifdef WIN32
        SetForegroundWindow(GetParent()->GetHWND());
        GetParent()->Center();
        getScreenShotCut();
    #endif
    
    m_CurrentStep = 0;
    if (m_GuidePanel) {
        m_GuidePanel->SetSize(FromDIP(480), FromDIP(378));
        RefreshRes();
        m_GuidePanel->Show();
    }

    {
        wxSize size = m_parent->GetSize();
        SetSize(size);
        SetPosition(wxPoint(0, 0));
        Show();
        Raise();
    }

}

void UITour::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);

#ifndef __APPLE__
    dc.DrawBitmap(m_backgroundMap, 0, 0);
    dc.DrawBitmap(m_platerMap, m_platerPosition.x, m_platerPosition.y);
#endif

    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    if (gc) {
        wxGraphicsPath path = gc->CreatePath();
        path.AddRectangle(0, 0, GetSize().x, GetSize().y);

        auto it = steps.find((m_CurrentStep));
        if (it == steps.end()) {
            return;
        }

        TourStep stepValue = (*it).second;
        wxRect   rect      = stepValue.rect;
        path.AddRectangle(rect.x, rect.y, rect.width, rect.height);

        // 设置填充规则为奇偶规则
        gc->SetBrush(gc->CreateBrush(wxColour(0, 0, 0, 180)));
        gc->FillPath(path, wxODDEVEN_RULE);

        delete gc;
    }
}

void UITour::OnClick(wxMouseEvent& evt)
{
    // this->Hide();
    Next();
    // this->Show();
}

void UITour::Previous()
{
    // this->Hide();
    if (--m_CurrentStep < 0) {
        End();
    } else {
        auto it = steps.find(m_CurrentStep);
        if (it != steps.end()) {
            TourStep stepInfo = (*it).second;
            m_GuidePanel->UpdateUI(GetCurrentRect(), m_CurrentStep, steps.size(), stepInfo.frontText, stepInfo.backText, stepInfo.guideImg,
                                   stepInfo.littleImg, stepInfo.arrowDir);
        }
    }

     Refresh();
    // Update();
    // this->Show();
}

void UITour::Next()
{
     //this->Hide();
    if (++m_CurrentStep >= steps.size()) {
        End();
    } else {
        auto it = steps.find(m_CurrentStep);
        if (it != steps.end()) {
            TourStep stepInfo = (*it).second;
            m_GuidePanel->UpdateUI(GetCurrentRect(), m_CurrentStep, steps.size(), stepInfo.frontText, stepInfo.backText, stepInfo.guideImg,
                                   stepInfo.littleImg, stepInfo.arrowDir);
        }
    }

     Refresh();
    // Update();
     //this->Show();
}

void UITour::End()
{
#ifndef __APPLE__
    wxWindow* parent = GetParent();
    if (parent) {
        MainFrame* mainFrame = static_cast<MainFrame*>(parent);
        mainFrame->set_content_visible(true);
        // mainFrame->Maximize();
    }
#endif // !1

    Hide();
    m_GuidePanel->Hide();
    m_CurrentStep = 0;
    wxGetApp().app_config->set("is_first_install", "1");
    wxGetApp().check_creality_privacy_version();
}

 static UITour* uiTour = nullptr;
UITour& UITour::Instance()
{
    //if (!uiTour) {
    //    wxWindow* parent = wxGetApp().mainframe;
    //    if (!parent) {
    //        parent = new wxWindow(nullptr, wxID_ANY);
    //        parent->Hide();
    //    }
    //    uiTour = new UITour(parent);
    //}
    //return *uiTour;
    static UITour uitour(wxGetApp().mainframe);
    return uitour;
}

void UITour::RefreshRes()
{
    auto it = steps.find(m_CurrentStep);
    if (it != steps.end()) {
        TourStep stepInfo = (*it).second;
        m_GuidePanel->UpdateUI(GetCurrentRect(), m_CurrentStep, steps.size(), stepInfo.frontText, stepInfo.backText, stepInfo.guideImg,
                               stepInfo.littleImg, stepInfo.arrowDir);
        Update();
    }
}

void UITour::deleteStep(int index)
{
    auto it = steps.find(index);
    if (it != steps.end()) {
        steps.erase(it);
    }
}

wxRect UITour::GetCurrentRect() const
{
    auto it = steps.find((m_CurrentStep));
    if (it == steps.end()) {
        return wxRect();
    }

    TourStep stepValue = (*it).second;
    wxRect   rect      = stepValue.rect;
    if (stepValue.target) {
        rect              = stepValue.target->GetRect();
        wxPoint screenPos = stepValue.target->GetScreenPosition();
        rect              = wxRect(screenPos.x, screenPos.y + FromDIP(6), rect.width, rect.height);
    }

    return rect;
}

// void UITour::on_dpi_changed(const wxRect& suggested_rect)
//{
//     //Fit();
//     //SetSize({75 * em_unit(), 60 * em_unit()});
//     //m_panel->msw_rescale();
//     //Refresh();
// }

void UITour::getScreenShotCut()
{
    MainFrame* mainFrame = static_cast<MainFrame*>(GetParent());
    wxImage image;
    auto       plater = mainFrame->plater();
    plater->canvas3D()->render_on_image(image);
    m_platerMap  = wxBitmap(image);
    m_platerPosition = wxPoint();
    wxWindow* parent = plater;
    while (parent != mainFrame && parent != NULL)
    {
        m_platerPosition += parent->GetPosition();
        parent = parent->GetParent();
    }

    auto       parentSize = GetParent()->GetSize();
    wxBitmap   bitmap(parentSize.GetWidth(), parentSize.GetHeight());
    wxMemoryDC memDC;
    memDC.SelectObject(bitmap);
    wxClientDC clientDC(GetParent());
    memDC.Blit(0, 0, parentSize.GetWidth(), parentSize.GetHeight(), &clientDC, 0, 0);
    m_backgroundMap = bitmap;

    //int borderSize       = wxSystemSettings::GetMetric(wxSYS_BORDER_X);    // 获取水平边框大小
    //int captionSize      = wxSystemSettings::GetMetric(wxSYS_CAPTION_Y);   // 获取标题栏高度
    //int frameBorderSize  = wxSystemSettings::GetMetric(wxSYS_FRAMESIZE_X); // 获取框架边缘大小（包括标题栏和边框）
    //int frameBorderSizeY = wxSystemSettings::GetMetric(wxSYS_FRAMESIZE_Y); // 获取框架边缘大小（包括标题栏和边框）
    //                                                                       // 获取窗口位置和大小
    //wxRect rect = GetParent()->GetScreenRect();                            // 使用GetScreenRect获取屏幕坐标

    //auto parentSize = GetParent()->GetSize();
    //#ifdef WIN32
    //SetForegroundWindow(GetParent()->GetHWND());
    //#endif
    //// 创建屏幕DC
    //wxScreenDC screenDC;
    //wxBitmap   screenshot(rect.width - frameBorderSize * 2, rect.height - frameBorderSizeY * 2);


    //// 创建内存DC并截图
    //wxMemoryDC memDC;
    //memDC.SelectObject(screenshot);
    //memDC.Blit(0, 0, parentSize.GetWidth(), rect.height, &screenDC, rect.x + frameBorderSize, rect.y + frameBorderSizeY);
    //memDC.SelectObject(wxNullBitmap); // 必须取消选择

    //// 保存截图
    //screenshot.SaveFile("screenshot.png", wxBITMAP_TYPE_PNG);
    //m_backgroundMap = screenshot;
}
}} // namespace Slic3r::GUI