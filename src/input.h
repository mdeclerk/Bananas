#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

void input_init(void);
void input_update(void);
bool input_pressed(uint8_t mask);
bool input_held(uint8_t mask);
bool input_repeated(uint8_t mask);

#endif
