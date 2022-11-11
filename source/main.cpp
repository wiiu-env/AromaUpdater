#include "MainApplicationState.h"
#include "input/CombinedInput.h"
#include "input/VPADInput.h"
#include "input/WPADInput.h"
#include "utils/DownloadUtils.h"
#include "utils/DrawUtils.h"
#include "utils/logger.h"
#include <coreinit/energysaver.h>
#include <filesystem>
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

    DownloadUtils::Deinit();

    WPADInput::close();

    DrawUtils::DeInit();

    AXQuit();
    deinitLogging();


    return 0;
}
