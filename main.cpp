#include "adc/Input.hpp"
#include "gpio/Base.hpp"
#include "Macro.hpp"
#include "pico/cyw43_arch.h"

#undef PICO_BOARD_TYPE
#define PICO_BOARD_TYPE PICO_W

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

    Adc::Input gp26A0(26, GPIO_26_ADC);

    gp26A0.installCallback(
        [](float voltage)
        {
            const float DIODE_V_DROP_ONE_WAY = .209;
            const float TOTAL_DIODE_DROP = DIODE_V_DROP_ONE_WAY * 2.0;
            const float MIN_SIGNAL = 0.05;
            printf("%f\n", voltage * 3.0 + ((voltage > MIN_SIGNAL)? TOTAL_DIODE_DROP : 0.0));
        }
    );

    MAIN_LOOP_START
    Adc::Input::runLoop();
    MAIN_LOOP_END
}
