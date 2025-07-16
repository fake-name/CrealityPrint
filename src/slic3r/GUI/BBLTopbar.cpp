#include "BBLTopbar.hpp"
#include "wx/artprov.h"
#include "wx/aui/framemanager.h"
#include "wx/display.h"
#include "I18N.hpp"
#include "GUI_App.hpp"
#include "GUI.hpp"
#include "wxExtensions.hpp"
#include "Plater.hpp"
#include "MainFrame.hpp"
#include "WebViewDialog.hpp"
#include "PartPlate.hpp"

#include <boost/log/trivial.hpp>
#include <wx/dcgraph.h>
#include "Notebook.hpp"
#include "libslic3r/common_header/common_header.h"
#define TOPBAR_ICON_SIZE  17

// original is 300, in some screen scale setting case(for example 175%), make the topbar too long
#define TOPBAR_TITLE_WIDTH  FromDIP(80)

static long UPLOAD_BTN_CODE = 12123;
static long HOME_BTN_CODE_CHECKED = 12124;
static long HOME_BTN_CODE_UNCHECKED = 12125;

using namespace Slic3r;
class ButtonsCtrl : public wxControl
{
public:
    // BBS
    ButtonsCtrl(wxWindow* parent, wxBoxSizer* side_tools = NULL);
    ~ButtonsCtrl() {}

    void SetSelection(int sel);
    int GetSelection();
    bool InsertPage(size_t n, const wxString& text, bool bSelect = false, const std::string& bmp_name = "", const std::string& inactive_bmp_name = "");
    void RefreshColor();
    void reLayout();
private:
    wxBoxSizer*      m_sizer;
    std::map<int,Button*> m_mapPageButtons;
    int                  m_selection{-1};
    int                  m_btn_margin;
    int                  m_line_margin;
    // ModeSizer*                      m_mode_sizer {nullptr};
};

wxDECLARE_EVENT(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED, wxCommandEvent);

ButtonsCtrl::ButtonsCtrl(wxWindow* parent, wxBoxSizer* side_tools)
    : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxTAB_TRAVERSAL)
{
#ifdef __WINDOWS__
    SetDoubleBuffered(true);
#endif //__WINDOWS__

    wxColour default_btn_bg;
#ifdef __APPLE__
    default_btn_bg = Slic3r::GUI::wxGetApp().dark_mode() ? wxColour("#010101") : wxColour(214, 214, 220); // Gradient #414B4E
#else
    default_btn_bg = Slic3r::GUI::wxGetApp().dark_mode() ? wxColour("#010101") : wxColour(214, 214, 220); // Gradient #414B4E

#endif

    SetBackgroundColour(default_btn_bg);

    int em = em_unit(this); // Slic3r::GUI::wxGetApp().em_unit();
    // BBS: no gap
    m_btn_margin  = FromDIP(5); // std::lround(0.3 * em);
    m_line_margin = FromDIP(1);

    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(m_sizer);

    if (side_tools != NULL) {
        m_sizer->AddStretchSpacer(1);
        for (size_t idx = 0; idx < side_tools->GetItemCount(); idx++) {
            wxSizerItem* item     = side_tools->GetItem(idx);
            wxWindow*    item_win = item->GetWindow();
            if (item_win) {
                item_win->Reparent(this);
            }
        }
        m_sizer->Add(side_tools, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, m_btn_margin);
    }

    // BBS: disable custom paint
    // this->Bind(wxEVT_PAINT, &ButtonsCtrl::OnPaint, this);
    Bind(wxEVT_SYS_COLOUR_CHANGED, [this](auto& e) {});
}
int  ButtonsCtrl::GetSelection() { return m_selection; }
void ButtonsCtrl::SetSelection(int sel)
{
    if (m_selection == sel)
        return;
    bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    if(-1!=sel && m_mapPageButtons.end() == m_mapPageButtons.find(sel))//not found
    {
        if (m_selection >= 0) {
            StateColor bg_color = StateColor(std::pair{ wxColour(21, 191, 89), (int)StateColor::Hovered },
                std::pair{ is_dark ? wxColour(1, 1, 1) : wxColour(214, 214, 220), (int)StateColor::Normal });
            m_mapPageButtons[m_selection]->SetBackgroundColor(bg_color);
            StateColor text_color = StateColor(std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int)StateColor::Hovered },
                std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(0,0,0), (int)StateColor::Normal});
            m_mapPageButtons[m_selection]->SetSelected(false);
            m_mapPageButtons[m_selection]->SetTextColor(text_color);
        }

        m_selection = -1;
        return;
    }

    if (-1 == sel) 
    {
        if (m_selection >= 0) {
            StateColor bg_color = StateColor(std::pair{ wxColour(21, 191, 89), (int) StateColor::Hovered},
                                             std::pair{is_dark ? wxColour(1, 1, 1) : wxColour(214, 214, 220), (int) StateColor::Normal});
            m_mapPageButtons[m_selection]->SetBackgroundColor(bg_color);
            StateColor text_color = StateColor(std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int)StateColor::Hovered }, 
                std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(0,0,0), (int) StateColor::Normal});
            m_mapPageButtons[m_selection]->SetSelected(false);
            m_mapPageButtons[m_selection]->SetTextColor(text_color);
        }

        m_selection = sel;
        return;
    }

    // BBS: change button color
    wxColour selected_btn_bg("#009688"); // Gradient #009688
    if (m_selection >= 0) {
        StateColor bg_color = StateColor(std::pair{wxColour(21, 191, 89), (int) StateColor::Hovered},
             std::pair{ is_dark ? wxColour(1,1,1) : wxColour(214, 214, 220), (int) StateColor::Normal});
        m_mapPageButtons[m_selection]->SetBackgroundColor(bg_color);
        StateColor text_color = StateColor(
            std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int)StateColor::Hovered },
            std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(0,0,0), (int)StateColor::Normal});
        m_mapPageButtons[m_selection]->SetSelected(false);
        m_mapPageButtons[m_selection]->SetTextColor(text_color);
        
    }
    m_selection = sel;


    StateColor text_color = StateColor(std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int)StateColor::Hovered },
        std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int) StateColor::Normal}
        );
    m_mapPageButtons[m_selection]->SetSelected(true);
    m_mapPageButtons[m_selection]->SetTextColor(text_color);

    StateColor bg_color = StateColor(std::pair{ wxColour(21, 191, 89), (int)StateColor::Hovered },
        std::pair{ wxColour(21, 191, 89), (int)StateColor::Normal });
    m_mapPageButtons[m_selection]->SetBackgroundColor(bg_color);
    m_mapPageButtons[m_selection]->SetFocus();

    Refresh();
}
void ButtonsCtrl::RefreshColor()
{
	//for (auto& it : m_mapPageButtons)
	//{
	//	Slic3r::GUI::wxGetApp().UpdateDarkUI(it.second);
	//}
    bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    wxColour default_btn_bg = Slic3r::GUI::wxGetApp().dark_mode() ? wxColour("#010101") : wxColour(214, 214, 220); // Gradient #414B4E
    SetBackgroundColour(default_btn_bg);
    for (auto& [index, button] : m_mapPageButtons) {
        StateColor bg_color = StateColor(std::pair{ wxColour(21, 191, 89), (int)StateColor::Hovered },
            std::pair{ is_dark ? wxColour(1, 1, 1) : wxColour(214, 214, 220), (int)StateColor::Normal });
        button->SetCornerRadius(FromDIP(3));
        button->SetFontBold(true);
        button->SetBackgroundColor(bg_color);     
        button->SetBackgroundColour(default_btn_bg);
        StateColor text_color = StateColor(std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int)StateColor::Hovered },
            std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(0,0,0), (int)StateColor::Normal });
        button->SetTextColor(text_color);
        if (m_selection == index)
        {
            button->SetSelected(true);
            bg_color = StateColor(std::pair{ wxColour(21, 191, 89), (int)StateColor::Hovered },
                std::pair{ wxColour(21, 191, 89), (int)StateColor::Normal });
            button->SetBackgroundColor(bg_color);
            StateColor text_color = StateColor(std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int)StateColor::Hovered },
                std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int)StateColor::Normal });
            button->SetTextColor(text_color);
        }
        button->Refresh();
        button->Update();
        //Slic3r::GUI::wxGetApp().UpdateDarkUI(button);
    }
    Refresh();
}
void ButtonsCtrl::reLayout()
{
    for (auto& it : m_mapPageButtons)
    {
        it.second->Rescale();
    }
    m_sizer->Layout();

}
bool ButtonsCtrl::InsertPage(
    size_t index, const wxString& text, bool bSelect /* = false*/, const std::string& bmp_name /* = ""*/, const std::string& inactive_bmp_name)
{
    Button* btn = new Button(this, text.empty() ? text : " " + text, bmp_name, wxNO_BORDER, 0, index);
    btn->SetCornerRadius(FromDIP(3));
    btn->SetFontBold(true);

    int em = em_unit(this);
    // BBS set size for button
    btn->SetMinSize({(text.empty() ? FromDIP(40) : FromDIP(100)), FromDIP(30)});
    bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    StateColor bg_color = StateColor(std::pair{wxColour(21, 191, 89), (int) StateColor::Hovered},
                                     std::pair{is_dark ? wxColour(1, 1, 1) : wxColour(214, 214, 220), (int) StateColor::Normal});

    btn->SetBackgroundColor(bg_color);
    StateColor text_color = StateColor(std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(255,255,255), (int)StateColor::Hovered },
        std::pair{ is_dark ? wxColour(254, 254, 254) : wxColour(0,0,0), (int)StateColor::Normal});
    btn->SetTextColor(text_color);
    //btn->SetInactiveIcon(inactive_bmp_name);
    btn->Bind(wxEVT_BUTTON, [this, btn](wxCommandEvent& event) {
        int id = btn->GetId();
        wxCommandEvent evt = wxCommandEvent(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED);
        evt.SetId(id);
        wxPostEvent(this->GetParent(), evt);
    });
    Slic3r::GUI::wxGetApp().UpdateDarkUI(btn);
    m_mapPageButtons[index] = btn;

    m_sizer->Add(btn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT , m_btn_margin);

    m_sizer->Layout();
  
    if (bSelect)
    {
        this->SetSelection(index);
    }

    return true;
}



