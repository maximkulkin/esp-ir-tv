#include <stdio.h>
#include <ir/generic.h>
#include "ir.h"


static ir_generic_config_t protocol = {
    .header_mark = 9000,
    .header_space = -4500,

    .bit1_mark = 560,
    .bit1_space = -1690,

    .bit0_mark = 560,
    .bit0_space = -560,

    .footer_mark = 560,
    .footer_space = -9000,

    .tolerance = 20,
};


void ir_tv_send(ir_tv_command_t command) {
    printf("Sending tv command %02x\n", command);
    uint8_t buffer[4] = {0x86, 0x05, command, ~command};
    ir_generic_send(&protocol, buffer, 4);
}


ir_decoder_t *ir_tv_make_decoder() {
    return ir_generic_make_decoder(&protocol);
}


void ir_hdmi_switch_send(ir_hdmi_switch_command_t command) {
    printf("Sending switch command %02x\n", command);
    uint8_t buffer[4] = {0x80, 0x7f, command, ~command};
    ir_generic_send(&protocol, buffer, 4);
}
