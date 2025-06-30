#ifndef RemotePrint_DmUtils_hpp_
#define RemotePrint_DmUtils_hpp_
#include "nlohmann/json.hpp"
#include <string>

class wxWebView;
namespace DM{

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
// ==================== �߳̿��ƺ����� ====================
class ThreadController {
public:
    ThreadController() : stop_requested(false) {}

    // �����ж�
    void requestStop() {
        std::lock_guard<std::mutex> lock(mtx);
        stop_requested = true;
        cv.notify_all();
    }
    void reset() {
        std::lock_guard<std::mutex> lock(mtx);
        stop_requested = false; // ����жϱ�־
    }
    // ����ж�״̬
    bool isStopRequested() const {
        return stop_requested.load(std::memory_order_relaxed);
    }

    // ����ʱ�ĵȴ��жϼ��
    template<typename Rep, typename Period>
    bool waitForStop(const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mtx);
        return cv.wait_for(lock, timeout, [this] {
            return stop_requested.load();
            });
    }

private:
    std::atomic<bool> stop_requested;
    mutable std::mutex mtx;
    std::condition_variable cv;
};

class AppUtils{
public:
    static void PostMsg(wxWebView*browse, const std::string&data);
    static void PostMsg(wxWebView*browse, nlohmann::json&data);
    static std::string MD5(const std::string&file);

    static std::string extractDomain(const std::string& url);
};
class LANConnectCheck {
public: 
    static bool pingHostWithRetry(const std::string& ip, ThreadController& ctrl,  // ע�������,
        int retries = 1, int timeout_ms = 1000, int delay_ms = 200);// �ж��豸�Ƿ���ͬһ���Σ�ping��
    static bool isPortOpen(const std::string& ip, int port, ThreadController& ctrl);    //// ���80�˿ڣ�9999�˿��Ƿ���������

    static int checkLan(const std::string& ip, ThreadController& ctrl);
};


}
#endif