#include "DownloadUtils.h"
#include "../common.h"
#include "FSUtils.h"
#include "logger.h"
#include <coreinit/cache.h>
#include <curl/curl.h>

#define IO_BUFSIZE (128 * 1024) // 128 KB

bool DownloadUtils::libInitDone         = false;
uint8_t *DownloadUtils::cacert_pem      = nullptr;
uint32_t DownloadUtils::cacert_pem_size = 0;

static int initSocket(void *ptr, curl_socket_t socket, curlsocktype type) {
    int o = 1;

    // Activate WinScale
    int r = setsockopt(socket, SOL_SOCKET, SO_WINSCALE, &o, sizeof(o));
    if (r != 0) {
        DEBUG_FUNCTION_LINE_ERR("initSocket: Error setting WinScale: %d", r);
        return CURL_SOCKOPT_ERROR;
    }

    o = IO_BUFSIZE;
    // Set receive buffersize
    r = setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &o, sizeof(o));
    if (r != 0) {
        DEBUG_FUNCTION_LINE_ERR("initSocket: Error setting RBS: %d", r);
        return CURL_SOCKOPT_ERROR;
    }

    return CURL_SOCKOPT_OK;
}

size_t writeCallback(char *buf, size_t size, size_t nmemb, void *up) {
    auto *data = (std::string *) up;
    data->append(buf, size * nmemb);
    return size * nmemb; //tell curl how many bytes we handled
}

int progress_callback(void *clientp,
                      curl_off_t dltotal,
                      curl_off_t dlnow,
                      curl_off_t ultotal,
                      curl_off_t ulnow) {
    if (clientp != nullptr) {
        auto *progress = (float *) clientp;
        if (dltotal > 0 && dlnow > 0) {
            *progress = (float) dlnow / (float) dltotal;
        } else {
            *progress = 0.0f;
        }
        OSMemoryBarrier();
    }
    return 0;
}

bool DownloadUtils::Init() {
    if (libInitDone) {
        return true;
    }
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        return false;
    }

    cacert_pem      = nullptr;
    cacert_pem_size = 0;
    if (LoadFileToMem(CERT_FILE_LOCATION, &cacert_pem, &cacert_pem_size) < 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to load cert");
        cacert_pem      = nullptr;
        cacert_pem_size = 0;
    }

    libInitDone = true;
    return true;
}

void DownloadUtils::Deinit() {
    if (!libInitDone) {
        return;
    }

    if (cacert_pem != nullptr) {
        free(cacert_pem);
        cacert_pem      = nullptr;
        cacert_pem_size = 0;
    }
    curl_global_cleanup();
    libInitDone = false;
}

int DownloadUtils::DownloadFileToBuffer(const std::string &url, std::string &outBuffer, int &responseCodeOut, int &errorOut, std::string &errorTextOut, float *progress) {
    if (!libInitDone) {
        return -1;
    }

    // Start a curl session
    CURL *curl = curl_easy_init();
    if (!curl) {
        curl_global_cleanup();
        return -1;
    }

    if (cacert_pem != nullptr) {
        struct curl_blob blob {};
        blob.data  = (void *) cacert_pem;
        blob.len   = cacert_pem_size;
        blob.flags = CURL_BLOB_COPY;

        // Use the certificate bundle in the data
        curl_easy_setopt(curl, CURLOPT_CAINFO_BLOB, &blob);
    } else {
        errorOut     = 0x13371337;
        errorTextOut = "cacert.pem is not loaded";
        DEBUG_FUNCTION_LINE_ERR("Warning, missing certificate.");
        return -4;
    }

    // Enable optimizations
    curl_easy_setopt(curl, CURLOPT_SOCKOPTFUNCTION, initSocket);

    // Follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Set the download URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    if (progress != nullptr) {
        /* pass struct to callback  */
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &outBuffer);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);

    char error[CURL_ERROR_SIZE]; /* needs to be at least this big */
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);

    errorTextOut = {};
    errorOut     = 0;

    responseCodeOut = -1;
    if ((errorOut = curl_easy_perform(curl)) != CURLE_OK) {
        errorTextOut = error;
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCodeOut);

    curl_easy_cleanup(curl);

    return 0;
}
