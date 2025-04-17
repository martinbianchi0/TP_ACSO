.text

movz x0, 5
movz x1, 4
cmp x0, x1           
bgt bgt_success      
movz x25, 3

bgt_success:
    movz x4, 5
    movz x8, 3
    cmp x8, x4
    bgt foo
    movz x12, 6
    movz x13, 6
    cmp x12, x13
    bgt river
    hlt 0 

foo:
    movz x4, 5
    movz x8, 3
    cmp x4, x8
    ble river
    hlt 0 

river:
    movz x25, 31
