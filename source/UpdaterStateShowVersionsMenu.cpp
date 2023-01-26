#include "UpdaterState.h"
#include "utils/UpdateUtils.h"
#include "utils/utils.h"

ApplicationState::eSubState UpdaterState::UpdateShowVersionsMenu(Input *input) {
    auto oldX = selectedOptionX;
    proccessMenuNavigationX(input, (int32_t) mVersionInfo.size());
    if (selectedOptionX != oldX) {
        selectedOptionY = 0;
    } else {
        auto &curPage = mVersionInfo[selectedOptionX];
        proccessMenuNavigationY(input, curPage.getPackagesCount(mOnlyRequired));
    }

    int32_t offset = 0;
    bool stop      = false;
    if (entrySelected(input)) {
        auto &curPage = mVersionInfo[selectedOptionX];
        for (auto &cat : curPage.getMutableCategory()) {
            if (stop) { break; }
            for (auto &package : cat.getMutablePackages()) {
                if (!package.getRequired() && mOnlyRequired) {
                    continue;
                }
                if (offset == selectedOptionY) {
                    package.toggleSelect();
                    stop = true;
                    break;
                }
                offset++;
            }
        }
    }

    if (buttonPressed(input, Input::BUTTON_PLUS)) {
        uint32_t countValid = 0;
        for (auto &base : this->mVersionInfo) {
            for (auto &cat : base.getCategory()) {
                for (auto &package : cat.getPackages()) {
                    if (package.getRequired() || package.getIsSelected()) {
                        if (package.getStatus() != VersionCheck::PackageStatus::Latest) {
                            countValid++;
                            break;
                        }
                    }
                }
            }
        }
        if (countValid > 0) {
            mState = STATE_CONFIRM_PACKAGES;
        } else {
            mState = STATE_SELECTED_PACKAGES_EMPTY;
        }
    }
    return SUBSTATE_RUNNING;
}


void UpdaterState::RenderShowVersions() {
    DrawUtils::setFontColor(COLOR_WHITE);
    DrawUtils::setFontSize(20);
    uint32_t x;
    uint32_t y = 100;

    DrawUtils::print(16, 70, "Please select the packages you want to update/install:");
    DrawUtils::print(SCREEN_WIDTH - 16, y, "Last update:", true);

    auto pageText = string_format("Page %d/%d", selectedOptionX + 1, mVersionInfo.size());
    DrawUtils::print(SCREEN_WIDTH / 2 - DrawUtils::getTextWidth(pageText.c_str()) / 2, y, pageText.c_str());

    y = 120;

    DrawUtils::setFontSize(20);

    bool isSelectedHint = false;
    bool isRequiredHint = false;

    int packageI  = 0;
    auto &curPage = mVersionInfo[selectedOptionX];
    for (auto &category : curPage.getCategory()) {
        x = 40;
        if (category.getPackagesCount(mOnlyRequired) == 0) {
            continue;
        }

        DrawUtils::setFontColor(COLOR_WHITE);
        DrawUtils::printf(x, y, false, "%s:", category.getName().c_str());
        y += 25;
        x += 25;

        for (auto &package : category.getPackages()) {
            if (!package.getRequired() && mOnlyRequired) {
                continue;
            }

            if (packageI == selectedOptionY) {
                DrawUtils::setFontColor(COLOR_WHITE);
                DrawUtils::print(17, y - 2, "\ue090");
                isSelectedHint = package.getIsSelected();
                isRequiredHint = package.getRequired();
            }

            uint32_t boxsize   = 16;
            uint32_t boxborder = 2;
            DrawUtils::drawRect(40, y - boxsize, boxsize, boxsize, boxborder, COLOR_WHITE);
            if (package.getRequired() || package.getIsSelected()) {
                DrawUtils::drawRectFilled(40 + (boxborder * 2), y - boxsize + (boxborder * 2), boxsize - boxborder * 4, boxsize - boxborder * 4, package.getRequired() ? COLOR_GRAY : COLOR_WHITE);
            }


            DrawUtils::setFontColor(COLOR_WHITE);
            DrawUtils::printf(x, y, false, "%s", package.getName().c_str());

            if (package.getStatus() == VersionCheck::PackageStatus::Latest) {
                DrawUtils::setFontColor(COLOR_GREEN);
            } else {
                DrawUtils::setFontColor(COLOR_YELLOW);
            }


            DrawUtils::printf(x + DrawUtils::getTextWidth(package.getName().c_str()) + (package.getRequired() ? DrawUtils::getTextWidth("*") : 0) + 5,
                              y,
                              false,
                              "[%s]",
                              UpdateUtils::StatusToString(package.getStatus()));


            if (package.getRequired()) {
                DrawUtils::setFontColor(COLOR_RED);
                DrawUtils::print(x + DrawUtils::getTextWidth(package.getName().c_str()), y, "*");
            }

            DrawUtils::setFontColor(COLOR_WHITE);

            DrawUtils::printf(SCREEN_WIDTH - 16, y, true, "%s", package.getLastUpdateDate().substr(0, 10).c_str());


            y += 20;
            packageI++;
        }
        y += 20;
    }


    DrawUtils::setFontSize(18);

    DrawUtils::setFontColor(COLOR_RED);
    DrawUtils::print(16, SCREEN_HEIGHT - 50, "*");
    DrawUtils::setFontColor(COLOR_WHITE);
    DrawUtils::print(16 + DrawUtils::getTextWidth("*"), SCREEN_HEIGHT - 50, "This package is required");

    DrawUtils::setFontColor(COLOR_WHITE);
    DrawUtils::print(16, SCREEN_HEIGHT - 14, "\ue07d Navigate packages / \ue083\ue084 Change pages");
    if (!isRequiredHint) {
        DrawUtils::printf(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, true, "\ue000 %s", isSelectedHint ? "Unselect" : "Select");
    } else {
        DrawUtils::printf(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, true, "(This package is required)");
    }

    const char *updateHint = "Press \ue045 to update";
    DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 50, updateHint, true);
}
