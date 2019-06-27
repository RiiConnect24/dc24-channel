#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source source/network
DATA		:=	data  
INCLUDES	:=
#---------------------------------------------------------------------------------
CFLAGS		= -g -O2 -Wall -DSIZEOF_SIZE_T=4 -DSIZEOF_UNSIGNED_LONG_LONG=8 $(MACHDEP) $(INCLUDE)
CXXFLAGS	= $(CFLAGS)
LDFLAGS		= -g $(MACHDEP) -Wl,-Map,$(notdir $@).map
#---------------------------------------------------------------------------------
LIBS	:= -lwiiuse -lbte -logc -lm
LIBDIRS	:=
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export OUTPUT	:= $(CURDIR)/$(TARGET)
export VPATH	:= $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR	:=$(CURDIR)/$(BUILD)
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif
#---------------------------------------------------------------------------------
export OFILES_BIN	:= $(addsuffix .o,$(BINFILES))
export OFILES_SOURCES 	:= $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(sFILES:.s=.o) $(SFILES:.S=.o)
export OFILES 		:= $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES 		:= $(addsuffix .h,$(subst .,_,$(BINFILES)))
export INCLUDE		:= $(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC)
export LIBPATHS		:= $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBOGC_LIB)
export OUTPUT		:= $(CURDIR)/$(TARGET)
#---------------------------------------------------------------------------------
.PHONY: $(BUILD) clean
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
#---------------------------------------------------------------------------------
clean:
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol
#---------------------------------------------------------------------------------
run:
	@wiiload $(TARGET).dol
#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)
#---------------------------------------------------------------------------------
$(OUTPUT).dol		: $(OUTPUT).elf
$(OUTPUT).elf		: $(OFILES)
$(OFILES_SOURCES) 	: $(HFILES)
#---------------------------------------------------------------------------------
%.jpg.o	%_jpg.h :	%.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