enum CUSTOM_ID
{
    ID_TOP_MENU_TOOL = 3100,
    ID_LOGO,
    ID_TOP_FILE_MENU,
    ID_TOP_DROPDOWN_MENU,
    ID_TITLE,
    ID_MODEL_STORE,
    ID_PUBLISH,
    ID_CALIB,
    ID_PREFERENCES,
    ID_CONFIG_RELATE,
    ID_DOWNMANAGER,
    ID_LOGIN,
    ID_TOOL_BAR = 3200,
    ID_AMS_NOTEBOOK,
    ID_UPLOAD3MF,
    //CX
    ID_3D = 4000,
    ID_PREVIEW,
    ID_DEVICE
};

class BBLTopbarArt : public wxAuiDefaultToolBarArt
{
public:
    enum BTNSTYPE {
        NORMAL,
        CHECKED,
    };

public:
    virtual void DrawLabel(wxDC& dc, wxWindow* wnd, const wxAuiToolBarItem& item, const wxRect& rect) wxOVERRIDE;
    virtual void DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) wxOVERRIDE;
    virtual void DrawButton(wxDC& dc, wxWindow* wnd, const wxAuiToolBarItem& item, const wxRect& rect) wxOVERRIDE;
    virtual void DrawSeparator(wxDC& dc, wxWindow* wnd, const wxRect& _rect) wxOVERRIDE;
};

void BBLTopbarArt::DrawLabel(wxDC& dc, wxWindow* wnd, const wxAuiToolBarItem& item, const wxRect& rect)
{
    dc.SetFont(m_font);
#ifdef __WINDOWS__
    dc.SetTextForeground(Slic3r::GUI::wxGetApp().dark_mode() ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) : wxColor(0,0,0));
#elif __linux__
    dc.SetTextForeground(Slic3r::GUI::wxGetApp().dark_mode() ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) : wxColor(0,0,0));
#else
    dc.SetTextForeground(*wxWHITE);
#endif

    int textWidth = 0, textHeight = 0;
    dc.GetTextExtent(item.GetLabel(), &textWidth, &textHeight);

    wxRect clipRect = rect;
    clipRect.width -= wnd->FromDIP(1);
    dc.SetClippingRegion(clipRect);

    int textX, textY;
    if (textWidth < rect.GetWidth()) {
        textX = rect.x + wnd->FromDIP(1) + (rect.width - textWidth) / 2;
    }
    else {
        textX = rect.x + wnd->FromDIP(1);
    }
    textY = rect.y + (rect.height - textHeight) / 2;
    dc.DrawText(item.GetLabel(), textX, textY);
    dc.DestroyClippingRegion();
}

void BBLTopbarArt::DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect)
{
    bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    dc.SetBrush(wxBrush(is_dark ? wxColour(1, 1, 1) : wxColour(214, 214, 220)));
    dc.SetPen(*wxTRANSPARENT_PEN);
    wxRect clipRect = rect;
    clipRect.y -= wnd->FromDIP(8);
    clipRect.height += wnd->FromDIP(8);
    dc.SetClippingRegion(clipRect);
    dc.DrawRectangle(rect);
    dc.DestroyClippingRegion();
}

