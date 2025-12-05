#if IS_WIRELESS
#include "pico/cyw43_arch.h"
#endif

#include "pico/time.h"
#include "math.h"

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
    Adc::Input gp28A2(26, Adc::Input::GPIO_28_ADC);
    gpio_pull_down(28);
    I2c::Ssd1306 oled(20, 21, i2c0);

    gp28A2.installCallback(
        [&currentTimeMs, &oled](float voltage)
        {
            float measuredVoltage = 11.0 * 1.05 * (voltage - 1.55613);
            if(abs(measuredVoltage) < 0.0393566074)
            {
                measuredVoltage = 0.0;
            }

            printf("%f\n", measuredVoltage);

            uint64_t now = to_ms_since_boot(get_absolute_time());
            if (now - currentTimeMs > 100)
            {
                std::string s = std::to_string(measuredVoltage);
                std::string padding = ((measuredVoltage < 0)? "" : " " );
                s = "\n\nVolts: " + padding + s;
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
