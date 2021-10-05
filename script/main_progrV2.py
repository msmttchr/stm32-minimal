from view_path import view_path
from set_path import set_path
from set_GPIO_pin import set_GPIO_pin



from pyocd.core.helpers import ConnectHelper
import logging
from time import sleep

logging.basicConfig(level=logging.ERROR)

class RFSwitch(object):
    """ Class to represent a PE42526 RF switch """
    connect_map = {
        'RF1': (0,0,0),
        'RF2': (1,0,0),
        'RF3': (0,1,0),
        'RF4': (1,1,0),
        'RF5': (0,0,1),
        'RF6': (1,0,1),
        'OFF': (0,1,1),
        }
    def __init__(self, name, v3,v2,v1):
        self.name = name
        self.v1=v1
        self.v2=v2
        self.v3=v3
        
    def connection(self,port, target=None):
        """ Connect RFC with RFn """
        v3,v2,v1= self.connect_map[port]
        
     
        if (target):
            set_GPIO_pin(target, self.v1[0], self.v1[1], v1)
            set_GPIO_pin(target, self.v2[0], self.v2[1], v2)
            set_GPIO_pin(target, self.v3[0], self.v3[1], v3)
        
    def switch_off(self, target=None):
        """ Switch off the connection """
        return self.connection("OFF", target=target)


U1=RFSwitch("SMA_A", ('A',12),('A',11),('A',6))
U2=RFSwitch("SMA_B", ('C',10),('C',11),('D',2))
U3=RFSwitch("SMA_C", ('C',5),('B',9),('C',6))
U4=RFSwitch("SMA_D", ('B',8),('C',8),('C',9))
U5=RFSwitch("SMA_E", ('A',0),('B',7),('A',15))
U6=RFSwitch("SMA_F", ('B',2),('A',9),('C',7))
U7=RFSwitch("SMA_G", ('B',6),('B',12),('A',7))

# values of keys are list indicates (portGPIO,pinGPIO,Value)
config_table={'A+B':[(U1, 'RF1'), (U2, 'RF6')],
              'A+C':[(U1, 'RF2'), (U3, 'RF6')],
              'A+D':[(U1, 'RF3'), (U4, 'RF5')],
              'A+E':[(U1, 'RF4'), (U5, 'RF2')],
              'A+F':[(U1, 'RF5'), (U6, 'RF2')],
              'A+G':[(U1, 'RF6'), (U7, 'RF1')],
              'B+C':[(U2, 'RF5'), (U3, 'RF1')],
              'B+D':[(U2, 'RF4'), (U4, 'RF2')],
              'B+E':[(U2, 'RF3'), (U5, 'RF5')],
              'B+F':[(U2, 'RF2'), (U6, 'RF5')],
              'B+G':[(U2, 'RF1'), (U7, 'RF6')],
              'C+D':[(U3, 'RF4'), (U4, 'RF6')],
              'D+E':[(U4, 'RF4'), (U5, 'RF3')],
              'E+F':[(U5, 'RF6'), (U6, 'RF4')],
              'F+G':[(U6, 'RF6'), (U7, 'RF4')],             
    }



view_path()
path=set_path()
print('Path Selected: ' + path)

recipe=config_table[path]
# print(recipe)

#for k, v in config_table.items():
    #print(k,v)


session = ConnectHelper.session_with_chosen_probe(target_override='cortex_m')
session.open()
 
board = session.board
target = board.target
target.reset_and_halt()


# Switch off all the switches
for rf_switch in [U1, U2, U3, U4, U5, U6, U7]:
    rf_switch.switch_off(target)
    
#for port,pin,value in recipe:
#    set_GPIO_pin(target,port,pin,value)
for switch, port in recipe:
    switch.connection(port, target)

# Enable internal regulator to generate -3 V
set_GPIO_pin(target,'C',12,1)

session.close()

