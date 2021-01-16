#ifndef __LIGHT_CONTROL_H__
#define __LIGHT_CONTROL_H__

// 输出反转
#define LIGHT_OUT_FLIP 1

void light_init(void);
bool light_status(void);
void light_on(void);
void light_off(void);
void light_off_delay(int seconds);

#endif