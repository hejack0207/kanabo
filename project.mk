# -*- mode: Makefile-gmake -*-

# 金棒 (kanabō)
# Copyright (c) 2012 Kevin Birch <kmb@pobox.com>.  All rights reserved.
#
# 金棒 is a tool to bludgeon YAML and JSON files from the shell: the strong
# made stronger.
#
# For more information, consult the README file in the project root.
#
# Distributed under an [MIT-style][license] license.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal with
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# - Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimers.
# - Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimers in the documentation and/or
#   other materials provided with the distribution.
# - Neither the names of the copyright holders, nor the names of the authors, nor
#   the names of other contributors may be used to endorse or promote products
#   derived from this Software without specific prior written permission.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
# OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
#
# [license]: http://www.opensource.org/licenses/ncsa

owner = com.webguys
package = kanabo
version = 0.4.0-SNAPSHOT
artifact = program
build = debug

DEPENDENCIES = yaml
TEST_DEPENDENCIES = check
CFLAGS = -std=c11 -fstrict-aliasing -Wall -Wextra -Werror -Wformat -Wformat-security -Wformat-y2k -Winit-self -Wmissing-include-dirs -Wswitch-default -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wbad-function-cast -Wconversion -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls -Wnested-externs -Wunreachable-code -Wno-switch-default -Wno-unknown-pragmas -Wno-gnu
debug_CFLAGS = -DUSE_LOGGING -fsanitize=address,integer,undefined -fno-sanitize=unsigned-integer-overflow
release_CFLAGS = -O3 -flto
LIBS = -lm
TEST_LIBS =
TEST_LDFLAGS = -fsanitize=address,integer,undefined -fno-sanitize=unsigned-integer-overflow -flto
release_LDFLAGS = -flto

ifeq ($(shell uname -s),Linux)
TEST_LIBS += -pthread -lrt
endif

VERSION_H = $(GENERATED_HEADERS_DIR)/version.h
CONFIG_H = $(GENERATED_HEADERS_DIR)/config.h

$(VERSION_H): $(GENERATED_HEADERS_DIR)
	@echo "Generating $(VERSION_H)"
	@CC=$(CC) build/generate_version_header.sh $(version) $(VERSION_H)

$(CONFIG_H): $(GENERATED_HEADERS_DIR)
	@echo "Generating $(CONFIG_H)"
	@build/generate_config_header.sh $(CONFIG_H) PREFX=$(prefix) LIBEXECDIR=$(package_libexecdir) DATADIR=$(package_datadir) LOGDIR=$(package_logdir) RUNDIR=$(package_rundir) MANDIR=$(man1dir) HTMLDIR=$(htmldir) INFODIR=$(infodir)

generate-version-header: $(VERSION_H)
generate-config-header: $(CONFIG_H)
GENERATE_SOURCES_HOOKS = generate-version-header generate-config-header
