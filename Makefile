# Copyright 2021, Stephen Fryatt (info@stevefryatt.org.uk)
#
# This file is part of StrongExtract:
#
#   http://www.stevefryatt.org.uk/software/
#
# Licensed under the EUPL, Version 1.1 only (the "Licence");
# You may not use this work except in compliance with the
# Licence.
#
# You may obtain a copy of the Licence at:
#
#   http://joinup.ec.europa.eu/software/page/eupl
#
# Unless required by applicable law or agreed to in
# writing, software distributed under the Licence is
# distributed on an "AS IS" basis, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#
# See the Licence for the specific language governing
# permissions and limitations under the Licence.

# This file really needs to be run by GNUMake.
# It is intended for native compilation on Linux (for use in a GCCSDK
# environment) or cross-compilation under the GCCSDK.

ARCHIVE := strongex

ifeq ($(TARGET),riscos)
  RUNIMAGE := strongex,ff8
else
  RUNIMAGE := strongex
endif

OBJS := args.o			\
	msg.o			\
	objectdb.o		\
	string.o		\
	strongex.o		\
	stronghelp.o

include $(SFTOOLS_MAKE)/Cross
