/*
 * 金棒 (kanabō)
 * Copyright (c) 2012 Kevin Birch <kmb@pobox.com>
 * 
 * 金棒 is a tool to bludgeon YAML and JSON files from the shell: the strong
 * made stronger.
 *
 * For more information, consult the README in the project root.
 *
 * Distributed under an [MIT-style] license.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * [MIT-style]: http://www.opensource.org/licenses/mit-license.php
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <getopt.h>

struct section
{
    char *name;
    char *description;
    int *options;
};

struct help
{
    char *option;
    char *description;
};

enum emit_mode
{
    BASH,
    ZSH
};

typedef enum emit_mode emit_mode;

struct settings
{
    emit_mode  emit_mode;
    const char *json_path;
    const char *input_file_name;
};

enum
{
    SHOW_VERSION,
    SHOW_WARRANTY,
    SHOW_HELP,
    ENTER_INTERACTIVE,
    EVAL_PATH
};

typedef int cmd;

cmd process_options(const int argc, char * const *argv, struct settings *settings);

#endif
