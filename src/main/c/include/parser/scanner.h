#pragma once

#include "parser/input.h"
#include "parser/token.h"
#include "parser/errors.h"

typedef void (*ErrorCallback)(Position position, ParserErrorCode code, void *parameter);

struct error_handler_s
{
    ErrorCallback  callback;
    void          *parameter;
};

typedef struct error_handler_s ErrorHandler;

struct scanner_s
{
    ErrorHandler handler;
    Token        current;
    Input        input;
};

typedef struct scanner_s Scanner;

Scanner *make_scanner(const char *data, size_t length);
void     dispose_scanner(Scanner *self);

void scanner_next(Scanner *self);
void scanner_reset(Scanner *self);
