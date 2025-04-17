.text
movz X1, 0x1000    
movz X2, 5         

cbz X2, foo         
cbnz X2, river      
HLT 0               

foo:
movz X9, 80         
HLT 0 

river:
movz X3, 8          
HLT 0