void BBLTopbarArt::DrawButton(wxDC& dc, wxWindow* wnd, const wxAuiToolBarItem& item, const wxRect& rect)
{
    int textWidth = 0, textHeight = 0;

    // Create a wxGraphicsContext from wxPaintDC
    // wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
    wxGraphicsContext* gc = wxGraphicsContext::CreateFromUnknownDC(dc);

    if (m_flags & wxAUI_TB_TEXT)
    {
        dc.SetFont(m_font);
        int tx, ty;

        dc.GetTextExtent(wxT("ABCDHgj"), &tx, &textHeight);
        textWidth = 0;
        dc.GetTextExtent(item.GetLabel(), &textWidth, &ty);
    }

    int bmpX = 0, bmpY = 0;
    int textX = 0, textY = 0;

    wxBitmap bmp;
    if (UPLOAD_BTN_CODE == item.GetUserData()) {
         bmp = item.GetState() & wxAUI_BUTTON_STATE_DISABLED ? item.GetDisabledBitmap() : 
             (item.GetState() & wxAUI_BUTTON_STATE_HOVER ? item.GetHoverBitmap() : item.GetBitmap());
    } else if (HOME_BTN_CODE_CHECKED == item.GetUserData()) {
        bmp = item.GetState() == wxAUI_BUTTON_STATE_NORMAL ? item.GetBitmap() : item.GetHoverBitmap();
    } 
    else {
        bmp = item.GetState() & wxAUI_BUTTON_STATE_DISABLED ?
                                  item.GetDisabledBitmap() : item.GetBitmap();
    }
   
    const wxSize bmpSize = bmp.IsOk() ? bmp.GetScaledSize() : wxSize(0, 0);

    if (m_textOrientation == wxAUI_TBTOOL_TEXT_BOTTOM)
    {
        bmpX = rect.x +
            (rect.width / 2) -
            (bmpSize.x / 2);

        bmpY = rect.y +
            ((rect.height - textHeight) / 2) -
            (bmpSize.y / 2);

        textX = rect.x + (rect.width / 2) - (textWidth / 2) + wnd->FromDIP(1);
        textY = rect.y + rect.height - textHeight - wnd->FromDIP(1);
    }
    else if (m_textOrientation == wxAUI_TBTOOL_TEXT_RIGHT)
    {
        bmpX = rect.x + wnd->FromDIP(3);

        bmpY = rect.y +
            (rect.height / 2) -
            (bmpSize.y / 2);

        textX = bmpX + wnd->FromDIP(3) + bmpSize.x;
        textY = rect.y +
            (rect.height / 2) -
            (textHeight / 2);
    }

    if (!(item.GetState() & wxAUI_BUTTON_STATE_DISABLED))
    {
        if (item.GetState() & wxAUI_BUTTON_STATE_PRESSED)
        {
            dc.SetPen(wxPen(StateColor::darkModeColorFor("#15BF59"))); // ORCA
            //dc.SetBrush(wxBrush(StateColor::darkModeColorFor("#15BF59"))); // ORCA
            if (UPLOAD_BTN_CODE != item.GetUserData()) {
                dc.DrawRoundedRectangle(rect, wnd->FromDIP(5));
            }
        }
        else if ((item.GetState() & wxAUI_BUTTON_STATE_HOVER) || item.IsSticky())
        {
            dc.SetPen(wxPen(StateColor::darkModeColorFor("#15BF59"))); // ORCA
            //dc.SetBrush(wxBrush(StateColor::darkModeColorFor("#009688"))); // ORCA

            // draw an even lighter background for checked item hovers (since
            // the hover background is the same color as the check background)
            if (item.GetState() & wxAUI_BUTTON_STATE_CHECKED)
                dc.SetBrush(wxBrush(StateColor::darkModeColorFor("#009688"))); // ORCA

            // dc.DrawRoundedRectangle(rect, 3);
            if(gc)
            {
                // Create a path for the rectangle
                wxGraphicsPath path = gc->CreatePath();
                // path.AddRectangle(rect.x, rect.y, rect.width, rect.height);
                path.AddRoundedRectangle(rect.x, rect.y, rect.width, rect.height, wnd->FromDIP(2));

                // gc->SetBrush(*wxGREEN_BRUSH);
                gc->SetPen(*wxGREEN_PEN);
                gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);
                // gc->DrawRoundedRectangle(rect.x, rect.y, rect.width, rect.height, 3);

                // Draw the border
                if (UPLOAD_BTN_CODE != item.GetUserData()) {
                    gc->StrokePath(path);
                }

                // Destroy the graphics context to free resources
                delete gc;
            }
            else
            {
                dc.DrawRoundedRectangle(rect, wnd->FromDIP(5));
            }
        }
        else if (item.GetState() & wxAUI_BUTTON_STATE_CHECKED)
        {
            if (HOME_BTN_CODE_CHECKED == item.GetUserData()) {
                dc.SetPen(wxPen(StateColor::darkModeColorFor("#15BF59")));     // ORCA
                dc.SetBrush(wxBrush(StateColor::darkModeColorFor("#15BF59"))); // ORCA
                dc.DrawRoundedRectangle(rect, wnd->FromDIP(5));
            } else {
                // it's important to put this code in an else statement after the
                // hover, otherwise hovers won't draw properly for checked items
                dc.SetPen(wxPen(StateColor::darkModeColorFor("#009688"))); // ORCA
                // dc.SetBrush(wxBrush(StateColor::darkModeColorFor("#009688"))); // ORCA
                dc.DrawRectangle(rect);
            }
        }
    }

    if (bmp.IsOk())
        dc.DrawBitmap(bmp, bmpX, bmpY, true);

    // set the item's text color based on if it is disabled
#ifdef __WINDOWS__
    dc.SetTextForeground(Slic3r::GUI::wxGetApp().dark_mode() ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) : wxColor(0,0,0));
#elif __linux__
    dc.SetTextForeground(Slic3r::GUI::wxGetApp().dark_mode() ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) : wxColor(0,0,0));
#else
    dc.SetTextForeground(*wxWHITE);
#endif
    if (item.GetState() & wxAUI_BUTTON_STATE_DISABLED)
    {
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }

    if ((m_flags & wxAUI_TB_TEXT) && !item.GetLabel().empty())
    {
        dc.DrawText(item.GetLabel(), textX, textY);
    }
}

void BBLTopbarArt::DrawSeparator(wxDC& dc, wxWindow* wnd, const wxRect& _rect)
{
    bool horizontal = true;
    if (m_flags & wxAUI_TB_VERTICAL)
        horizontal = false;

    wxRect rect = _rect;
    rect.height = wnd->FromDIP(30);

    if (horizontal) {
        rect.x += (rect.width / 2);
        rect.width     = wnd->FromDIP(1);
        int new_height = (rect.height * 3) / 4;
        rect.y += wnd->FromDIP(3);
        //((_rect.height - rect.height));
        rect.height = new_height;
    } else {
        rect.y += (rect.height / 2);
        rect.height   = wnd->FromDIP(1);
        int new_width = (rect.width * 3) / 4;
        rect.x += (rect.width / 2) - (new_width / 2);
        rect.width = new_width;
    }

     bool     is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    wxColour startColour = is_dark ? ("#454548") : ("#AAAAB0");
    wxColour endColour   = is_dark ? ("#454548") : ("#AAAAB0");
    dc.GradientFillLinear(rect, startColour, endColour, horizontal ? wxSOUTH : wxEAST);
}

BBLTopbar::BBLTopbar(wxFrame* parent) 
    : wxAuiToolBar(parent, ID_TOOL_BAR, wxDefaultPosition, wxSize(-1, 40), wxAUI_TB_TEXT | wxAUI_TB_HORZ_TEXT)
{ 
    Init(parent);
}

static wxFrame* topbarParent = NULL;

BBLTopbar::BBLTopbar(wxWindow* pwin, wxFrame* parent)
    : wxAuiToolBar(pwin, ID_TOOL_BAR, wxDefaultPosition, wxSize(-1, 40), wxAUI_TB_TEXT | wxAUI_TB_HORZ_TEXT)
{
    Init(parent);
    topbarParent = parent;
}

