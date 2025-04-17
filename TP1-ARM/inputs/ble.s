.text

movz x0, 4
movz x1, 4
cmp x0, x1           
ble ble_success      
movz x25, 3

ble_success:
    movz x4, 5
    movz x8, 3
    cmp x8, x4
    ble foo
    movz x12, 6

foo:
    movz x4, 5
    movz x8, 3
    cmp x4, x8
    ble river
    hlt 0 

river:
    movz x25, 31

