#include "MainApplicationState.h"
#include "input/CombinedInput.h"
#include "input/VPADInput.h"
#include "input/WPADInput.h"
#include "utils/DownloadUtils.h"
#include "utils/DrawUtils.h"
#include "utils/logger.h"
#include <coreinit/energysaver.h>
#include <filesystem>
#include <mocha/mocha.h>
#include <rpxloader/rpxloader.h>
#include <sndcore2/core.h>
#include <whb/proc.h>

void main_loop() {
    DEBUG_FUNCTION_LINE_VERBOSE("Creating state");
    std::unique_ptr<MainApplicationState> state = std::make_unique<MainApplicationState>();
    CombinedInput baseInput;
    VPadInput vpadInput;
    WPADInput wpadInputs[4] = {
            WPAD_CHAN_0,
            WPAD_CHAN_1,
            WPAD_CHAN_2,
            WPAD_CHAN_3};

    DEBUG_FUNCTION_LINE_VERBOSE("Entering main loop");
    while (WHBProcIsRunning()) {
        baseInput.reset();
        if (vpadInput.update(1280, 720)) {
            baseInput.combine(vpadInput);
        }
        for (auto &wpadInput : wpadInputs) {
            if (wpadInput.update(1280, 720)) {
                baseInput.combine(wpadInput);
            }
        }
        baseInput.process();
        state->update(&baseInput);
        state->render();
    }
}

int main() {
    initLogging();

    WHBProcInit();
    if (!DrawUtils::Init()) {
        OSFatal("Failed to init DrawUtils");
    }
    AXInit();
    WPADInput::init();

    if (!DownloadUtils::Init()) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init DownloadUtils");
        OSFatal("Failed to init DownloadUtils");
    }

    int mochaInitResult;
    if ((mochaInitResult = Mocha_InitLibrary()) != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Mocha_InitLibrary() failed %d", mochaInitResult);
    }

    RPXLoaderStatus resRPX;
    if ((resRPX = RPXLoader_InitLibrary()) == RPX_LOADER_RESULT_SUCCESS) {
        if ((resRPX = RPXLoader_UnmountCurrentRunningBundle()) != RPX_LOADER_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("RPXLoader_UnmountCurrentRunningBundle failed: %d, %s", resRPX, RPXLoader_GetStatusStr(resRPX));
        }
        RPXLoader_DeInitLibrary();
    } else {
        DEBUG_FUNCTION_LINE_ERR("RPXLoader_InitLibrary failed %d %s", resRPX, RPXLoader_GetStatusStr(resRPX));
    }

    uint32_t isAPDEnabled;
    IMIsAPDEnabled(&isAPDEnabled);

    if (isAPDEnabled) {
        DEBUG_FUNCTION_LINE_VERBOSE("Disable auto shutdown");
        IMDisableAPD();
    }

    main_loop();

    if (isAPDEnabled) {
        DEBUG_FUNCTION_LINE_VERBOSE("Enable auto shutdown");
        IMEnableAPD();
    }

    if (mochaInitResult == MOCHA_RESULT_SUCCESS) {
        Mocha_DeInitLibrary();
    }

    DownloadUtils::Deinit();

    WPADInput::close();

    DrawUtils::DeInit();

    AXQuit();
    deinitLogging();


    return 0;
}