void BBLTopbar::Init(wxFrame* parent) 
{
    m_title_item = nullptr;
    m_calib_item = nullptr;
    m_tabCtrol = nullptr;

    SetArtProvider(new BBLTopbarArt());
    m_frame = parent;
    m_skip_popup_file_menu = false;
    m_skip_popup_dropdown_menu = false;
    m_skip_popup_calib_menu    = false;

    wxInitAllImageHandlers();
    //auto  = [=](int x) {return x * em_unit(this) / 10; };
    //this->SetMargins(wxSize(10, 10));
    this->AddSpacer(FromDIP(5));

    bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
    wxBitmap logo_bitmap = create_scaled_bitmap("logo", nullptr, (20));
    wxBitmap logo_bitmap_checked = create_scaled_bitmap("logo_checked", nullptr, (20));
    logo_item   = this->AddTool(ID_LOGO, "", logo_bitmap);
    logo_item->SetHoverBitmap(logo_bitmap_checked);

    this->AddSpacer(FromDIP(10));
    this->AddSeparator(); 
#ifndef __APPLE__
    wxBitmap file_bitmap = create_scaled_bitmap(is_dark ? "file_down" : "file_down_light", this, (TOPBAR_ICON_SIZE));
    m_file_menu_item = this->AddTool(ID_TOP_FILE_MENU, _L("File"), file_bitmap, wxEmptyString, wxITEM_NORMAL);

    wxFont basic_font = this->GetFont();
    basic_font.SetPointSize(10);
    this->SetFont(basic_font);

    this->SetForegroundColour(is_dark ? wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT) : wxColour(214, 214, 220));

    wxBitmap dropdown_bitmap = create_scaled_bitmap(is_dark ? "menu_down" : "menu_down_light", this, (8));
    m_dropdown_menu_item = this->AddTool(ID_TOP_DROPDOWN_MENU, "", dropdown_bitmap);
 
    this->AddSpacer(FromDIP(5));
    this->AddSeparator();
    this->AddSpacer(FromDIP(5));
#endif
    wxBitmap open_bitmap = create_scaled_bitmap(is_dark ? "open_file" : "open_file_light" , this, (TOPBAR_ICON_SIZE));
    tool_item            = this->AddTool(wxID_OPEN, "", open_bitmap, _L("Open Project"));

    this->AddSpacer(FromDIP(10));

    wxBitmap save_bitmap = create_scaled_bitmap(is_dark ? "topbar_save" : "topbar_save_light", this, (TOPBAR_ICON_SIZE));
    wxBitmap save_inactive_bitmap = create_scaled_bitmap(is_dark ? "topbar_save_disabled" : "topbar_save_disabled_light", this, (TOPBAR_ICON_SIZE));
    m_save_project_item = this->AddTool(wxID_SAVE, "", save_bitmap, _L("Save the project file"));
    m_save_project_item->SetDisabledBitmap(save_inactive_bitmap);

    this->AddSpacer(FromDIP(10));

    m_preference_item = this->AddTool(ID_PREFERENCES, "", create_scaled_bitmap(is_dark ? "preferences" : "preferences_light", this, (TOPBAR_ICON_SIZE)), _L("Preferences"));

#ifdef __APPLE__
    this->AddSpacer(FromDIP(10));
    this->AddTool(ID_CONFIG_RELATE, "", create_scaled_bitmap(is_dark ? "config_relate" : "config_relate_light", nullptr, TOPBAR_ICON_SIZE), _L("Relations"));
    auto item = this->FindTool(ID_CONFIG_RELATE);
    if (item)
    {
        wxBitmap bitmap("");
        item->SetDisabledBitmap(bitmap);
    }
    this->AddSpacer(FromDIP(10));
#endif

    wxBitmap undo_bitmap = create_scaled_bitmap(is_dark ? "topbar_undo" : "topbar_undo_light", this, (TOPBAR_ICON_SIZE));
    wxBitmap undo_inactive_bitmap = create_scaled_bitmap(is_dark ? "topbar_undo_disabled" : "topbar_undo_disabled_light", this, (TOPBAR_ICON_SIZE));
    m_undo_item                   = this->AddTool(wxID_UNDO, "", undo_bitmap, _L("Undo"));   
    m_undo_item->SetDisabledBitmap(undo_inactive_bitmap);

    this->AddSpacer(FromDIP(10));

    wxBitmap redo_bitmap = create_scaled_bitmap(is_dark ? "topbar_redo" : "topbar_redo_light", this, (TOPBAR_ICON_SIZE));
    wxBitmap redo_inactive_bitmap = create_scaled_bitmap(is_dark ? "topbar_redo_disabled" : "topbar_redo_disabled_light", this, (TOPBAR_ICON_SIZE));
    m_redo_item                   = this->AddTool(wxID_REDO, "", redo_bitmap, _L("Redo"));
    m_redo_item->SetDisabledBitmap(redo_inactive_bitmap);
    /*
    this->AddSpacer(FromDIP(10));

    
    wxBitmap calib_bitmap          = create_scaled_bitmap("calib_sf", nullptr, TOPBAR_ICON_SIZE);
    wxBitmap calib_bitmap_inactive = create_scaled_bitmap("calib_sf_inactive", nullptr, TOPBAR_ICON_SIZE);
    m_calib_item                   = this->AddTool(ID_CALIB, _L("Calibration"), calib_bitmap);
    m_calib_item->SetDisabledBitmap(calib_bitmap_inactive);*/

    this->AddSpacer(FromDIP(10));
    this->AddStretchSpacer(1);
    //CX
    ButtonsCtrl* pCtr = new ButtonsCtrl(this);
    pCtr->InsertPage(MainFrame::tp3DEditor, _L("Prepare"), 0);
    pCtr->InsertPage(MainFrame::tpPreview, _L("Preview"), 0);
    pCtr->InsertPage(MainFrame::tpDeviceMgr, _L("Device"), 0);
    //pCtr->InsertPage(3, _L("Project"), 0);
    m_tabCtrol = (wxControl*)pCtr;
    wxAuiToolBarItem* item_ctrl = this->AddControl( m_tabCtrol);
    item_ctrl->SetAlignment(wxALIGN_CENTRE);
 
    this->Bind(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED, [this](wxCommandEvent& evt) {
        //         wxGetApp().mainframe->select_tab(evt.GetId());
        logo_item->SetUserData(HOME_BTN_CODE_UNCHECKED);
        logo_item->SetState(wxAUI_BUTTON_STATE_NORMAL);
        if (nullptr != m_tabCtrol) 
        {
            ButtonsCtrl* pCtr = dynamic_cast<ButtonsCtrl*>(m_tabCtrol);
            pCtr->SetSelection(evt.GetId());
        }
        
        wxCommandEvent e = wxCommandEvent(wxCUSTOMEVT_NOTEBOOK_SEL_CHANGED);
        e.SetId(evt.GetId());
        
        wxPostEvent(wxGetApp().tab_panel(), e);
        });
    //CX END

    this->AddStretchSpacer(1);
    m_title_LabelItem = new Label(this, Label::Head_12, _L(""));
    m_title_LabelItem->SetMinSize(wxSize(TOPBAR_TITLE_WIDTH, FromDIP(-1)));
    m_title_LabelItem->SetSize(wxSize(TOPBAR_TITLE_WIDTH, FromDIP(-1)));
    m_title_LabelItem->Bind(wxEVT_MOTION, &BBLTopbar::OnMouseMotion, this);
    m_title_LabelItem->Bind(wxEVT_LEFT_DOWN, &BBLTopbar::OnMouseLeftDown, this);

    m_title_LabelItem->SetWindowStyleFlag(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL);
    wxColour bgColor  = Slic3r::GUI::wxGetApp().dark_mode() ? wxColour("#010101") : wxColour(214, 214, 220);
    m_title_LabelItem->SetBackgroundColour(bgColor);
    m_title_item = this->AddControl(m_title_LabelItem);

    this->AddSpacer(FromDIP(10));
    wxAuiToolBarItem * tool_sep = this->AddSeparator();
    this->AddSpacer(FromDIP(10));
