#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"
#include <stdint.h>
#include "routing.h"

typedef enum switch_pin_e
  {
   RF1,
   RF2,
   RF3,
   RF4,
   RF5,
   RF6,
   RFC,
   NO_PIN=-1
  } switch_pin_t;
	  
typedef struct routing_table_item_s {
  switch_pin_t pin1;
  switch_pin_t pin2;
  int active;
} routing_table_item_t;

#define PINS(a,b) {RF##a, RF##b, 0}
#define NO_PINS {NO_PIN,NO_PIN, 0}

routing_table_item_t routing_table[][7]=
  {          /* A        B          C          D          E          F          G */
    /* A */    {NO_PINS, PINS(1,6), PINS(2,6), PINS(3,5), PINS(4,2), PINS(5,2), PINS(6,1)},
    /* B */    {NO_PINS, NO_PINS,   PINS(5,1), PINS(4,2), PINS(3,5), PINS(2,5), PINS(1,6)},
    /* C */    {NO_PINS, NO_PINS,   NO_PINS,   PINS(4,6), NO_PINS,   NO_PINS,   NO_PINS},
    /* D */    {NO_PINS, NO_PINS,   NO_PINS,   NO_PINS,   PINS(4,3), NO_PINS,   NO_PINS},
    /* E */    {NO_PINS, NO_PINS,   NO_PINS,   NO_PINS,   NO_PINS,   PINS(6,4), NO_PINS},
    /* F */    {NO_PINS, NO_PINS,   NO_PINS,   NO_PINS,   NO_PINS,   NO_PINS,   PINS(6,4)},
    /* G */    {NO_PINS, NO_PINS,   NO_PINS,   NO_PINS,   NO_PINS,   NO_PINS,   NO_PINS},
};

typedef struct GPIO_definition_s {
  GPIO_TypeDef *port;
  uint32_t      pin;
} GPIO_definition_t;

typedef struct PE42526_switch_control_s {
  GPIO_definition_t v1;
  GPIO_definition_t v2;
  GPIO_definition_t v3;
} PE42526_switch_control_t;

typedef struct PE42526_control_s {
  uint32_t v3;
  uint32_t v2;
  uint32_t v1;
} PE42526_control_t;

const PE42526_control_t PE42526_connection_map[]={
  /* v3, v2, v1*/
  {0,0,0}, /* RFC to RF1 */
  {1,0,0}, /* RFC to RF2 */
  {0,1,0}, /* RFC to RF3 */
  {1,1,0}, /* RFC to RF4 */
  {0,0,1}, /* RFC to RF5 */
  {1,0,1}, /* RFC to RF6 */
  {0,1,1}, /* RFC to none (OFF) */
};

struct PE42526_s;
typedef void (*init_t) (struct PE42526_s, PE42526_switch_control_t);
typedef void (*connection_t) (struct PE42526_s, switch_pin_t);
typedef void (*off_t) (struct PE42526_s);
/* PE42526 RF Switch */
typedef struct PE42526_s {
  char *name;           /* Switch name */
  PE42526_switch_control_t control;
  init_t init;       /* callback to init */
  connection_t connection; /* callback to make a connection */
  off_t switch_off; /* callback to switch off all connections */
} PE42526_t;

PE42526_t RFswitch[7];

static void switch_connection(PE42526_t rf_switch, switch_pin_t pin)
{
  PE42526_control_t set_value = PE42526_connection_map[pin];
  if (set_value.v1) {
    LL_GPIO_SetOutputPin(rf_switch.control.v1.port, rf_switch.control.v1.pin);
  } else {
    LL_GPIO_ResetOutputPin(rf_switch.control.v1.port, rf_switch.control.v1.pin);
  }
  if (set_value.v2) {
    LL_GPIO_SetOutputPin(rf_switch.control.v2.port, rf_switch.control.v2.pin);
  } else {
    LL_GPIO_ResetOutputPin(rf_switch.control.v2.port, rf_switch.control.v2.pin);
  }
  if (set_value.v3) {
    LL_GPIO_SetOutputPin(rf_switch.control.v3.port, rf_switch.control.v1.pin);
  } else {
    LL_GPIO_ResetOutputPin(rf_switch.control.v3.port, rf_switch.control.v1.pin);
  }
}

