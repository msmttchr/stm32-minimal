def setField(read_value,bit_start,bit_stop,value):
    #print(f'read_value:{format(read_value,"08b")}')
    #print(f'value:{format(value,"08b")}')
    fs=(bit_stop-bit_start)+1
    v=2**fs-1
    v=v<<bit_start
    v=(~v) & 0xffffffff  #creo la maschera 
    
    read_value=read_value & v
   #print(f'read_value_with_zeros:{format(read_value,"08b")}')  #ho azzerato i bit
    
    value=value<<bit_start
   # print(f'value_shifted:{format(value,"08b")}')
    
    return(read_value|value)
    


    
  