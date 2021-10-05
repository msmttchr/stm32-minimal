def set_path():
    num=0
    while num not in range(1,16):
        num=int(input('Select a bidirectional path: '))
    
    if num==1:
        return ('A+B')
    if num==2:
        return ('A+C')
    if num==3:
        return ('A+D')
    if num==4:
        return ('A+E')
    if num==5:
        return ('A+F')
    if num==6:
        return ('A+G')
    if num==7:
        return ('B+C')
    if num==8:
        return ('B+D')
    if num==9:
        return ('B+E')
    if num==10:
        return ('B+F')
    if num==11:
        return ('B+G')
    if num==12:
        return ('C+D')
    if num==13:
        return ('D+E')
    if num==14:
        return ('E+F')
    if num==15:
        return ('F+G')
