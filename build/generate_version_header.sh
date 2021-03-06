#!/usr/bin/env bash

if [ 2 -ne $# ]; then
    echo "usage: $(basename $0) <version> <filename>"
    exit 1
fi

VERSION=$1
FILENAME=$2

mkdir -p $(dirname $FILENAME)

case $VERSION in
    *-SNAPSHOT) is_snapshot=true;;
    *) is_snapshot=false;;
esac

base_version=${VERSION%-SNAPSHOT}

IFS=$'.'
read -r major_version minor_version point_version <<< "${base_version}"
unset IFS

cat >$FILENAME <<EOF
/*
 * 金棒 (kanabō)
 * Copyright (c) 2012 Kevin Birch <kmb@pobox.com>.  All rights reserved.
 * 
 * 金棒 is a tool to bludgeon YAML and JSON files from the shell: the strong
 * made stronger.
 *
 * For more information, consult the README file in the project root.
 *
 * Distributed under an [MIT-style][license] license.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal with
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimers.
 * - Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimers in the documentation and/or
 *   other materials provided with the distribution.
 * - Neither the names of the copyright holders, nor the names of the authors, nor
 *   the names of other contributors may be used to endorse or promote products
 *   derived from this Software without specific prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS
 * OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 *
 * [license]: http://www.opensource.org/licenses/ncsa
 */

#pragma once

#include <stdbool.h>

#define COPYRIGHT "Copyright (c) $(date "+%Y") Kevin Birch <kmb@pobox.com>.  All rights reserved."
static const char * const COPYRIGHT_c = "Copyright (c) $(date "+%Y") Kevin Birch <kmb@pobox.com>.  All rights reserved.";
#define VERSION "${VERSION}"
static const char * const VERSION_c = "${VERSION}";
#define MAJOR_VERSION ${major_version}
static const unsigned int MAJOR_VERSION_c = ${major_version};
#define MINOR_VERSION ${minor_version}
static const unsigned int MINOR_VERSION_c = ${minor_version};
#define POINT_VERSION ${point_version}
static const unsigned int POINT_VERSION_c = ${point_version};
#define IS_SNAPSHOT ${is_snapshot}
static const bool IS_SNAPSHOT_c = ${is_snapshot};

#define BUILD_COMPILER "$(${CC} --version | head -1)"
static const char * const BUILD_COMPILER_c = "$(${CC} --version | head -1)";
#define BUILD_DATE "$(date)"
static const char * const BUILD_DATE_c = "$(date)";
#define BUILD_TIMESTAMP $(date "+%s")
static const unsigned long BUILD_TIMESTAMP_c = $(date "+%s");
#define BUILD_HOSTNAME "$(hostname -s)"
static const char * const BUILD_HOSTNAME_c = "$(hostname -s)";
#define BUILD_HOST_ARCHITECHTURE "$(uname -m)"
static const char * const BUILD_HOST_ARCHITECHTURE_c = "$(uname -m)";
#define BUILD_HOST_OS "$(uname -s)"
static const char * const BUILD_HOST_OS_c = "$(uname -s)";
#define BUILD_HOST_OS_VERSION "$(uname -r)"
static const char * const BUILD_HOST_OS_VERSION_c = "$(uname -r)";

EOF
