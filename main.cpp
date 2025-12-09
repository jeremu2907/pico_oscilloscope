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
    constexpr float VOLTAGE_LEVEL_BIAS = 1.509f;
    constexpr float FRONTEND_VOLTAGE_DIVIDER_RATIO = (1e3 + 10e3) / 1e3;
    constexpr float MINIMUN_VOLTAGE_THRESHOLD = 0.0075;
    constexpr float REVERSED_POLARITY_VOLTAGE_BIAS = 0.0;
    constexpr uint64_t MS_BETWEEN_SSD_1306_UPDATE = 100;

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
    Adc::Input gp28A2(28, Adc::Input::GPIO_28_ADC);
    Adc::Input gp27A1(27, Adc::Input::GPIO_27_ADC);
    gpio_pull_down(28);
    gpio_pull_down(27);
    I2c::Ssd1306 oled(20, 21, i2c0);

    float vChannel0 = 0.0;
    float vChannel1 = 0.0;

    gp28A2.installCallback(
        [&currentTimeMs, &oled, &vChannel0](float voltage)
        {
            float unbiasedVoltage = voltage - VOLTAGE_LEVEL_BIAS;
            vChannel0 = FRONTEND_VOLTAGE_DIVIDER_RATIO * unbiasedVoltage;
            if (abs(unbiasedVoltage) < MINIMUN_VOLTAGE_THRESHOLD)
            {
                vChannel0 = 0.0;
            }
            else if(vChannel0 < 0.0)
            {
                vChannel0 -= REVERSED_POLARITY_VOLTAGE_BIAS;
            }

            printf("ch_0 %f\n", vChannel0);
        });

    gp27A1.installCallback(
        [&currentTimeMs, &oled, &vChannel1](float voltage)
        {
            float unbiasedVoltage = voltage - VOLTAGE_LEVEL_BIAS;
            vChannel1 = FRONTEND_VOLTAGE_DIVIDER_RATIO * unbiasedVoltage;
            if (abs(unbiasedVoltage) < MINIMUN_VOLTAGE_THRESHOLD)
            {
                vChannel1 = 0.0;
            }
            else if (vChannel1 < 0.0)
            {
                vChannel1 -= REVERSED_POLARITY_VOLTAGE_BIAS;
            }

            printf("ch_1 %f\n", vChannel1);
        });

    MAIN_LOOP_START
    Adc::Input::runLoop();
    uint64_t now = to_ms_since_boot(get_absolute_time());
    if (now - currentTimeMs > MS_BETWEEN_SSD_1306_UPDATE)
    {
        std::string vChannel0Str = std::to_string(vChannel0);
        std::string vChannel1Str = std::to_string(vChannel1);
        std::string paddingChannel0 = ((vChannel0 < 0) ? "" : " ");
        std::string paddingChannel1 = ((vChannel1 < 0) ? "" : " ");
        std::string s = (std::string("\n") +
                        "ch_0:" + paddingChannel0 + vChannel0Str +
                        "\n\n" +
                        "ch_1:" + paddingChannel1 + vChannel1Str);
        oled.clearData();
        oled.setData(s);
        oled.writeData();
        currentTimeMs = now;
    }
    MAIN_LOOP_END
}
