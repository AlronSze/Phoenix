###########################################################

PROJECT = pheonix
TOOLPREFIX = arm-none-eabi-
OUTPUT = output

###########################################################

DEFS += -DSTM32F401xE -DSTM32F401xx -DUSE_STDPERIPH_DRIVER

INCS += -ILibraries/CMSIS -ILibraries/STM32F4xx_StdPeriph_Driver/inc
INCS += -IuCOS-II/App -IuCOS-II/Bsp -IuCOS-II/App -IuCOS-II/Port -IuCOS-II/Source
INCS += -IProject/User

C_SRC += $(wildcard Libraries/STM32F4xx_StdPeriph_Driver/src/*.c)
C_SRC += $(wildcard uCOS-II/Bsp/*.c)
C_SRC += $(wildcard uCOS-II/Port/*.c)
C_SRC += $(wildcard uCOS-II/Source/*.c)
C_SRC += $(wildcard Project/User/*.c)
OBJS += $(patsubst %.c, %.o, $(C_SRC))
DEPS += $(patsubst %.c, %.d, $(C_SRC))
LSTS += $(patsubst %.c, %.lst, $(C_SRC))

ASM_SRC += $(wildcard Libraries/CMSIS/gcc/*.s)
ASM_SRC += $(wildcard uCOS-II/Port/gcc/*.s)
OBJS += $(patsubst %.s, %.o, $(ASM_SRC))
DEPS += $(patsubst %.s, %.d, $(ASM_SRC))
LSTS += $(patsubst %.s, %.lst, $(ASM_SRC))

CFLAGS += -g -O0 -Wall
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mapcs-frame -mapcs-stack-check
CFLAGS += -mthumb -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -ffunction-sections -fdata-sections

# AFLAGS += -g
AFLAGS += -mcpu=cortex-m4
AFLAGS += -mthumb -mthumb-interwork
AFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

LFLAGS += -mcpu=cortex-m4
LFLAGS += -mthumb -mthumb-interwork
LFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
LFLAGS += -Wl,--gc-sections
LFLAGS += --specs=nano.specs --specs=rdimon.specs #--specs=nosys.specs

###########################################################

all: $(PROJECT).hex move
	@echo "Build done."

clean:
	@rm -rf $(OBJS) $(OUTPUT)

move:
	@rm -rf $(OUTPUT)
	@mkdir $(OUTPUT) $(OUTPUT)/obj $(OUTPUT)/dep $(OUTPUT)/lst $(OUTPUT)/bin
	@mv $(OBJS) $(OUTPUT)/obj/
	@mv $(DEPS) $(OUTPUT)/dep/
	@mv $(LSTS) $(OUTPUT)/lst/
	@mv $(PROJECT).hex $(PROJECT).elf $(PROJECT).map $(OUTPUT)/bin/
	@rm -f $(PROJECT).elf.strip

###########################################################

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
	@${TOOLPREFIX}as $(AFLAGS) -c $< -o $@ --MD $(@:.o=.d) -alhms=$(@:.o=.lst)

%.o: %.c
	@echo " [CC] $@"
	@${TOOLPREFIX}gcc $(CFLAGS) $(DEFS) $(INCS) -c $< -o $@ -MD -MF $(@:.o=.d) -MP -Wa,-alhms=$(@:.o=.lst)
