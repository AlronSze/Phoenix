###########################################################

PROJECT = pheonix
TOOLPREFIX = arm-none-eabi-
OUTPUT = output

###########################################################

DEFS += -DUSE_STDPERIPH_DRIVER -DSTM32F401xE -DSTM32F401xx

INCS += -ILibraries/CMSIS -ILibraries/STM32F4xx_StdPeriph_Driver/inc
INCS += -IuCOS-II/App -IuCOS-II/Bsp -IuCOS-II/App -IuCOS-II/Port -IuCOS-II/Source
INCS += -IProject/User

C_SRC += $(wildcard Libraries/STM32F4xx_StdPeriph_Driver/src/*.c)
C_SRC += $(wildcard uCOS-II/Bsp/*.c)
C_SRC += $(wildcard uCOS-II/Port/*.c)
C_SRC += $(wildcard uCOS-II/Source/*.c)
C_SRC += $(wildcard Project/User/*.c)
OBJS += $(patsubst %.c, %.o, $(C_SRC))

ASM_SRC += $(wildcard Libraries/CMSIS/gcc/*.s)
ASM_SRC += $(wildcard uCOS-II/Port/gcc/*.s)
OBJS += $(patsubst %.s, %.o, $(ASM_SRC))

CFLAGS += -mcpu=cortex-m4 -mthumb -Wall 
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -Os
CFLAGS += -ffunction-sections -fdata-sections

LFLAGS += -mcpu=cortex-m4 -mthumb
LFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
LFLAGS += -Wl,--gc-sections --specs=rdimon.specs

###########################################################

all: $(PROJECT).hex move
	@echo "Build done."

clean:
	@rm -rf $(OBJS) $(OUTPUT)

move:
	@rm -rf $(OUTPUT)
	@mkdir $(OUTPUT) $(OUTPUT)/obj $(OUTPUT)/bin
	@mv $(OBJS) $(OUTPUT)/obj/
	@mv $(PROJECT).hex $(PROJECT).elf $(PROJECT).map $(OUTPUT)/bin/
	@rm -f $(PROJECT).elf.strip

$(PROJECT).hex: $(PROJECT).elf.strip
	@echo " [OBJCOPY] $@"
	@${TOOLPREFIX}objcopy -O ihex $< $@
$(PROJECT).elf.strip: $(PROJECT).elf
	@echo " [STRIP] $@"
	@${TOOLPREFIX}strip -s -o $@ $<
$(PROJECT).elf: $(OBJS)
	@echo " [LD] $@"
	@${TOOLPREFIX}gcc $(LFLAGS) $^ -TLibraries/STM32F4xx_StdPeriph_Driver/STM32F401RC_FLASH.ld -o $@ -Wl,-Map=$(@:.elf=.map)
	@echo " [SIZE] $@"
	@${TOOLPREFIX}size $@

%.o: %.s
	@echo " [AS] $@"
	@${TOOLPREFIX}gcc $(CFLAGS) -c $< -o $@
%.o: %.c
	@echo " [CC] $@"
	@${TOOLPREFIX}gcc $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@
