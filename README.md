# Sew
Sew creates binary files from annotated text files. For example,
```
.format binary
; two bytes, each byte most-significant bit first
10101011 11001101
.format binary_little
; two bytes, each byte least-significant bit first
10000100 11000010
```
will compile to the four byte binary file conventionally written: `0xab 0xcd 0x21 0x43`.
Hex is also supported. To create the same file:
```
.format hex
ab cd
.format hex_little
12 34
```

## Build:
``cc -o sew sew.c``

## Run:
``./sew -o test test.sew``

