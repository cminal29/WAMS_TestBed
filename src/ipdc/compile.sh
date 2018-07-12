#!/bin/sh

# Compilation commands are: first run command "chmod +x compile.sh" then use -> 1) sh compile.sh, 2)  ./compile.sh

	gcc -c new_pmu_or_pdc.c -o new_pmu_or_pdc.o -lcrypto
	gcc -c parser.c -o parser.o -lcrypto
	gcc -c connections.c -o connections.o -lrt -lm -lpthread -lcrypto
	gcc -c align_sort.c -o align_sort.o -lcrypto
	gcc -c dallocate.c -o dallocate.o -lcrypto
	gcc -g ipdc.c new_pmu_or_pdc.o parser.o connections.o align_sort.o dallocate.o -o iPDC -lrt -lm -lpthread -lcrypto 
	
#	rm new_pmu_or_pdc.o parser.o connections.o align_sort.o dallocate.o iPDC
	
