typedef enum swtich_pin_e
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
  {
   /* A */ {NO_PINS,   PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3)},
   /* B */ {PINS(1,3), NO_PINS,   PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3)},
   /* C */ {PINS(1,3), PINS(1,3), NO_PINS,   PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3)},
   /* D */ {PINS(1,3), PINS(1,3), PINS(1,3), NO_PINS,   PINS(1,3), PINS(1,3), PINS(1,3)},
   /* E */ {PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), NO_PINS,   PINS(1,3), PINS(1,3)},
   /* F */ {PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), NO_PINS,   PINS(1,3)},
   /* G */ {PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), PINS(1,3), NO_PINS},
};
