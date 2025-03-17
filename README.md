# Sew
Sew creates binary files from annotated text files. For example,
```
.format binary
; ab cd
10101011110011011101 
```
will compile to two byte binary file `0xabcd` 

## Build:
``cc -o sew sew.c``

## Run:
``./sew -o test test.sew``