#if CUSTOM_CXCLOUD  
    wxSize   targetSize(FromDIP(40), FromDIP(24));
    wxBitmap upload_bitmap = create_scaled_bitmap3("toolbar_upload3mf", this, (TOPBAR_ICON_SIZE),targetSize);
    wxImage  upload_image  = upload_bitmap.ConvertToImage();
    upload_image.Rescale(targetSize.GetWidth(), targetSize.GetHeight(), wxIMAGE_QUALITY_BICUBIC);
    wxBitmap upload_bitmap1(upload_image);
    m_upload_btn = this->AddTool(ID_UPLOAD3MF, "", upload_bitmap1, _L("upload 3mf to crealitycloud"));
    m_upload_btn->SetUserData(UPLOAD_BTN_CODE);

    wxBitmap upload_disable_bitmap = create_scaled_bitmap3("toolbar_upload3mf_disable", this, (TOPBAR_ICON_SIZE), targetSize);
    upload_image = upload_disable_bitmap.ConvertToImage();
    upload_image.Rescale(targetSize.GetWidth(), targetSize.GetHeight(), wxIMAGE_QUALITY_HIGH);
    wxBitmap upload_bitmap2(upload_image);
    m_upload_btn->SetDisabledBitmap(upload_bitmap2);

    wxBitmap upload_hover_bitmap = create_scaled_bitmap3("toolbar_upload3mf_hover", this, (TOPBAR_ICON_SIZE), targetSize);
    upload_image                 = upload_hover_bitmap.ConvertToImage();
    upload_image.Rescale(targetSize.GetWidth(), targetSize.GetHeight(), wxIMAGE_QUALITY_HIGH);
    wxBitmap upload_bitmap3(upload_image);
    m_upload_btn->SetHoverBitmap(upload_bitmap3);

    this->AddSpacer(FromDIP(5));
    wxAuiToolBarItem* tool_sep1 = this->AddSeparator();
    this->AddSpacer(FromDIP(5));

    EnableUpload3mf();
#endif
#ifdef __WIN32__
    wxBitmap iconize_bitmap = create_scaled_bitmap(is_dark ? "topbar_min" : "topbar_min_light", this, (TOPBAR_ICON_SIZE));
    wxAuiToolBarItem* iconize_btn = this->AddTool(wxID_ICONIZE_FRAME, "", iconize_bitmap);

    maximize_bitmap = create_scaled_bitmap(is_dark ? "topbar_max" : "topbar_max_light", this, (TOPBAR_ICON_SIZE));
    window_bitmap = create_scaled_bitmap(is_dark ? "topbar_win" : "topbar_win_light", this, (TOPBAR_ICON_SIZE));
    if (m_frame->IsMaximized()) {
        maximize_btn = this->AddTool(wxID_MAXIMIZE_FRAME, "", window_bitmap);
    }
    else {
        maximize_btn = this->AddTool(wxID_MAXIMIZE_FRAME, "", maximize_bitmap);
    }

    wxBitmap close_bitmap = create_scaled_bitmap(is_dark ? "topbar_close" : "topbar_close_light", this, (TOPBAR_ICON_SIZE));
    wxAuiToolBarItem* close_btn    = this->AddTool(wxID_CLOSE_FRAME, "", close_bitmap, wxString("Models"));
    //this->AddSpacer(FromDIP(5));
#endif

    Realize();
    // m_toolbar_h = this->GetSize().GetHeight();
    m_toolbar_h = FromDIP(40);

    int client_w = parent->GetClientSize().GetWidth();
    this->SetSize(client_w, m_toolbar_h);
    
    this->Bind(wxEVT_MOTION, &BBLTopbar::OnMouseMotion, this);
    this->Bind(wxEVT_MOUSE_CAPTURE_LOST, &BBLTopbar::OnMouseCaptureLost, this);
    this->Bind(wxEVT_MENU_CLOSE, &BBLTopbar::OnMenuClose, this);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnFileToolItem, this, ID_TOP_FILE_MENU);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnDropdownToolItem, this, ID_TOP_DROPDOWN_MENU);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnCalibToolItem, this, ID_CALIB);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnUpload3mf, this, ID_UPLOAD3MF);
    //this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnIconize, this, wxID_ICONIZE_FRAME);
    //this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnFullScreen, this, wxID_MAXIMIZE_FRAME);
    //this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnCloseFrame, this, wxID_CLOSE_FRAME);
    this->Bind(wxEVT_LEFT_DCLICK, &BBLTopbar::OnMouseLeftDClock, this);
    #ifdef WIN32
    this->Bind(wxEVT_LEFT_DOWN, &BBLTopbar::OnMouseLeftDown, this);
    this->Bind(wxEVT_LEFT_UP, &BBLTopbar::OnMouseLeftUp, this);
    #endif
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnOpenProject, this, wxID_OPEN);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnSaveProject, this, wxID_SAVE);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnRedo, this, wxID_REDO);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnUndo, this, wxID_UNDO);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnModelStoreClicked, this, ID_MODEL_STORE);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnPublishClicked, this, ID_PUBLISH);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnPreferences, this, ID_PREFERENCES);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnConfigRelate, this, ID_CONFIG_RELATE);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnLogo, this, ID_LOGO);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnDownMgr, this, ID_DOWNMANAGER);
    this->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, &BBLTopbar::OnLogin, this, ID_LOGIN);


    //Creality: relations
    // int mode = wxGetApp().app_config->get("role_type") != "0";
    // update_mode(mode);
    m_top_menu.Bind(
        wxEVT_MENU,
        [=](auto& e) {
            wxGetApp().open_config_relate();
            wxGetApp().plater()->get_current_canvas3D()->force_set_focus();
        },
        ID_CONFIG_RELATE);
}

BBLTopbar::~BBLTopbar()
{
    m_file_menu_item = nullptr;
    m_dropdown_menu_item = nullptr;
    m_file_menu = nullptr;
}

void BBLTopbar::OnOpenProject(wxAuiToolBarEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    Plater* plater = main_frame->plater();
    plater->load_project();
}

void BBLTopbar::show_publish_button(bool show)
{
    //this->EnableTool(m_publish_item->GetId(), show);
    //Refresh();
}

void BBLTopbar::OnSaveProject(wxAuiToolBarEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    Plater* plater = main_frame->plater();
    plater->save_project(false, FT_PROJECT);
}

void BBLTopbar::OnUndo(wxAuiToolBarEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    Plater* plater = main_frame->plater();
    plater->undo();
}

void BBLTopbar::OnRedo(wxAuiToolBarEvent& event)
{
    MainFrame* main_frame = dynamic_cast<MainFrame*>(m_frame);
    Plater* plater = main_frame->plater();
    plater->redo();
}

void BBLTopbar::EnableUpload3mf()
{
#if CUSTOM_CXCLOUD
    if (wxGetApp().plater()) { 
        this->EnableTool(m_upload_btn->GetId(), wxGetApp().plater()->can_arrange());
        Refresh();
    }
#endif
}
bool BBLTopbar::GetSaveProjectItemEnabled()
{
    if(nullptr != m_save_project_item)
        return GetToolEnabled(m_save_project_item->GetId());
    return true;
}
void BBLTopbar::EnableUndoRedoItems()
{
    this->EnableTool(m_undo_item->GetId(), true);
    this->EnableTool(m_redo_item->GetId(), true);
    this->EnableTool(m_save_project_item->GetId(), true);
    if (nullptr!= m_calib_item)
        this->EnableTool(m_calib_item->GetId(), true);
    Refresh();
}

