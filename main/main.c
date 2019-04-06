#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <etstimer.h>
#include <esplibs/libmain.h>
#include <sysparam.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>
#include <ir/ir.h>
#include "ir.h"

#define LED_GPIO 2
#define IR_RX_GPIO 5
#define POWERED_ON_GPIO 12


void led_write(bool on) {
    gpio_write(LED_GPIO, on ? 0 : 1);
}


void led_init() {
    gpio_enable(LED_GPIO, GPIO_OUTPUT);
    led_write(false);
}


void tv_identify_task(void *_args) {
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            led_write(true);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            led_write(false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    led_write(false);

    vTaskDelete(NULL);
}

void tv_identify(homekit_value_t _value) {
    xTaskCreate(tv_identify_task, "TV identify", 128, NULL, 2, NULL);
}


void ir_dump_task(void *arg) {
    ir_rx_init(IR_RX_GPIO, 128);
    ir_decoder_t *tv_decoder = ir_tv_make_decoder();

    printf("Receiving IR commands\n");

    uint8_t buffer[4];
    while (1) {
        int size = ir_recv(tv_decoder, 0, buffer, sizeof(buffer));
        if (size <= 0)
            continue;

        printf("Decoded command 0x%02x 0x%02x 0x%02x 0x%02x\n",
               buffer[0], buffer[1], buffer[2], buffer[3]);
    }
}

void on_input_configured_name(homekit_characteristic_t *ch, homekit_value_t value, void *arg);


void on_configured_name(homekit_characteristic_t *ch, homekit_value_t value, void *arg) {
    sysparam_set_string("tv_name", value.string_value);
}

void on_active(homekit_characteristic_t *ch, homekit_value_t value, void *arg) {
    ir_tv_send(ir_tv_power);
}

void on_active_identifier(homekit_characteristic_t *ch, homekit_value_t value, void *arg) {
    switch (value.int_value) {
    case 1:
        ir_hdmi_switch_send(ir_hdmi_switch_input_1);
        break;
    case 2:
        ir_hdmi_switch_send(ir_hdmi_switch_input_2);
        break;
    case 3:
        ir_hdmi_switch_send(ir_hdmi_switch_input_3);
        break;
    default:
        printf("Unknown active identifier: %d", value.int_value);
    }
}

void on_remote_key(homekit_characteristic_t *ch, homekit_value_t value, void *arg) {
    switch (value.int_value) {
    case HOMEKIT_REMOTE_KEY_ARROW_UP:
        ir_tv_send(ir_tv_up);
        break;
    case HOMEKIT_REMOTE_KEY_ARROW_DOWN:
        ir_tv_send(ir_tv_down);
        break;
    case HOMEKIT_REMOTE_KEY_ARROW_LEFT:
        ir_tv_send(ir_tv_left);
        break;
    case HOMEKIT_REMOTE_KEY_ARROW_RIGHT:
        ir_tv_send(ir_tv_right);
        break;
    case HOMEKIT_REMOTE_KEY_SELECT:
        ir_tv_send(ir_tv_enter);
        break;
    case HOMEKIT_REMOTE_KEY_EXIT:
        ir_tv_send(ir_tv_exit);
        break;
    default:
        printf("Unsupported remote key code: %d", value.int_value);
    }
}

void on_mute(homekit_characteristic_t *ch, homekit_value_t value, void *arg) {
    ir_tv_send(ir_tv_mute);
}

void on_volume_selector(homekit_characteristic_t *ch, homekit_value_t value, void *arg) {
    ir_tv_send(value.int_value == HOMEKIT_VOLUME_SELECTOR_INCREMENT ?
               ir_tv_volume_up :
               ir_tv_volume_down);
}


homekit_characteristic_t input_source1_name = HOMEKIT_CHARACTERISTIC_(
    CONFIGURED_NAME, "HDMI 1",
    .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_input_configured_name)
);

homekit_service_t input_source1 =
    HOMEKIT_SERVICE_(INPUT_SOURCE, .characteristics=(homekit_characteristic_t*[]){
        HOMEKIT_CHARACTERISTIC(NAME, "hdmi1"),
        HOMEKIT_CHARACTERISTIC(IDENTIFIER, 1),
        &input_source1_name,
        HOMEKIT_CHARACTERISTIC(INPUT_SOURCE_TYPE, HOMEKIT_INPUT_SOURCE_TYPE_HDMI),
        HOMEKIT_CHARACTERISTIC(IS_CONFIGURED, true),
        HOMEKIT_CHARACTERISTIC(CURRENT_VISIBILITY_STATE, HOMEKIT_CURRENT_VISIBILITY_STATE_SHOWN),
        NULL
    });

homekit_characteristic_t input_source2_name = HOMEKIT_CHARACTERISTIC_(
    CONFIGURED_NAME, "HDMI 2",
    .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_input_configured_name)
);

homekit_service_t input_source2 =
    HOMEKIT_SERVICE_(INPUT_SOURCE, .characteristics=(homekit_characteristic_t*[]){
        HOMEKIT_CHARACTERISTIC(NAME, "hdmi2"),
        HOMEKIT_CHARACTERISTIC(IDENTIFIER, 2),
        &input_source2_name,
        HOMEKIT_CHARACTERISTIC(INPUT_SOURCE_TYPE, HOMEKIT_INPUT_SOURCE_TYPE_HDMI),
        HOMEKIT_CHARACTERISTIC(IS_CONFIGURED, true),
        HOMEKIT_CHARACTERISTIC(CURRENT_VISIBILITY_STATE, HOMEKIT_CURRENT_VISIBILITY_STATE_SHOWN),
        NULL
    });

