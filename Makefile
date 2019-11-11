OBJECTS = main.o adc.o   common.o   debug.o  flash.o  GTM900C.o    hydrologytask.o  led.o   message.o  reportbinding.o  rtc.o      store.o  uart0.o  uart3.o        wifi_config.o blueTooth.o  Console.o  dtu.o    GSM.o    hydrologycommand.o  ioDev.o        packet.o   rom.o            Sampler.o  timer.o  uart1.o  uart_config.o


GCC_DIR =  /home/howard/app/ti/msp430-gcc/bin

SUPPORT_FILE_DIRECTORY = /home/howard/app/ti/msp430-gcc/include

 

DEVICE  = msp430f5438a
CC      = $(GCC_DIR)/msp430-elf-gcc

GDB     = $(GCC_DIR)/msp430-elf-gdb

 

CFLAGS = -I $(SUPPORT_FILE_DIRECTORY) -mmcu=$(DEVICE) -O2 -g

LFLAGS = -L $(SUPPORT_FILE_DIRECTORY)

 

all: ${OBJECTS}

	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(DEVICE).out

clean:

	rm $(OBJECTS) $(DEVICE).out