#ifndef slic3r_PrinterMgrView_hpp_
#define slic3r_PrinterMgrView_hpp_


#include "wx/artprov.h"
#include "wx/cmdline.h"
#include "wx/notifmsg.h"
#include "wx/settings.h"
#include <vector>
#include <wx/webview.h>
#include <wx/string.h>
#include <boost/thread.hpp>
#if wxUSE_WEBVIEW_EDGE
#include "wx/msw/webview_edge.h"
#endif

#include "wx/webviewarchivehandler.h"
#include "wx/webviewfshandler.h"
#include "wx/numdlg.h"
#include "wx/infobar.h"
#include "wx/filesys.h"
#include "wx/fs_arc.h"
#include "wx/fs_mem.h"
#include "wx/stdpaths.h"
#include <wx/panel.h>
#include <wx/tbarbase.h>
#include "wx/textctrl.h"
#include <wx/timer.h>
#include "nlohmann/json_fwd.hpp"
#include <slic3r/GUI/print_manage/AppUtils.hpp>



namespace Slic3r {
    namespace GUI {

        class PrinterMgrView : public wxPanel {
        public:
            PrinterMgrView(wxWindow* parent);
            virtual ~PrinterMgrView();

            void load_url(const wxString& url, wxString apikey = "");
            void UpdateState();
            void OnClose(wxCloseEvent& evt);
            void OnError(wxWebViewEvent& evt);
            void OnLoaded(wxWebViewEvent& evt);
            
            void reload();
            void RequestDeviceListFromDB();

            void OnScriptMessage(wxWebViewEvent& evt);
            void ExecuteScriptCommand(const std::string& commandInfo, bool async = false);
            void RegisterHandler(const std::string& command, std::function<void(const nlohmann::json&)> handler);
            void UnregisterHandler(const std::string& command);
            virtual bool Show(bool show = true) wxOVERRIDE;
            void run_script(std::string content);
            
            void on_switch_to_device_page();
            void forward_init_device_cmd_to_printer_list();
            void request_refresh_all_device();
            int load_machine_preset_data();
            int getFileListFromLanDevice(const std::string strIp);
            int deleteFileListFromLanDevice(const std::string strIp, const std::string strName);
            int uploadeFileLanDevice(const std::string strIp);

            bool LoadFile(std::string jPath, std::string & sContent);

            // need to close the video when send page is opened
            void request_close_detail_page();

            void request_reopen_detail_video();

            std::vector<std::string> get_all_device_macs() const;
            bool should_upload_device_info() const;
            void set_finish_upload_device_state(bool finish) { m_finish_upload_device_state = finish; }
            bool get_finish_upload_device_state() const { return m_finish_upload_device_state; }

        private:
            void SendAPIKey();
            std::string get_plate_data_on_show();
            void handle_set_device_relate_to_account(const nlohmann::json& json_data);
            void handle_request_update_device_relate_to_account(const nlohmann::json& json_data);
        private:
            void down_file(std::string url, std::string name, std::string path_type);
            void down_files(std::vector<std::string> download_infos, std::string savePath, std::string path_type );
            void scan_device();
            void correct_device();

            wxWebView* m_browser;
            long m_zoomFactor;
            wxString m_apikey;
            bool m_apikey_sent;
            bool m_finish_upload_device_state {false};

            std::unordered_map<std::string, std::function<void(const nlohmann::json&)>> m_commandHandlers;
            std::unordered_map<std::string,std::string> m_devicePool;
            boost::thread m_scanPoolThread;
            bool m_scanExit = false;
            #ifdef __WXGTK__
            // When using GTK, there may be a problem of synthetic dirty area failure, so perform a low-frequency refresh
            wxTimer* m_freshTimer;
            #endif
            DM::ThreadController _ctrl;
            std::chrono::steady_clock::time_point lastSendTime;
            std::mutex sendMutex;
            void sendProgressWithRateLimit(std::string ip,float progress,double speed);
            // DECLARE_EVENT_TABLE()
        };

    } // GUI
} // Slic3r

#endif /* slic3r_Tab_hpp_ */
