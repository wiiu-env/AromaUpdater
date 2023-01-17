#pragma once

#include "common.h"
#include "input/Input.h"
#include "utils/DrawUtils.h"
#include "version.h"
#include <cstdint>

class ApplicationState {
public:
    enum eSubState {
        SUBSTATE_RUNNING,
        SUBSTATE_RETURN,
        SUBSTATE_RESTART,
    };

    virtual ~ApplicationState() = default;

    virtual void render() = 0;

    virtual eSubState update(Input *input) = 0;

    virtual void proccessMenuNavigationY(Input *input, int32_t maxOptionValue) {
        if (input->data.buttons_d & Input::BUTTON_UP) {
            this->selectedOptionY--;
        } else if (input->data.buttons_d & Input::BUTTON_DOWN) {
            this->selectedOptionY++;
        }
        if (this->selectedOptionY < 0) {
            this->selectedOptionY = maxOptionValue - 1;
        } else if (this->selectedOptionY >= maxOptionValue) {
            this->selectedOptionY = 0;
        }
    }

    virtual void proccessMenuNavigationX(Input *input, int32_t maxOptionValue) {
        if ((input->data.buttons_d & Input::BUTTON_LEFT) || (input->data.buttons_d & Input::BUTTON_L)) {
            this->selectedOptionX--;
        } else if ((input->data.buttons_d & Input::BUTTON_RIGHT) || (input->data.buttons_d & Input::BUTTON_R)) {
            this->selectedOptionX++;
        }

        if (this->selectedOptionX < 0) {
            this->selectedOptionX = maxOptionValue - 1;
        } else if (this->selectedOptionX >= maxOptionValue) {
            this->selectedOptionX = 0;
        }
    }

    virtual bool buttonPressed(Input *input, Input::eButtons button) {
        return input->data.buttons_d & button;
    }

    virtual bool entrySelected(Input *input) {
        return input->data.buttons_d & Input::BUTTON_A;
    }

    virtual void printHeader() {
        DrawUtils::setFontColor(COLOR_WHITE);
        DrawUtils::drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 4, COLOR_WHITE);

        DrawUtils::setFontSize(24);
        DrawUtils::print(16, 34, "Aroma Updater " UPDATER_VERSION_FULL);
        DrawUtils::setFontSize(20);

        DrawUtils::drawRectFilled(0, 44, SCREEN_WIDTH, 2, COLOR_WHITE);
        DrawUtils::drawRectFilled(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 2, COLOR_WHITE);
    }

    virtual void printFooter() {
    }

    int selectedOptionY = 0;
    int selectedOptionX = 0;
};
