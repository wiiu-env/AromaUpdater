/****************************************************************************
 * Copyright (C) 2021 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "MainApplicationState.h"
#include "UpdaterState.h"
#include "common.h"
#include "utils/DrawUtils.h"
#include "utils/logger.h"

MainApplicationState::MainApplicationState() {
    this->state = STATE_WELCOME_SCREEN;
}

MainApplicationState::~MainApplicationState() = default;

void MainApplicationState::render() {
    if (this->state == STATE_DO_SUBSTATE) {
        if (this->subState == nullptr) {
            OSFatal("SubState was null");
        }
        this->subState->render();
        return;
    }
    DrawUtils::beginDraw();
    DrawUtils::clear(BACKGROUND_COLOR);

    if (this->state == STATE_WELCOME_SCREEN) {
        printHeader();

        DrawUtils::setFontColor(COLOR_WHITE);
        DrawUtils::setFontSize(30);
        const char *text = "Welcome to the Aroma Updater";
        DrawUtils::print(SCREEN_WIDTH / 2 - DrawUtils::getTextWidth(text) / 2, SCREEN_HEIGHT / 2 - 15, text);
        DrawUtils::setFontSize(18);
        DrawUtils::print(SCREEN_WIDTH - 16, SCREEN_HEIGHT - 14, "\ue000 Check for updates", true);
        const char *exitHint = "\ue044 Exit";
        DrawUtils::print(SCREEN_WIDTH / 2 + DrawUtils::getTextWidth(exitHint) / 2, SCREEN_HEIGHT - 14, exitHint, true);
        printFooter();
    }


    DrawUtils::endDraw();
}

ApplicationState::eSubState MainApplicationState::update(Input *input) {
    if (this->state == STATE_WELCOME_SCREEN) {
        proccessMenuNavigationY(input, 1);
        if (entrySelected(input)) {
            if (this->selectedOptionY == 0) {
                this->state    = STATE_DO_SUBSTATE;
                this->subState = std::make_unique<UpdaterState>();
            }
            this->selectedOptionY = 0;
        }

    } else if (this->state == STATE_DO_SUBSTATE) {
        auto retSubState = this->subState->update(input);
        if (retSubState == SUBSTATE_RUNNING) {
            // keep running.
            return SUBSTATE_RUNNING;
        } else if (retSubState == SUBSTATE_RETURN) {
            this->subState.reset();
            this->state = STATE_WELCOME_SCREEN;
        } else if (retSubState == SUBSTATE_SHUTDOWN) {
            this->subState.reset();
            this->state = STATE_SHUTDOWN;
        }
    }
    return SUBSTATE_RUNNING;
}