homekit_characteristic_t input_source3_name = HOMEKIT_CHARACTERISTIC_(
    CONFIGURED_NAME, "HDMI 3",
    .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_input_configured_name)
);

homekit_service_t input_source3 =
    HOMEKIT_SERVICE_(INPUT_SOURCE, .characteristics=(homekit_characteristic_t*[]){
        HOMEKIT_CHARACTERISTIC(NAME, "hdmi3"),
        HOMEKIT_CHARACTERISTIC(IDENTIFIER, 3),
        &input_source3_name,
        HOMEKIT_CHARACTERISTIC(INPUT_SOURCE_TYPE, HOMEKIT_INPUT_SOURCE_TYPE_HDMI),
        HOMEKIT_CHARACTERISTIC(IS_CONFIGURED, true),
        HOMEKIT_CHARACTERISTIC(CURRENT_VISIBILITY_STATE, HOMEKIT_CURRENT_VISIBILITY_STATE_SHOWN),
        NULL
    });

homekit_characteristic_t tv_name = HOMEKIT_CHARACTERISTIC_(
    CONFIGURED_NAME, "My TV",
    .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_configured_name)
);

homekit_characteristic_t tv_active = HOMEKIT_CHARACTERISTIC_(
    ACTIVE, false,
    .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_active)
);


homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_television, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "TV"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "HaPK"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "1"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266-1"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, tv_identify),
            NULL
        }),
        HOMEKIT_SERVICE(TELEVISION, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Television"),
            &tv_active,
            HOMEKIT_CHARACTERISTIC(
                ACTIVE_IDENTIFIER, 1,
                .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_active_identifier)
            ),
            &tv_name,
            HOMEKIT_CHARACTERISTIC(
                SLEEP_DISCOVERY_MODE,
                HOMEKIT_SLEEP_DISCOVERY_MODE_ALWAYS_DISCOVERABLE
            ),
            HOMEKIT_CHARACTERISTIC(
                REMOTE_KEY, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_remote_key)
            ),
            HOMEKIT_CHARACTERISTIC(PICTURE_MODE, HOMEKIT_PICTURE_MODE_STANDARD),
            HOMEKIT_CHARACTERISTIC(POWER_MODE_SELECTION),
            NULL
        }, .linked=(homekit_service_t*[]) {
            &input_source1,
            &input_source2,
            &input_source3,
            NULL
        }),
        &input_source1,
        &input_source2,
        &input_source3,
        HOMEKIT_SERVICE(TELEVISION_SPEAKER, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(MUTE, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_mute)),
            HOMEKIT_CHARACTERISTIC(ACTIVE, true),
            HOMEKIT_CHARACTERISTIC(VOLUME_CONTROL_TYPE, HOMEKIT_VOLUME_CONTROL_TYPE_RELATIVE),
            HOMEKIT_CHARACTERISTIC(VOLUME_SELECTOR, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(on_volume_selector)),
            NULL
        }),
        NULL
    }),
    NULL
};


void on_input_configured_name(homekit_characteristic_t *ch, homekit_value_t value, void *arg) {
    const char *input_param_name = NULL;
    if (ch == &input_source1_name) {
        input_param_name = "input1_name";
    } else if (ch == &input_source2_name) {
        input_param_name = "input2_name";
    } else if (ch == &input_source3_name) {
        input_param_name = "input3_name";
    }

    if (!input_param_name)
        return;

    sysparam_set_string(input_param_name, value.string_value);
}


homekit_server_config_t config = {
    .accessories = accessories,
    .password = "058-66-789",
    .setupId = "F73K",
};


void on_wifi_event(wifi_config_event_t event) {
    if (event == WIFI_CONFIG_CONNECTED) {
        homekit_server_init(&config);
    }
}


void powered_on_intr_callback(uint8_t gpio) {
    tv_active.value = HOMEKIT_UINT8(gpio_read(POWERED_ON_GPIO));
    homekit_characteristic_notify(&tv_active, tv_active.value);
}


void user_init(void) {
    uart_set_baud(0, 115200);

    led_init();
    ir_tx_init();

    wifi_config_init2("insignia-tv", "bigsecret", on_wifi_event);

    gpio_enable(POWERED_ON_GPIO, GPIO_INPUT);
    tv_active.value = HOMEKIT_UINT8(gpio_read(POWERED_ON_GPIO));
    gpio_set_interrupt(POWERED_ON_GPIO, GPIO_INTTYPE_EDGE_ANY, powered_on_intr_callback);

    char *s = NULL;
    if (sysparam_get_string("tv_name", &s) == SYSPARAM_OK) {
        homekit_value_destruct(&tv_name.value);
        tv_name.value = HOMEKIT_STRING(s);
    }

    if (sysparam_get_string("input1_name", &s) == SYSPARAM_OK) {
        homekit_value_destruct(&input_source1_name.value);
        input_source1_name.value = HOMEKIT_STRING(s);
    }

    if (sysparam_get_string("input2_name", &s) == SYSPARAM_OK) {
        homekit_value_destruct(&input_source2_name.value);
        input_source2_name.value = HOMEKIT_STRING(s);
    }

    if (sysparam_get_string("input3_name", &s) == SYSPARAM_OK) {
        homekit_value_destruct(&input_source3_name.value);
        input_source3_name.value = HOMEKIT_STRING(s);
    }

    xTaskCreate(ir_dump_task, "IR receiver", 2048, NULL, tskIDLE_PRIORITY, NULL);
}
