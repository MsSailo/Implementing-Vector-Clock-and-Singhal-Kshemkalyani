# Implementing-Vector-Clock-and-Singhal-Kshemkalyani

Compile as:
g++ -pthread VC_CS23MTECH11010.cpp -o vc
g++ -pthread SK_CS23MTECH11010.cpp -o sk

Run as:
./vc
./sk

It produces the following files:
VC-log.txt, for Vector Clock
SK-log.txt, for Singhal-Kshemkalyani optimization

The content of inp-params.txt should be like

n lambda alpha m
vertex-i e1 e2 e3 ....en

ex:
for number of processes = 4, and a complete graph:

4 5 1.5 40
1 2 3 4
2 1 3 4
3 1 2 4
4 1 2 3
