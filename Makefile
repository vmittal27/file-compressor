HEADERS = decompress.h compress.h string_table.h stack.h binaryIO.h
OBJECTS = program.o decompress.o compress.o string_table.o stack.o binaryIO.o

default: program

program.o: main.c $(HEADERS)
	gcc -c -g main.c -o program.o

decompress.o: decompress.c decompress.h string_table.h binaryIO.h stack.h
	gcc -c -g decompress.c -o decompress.o

compress.o: compress.c compress.h string_table.h binaryIO.h
	gcc -c -g compress.c -o compress.o

string_table.o: string_table.c string_table.h
	gcc -c -g string_table.c -o string_table.o

stack.o: stack.c stack.h
	gcc -c -g stack.c -o stack.o

binaryIO.o: binaryIO.c binaryIO.h
	gcc -c -g binaryIO.c -o binaryIO.o

program: $(OBJECTS)
	touch compress
	touch decompress
	touch string_table
	touch stack
	touch binaryIO
	rm -f decompress
	rm -f compress
	rm -f string_table
	rm -f stack
	rm -f binaryIO
	
	gcc $(OBJECTS) -o program

	ln -s program decompress
	ln -s program compress

clean:
	-rm -f $(OBJECTS)
	-rm -f program
	-rm -f decompress
	-rm -f compress
	-rm -f string_table
	-rm -f stack
	-rm -f binaryIO
	-rm -f DBG.*
