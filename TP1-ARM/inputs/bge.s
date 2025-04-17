.text

movz x0, 4
movz x1, 4
cmp x0, x1           
bge bge_success      
movz x25, 3

bge_success:
    movz x4, 5
    movz x8, 3
    cmp x4, x8
    bge foo
    movz x12, 6

foo:
    movz x4, 5
    movz x8, 3
    cmp x8, x4
    bge river
    hlt 0 

river:
    movz x25, 31

