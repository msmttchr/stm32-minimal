#ifndef __ATTENUATION_H__
#define __ATTENUATION_H__
#include <stdint.h>
#define ATTENUATION_BASE_FREQUENCY 300000
#define ATTENUATION_STEP_FREQUENCY 10624625
#define ATTENUATION_POINTS_FREQUENCY 801
typedef uint16_t attenuation_t;
extern const attenuation_t *attenuation_table[][7];
#endif /* __ATTENUATION_H__ */
