make: main.c matrix.c feyutils.c
	gcc main.c matrix.c feyutils -std=c11 -o bluntaxe
run: 
	make
	./bluntaxe