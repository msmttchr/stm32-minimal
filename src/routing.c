#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"
#include <stdint.h>
#include <string.h>
#include "routing.h"
#include "system_def.h"

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
} routing_table_item_t;

#define PINS(a,b) {RF##a, RF##b}
#define NO_PINS {NO_PIN,NO_PIN}

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
typedef void (*init_t) (struct PE42526_s *, PE42526_switch_control_t);
typedef void (*connection_t) (struct PE42526_s *, switch_pin_t);
typedef int (*connection_active_t) (struct PE42526_s *, switch_pin_t);
typedef void (*switch_off_t) (struct PE42526_s *);
/* PE42526 RF Switch */
typedef struct PE42526_s {
  char *name;           /* Switch name */
  PE42526_switch_control_t control;
  init_t init;       /* callback to init */
  connection_t connection; /* callback to make a connection */
  connection_active_t connection_active; /* callback to make a connection */
  switch_off_t switch_off; /* callback to switch off all connections */
} PE42526_t;

PE42526_t RFswitch[7];

static void set_gpio_pin(GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t value)
{
  if (value)
    LL_GPIO_SetOutputPin(GPIOx, pin);
  else
    LL_GPIO_ResetOutputPin(GPIOx, pin);
}

static int gpio_match(GPIO_definition_t gpio, uint32_t value)
{
  return (LL_GPIO_IsOutputPinSet(gpio.port, gpio.pin) == value);
}

static void switch_connection(PE42526_t *rf_switch, switch_pin_t pin)
{
  PE42526_control_t set_value = PE42526_connection_map[pin];
  set_gpio_pin(rf_switch->control.v1.port, rf_switch->control.v1.pin, set_value.v1);
  set_gpio_pin(rf_switch->control.v2.port, rf_switch->control.v2.pin, set_value.v2);
  set_gpio_pin(rf_switch->control.v3.port, rf_switch->control.v3.pin, set_value.v3);
}

static int switch_connection_active(PE42526_t *rf_switch, switch_pin_t pin)
{
  PE42526_control_t set_value = PE42526_connection_map[pin];
  return (gpio_match(rf_switch->control.v1, set_value.v1) &&	\
	  gpio_match(rf_switch->control.v2, set_value.v2) &&	\
	  gpio_match(rf_switch->control.v3, set_value.v3));
}

static void switch_off(PE42526_t *rf_switch)
{
  switch_connection(rf_switch, RFC);
}

static void switch_init(PE42526_t *rf_switch, PE42526_switch_control_t switch_control)
{
  rf_switch->control.v1 = switch_control.v1;
  rf_switch->control.v2 = switch_control.v2;
  rf_switch->control.v3 = switch_control.v3;
  LL_GPIO_SetPinMode(rf_switch->control.v1.port, rf_switch->control.v1.pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinOutputType(rf_switch->control.v1.port, rf_switch->control.v1.pin, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinMode(rf_switch->control.v2.port, rf_switch->control.v2.pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinOutputType(rf_switch->control.v3.port, rf_switch->control.v3.pin, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinMode(rf_switch->control.v3.port, rf_switch->control.v3.pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinOutputType(rf_switch->control.v3.port, rf_switch->control.v3.pin, LL_GPIO_OUTPUT_PUSHPULL);
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
  
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  for (i = 0; i < sizeof(RFswitch)/sizeof(PE42526_t); i++) {
    RFswitch[i].init = switch_init;
    RFswitch[i].connection = switch_connection;
    RFswitch[i].connection_active = switch_connection_active;
    RFswitch[i].switch_off = switch_off;
  }
  for (i = 0; i < sizeof(RFswitch)/sizeof(PE42526_t); i++) {
    RFswitch[i].init(&RFswitch[i], RFswitch_control[i]);
  }
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

  RFswitch[sw1].connection(&RFswitch[sw1], entry.pin1);
  RFswitch[sw2].connection(&RFswitch[sw2], entry.pin2);

  return 0;
}

int routing_connection_query(char *endpoints, char *reply_buffer)
{
  int ep1 = -1;
  int ep2 = -1;
  int sw1, sw2;
  int retval = 0;
  char reply[32];
  char *ptr=reply;
  routing_table_item_t pins;
  
  if (endpoints != NULL) {
    ep1 = endpoints[0] - 'A';
    ep2 = endpoints[1] - 'A';
  }
  memset (reply, 0, sizeof(reply));
  my_fprintf(stderr, "\nHere\r\n");
  if (ep1 == -1) {
    for (sw1 = 0; sw1 < (sizeof(RFswitch)/sizeof(PE42526_t)); sw1++)
      for (sw2 = 0; sw2 < (sizeof(RFswitch)/sizeof(PE42526_t)); sw2++) {
	pins = routing_table[sw1][sw2];
	if (pins.pin1 != NO_PIN) {
	  int cond;
	  cond = RFswitch[sw1].connection_active(&RFswitch[sw1], pins.pin1) && \
	    RFswitch[sw2].connection_active(&RFswitch[sw2], pins.pin2);
	  if (cond) {
	    if (ptr > reply)
	      *ptr++ = ',';
	    *ptr++ = sw1+65;
	    *ptr++ = sw2+65;
	  }
	}
      }
  } else {
    pins = routing_table[ep1][ep2];
    if (pins.pin1 != NO_PIN) {
      int cond;
      cond = RFswitch[ep1].connection_active(&RFswitch[ep1], pins.pin1) && \
	RFswitch[ep2].connection_active(&RFswitch[ep2], pins.pin2);
      if (cond) {
	*ptr++ = ep1+65;
	*ptr++ = ep2+65;
      }
    } else {
      /* Invalid connection */
      retval = -1;
    }
  }
  strcpy(reply_buffer, reply);
  return retval;
}
void routing_disconnect_all(void)
{
  int sw;
  for (sw = 0; sw < (sizeof(RFswitch)/sizeof(PE42526_t)); sw++)
    RFswitch[sw].switch_off(&RFswitch[sw]);
}
