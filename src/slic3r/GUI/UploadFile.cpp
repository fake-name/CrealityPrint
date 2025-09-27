#include "UploadFile.hpp"

#include <fstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "GUI.hpp"
#include "GUI_App.hpp"
#include "../Utils/Http.hpp"
#include "../Utils/json_diff.hpp"


#include <iostream>
#include <fstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
namespace Slic3r { 
namespace GUI {
UploadFile::UploadFile() {
    AlibabaCloud::OSS::InitializeSdk();
}
UploadFile::~UploadFile(){
    AlibabaCloud::OSS::ShutdownSdk();
}
int UploadFile::getAliyunInfo()
{
    int nRet = -1;

    std::string base_url = get_cloud_api_url();
    auto               preupload_profile_url = "/api/cxy/account/v2/getAliyunInfo";
    std::string url                   = base_url + preupload_profile_url;

    std::map<std::string, std::string> mapHeader;
    wxGetApp().getExtraHeader(mapHeader);
    Http::set_extra_headers(mapHeader);
    Http               http                  = Http::post(url);
    boost::uuids::uuid uuid                  = boost::uuids::random_generator()();
    json               j = json::object();


    std::string body = j.dump();
    http.header("Content-Type", "application/json")
        .header("Connection", "keep-alive")
        .header("__CXY_REQUESTID_", to_string(uuid))
        .set_post_body(body)
        .on_complete([&](std::string body, unsigned status) { 
            json jBody = json::parse(body);
            if (jBody["code"].is_number_integer()) {
                nRet = jBody["code"];
                if (jBody["code"].get<int>() == 4) {
                    nRet = 4;
                    return;
                }
            }
            if (nRet != 0)
            {
                throw ErrorCodeException("getAliyunInfo", nRet,jBody["msg"].get<std::string>());
                return;
            }
            if (!jBody["result"].is_object()) {
                nRet = -2;
                return;
            }
            if (!jBody["result"]["aliyunInfo"].is_object()) {
                nRet = -3;
                return;
            }
            m_token           = jBody["result"]["aliyunInfo"]["sessionToken"];
            m_accessKeyId = jBody["result"]["aliyunInfo"]["accessKeyId"];
            m_secretAccessKey = jBody["result"]["aliyunInfo"]["secretAccessKey"];

            nRet = 0;
        })
        .on_error([&](std::string body, std::string error, unsigned status) { nRet = -4;
        })
        .on_progress([&](Http::Progress progress, bool& cancel) {

        })
        .perform_sync();

    return nRet;
}

int UploadFile::getOssInfo()
{
    int nRet = -1;

    std::string        base_url = get_cloud_api_url();
    auto               preupload_profile_url = "/api/cxy/v2/common/getOssInfo";
    std::string        url                   = base_url + preupload_profile_url;

    std::map<std::string, std::string> mapHeader;
    wxGetApp().getExtraHeader(mapHeader);
    Http::set_extra_headers(mapHeader);
    Http               http                  = Http::post(url);
    boost::uuids::uuid uuid                  = boost::uuids::random_generator()();
    json               j                     = json::object();


    std::string body = j.dump();
    http.header("Content-Type", "application/json")
        .header("Connection", "keep-alive")
        .header("__CXY_REQUESTID_", to_string(uuid))
        .set_post_body(body)
        .on_complete([&](std::string body, unsigned status) {
            json jBody = json::parse(body);
            if (jBody["code"].is_number_integer()) {
                nRet = jBody["code"];
                if (jBody["code"].get<int>() == 4) {
                    nRet = 4;
                    return;
                }
            }
            if (nRet != 0)
            {
                throw ErrorCodeException("getOssInfo", nRet,jBody["msg"].get<std::string>());
                return;
            }
               
            if (!jBody["result"].is_object()) {
                nRet = -2;
                return;
            }
            if (!jBody["result"]["info"].is_object()) {
                nRet = -3;
                return;
            }
            m_endPoint           = jBody["result"]["info"]["endpoint"];
            m_bucket     = jBody["result"]["info"]["file"]["bucket"];

            nRet = 0;
        })
        .on_error([&](std::string body, std::string error, unsigned status) { nRet = -4;
        })
        .on_progress([&](Http::Progress progress, bool& cancel) {

        })
        .perform_sync();

    return nRet;
}

int UploadFile::uploadGcodeToCXCloud(const std::string& name, const std::string& fileName, std::function<void(std::string)> onCompleteCallback) {
    int nRet = -1;

    std::string        base_url = get_cloud_api_url();
    auto               preupload_profile_url = "/api/cxy/v2/gcode/uploadGcode";
    std::string        url                   = base_url + preupload_profile_url;

    std::map<std::string, std::string> mapHeader;
    wxGetApp().getExtraHeader(mapHeader);
    Http::set_extra_headers(mapHeader);
    Http               http                  = Http::post(url);
    boost::uuids::uuid uuid                  = boost::uuids::random_generator()();
    json               j                     = json::object();
    j["list"]                                = json::array();
    j["list"][0]                             = json::object();
    j["list"][0]["name"]                     = name;
    j["list"][0]["fileKey"]                  = fileName;
    j["list"][0]["type"]                  = 1;

    std::string body = j.dump();
    http.header("Content-Type", "application/json")
        .header("Connection", "keep-alive")
        .header("__CXY_REQUESTID_", to_string(uuid))
        .timeout_max(15)
        .set_post_body(body)
        .on_complete([&](std::string body, unsigned status) {
            json jBody = json::parse(body);
             if (jBody["code"].is_number_integer()) {
                nRet = jBody["code"];
                if(nRet !=0)
                {
                    if (jBody["code"].get<int>() == 4) {
                        nRet = 4;
                        return;
                    }
                   BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << "uploadGcodeToCXCloud body: " << body;
                   std::string errMsg = jBody["msg"].get<std::string>();
                   throw ErrorCodeException("uploadGcodeToCXCloud", nRet,errMsg);
                }
            }

            if(onCompleteCallback)onCompleteCallback(body);
             if (nRet != 0)
                return;
            //if (!jBody["result"].is_object()) {
            //    nRet = -2;
            //    return;
           //}
            //if (!jBody["result"]["info"].is_object()) {
            //    nRet = -3;
            //    return;
            //}

            nRet = 0;
        })
        .on_error([&](std::string body, std::string error, unsigned status) { nRet = -4;
        })
        .on_progress([&](Http::Progress progress, bool& cancel) {
            
        })
        .perform_sync();

    return nRet;
}

void UploadFile::setProcessCallback(std::function<void(int,double)> funcProcessCb)
{
    m_funcProcessCb = funcProcessCb;
}
void UploadFile::UploadProgressCallback(int partNumber, int totalParts, double percentage) {
    int i = (int)(percentage * 100);
    if (m_funcProcessCb != nullptr) {
        m_funcProcessCb(i,0);
    }
}

void UploadFile::ProgressCallback(size_t increment, int64_t transfered, int64_t total, void* userData) {
    int i = (int)((transfered * 1.0 / total * 1.0) * 100);
    if (m_funcProcessCb != nullptr) {
        m_funcProcessCb(i,0);
    }
}

const int64_t PART_SIZE = 1024 * 1024; // 1MB per part
string initiateUploadWithMeta(AlibabaCloud::OSS::OssClient& client, 
                            const string& bucketName, 
                            const string& objectName,
                            const AlibabaCloud::OSS::ObjectMetaData& metaData) {
    AlibabaCloud::OSS::InitiateMultipartUploadRequest request(bucketName, objectName,metaData);
    

    
    auto outcome = client.InitiateMultipartUpload(request);
    
    if (!outcome.isSuccess()) {
        throw runtime_error("InitiateMultipartUpload fail: " + 
                          outcome.error().Code() + ", " + 
                          outcome.error().Message());
    }
    
    return outcome.result().UploadId();
}

vector<AlibabaCloud::OSS::Part> UploadFile::uploadParts(AlibabaCloud::OSS::OssClient& client,
                       const string& bucketName,
                       const string& objectName,
                       const string& uploadId,
                       const string& filePath,
                       Slic3r::GUI::ProgressCallback callback) {
    vector<AlibabaCloud::OSS::Part> partList;
    ifstream file(filePath, ios::binary | ios::ate);
    
    if (!file.is_open()) {
        throw ErrorCodeException("uploadParts",1001,"Failed to open file: " + filePath);
    }
    
    int64_t fileSize = file.tellg();
    file.seekg(0, ios::beg);
    int partCount = static_cast<int>((fileSize + PART_SIZE - 1) / PART_SIZE);
    
    for (int i = 1; i <= partCount; i++) {
        int64_t partSize = min(PART_SIZE, fileSize - (i - 1) * PART_SIZE);
    
        char* buffer = new char[partSize];
        file.read(buffer, partSize);
        size_t bytesRead = file.gcount();
        shared_ptr<std::iostream> streambuffer = make_shared<std::stringstream>(std::string(buffer,bytesRead));
        
        AlibabaCloud::OSS::UploadPartRequest request(bucketName, objectName,i, uploadId, streambuffer);
        
        auto outcome = client.UploadPart(request);
        
        if (!outcome.isSuccess()) {
            file.close();
            throw ErrorCodeException("UploadPart",1002,"UploadPart fail: " + 
                              outcome.error().Code() + ", " + 
                              outcome.error().Message());
        }
        
        partList.emplace_back(i, outcome.result().ETag());
        
        if (callback) {
            double percentage = 70.0 * i / partCount/100;
            callback(i, partCount, 0.3+percentage);
        }
        if(m_cancel)
        {
            file.close();
            throw ErrorCodeException("UploadPart",601,"Upload canceled");
        }
    }
    
    file.close();
    return partList;
}

void completeMultipartUpload(AlibabaCloud::OSS::OssClient& client,
                           const string& bucketName,
                           const string& objectName,
                           const string& uploadId,
                           const vector<AlibabaCloud::OSS::Part>& partList) {
    AlibabaCloud::OSS::CompleteMultipartUploadRequest request(bucketName, objectName, partList,uploadId);
    auto outcome = client.CompleteMultipartUpload(request);
    
    if (!outcome.isSuccess()) {
        throw ErrorCodeException("CompleteMultipartUpload",1003,"CompleteMultipartUpload fail: " + 
                          outcome.error().Code() + ", " + 
                          outcome.error().Message());
    }
    
    cout << "\nMultipart upload completed. ETag: " 
         << outcome.result().ETag() << endl;
}

int UploadFile::uploadFileToAliyun(const std::string& local_path, const std::string& target_path, const std::string& fileName) {
    int nRet = 0;
    boost::nowide::ifstream file_in(local_path, std::ios_base::binary);
        if (!file_in)
        {
            std::cerr << "无法打开输入文件。" << std::endl;
            return 1;
        }
        auto call_back = std::bind(&UploadFile::UploadProgressCallback, this,
        std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3);
        call_back(0, 0, 0.01);
        // 创建输出文件，文件名后缀为.gz
        boost::filesystem::path temp_path = boost::filesystem::temp_directory_path();
        boost::filesystem::path outputfile("output.gz");
        boost::filesystem::path joined_path = temp_path / outputfile;
        boost::nowide::ofstream file_out(joined_path.string(), boost::nowide::ofstream::binary);
        if (!file_out)
        {
            std::cerr << "无法打开输出文件。" << std::endl;
            return 1;
        }
        // 创建过滤流缓冲区，用于压缩
        boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
        out.push(boost::iostreams::gzip_compressor());
        out.push(file_in);
        // 将压缩后的数据写入输出文件
        boost::iostreams::copy(out, file_out);
        file_out.close();
        if(m_cancel)
        {
            return 601;
        }
        std::string upload_path = target_path;// + ".gz";
    
    //AlibabaCloud::OSS::TransferProgress progress_callback = { call_back, nullptr };

    //std::shared_ptr<std::iostream> content = std::make_shared<std::fstream>(joined_path.string().c_str(), std::ios::in | std::ios::binary);
    //AlibabaCloud::OSS::PutObjectRequest request(m_bucket, upload_path, content);
    string uploadId;
    AlibabaCloud::OSS::ClientConfiguration conf;
    AlibabaCloud::OSS::OssClient oss_client(m_endPoint, m_accessKeyId, m_secretAccessKey, m_token, conf);
    try{
    AlibabaCloud::OSS::ObjectMetaData metaData;
    metaData.addHeader("Content-Disposition", "attachment;filename=\"" + wxGetApp().url_encode(fileName+".gcode.gz") + "\"");
    metaData.addHeader("Content-Type", "application/x-www-form-urlencoded");
    metaData.addHeader("disabledMD5", "false");
    //request.setTransferProgress(progress_callback);


    call_back(0, 0, 0.3);
    uploadId = initiateUploadWithMeta(oss_client, m_bucket, upload_path, metaData);
    auto partList = uploadParts(oss_client, m_bucket, upload_path, uploadId, joined_path.string(),call_back);
        
    cout << "Completing upload..." << endl;
    completeMultipartUpload(oss_client, m_bucket, upload_path, uploadId, partList);
    }catch(const ErrorCodeException& e){
        if (!uploadId.empty()) {
            AlibabaCloud::OSS::AbortMultipartUploadRequest request(m_bucket, upload_path, uploadId);
            oss_client.AbortMultipartUpload(request);
        }
        nRet = e.code();
        m_lastError.code = std::to_string(e.code());
        m_lastError.message = e.msg();
    }catch(const exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        if (!uploadId.empty()) {
            AlibabaCloud::OSS::AbortMultipartUploadRequest request(m_bucket, upload_path, uploadId);
            oss_client.AbortMultipartUpload(request);
        }
    
    }
    //auto outcome = oss_client.PutObject(request);
    if(m_cancel)
    {
        return 601;
    }
    

    return nRet; 
}

int UploadFile::downloadFileFromAliyun(const std::string& target_path, const std::string& local_path) {
    int nRet = 0;

    auto call_back = std::bind(&UploadFile::ProgressCallback, this,
        std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4);
    AlibabaCloud::OSS::TransferProgress progress_callback = { call_back, nullptr };

    AlibabaCloud::OSS::DownloadObjectRequest request(m_bucket, target_path, local_path);
    request.setTransferProgress(progress_callback);
    request.setPartSize(100 * 1024);
    request.setThreadNum(4);
    //request.setCheckpointDir("E:/999");

    AlibabaCloud::OSS::ClientConfiguration conf;
    AlibabaCloud::OSS::OssClient oss_client(m_endPoint, m_accessKeyId, m_secretAccessKey, m_token, conf);
    auto outcome = oss_client.ResumableDownloadObject(request);
    if (!outcome.isSuccess()) {
        m_lastError.code = outcome.error().Code();
        m_lastError.message = outcome.error().Message();
        m_lastError.requestId = outcome.error().RequestId();
        nRet = 1;
        return nRet;
    }

    return nRet;
}


}
} // namespace Slic3r::GUI
