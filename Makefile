SOURCES = main.c
TARGET = main
CC = gcc

all: $(TARGET) 

main: main.c
	$(CC) -o main $^ 

clean:
	rm  $(TARGET)
	
