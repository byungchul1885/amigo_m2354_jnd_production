#ifndef MX25R4035F_INTERFACE_H
#define MX25R4035F_INTERFACE_H

#include <stdint.h>

void mx25r4035f_interface_init();

void mx25r4035f_request_write_protect();
void mx25r4035f_release_write_protect();

#endif //MX25R4035F_INTERFACE_H
