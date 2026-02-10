CC = gcc
CFLAGS = -Wall -O2 -g
LDFLAGS = -lrt
TARGET = hardware_benchmark
SRC = hardware_benchmark.c
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) hardware_info.txt hardware_benchmark.txt

.PHONY: all run clean