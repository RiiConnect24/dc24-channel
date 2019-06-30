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
SOURCES		:=	source
#---------------------------------------------------------------------------------
CXXFLAGS	= -g -O2 -Wall -DSIZEOF_SIZE_T=4 -DSIZEOF_UNSIGNED_LONG_LONG=8 $(MACHDEP) $(INCLUDE)
LDFLAGS		= -g $(MACHDEP) -Wl,-Map,$(notdir $@).map
LIBS		:= -lwiiuse -lbte -logc -lm
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export OUTPUT	:= $(CURDIR)/$(TARGET)
export VPATH	:= $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR	:= $(CURDIR)/$(BUILD)
#---------------------------------------------------------------------------------
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
#---------------------------------------------------------------------------------
export LD 				:= $(CXX)
export OFILES_BIN		:= $(addsuffix .o,$(BINFILES))
export OFILES_SOURCES 	:= $(CPPFILES:.cpp=.o)
export OFILES 			:= $(OFILES_BIN) $(OFILES_SOURCES)
export HFILES 			:= $(addsuffix .h,$(subst .,_,$(BINFILES)))
export INCLUDE			:= $(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD) -I$(LIBOGC_INC)
export LIBPATHS			:= $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBOGC_LIB)
export OUTPUT			:= $(CURDIR)/$(TARGET)
#---------------------------------------------------------------------------------
.PHONY: $(BUILD) clean release
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
#---------------------------------------------------------------------------------
clean:
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol apps $(TARGET).zip
#---------------------------------------------------------------------------------
release: $(BUILD)
	@mkdir -p $(CURDIR)/apps/mail-patch
	@mv $(OUTPUT).elf $(CURDIR)/apps/mail-patch/boot.elf
	@mv $(OUTPUT).dol $(CURDIR)/apps/mail-patch/boot.dol
	@cp -R $(CURDIR)/release/icon.png $(CURDIR)/apps/mail-patch/
	@cp -R $(CURDIR)/release/meta.xml $(CURDIR)/apps/mail-patch/
	@zip -r $(TARGET).zip apps
	@rm -rf apps
#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)
#---------------------------------------------------------------------------------
$(OUTPUT).dol		: $(OUTPUT).elf
$(OUTPUT).elf		: $(OFILES)
$(OFILES_SOURCES) 	: $(HFILES)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------