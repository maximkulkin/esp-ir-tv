#pragma once

#include <stdint.h>

typedef enum {
    ir_tv_1 = 0x00,
    ir_tv_2 = 0x01,
    ir_tv_3 = 0x02,
    ir_tv_4 = 0x03,
    ir_tv_5 = 0x04,
    ir_tv_6 = 0x05,
    ir_tv_7 = 0x06,
    ir_tv_8 = 0x07,
    ir_tv_9 = 0x08,
    ir_tv_0 = 0x09,
    ir_tv_power = 0x0f,
    ir_tv_volume_up = 0x0c,
    ir_tv_volume_down = 0x0d,
    ir_tv_mute = 0x0e,
    ir_tv_next_channel = 0x0a,
    ir_tv_prev_channel = 0x0b,
    ir_tv_menu = 0x14,
    ir_tv_exit = 0x1b,
    ir_tv_right = 0x15,
    ir_tv_left = 0x16,
    ir_tv_up = 0x42,
    ir_tv_down = 0x43,
    ir_tv_enter = 0x18,
    ir_tv_home = 0xb9,
    ir_tv_picture = 0x49,
} ir_tv_command_t;

typedef enum {
    ir_hdmi_switch_input_1 = 0x02,
    ir_hdmi_switch_input_2 = 0x04,
    ir_hdmi_switch_input_3 = 0x06,
    ir_hdmi_switch_next_input = 0x08,
} ir_hdmi_switch_command_t;

void ir_tv_send(ir_tv_command_t command);
ir_decoder_t *ir_tv_make_decoder();

void ir_hdmi_switch_send(ir_hdmi_switch_command_t command);
