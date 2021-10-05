from setField import setField
def dbgWrite(target, address, value):
    print ("{:08x} <- {:08x}".format(address, value))
    
def set_GPIO_pin(target,port,pin,value_pin):
    print(port,pin,value_pin)
    base_address={'A':0x40020000,'B':0x40020400,'C':0x40020800,'D':0x40020C00}
    moder=base_address[port]
    otyper=base_address[port]+0x04
    bsrr=base_address[port]+0x18
    
    RCC_base_address=0x40023800
    ahbenr = RCC_base_address + 0x1C
    

    #RCC_AHBENR Clock abilitato per ciascun port 
    ahbenr_portbit_start={'A':0x0,'B':0x1,'C':0x2,'D':0x3}
    RCC_read=target.read_memory(ahbenr)
    bit_start=ahbenr_portbit_start[port]
    value=setField(RCC_read,bit_start,bit_start,1)
    target.write_memory(ahbenr, value)

    #Moder
    GPIO_read=target.read_memory(moder)
    bit_start=pin*2
    value=setField(GPIO_read,bit_start,bit_start+1,1) 
    target.write_memory(moder, value)
    
    #otyper
    GPIO_read=target.read_memory(otyper)
    bit_start=pin
    value=setField(GPIO_read,bit_start,bit_start,0)
    target.write_memory(otyper, value)
    
    
    #bsrr
    value= (1<< pin) if value_pin else (1<<pin+16)
    target.write_memory(bsrr, value)