void BBLTopbar::DisableUndoRedoItems()
{
    this->EnableTool(m_undo_item->GetId(), false);
    this->EnableTool(m_redo_item->GetId(), false);
    this->EnableTool(m_save_project_item->GetId(), false);
    if (nullptr != m_calib_item)
        this->EnableTool(m_calib_item->GetId(), false);
    Refresh();
}

void BBLTopbar::DisableGuideModeItems()
{
    bool res = this->GetToolEnabled(logo_item->GetId());
    if (!res) {
        logo_item->SetUserData(0);   
    }
       
    res = this->GetToolEnabled(m_file_menu_item->GetId());
    if (!res) {
        m_file_menu_item->SetUserData(0);
    }
    
    res = this->GetToolEnabled(m_dropdown_menu_item->GetId());
    if (!res) {
        m_dropdown_menu_item->SetUserData(0);
    }

    res = this->GetToolEnabled(tool_item->GetId());
    if (!res) {
        tool_item->SetUserData(0);
    }

    res = this->GetToolEnabled(m_save_project_item->GetId());
    if (!res) {
        m_save_project_item->SetUserData(0);
    }

    res = this->GetToolEnabled(m_preference_item->GetId());
    if (!res) {
        m_preference_item->SetUserData(0);
    }

    res = this->GetToolEnabled(m_undo_item->GetId());
    if (!res) {
        m_undo_item->SetUserData(0);
    }
    
    res = this->GetToolEnabled(m_redo_item->GetId());
    if (!res) {
        m_redo_item->SetUserData(0);
    }

    this->EnableTool(logo_item->GetId(), false);
    this->EnableTool(m_file_menu_item->GetId(), false);
    this->EnableTool(m_dropdown_menu_item->GetId(), false);
    this->EnableTool(tool_item->GetId(), false);
    this->EnableTool(m_save_project_item->GetId(), false);
    this->EnableTool(m_preference_item->GetId(), false);
    this->EnableTool(m_undo_item->GetId(), false);
    this->EnableTool(m_redo_item->GetId(), false);
    m_tabCtrol->Enable(false);
    m_title_LabelItem->Enable(false);
    this->EnableTool(m_upload_btn->GetId(), false);

    Refresh();
}

void BBLTopbar::EnableGuideModeItems()
{
    if (logo_item->GetUserData() != 0)
        logo_item->SetUserData(-1);
    if (m_file_menu_item->GetUserData() != 0)
        m_file_menu_item->SetUserData(-1);
    if (m_dropdown_menu_item->GetUserData() != 0)
        m_dropdown_menu_item->SetUserData(-1);
    if (tool_item->GetUserData() != 0)
        tool_item->SetUserData(-1);
    if (m_save_project_item->GetUserData() != 0)
        m_save_project_item->SetUserData(-1);
    if (m_preference_item->GetUserData() != 0)
        m_preference_item->SetUserData(-1);
    if (m_undo_item->GetUserData() != 0)
        m_undo_item->SetUserData(-1);
    if (m_redo_item->GetUserData() != 0)
        m_redo_item->SetUserData(-1);

    this->EnableTool(logo_item->GetId(), true);
    this->EnableTool(m_file_menu_item->GetId(), true);
    this->EnableTool(m_dropdown_menu_item->GetId(), true);
    this->EnableTool(tool_item->GetId(), true);
    this->EnableTool(m_save_project_item->GetId(), true);
    this->EnableTool(m_preference_item->GetId(), true);
    this->EnableTool(m_undo_item->GetId(), true);
    this->EnableTool(m_redo_item->GetId(), true);

    m_tabCtrol->Enable(true);
    m_title_LabelItem->Enable(true);
    this->EnableTool(m_upload_btn->GetId(), true);

    Refresh();
}

void BBLTopbar::SaveNormalRect()
{
    m_normalRect = m_frame->GetRect();
}

void BBLTopbar::ShowCalibrationButton(bool show)
{
    if (nullptr != m_calib_item)
        m_calib_item->GetSizerItem()->Show(show);

    m_sizer->Layout();
    if (!show && nullptr != m_calib_item)
        m_calib_item->GetSizerItem()->SetDimension({-1000, 0}, {0, 0});
    Refresh();
}

void BBLTopbar::SetSelection(size_t index)
{
    if (index == MainFrame::tpHome)
    {
        wxAuiToolBarEvent evt;
        OnLogo(evt);
    }
    else if (nullptr != m_tabCtrol)
    {
        wxAuiToolBarItem* item = this->FindTool(ID_LOGO);
        item->SetState(wxAUI_BUTTON_STATE_NORMAL);
        ButtonsCtrl* pCtr = dynamic_cast<ButtonsCtrl*>(m_tabCtrol);
        pCtr->SetSelection(index);
    }
}

void BBLTopbar::update_mode(int mode)
{
    if (mode == 0) 
    {
#ifdef __APPLE__
        this->EnableTool(ID_CONFIG_RELATE, false);
        this->GetParent()->Layout();
        return;
#endif
        m_top_menu.Remove(ID_CONFIG_RELATE);
        m_relationsItem = NULL;
    } 
    else if (mode == 1) 
    {
#ifdef __APPLE__
        this->EnableTool(ID_CONFIG_RELATE, true);
        this->GetParent()->Layout();
        return;
#endif
        if (m_relationsItem)
            return;

        bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
        m_top_menu.Remove(ID_CONFIG_RELATE);
        m_relationsItem = m_top_menu.Append(ID_CONFIG_RELATE, _L("Relations"));
        //m_relationsItem->SetBitmap(create_scaled_bitmap(is_dark ? "config_relate" : "config_relate_light", this, TOPBAR_ICON_SIZE));   
    }
}

void BBLTopbar::OnModelStoreClicked(wxAuiToolBarEvent& event)
{
    //GUI::wxGetApp().load_url(wxString(wxGetApp().app_config->get_web_host_url() + MODEL_STORE_URL));
}

void BBLTopbar::OnPublishClicked(wxAuiToolBarEvent& event)
{
    if (!wxGetApp().getAgent()) {
        BOOST_LOG_TRIVIAL(info) << "publish: no agent";
        return;
    }

    //no more check
    //if (GUI::wxGetApp().plater()->model().objects.empty()) return;

#ifdef ENABLE_PUBLISHING
    wxGetApp().plater()->show_publish_dialog();
#endif
    wxGetApp().open_publish_page_dialog();
}

void BBLTopbar::OnPreferences(wxAuiToolBarEvent& evt) 
{ 
    wxGetApp().open_preferences();
    wxGetApp().plater()->get_current_canvas3D()->force_set_focus();
}

void BBLTopbar::OnConfigRelate(wxAuiToolBarEvent& evt)
{
    wxGetApp().open_config_relate();
    wxGetApp().plater()->get_current_canvas3D()->force_set_focus();
}
    
void BBLTopbar::OnLogo(wxAuiToolBarEvent& evt) 
{ 
#if CUSTOM_CXCLOUD
    wxAuiToolBarItem* item = this->FindTool(ID_LOGO);
    item->SetUserData(HOME_BTN_CODE_CHECKED);
    item->SetState(wxAUI_BUTTON_STATE_CHECKED);
    wxGetApp().mainframe->select_tab(size_t(0));
    if (nullptr != m_tabCtrol)
    {
        ButtonsCtrl* pCtr = dynamic_cast<ButtonsCtrl*>(m_tabCtrol);
        pCtr->SetSelection(-1);
    }
#endif
}

