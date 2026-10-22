/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "platform.h"

#include "common/time.h"

#include "drivers/time.h"

#include "pg/pg_ids.h"

#include "drivers/io.h"

#include "light_led.h"

#if !defined(USE_VIRTUAL_LED)

static IO_t leds[STATUS_LED_COUNT];
static uint8_t ledInversion = 0;

#if defined(USE_HSI_RGB_STATUS)
static bool ledStates[STATUS_LED_COUNT];
static IO_t heartbeatLed;
static IO_t armedLed;
static timeMs_t heartbeatNextToggle;

static void hsiRgbRender(void)
{
    const bool red = ledStates[2];
    const bool green = ledStates[1] || ledStates[2];
    const bool blue = ledStates[0];

    IOWrite(leds[0], blue ? (ledInversion & BIT(0)) : !(ledInversion & BIT(0)));
    IOWrite(leds[1], green ? (ledInversion & BIT(1)) : !(ledInversion & BIT(1)));
    IOWrite(leds[2], red ? (ledInversion & BIT(2)) : !(ledInversion & BIT(2)));
}

static void hsiIndicatorInit(void)
{
    heartbeatLed = IOGetByTag(IO_TAG(HEARTBEAT_LED_PIN));
    if (heartbeatLed) {
        IOInit(heartbeatLed, OWNER_LED, 3);
        IOConfigGPIO(heartbeatLed, IOCFG_OUT_OD);
        IOHi(heartbeatLed);
        heartbeatNextToggle = millis() + 500;
    }

    armedLed = IOGetByTag(IO_TAG(ARMED_PIN));
    if (armedLed) {
        IOInit(armedLed, OWNER_LED, 4);
        IOConfigGPIO(armedLed, IOCFG_OUT_OD);
        IOHi(armedLed);
    }
}
#endif

PG_REGISTER_WITH_RESET_TEMPLATE(statusLedConfig_t, statusLedConfig, PG_STATUS_LED_CONFIG, 0);

PG_RESET_TEMPLATE(statusLedConfig_t, statusLedConfig,
    .ioTags = {
#if STATUS_LED_COUNT > 0 && defined(LED0_PIN)
        [0] = IO_TAG(LED0_PIN),
#endif
#if STATUS_LED_COUNT > 1 && defined(LED1_PIN)
        [1] = IO_TAG(LED1_PIN),
#endif
#if STATUS_LED_COUNT > 2 && defined(LED2_PIN)
        [2] = IO_TAG(LED2_PIN),
#endif
    },
    .inversion = 0
#ifdef LED0_INVERTED
    | BIT(0)
#endif
#ifdef LED1_INVERTED
    | BIT(1)
#endif
#ifdef LED2_INVERTED
    | BIT(2)
#endif
    ,
);

void ledInit(const statusLedConfig_t *statusLedConfig)
{
    ledInversion = statusLedConfig->inversion;
    for (int i = 0; i < (int)ARRAYLEN(leds); i++) {
        leds[i] = IOGetByTag(statusLedConfig->ioTags[i]);
        if (leds[i]) {
            IOInit(leds[i], OWNER_LED, RESOURCE_INDEX(i));
            ioConfig_t config = IOCFG_OUT_PP;
#if defined(LED0_OPEN_DRAIN)
            if (i == 0) {
                config = IOCFG_OUT_OD;
            }
#endif
#if defined(LED1_OPEN_DRAIN)
            if (i == 1) {
                config = IOCFG_OUT_OD;
            }
#endif
#if defined(LED2_OPEN_DRAIN)
            if (i == 2) {
                config = IOCFG_OUT_OD;
            }
#endif
            IOConfigGPIO(leds[i], config);
        }
#if defined(USE_HSI_RGB_STATUS)
        ledStates[i] = false;
#else
        ledSet(i, false);
#endif
    }

#if defined(USE_HSI_RGB_STATUS)
    hsiRgbRender();
    hsiIndicatorInit();
#endif
}

void ledToggle(int led)
{
    if (led < 0 || led >= (int)ARRAYLEN(leds)) {
        return;
    }
#if defined(USE_HSI_RGB_STATUS)
    ledStates[led] = !ledStates[led];
    hsiRgbRender();
#else
    IOToggle(leds[led]);
#endif
}

void ledSet(int led, bool on)
{
    if (led < 0 || led >= (int)ARRAYLEN(leds)) {
        return;
    }
#if defined(USE_HSI_RGB_STATUS)
    ledStates[led] = on;
    hsiRgbRender();
#else
    const bool inverted = ledInversion & (1 << led);
    IOWrite(leds[led], on ? inverted : !inverted);
#endif
}

#if defined(USE_HSI_RGB_STATUS)
void hsiHeartbeatUpdate(void)
{
    if (heartbeatLed && cmpTimeMs(millis(), heartbeatNextToggle) >= 0) {
        IOToggle(heartbeatLed);
        heartbeatNextToggle += 500;
    }
}

void hsiArmedLedSet(bool armed)
{
    if (armedLed) {
        IOWrite(armedLed, !armed);
    }
}
#endif

#endif
