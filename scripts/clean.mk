# SPDX-License-Identifier: GPL-2.0
# ==========================================================================
# Cleaning up
# ==========================================================================

_clean:
src := $(obj)

#
# Include Buildsystem function
include $(BUILD_HOME)/include/define.mk

#
# Read auto.conf if it exists, otherwise ignore
-include $(MAKE_HOME)/include/config/auto.conf

# The filename Kbuild has precedence over Makefile
clean-dir := $(if $(filter /%,$(src)),$(src),$(MAKE_HOME)/$(src))
build-file := $(if $(wildcard $(clean-dir)/Kbuild),$(clean-dir)/Kbuild,$(clean-dir)/Makefile)
include $(build-file)

#
# Include Buildsystem function
include $(BUILD_HOME)/include/rule.mk

########################################
# Filter files                         #
########################################

# We need to save clean-y
# User delete rule
clean-file       := $(filter-out %/,$(clean-y))
clean-file       := $(addprefix $(obj)/,$(clean-file))

# Delete process file
clean-files      := $(wildcard $(clean-files))
# Match delete CMD
clean-cmd        := $(wildcard $(foreach f,$(clean-file) $(clean-files),$(dir $(f)).$(notdir $(f)).cmd))

########################################
# Finishing parameters                 #
########################################

# These files will be cleaned up
clean_files      := $(clean-file) $(clean-files) $(clean-cmd)
clean_files      := $(wildcard $(clean_files))

# These folders will be cleaned up
clean_dirs       := $(strip $(sort $(clean-y)))
clean_dirs       := $(filter %/, $(clean_dirs))
clean_dirs       := $(patsubst %/,%,$(clean_dirs))
clean_dirs       := $(addprefix $(obj)/,$(clean_dirs))
clean_dirs       := $(wildcard $(clean_dirs))

# The system recurses these folders down
clean-subdir-y   := $(clean-subdir-y)
clean-subdir-y   := $(strip $(sort $(clean-subdir-y)))
clean-subdir-y   := $(filter %/, $(clean-subdir-y))
clean-subdir-y   := $(patsubst %/,%,$(clean-subdir-y))
clean-subdir-y   := $(addprefix $(obj)/,$(clean-subdir-y))
clean_subdir     := $(clean-subdir-y) $(clean-subdir)

########################################
# clean rule                           #
########################################

quiet_cmd_clean_file = $(ECHO_CLEAN) $@
      cmd_clean_file = $(RM) -f $@
$(clean_files): FORCE
	$(call cmd,clean_file)

quiet_cmd_clean_dir = $(ECHO_CLEAN_DIR) $@
      cmd_clean_dir = $(RM) -rf $(clean_dirs)
$(clean_dirs): FORCE
	$(call cmd,clean_dir)

########################################
# Start clean                          #
########################################

PHONY += _clean
_clean: $(clean_subdir) $(clean_files) $(clean_dirs)

########################################
# Descending clean                     #
########################################

PHONY += $(clean_subdir)
$(clean_subdir):
	$(Q)$(MAKE) $(clean)=$@

PHONY += FORCE 
FORCE:

# Declare the contents of the PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