void BBLTopbar::OnDownMgr(wxAuiToolBarEvent& evt) {}

void BBLTopbar::OnLogin(wxAuiToolBarEvent& evt) {}

void BBLTopbar::SetFileMenu(wxMenu* file_menu)
{
    m_file_menu = file_menu;
}

void BBLTopbar::AddDropDownSubMenu(wxMenu* sub_menu, const wxString& title)
{
    if (title == _L("Help"))
    {
        m_helpItem = sub_menu;
    }
    m_top_menu.AppendSubMenu(sub_menu, title);
}

void BBLTopbar::AddDropDownMenuItem(wxMenuItem* menu_item)
{
    m_top_menu.Append(menu_item);
}

wxMenu* BBLTopbar::GetTopMenu()
{
    return &m_top_menu;
}

wxMenu* BBLTopbar::GetCalibMenu()
{
    return &m_calib_menu;
}

void BBLTopbar::SetTitle(wxString title)
{
    wxGCDC dc(this);
    wxString newTitle = wxControl::Ellipsize(title, dc, wxELLIPSIZE_END, TOPBAR_TITLE_WIDTH - FromDIP(30));

    if (m_title_LabelItem) {
        m_title_LabelItem->SetLabel(newTitle);
        m_title_LabelItem->SetToolTip(title);
    }

    if (m_title_item!=nullptr)
    {
        m_title_item->SetLabel(title);
        m_title_item->SetAlignment(wxALIGN_CENTRE);
        this->Refresh();
    }
}

void BBLTopbar::SetMaximizedSize()
{
#ifndef __APPLE__
if (maximize_bitmap.IsOk())
    maximize_btn->SetBitmap(maximize_bitmap);
#endif
}

void BBLTopbar::SetWindowSize()
{
#ifndef __APPLE__
if (window_bitmap.IsOk())
    maximize_btn->SetBitmap(window_bitmap);
#endif
}

void BBLTopbar::UpdateToolbarWidth(int width)
{
    this->SetSize(width, m_toolbar_h);
}

void BBLTopbar::Rescale(bool isResize) {
    int em = em_unit(this);
    //auto  = [=](int x) {return x * em_unit(this) / 10; };
    wxAuiToolBarItem* item;
    bool is_dark = Slic3r::GUI::wxGetApp().dark_mode();
#ifndef __APPLE__
    item = this->FindTool(ID_LOGO);
    item->SetBitmap(create_scaled_bitmap("logo", this, (20)));
    item = this->FindTool(ID_TOP_FILE_MENU);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "file_down" : "file_down_light", this, (TOPBAR_ICON_SIZE)));
    item = this->FindTool(ID_TOP_DROPDOWN_MENU);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "menu_down" : "menu_down_light", this, (8)));
#endif
    item = this->FindTool(wxID_OPEN);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "open_file" : "open_file_light", this, (TOPBAR_ICON_SIZE)));

    item = this->FindTool(wxID_SAVE);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "topbar_save" : "topbar_save_light", this, (TOPBAR_ICON_SIZE)));
    item->SetDisabledBitmap(create_scaled_bitmap(is_dark ? "topbar_save_disabled" : "topbar_save_disabled_light", this, (TOPBAR_ICON_SIZE)));

    item = this->FindTool(ID_PREFERENCES);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "preferences" : "preferences_light", this, (TOPBAR_ICON_SIZE)));

#ifdef __APPLE__
     item = this->FindTool(ID_CONFIG_RELATE);
     if (item)
         item->SetBitmap(create_scaled_bitmap(is_dark ? "config_relate" : "config_relate_light", this, TOPBAR_ICON_SIZE));
     if (m_relationsItem)
         m_relationsItem->SetBitmap(create_scaled_bitmap(is_dark ? "config_relate" : "config_relate_light", this, (TOPBAR_ICON_SIZE)));
#endif
    item = this->FindTool(wxID_UNDO);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "topbar_undo" : "topbar_undo_light", this, (TOPBAR_ICON_SIZE)));
    item->SetDisabledBitmap(create_scaled_bitmap(is_dark ? "topbar_undo_disabled" : "topbar_undo_disabled_light", this, (TOPBAR_ICON_SIZE)));

    item = this->FindTool(wxID_REDO);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "topbar_redo" : "topbar_redo_light", this, (TOPBAR_ICON_SIZE)));
    item->SetDisabledBitmap(create_scaled_bitmap(is_dark ? "topbar_redo_disabled" : "topbar_redo_disabled_light", this, (TOPBAR_ICON_SIZE)));

//     item = this->FindTool(ID_CALIB);
//     item->SetBitmap(create_scaled_bitmap("calib_sf", nullptr, TOPBAR_ICON_SIZE));
//     item->SetDisabledBitmap(create_scaled_bitmap("calib_sf_inactive", nullptr, TOPBAR_ICON_SIZE));

    //item = this->FindTool(ID_TITLE);

    /*item = this->FindTool(ID_PUBLISH);
    item->SetBitmap(create_scaled_bitmap("topbar_publish", this, TOPBAR_ICON_SIZE));
    item->SetDisabledBitmap(create_scaled_bitmap("topbar_publish_disable", nullptr, TOPBAR_ICON_SIZE));*/

    /*item = this->FindTool(ID_MODEL_STORE);
    item->SetBitmap(create_scaled_bitmap("topbar_store", this, TOPBAR_ICON_SIZE));
    */
    
#ifdef __WIN32__
    item = this->FindTool(wxID_ICONIZE_FRAME);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "topbar_min" : "topbar_min_light", this, (TOPBAR_ICON_SIZE)));
    item = this->FindTool(wxID_MAXIMIZE_FRAME);
    maximize_bitmap = create_scaled_bitmap(is_dark ? "topbar_max" : "topbar_max_light", this, (TOPBAR_ICON_SIZE));
    window_bitmap   = create_scaled_bitmap(is_dark ? "topbar_win" : "topbar_win_light", this, (TOPBAR_ICON_SIZE));
    if (m_frame->IsMaximized()) {
        item->SetBitmap(window_bitmap);
    }
    else {
        item->SetBitmap(maximize_bitmap);
    }

    item = this->FindTool(wxID_CLOSE_FRAME);
    item->SetBitmap(create_scaled_bitmap(is_dark ? "topbar_close" : "topbar_close_light", this, (TOPBAR_ICON_SIZE)));
#endif
    if (m_tabCtrol) {
        ButtonsCtrl* pCtr = dynamic_cast<ButtonsCtrl*>(m_tabCtrol);
        pCtr->RefreshColor();
    }
    Refresh();
    //refresh layout
    if (isResize)
    {
        ButtonsCtrl* pCtr = dynamic_cast<ButtonsCtrl*>(m_tabCtrol);

        pCtr->Layout();
        pCtr->Centre();
        pCtr->reLayout();
        Realize();
    }

    wxColour bgColor = Slic3r::GUI::wxGetApp().dark_mode() ? wxColour("#010101") : wxColour(214, 214, 220);
    m_title_LabelItem->SetBackgroundColour(bgColor);
}

void BBLTopbar::OnIconize(wxAuiToolBarEvent& event)
{
    BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " start";   
    m_frame->Iconize();
    BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " end";
}

