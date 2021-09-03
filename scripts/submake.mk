# SPDX-License-Identifier: GPL-2.0
# ==========================================================================
# Recursion build system
# ==========================================================================
submk := $(obj)

_build:

#
# Include Buildsystem function
include $(BUILD_HOME)/include/define.mk

#
# Read auto.conf if it exists, otherwise ignore
-include $(MAKE_HOME)/include/config/auto.conf

#
# Include sub makefile
sub-dir := $(if $(filter /%,$(submk)),$(submk),$(MAKE_HOME)/$(submk))
sub-file := $(if $(wildcard $(sub-dir)/Kbuild),$(sub-dir)/Kbuild,$(sub-dir)/Makefile)
include $(sub-file)

########################################
# Start project                        #
########################################

project_y   := $(project-y)
project_y   := $(strip $(sort $(project_y)))
project_y   := $(filter %/, $(project_y))
project_y   := $(patsubst %/,%,$(project_y))
project_y   := $(addprefix $(obj)/,$(project_y))

project_n   := $(project-)
project_n   := $(strip $(sort $(project_n)))
project_n   := $(filter %/, $(project_n))
project_n   := $(patsubst %/,%,$(project_n))
project_n   := $(addprefix $(obj)/,$(project_n))

########################################
# include dirs                         #
########################################

project_include := $(addprefix $(obj)/,$(project-include-y)) \
                $(project-include-direct-y) $(project_include)
export project_include

########################################
# Start remake                         #
########################################

PHONY += _remake
_remake: 
	$(Q)$(MAKE) $(submake)=$(MAKE_HOME) _clean
	$(Q)$(MAKE) $(submake)=$(MAKE_HOME) _build

########################################
# Start build                          #
########################################

PHONY += _build
_build: $(project_y)
	$(Q)$(MAKE) $(basic)
	$(Q)$(MAKE) $(build)=$(sub-dir)
	$(call hook_build)

########################################
# Display system information           #
########################################

PHONY += _info
_info:
	$(Q)$(ECHO)  'System information:'
	$(Q)$(ECHO)  '    Build-Version  = $(LIGHYBUILD_VERSION)'
	$(Q)$(ECHO)  '    CC             = $(CC)'
	$(Q)$(ECHO)  '    CC-Version     = $(call cc-version)'
	$(Q)$(ECHO)  '    Real Home      = $(home)'
	$(Q)$(ECHO)  '    Make Home      = $(MAKE_HOME)'
	$(Q)$(ECHO)  '    Build Home     = $(BUILD_HOME)'
	$(Q)$(ECHO)  '    Kconfig file   = $(Kconfig)'
	$(Q)$(ECHO)  'Build information:'
	$(Q)$(ECHO)  '    Projects       = $(project_y)'
	$(Q)$(ECHO)  '    Project include= $(project_include)'

########################################
# Start clean                          #
########################################

#
# clean
PHONY += $(clean-dirs) clean 

RCS_FIND_IGNORE := \( -name SCCS -o -name BitKeeper -o \
                      -name .svn -o -name CVS -o -name .pc -o \
                      -name .hg -o -name .git \) -prune -o

_clean: $(project_y) $(project_n)
	$(Q)$(MAKE) $(clean)=$(sub-dir)
	$(call hook_clean)

#
# mrproper
MRPROPER_DIRS	+= include/config include/generated
MRPROPER_FILES	+= .config .config.old tags TAGS cscope* GPATH GTAGS GRTAGS GSYMS

MRPROPER_DIRS	:= $(addprefix $(obj)/,$(MRPROPER_DIRS))
MRPROPER_FILES	:= $(addprefix $(obj)/,$(MRPROPER_FILES))

rm-dirs  := $(wildcard $(MRPROPER_DIRS))
rm-files := $(wildcard $(MRPROPER_FILES))
mrproper-dirs := $(addprefix _mrproper_,$(rm-dirs))
mrproper-files := $(addprefix _mrproper_,$(rm-files))

PHONY += $(mrproper-dirs) 
$(mrproper-dirs):
	$(Q)$(ECHO) "  $(ECHO_RMDIR) $(patsubst _mrproper_%,%,$@)"
	$(Q)$(RMDIR) $(patsubst _mrproper_%,%,$@)

PHONY += $(mrproper-files)
$(mrproper-files):
	$(Q)$(ECHO) "  $(ECHO_RM) $(patsubst _mrproper_%,%,$@)"
	$(Q)$(RM) $(patsubst _mrproper_%,%,$@)

PHONY += _mrproper
_mrproper: $(mrproper-files) $(mrproper-dirs) _clean
	$(call cmd,rmdirs)
	$(call cmd,rmfiles)

_distclean: _mrproper
	$(Q)$(MAKE) $(clean)=$(BUILD_HOME)

########################################
# Start checkstack                     #
########################################

CHECKSTACK_ARCH := $(ARCH)
CHECKSTACK_EXE  := 

_checkstack:
	$(OBJDUMP) -d $(CHECKSTACK_EXE) | $(PERL) \
	$(BUILD_HOME)/checkstack.pl $(CHECKSTACK_ARCH)

########################################
# Descending operation                 #
########################################

PHONY += $(project_y)
$(project_y): FORCE
	$(Q)$(MAKE) $(submake)=$@ $(MAKECMDGOALS)

PHONY += $(project_n)
$(project_n): FORCE
	$(Q)$(MAKE) $(submake)=$@ $(MAKECMDGOALS)

########################################
# Start FORCE                          #
########################################

PHONY += FORCE 
FORCE:

# Declare the contents of the PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
