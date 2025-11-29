#include <vector>
#include <cstring>

#include "gpio/Output.hpp"
#include "gpio/Input.hpp"
#include "adc/Input.hpp"
#include "i2c/Ssd1306.h"
#include "i2c/Font8x8.h"
#include "Macro.hpp"

int main()
{
#if PICO_BOARD_TYPE == PICO_W
    if (cyw43_arch_init())
    {
        printf("Wi-Fi init failed!\n");
        return 1;
    }
#endif

    stdio_init_all();
    Gpio::Base::onboardLedOn();
    
    I2c::Ssd1306 oled;

    double i = 0.0;
    while(true)
    {
        sleep_ms(500);
        std::string s = std::to_string(i);
        uint8_t screenData[8 * 128] = {0x00};
        Font8x8::getFont(screenData, s);
        printf("write size %d", oled.writeScreen(screenData, 8 * 128));
        i += 0.5;
    }

    MAIN_LOOP_START
    Gpio::Input::runLoop();
    Adc::Input::runLoop();
    MAIN_LOOP_END
}