void BBLTopbar::OnUpload3mf(wxAuiToolBarEvent& event)
{
    wxGetApp().open_upload_3mf();
  //wxGetApp().mainframe->print_plate(MainFrame::eUpload3mf);
 
}



void BBLTopbar::OnFullScreen(wxAuiToolBarEvent& event)
{
    if (m_frame->IsMaximized()) {
        BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " Restore";   
        m_frame->Restore();
    }
    else {
        BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " Maximize";   
        m_normalRect = m_frame->GetRect();
        m_frame->Maximize();
    }
}

void BBLTopbar::OnCloseFrame(wxAuiToolBarEvent& event)
{
    BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " start";   
    m_frame->Close();
    BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " end";
}

void BBLTopbar::OnMouseLeftDClock(wxMouseEvent& mouse)
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    // check whether mouse is not on any tool item
    if (this->FindToolByCurrentPosition() != NULL &&
        this->FindToolByCurrentPosition() != m_title_item) {
        mouse.Skip();
        return;
    }
#ifdef __W1XMSW__
    ::PostMessage((HWND) m_frame->GetHandle(), WM_NCLBUTTONDBLCLK, HTCAPTION, MAKELPARAM(mouse_pos.x, mouse_pos.y));
    return;
#endif //  __WXMSW__

    wxAuiToolBarEvent evt;
    OnFullScreen(evt);
}

void BBLTopbar::OnFileToolItem(wxAuiToolBarEvent& evt)
{
    wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(evt.GetEventObject());

    tb->SetToolSticky(evt.GetId(), true);

    if (!m_skip_popup_file_menu) {
        GetParent()->PopupMenu(m_file_menu, wxPoint(FromDIP(1), this->GetSize().GetHeight() - 2));
    }
    else {
        m_skip_popup_file_menu = false;
    }

    // make sure the button is "un-stuck"
    tb->SetToolSticky(evt.GetId(), false);
}

void BBLTopbar::OnDropdownToolItem(wxAuiToolBarEvent& evt)
{
    wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(evt.GetEventObject());

    tb->SetToolSticky(evt.GetId(), true);

    if (m_helpItem)
    {
        auto         guideItem = m_helpItem->FindItem(wxID_FIND);
        ButtonsCtrl* pCtr      = dynamic_cast<ButtonsCtrl*>(m_tabCtrol);
        int          index     = pCtr->GetSelection();
        if (guideItem)
            guideItem->Enable(index == 1);
    }

    if (!m_skip_popup_dropdown_menu) {
        GetParent()->PopupMenu(&m_top_menu, wxPoint(FromDIP(1), this->GetSize().GetHeight() - 2));
    }
    else {
        m_skip_popup_dropdown_menu = false;
    }

    // make sure the button is "un-stuck"
    tb->SetToolSticky(evt.GetId(), false);
}

void BBLTopbar::OnCalibToolItem(wxAuiToolBarEvent &evt)
{
    wxAuiToolBar *tb = static_cast<wxAuiToolBar *>(evt.GetEventObject());

    tb->SetToolSticky(evt.GetId(), true);

    if (!m_skip_popup_calib_menu) {
        auto rec = this->GetToolRect(ID_CALIB);
        GetParent()->PopupMenu(&m_calib_menu, wxPoint(rec.GetLeft(), this->GetSize().GetHeight() - 2));
    } else {
        m_skip_popup_calib_menu = false;
    }

    // make sure the button is "un-stuck"
    tb->SetToolSticky(evt.GetId(), false);
}

void BBLTopbar::OnMouseLeftDown(wxMouseEvent& event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    wxPoint frame_pos = m_frame->GetScreenPosition();
    m_delta = mouse_pos - frame_pos;

    if (FindToolByCurrentPosition() == NULL 
        || this->FindToolByCurrentPosition() == m_title_item)
    {
        CaptureMouse();
#ifdef __WXMSW__
        ReleaseMouse();
        ::PostMessage((HWND) m_frame->GetHandle(), WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(mouse_pos.x, mouse_pos.y));
        return;
#endif //  __WXMSW__
    }
    
    event.Skip();
}

void BBLTopbar::OnMouseLeftUp(wxMouseEvent& event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    if (HasCapture())
    {
        ReleaseMouse();
    }
    wxPoint           client_pos = this->ScreenToClient(mouse_pos);
    wxAuiToolBarItem* item       = this->FindToolByPosition(client_pos.x, client_pos.y);
    wxAuiToolBarItem* max_item   = this->FindTool(wxID_MAXIMIZE_FRAME);
    wxAuiToolBarItem* min_item   = this->FindTool(wxID_ICONIZE_FRAME);
    wxAuiToolBarItem* close_item = this->FindTool(wxID_CLOSE_FRAME);
    if (item == max_item) {
        wxAuiToolBarEvent evt;
        BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " OnFullScreen "
                                   << " mouse_pos.x=" << mouse_pos.x << " mouse_pos.y=" << mouse_pos.y;
        boost::log::core::get()->flush();

        OnFullScreen(evt);
    } else if (item == min_item) {
        wxAuiToolBarEvent evt;
        BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " OnIconize"
                                   << " mouse_pos.x=" << mouse_pos.x << " mouse_pos.y=" << mouse_pos.y;
        boost::log::core::get()->flush();

        OnIconize(evt);
    } else if (item == close_item) {
        wxAuiToolBarEvent evt;
        BOOST_LOG_TRIVIAL(warning) << __FUNCTION__ << " OnCloseFrame"
                                   << " mouse_pos.x=" << mouse_pos.x << " mouse_pos.y=" << mouse_pos.y;   
        boost::log::core::get()->flush();

        OnCloseFrame(evt);
    }
    event.Skip();
}

void BBLTopbar::OnMouseMotion(wxMouseEvent& event)
{
    wxPoint mouse_pos = ::wxGetMousePosition();

    if (!HasCapture()) {
        //m_frame->OnMouseMotion(event);

        event.Skip();
        return;
    }

    if (event.Dragging() && event.LeftIsDown())
    {
        // leave max state and adjust position 
        if (m_frame->IsMaximized()) {
            wxRect rect = m_frame->GetRect();
            // Filter unexcept mouse move
            if (m_delta + rect.GetLeftTop() != mouse_pos) {
                m_delta = mouse_pos - rect.GetLeftTop();
                m_delta.x = m_delta.x * m_normalRect.width / rect.width;
                m_delta.y = m_delta.y * m_normalRect.height / rect.height;
                m_frame->Restore();
            }
        }
        m_frame->Move(mouse_pos - m_delta);
    }
    event.Skip();
}

void BBLTopbar::OnMouseCaptureLost(wxMouseCaptureLostEvent& event)
{
}

void BBLTopbar::OnMenuClose(wxMenuEvent& event)
{
    wxAuiToolBarItem* item = this->FindToolByCurrentPosition();
    if (item == m_file_menu_item) {
        m_skip_popup_file_menu = true;
    }
    else if (item == m_dropdown_menu_item) {
        m_skip_popup_dropdown_menu = true;
    }
}

wxAuiToolBarItem* BBLTopbar::FindToolByCurrentPosition()
{
    wxPoint mouse_pos = ::wxGetMousePosition();
    wxPoint client_pos = this->ScreenToClient(mouse_pos);
    return this->FindToolByPosition(client_pos.x, client_pos.y);
}
