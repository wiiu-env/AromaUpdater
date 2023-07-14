#pragma once
#include <cstdint>
#include <string>

class DownloadUtils {
public:
    static bool Init();
    static void Deinit();

    static int DownloadFileToBuffer(const std::string &url, std::string &outBuffer, int &responseCodeOut, int &errorOut, std::string &errorTextOut, float *progress);

    static uint32_t cacert_pem_size;

private:
    static bool libInitDone;
    static uint8_t *cacert_pem;
};