for i in range(15,0,-1):
    for j in range(15,0,-1):
        if 14*14 <= round((i-16)*(i-16)+(j-16)*(j-16)) <= 16*16:
            print('0', end='  ')
        else:
            print(' ', end='  ')

    print('')