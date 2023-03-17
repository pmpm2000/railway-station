## ALP
### Typy wiadomości
s - 00  
u - 01  
p - 10  
w - 11  

### Typy contentu
numer - 00  
relacja - 01  
przewoźnik - 10  
typ - 11  

### ID peronu
I - 00  
II - 01  
III - 10  
IV - 11  

### Wiadomość s/u
s,[liczba contentów],content1,content2...  
u,[liczba contentów],content1,content2...  

1 content - 00  
2 contenty - 01  
3 contenty - 10  
4 contenty - 11  

00010001  
00|01|00|01  
s,2,numer,relacja  

### Wiadomość p
p,TYP,PERON,WartoscSENSORA  
1001001100  
10|01|00|1100  
p,relacja,peronI,12  

### Wiadomość w
w,PERON,WartoscSENSORA  
11|00|1100  
w,peronI,12  

## Publisher
Po kompilacji publishera należy uruchomić go na maszynie wirtualnej poleceniem
```
./EBSimUnoEthCurses -ip <ip interfejsu do pakietow UDP> kompilat.hex
```
Na maszynie musi być dostępny plik ```infile.txt```, który pozwoli na przeprowadzenie symulacji. Symulowane są następujące parametry: numer pociągu, relacja (identyfikator), przewoźnik (identyfikator), typ pociągu (0 jeśli EZT, 1 jeśli lokomotywa + wagony).\

Projekt zakłada użycie emulatora Arduino należącego do Politechniki Warszawskiej. Aby kod działał z fizyczną wersją Arduino, niezbędne będą zmiany w plikach publisher.ino oraz subscriber.ino.\
The project assume owning Arduino emulator from Warsaw University of Technology. You need to modify publisher.ino and subscriber.ino files in order to use it with real Arduino.