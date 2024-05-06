#pragma once

#include <coreinit/title.h>

#define BACKGROUND_COLOR                     COLOR_BLACK

#define AROMA_DOWNLOAD_URL                   "https://aroma.foryour.cafe/"
#define UPDATE_SERVER_URL                    "https://aroma.foryour.cafe"
#define SD_PATH                              "fs:/vol/external01/"
#define DEFAULT_AROMA_ENVIRONMENT_PATH       "wiiu/environments/aroma"
#define DEFAULT_TIRAMISU_ENVIRONMENT_PATH    "wiiu/environments/tiramisu"
#define DEFAULT_AROMA_ENVIRONMENT_SD_PATH    SD_PATH DEFAULT_AROMA_ENVIRONMENT_PATH
#define DEFAULT_TIRAMISU_ENVIRONMENT_SD_PATH SD_PATH DEFAULT_TIRAMISU_ENVIRONMENT_PATH
#define UPDATE_OLD_SUFFIX                    ".update.old"
#define UPDATE_TEMP_SUFFIX                   ".update.temp"

#define UPDATER_VERSION                      "v0.2.3"
#define UPDATER_VERSION_FULL                 UPDATER_VERSION UPDATER_VERSION_EXTRA

inline bool RunningFromMiiMaker() {
    return (OSGetTitleID() & 0xFFFFFFFFFFFFF0FFull) == 0x000500101004A000ull;
}

extern bool gDeleteDefaultEnvironmentOnSuccess;