static void switch_off(PE42526_t rf_switch)
{
  switch_connection(rf_switch, RFC);
}

static void switch_init(PE42526_t rf_switch, PE42526_switch_control_t switch_control)
{
  rf_switch.control.v1 = switch_control.v1;
  rf_switch.control.v2 = switch_control.v2;
  rf_switch.control.v3 = switch_control.v3;
  LL_GPIO_SetPinMode(rf_switch.control.v1.port, rf_switch.control.v1.pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinOutputType(rf_switch.control.v1.port, rf_switch.control.v1.pin, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinMode(rf_switch.control.v2.port, rf_switch.control.v2.pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinOutputType(rf_switch.control.v3.port, rf_switch.control.v3.pin, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinMode(rf_switch.control.v3.port, rf_switch.control.v3.pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinOutputType(rf_switch.control.v3.port, rf_switch.control.v3.pin, LL_GPIO_OUTPUT_PUSHPULL);
  switch_connection(rf_switch, RFC);
}

PE42526_switch_control_t RFswitch_control[7] = {
  {{GPIOA,LL_GPIO_PIN_12},{GPIOA,LL_GPIO_PIN_11},{GPIOA,LL_GPIO_PIN_6}},
  {{GPIOC,LL_GPIO_PIN_10},{GPIOC,LL_GPIO_PIN_11},{GPIOD,LL_GPIO_PIN_2}},
  {{GPIOC,LL_GPIO_PIN_5}, {GPIOB,LL_GPIO_PIN_9},{GPIOC,LL_GPIO_PIN_6}},
  {{GPIOB,LL_GPIO_PIN_8},{GPIOC,LL_GPIO_PIN_8},{GPIOC,LL_GPIO_PIN_9}},
  {{GPIOA,LL_GPIO_PIN_0},{GPIOB,LL_GPIO_PIN_7},{GPIOA,LL_GPIO_PIN_15}},
  {{GPIOB,LL_GPIO_PIN_2},{GPIOA,LL_GPIO_PIN_9},{GPIOC,LL_GPIO_PIN_7}},
  {{GPIOB,LL_GPIO_PIN_6},{GPIOB,LL_GPIO_PIN_12},{GPIOA,LL_GPIO_PIN_7}}
};

void routing_init(void)
{
  int i;
  
  /* TODO: Make routing table symmetric */
  /* routing_table[i][j] = routing_table[j][i] below diagonal */

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  for (i = 0; i < sizeof(RFswitch)/sizeof(PE42526_t); i++) {
    RFswitch[i].init = switch_init;
    RFswitch[i].connection = switch_connection;
    RFswitch[i].switch_off = switch_off;
  }
  for (i = 0; i < sizeof(RFswitch)/sizeof(PE42526_t); i++) {
    RFswitch[i].init(RFswitch[i], RFswitch_control[i]);
  }
  routing_disconnect_all();
}

int routing_connection(char ep1, char ep2)
{
  routing_table_item_t entry;
  int sw1 = ep1 - 'A';
  int sw2 = ep2 - 'A';
  entry = routing_table[sw1][sw2];

  if ((entry.pin1 == NO_PIN) || (entry.pin2 == NO_PIN))
    /* Connection not possible */
    return -1;

  RFswitch[sw1].connection(RFswitch[sw1], entry.pin1);
  RFswitch[sw2].connection(RFswitch[sw2], entry.pin2);
  routing_table[sw1][sw2].active = 1;

  return 0;
}

void routing_disconnect_all(void)
{
  int sw, sw1, sw2;
  for (sw = 0; sw < (sizeof(RFswitch)/sizeof(PE42526_t)); sw++)
    RFswitch[sw].switch_off(RFswitch[sw]);

  for (sw1 = 0; sw1 < (sizeof(RFswitch)/sizeof(PE42526_t)); sw1++)
    for (sw2 = 0; sw2 < (sizeof(RFswitch)/sizeof(PE42526_t)); sw2++)
      routing_table[sw1][sw2].active = 0;
}
