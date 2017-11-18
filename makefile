
SHELL=/bin/bash
CC=gcc
SRC_DIR=./src
INFRA_DIR=./src/infra
CFLAGS=-g -lm -I $(SRC_DIR) -I $(INFRA_DIR)
DISASSEMBLE=objdump -d
TARGET=fdup
ASM_TARGET=$(TARGET).asm

all: $(TARGET)

$(TARGET): $(SRC_DIR)/*.c $(INFRA_DIR)/*.c $(INFRA_DIR)/*.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_DIR)/*.c $(INFRA_DIR)/*.c
#	rm -rf $(TARGET).dSYM

clean:
	rm -f $(TARGET)
	rm -f $(ASM_TARGET)

asm:	$(TARGET)
	$(DISASSEMBLE) $(TARGET) > $(ASM_TARGET)

