#include "pico/cyw43_arch.h"
#include "pico/time.h"

#include "Macros.hpp"

#include "adc/Input.hpp"
#include "gpio/Base.hpp"
#include "i2c/Ssd1306.h"
#include "i2c/Font8x8.h"

int main()
{
#if IS_WIRELESS
    if (cyw43_arch_init())
    {
        printf("Wi-Fi init failed!\n");
        return 1;
    }
#endif

    stdio_init_all();
    Gpio::Base::onboardLedOn();

    uint64_t currentTimeMs = to_ms_since_boot(get_absolute_time());
    Adc::Input gp26A0(26, Adc::Input::GPIO_26_ADC);
    I2c::Ssd1306 oled(20, 21, i2c0);

    gp26A0.installCallback(
        [&currentTimeMs, &oled](float voltage)
        {
            const float DIODE_V_DROP_ONE_WAY = .209;
            const float TOTAL_DIODE_DROP = DIODE_V_DROP_ONE_WAY * 2.0;
            const float MIN_SIGNAL = 0.05;
            const float VOLTS = voltage * 3.0 + ((voltage > MIN_SIGNAL) ? TOTAL_DIODE_DROP : 0.0);
            printf("%f\n", VOLTS);

            uint64_t now = to_ms_since_boot(get_absolute_time());
            if (now - currentTimeMs > 100)
            {
                std::string s = std::to_string(VOLTS);
                s = "\n\nVolts: " + s;
                oled.clearData();
                oled.setData(s);
                oled.writeData();
                currentTimeMs = now;
            }
        });

    MAIN_LOOP_START
    Adc::Input::runLoop();
    MAIN_LOOP_END
}
