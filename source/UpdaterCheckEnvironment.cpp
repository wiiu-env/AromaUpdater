#include "UpdaterState.h"
#include "utils/logger.h"
#include <mocha/mocha.h>

ApplicationState::eSubState UpdaterState::UpdateGetEnvironmentDirectory(Input *input) {
    char environmentPathBuffer[0x100];
    MochaUtilsStatus status;
    if ((status = Mocha_GetEnvironmentPath(environmentPathBuffer, sizeof(environmentPathBuffer))) == MOCHA_RESULT_SUCCESS) {
        this->mCurEnvironmentPath = environmentPathBuffer;
        DEBUG_FUNCTION_LINE("The environment path is %s", this->mCurEnvironmentPath.c_str());
    } else {
        DEBUG_FUNCTION_LINE_ERR("Failed to get the environment path. %s", Mocha_GetStatusStr(status));
        this->mCurEnvironmentPath = {};
    }
    this->mState = STATE_CHECK_ENVIRONMENT_DIRECTORY;
    return SUBSTATE_RUNNING;
}

ApplicationState::eSubState UpdaterState::UpdateCheckEnvironmentDirectory(Input *input) {
    if (this->mCurEnvironmentPath == DEFAULT_AROMA_ENVIRONMENT_SD_PATH) {
        this->mState = STATE_CHECK_VERSIONS;
        return SUBSTATE_RUNNING;
    }

    if (this->mCurEnvironmentPath == DEFAULT_TIRAMISU_ENVIRONMENT_SD_PATH) {
        mCurEnvironmentPath = DEFAULT_AROMA_ENVIRONMENT_SD_PATH;
        this->mState        = STATE_CHECK_VERSIONS;
        return SUBSTATE_RUNNING;
    }

    if (buttonPressed(input, Input::BUTTON_A)) {
        std::string sNeedle  = DEFAULT_AROMA_ENVIRONMENT_PATH;
        std::string sReplace = this->mCurEnvironmentPath;
        if (sReplace.starts_with(SD_PATH)) {
            sReplace = sReplace.substr(strlen(SD_PATH));
        }

        for (auto &base : mVersionInfo) {
            for (auto &cat : base.getMutableCategory()) {
                for (auto &package : cat.getMutablePackages()) {
                    for (auto &repo : package.getMutableRepositories()) {
                        for (auto &file : repo.getMutableFiles()) {
                            if (file.getPath().find(sNeedle) != std::string::npos) {
                                try {
                                    file.getMutablePath().replace(file.getMutablePath().find(sNeedle), sNeedle.size(), sReplace);
                                } catch (std::exception &e) {
                                    DEBUG_FUNCTION_LINE_WARN("%s", e.what());
                                }
                            }
                        }
                    }
                }
            }
        }
        this->mState = STATE_CHECK_VERSIONS;
    } else if (buttonPressed(input, Input::BUTTON_X) || buttonPressed(input, Input::BUTTON_1)) {
        this->mState = STATE_CHECK_VERSIONS;
    }

    return SUBSTATE_RUNNING;
}

void UpdaterState::RenderCheckEnvironmentDirectory() {
    if (!this->mCurEnvironmentPath.empty() && this->mCurEnvironmentPath != DEFAULT_AROMA_ENVIRONMENT_SD_PATH) {
        DrawUtils::setFontColor(COLOR_RED);
        DrawUtils::setFontSize(30);
        DrawUtils::print(16, 90, "Warning");

        DrawUtils::setFontColor(COLOR_WHITE);
        DrawUtils::setFontSize(20);
        std::string curEnvironmentDir = std::filesystem::path(this->mCurEnvironmentPath).filename();
        DrawUtils::print(16, 140, "By default the \"aroma\" environment will be updated, but the updater was launched");
        DrawUtils::printf(16, 160, false, "in the \"%s\" environment.", curEnvironmentDir.c_str());

        DrawUtils::printf(16, 220, false, "Press \ue042 to update the \"%s\" environment", curEnvironmentDir.c_str());
        DrawUtils::printf(16, 240, false, "Press \ue04e/\ue047 to update the \"aroma\" environment");
    }
}
