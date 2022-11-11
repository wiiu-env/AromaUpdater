#pragma once
#include <string>

class DownloadUtils {
public:
    static bool Init();
    static void Deinit();
    static int DownloadFileToBuffer(const char *url, std::string &outBuffer, int &responseCodeOut, float *progress);

private:
    static bool libInitDone;
    static uint8_t *cacert_pem;
    static uint32_t cacert_pem_size;
};