#include "InterfaceBoard.hh"
#include "Configuration.hh"
#include "LiquidCrystal.hh"
#include "Host.hh"

#if defined HAS_INTERFACE_BOARD

InterfaceBoard::InterfaceBoard(ButtonArray& buttons_in,
                               LiquidCrystal& lcd_in,
                               const Pin& foo_pin_in,
                               const Pin& bar_pin_in,
                               Screen* mainScreen_in,
                               Screen* buildScreen_in,
                               MoodLightController& moodLight_in) :
    lcd(lcd_in),
    buttons(buttons_in),
    foo_pin(foo_pin_in),
    bar_pin(bar_pin_in),
    moodLight(moodLight_in)
{
    buildScreen = buildScreen_in;
    mainScreen = mainScreen_in;
}

void InterfaceBoard::init()
{
    buttons.init();

    lcd.begin(lcd.getDisplayWidth(), lcd.getDisplayHeight());
    lcd.clear();
    lcd.home();

    foo_pin.setValue(false);
    foo_pin.setDirection(true);
    bar_pin.setValue(false);
    bar_pin.setDirection(true);

    building = false;

    screenIndex = -1;

    pushScreen(mainScreen);
}

void InterfaceBoard::doInterrupt()
{
    buttons.scanButtons();
}

micros_t InterfaceBoard::getUpdateRate()
{
    return screenStack[screenIndex]->getUpdateRate();
}

void InterfaceBoard::doUpdate()
{

    // If we are building, make sure we show a build menu; otherwise,
    // turn it off.
    switch(host::getHostState())
    {
    case host::HOST_STATE_BUILDING:
    case host::HOST_STATE_BUILDING_FROM_SD:
    case host::HOST_STATE_ESTIMATING_FROM_SD:
        if (!building)
        {
            pushScreen(buildScreen);
            building = true;
        }
        break;
    default:
        if (building)
        {
            popScreen();
            building = false;
        }
        break;
    }


    static ButtonArray::ButtonName button;


    if (buttons.getButton(button))
    {
        screenStack[screenIndex]->notifyButtonPressed(button);
    }

    screenStack[screenIndex]->update(lcd, false);
}

bool InterfaceBoard::isButtonPressed(ButtonArray::ButtonName button)
{
    bool buttonPressed = buttons.isButtonPressed(button);

    if ( buttonPressed ) screenStack[screenIndex]->notifyButtonPressed(button);

    return buttonPressed;
}

void InterfaceBoard::pushScreen(Screen* newScreen)
{
    if (screenIndex < SCREEN_STACK_DEPTH - 1)
    {
        screenIndex++;
        screenStack[screenIndex] = newScreen;
    }
    screenStack[screenIndex]->reset();
    screenStack[screenIndex]->update(lcd, true);
}

void InterfaceBoard::popScreen()
{
    // Don't allow the root menu to be removed.
    if (screenIndex > 0)
    {
        screenIndex--;
    }

    screenStack[screenIndex]->update(lcd, true);
}

#endif
