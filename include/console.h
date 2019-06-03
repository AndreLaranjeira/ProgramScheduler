/**
 *  File: console.h
 *  Description: This header contains some static functions to print custom messages on the terminal using ANSI escape characters
 *
 *  Available methods: success (green), error (red), warning (yellow), info (blue), message (default print)
 *
 *  Prototype: void success(const char *context, const char *msg_fmt, ...);
 *
 *  Arguments:
 *      # context: string that will represent from where the function is being called.
 *          # You must nullify this argument in case you don't to inform the current context
 *
 *      # msg_fmt: string that will interpolate a message to print
 *          # The following arguments are use to interpolate your message
 *          # This has the exactly same behavior as a printf
 *
 *  Examples:
 *      success("main", "A = %d", A);
 *      >> [main]: [SUCCESS] A = 10
 *
 *
 *      success(NULL, "A = %d", A);
 *      >> [SUCCESS] A = 10
 * */

 /* Code authors:
  * André Filipe Caldas Laranjeira - 16/0023777
  * Hugo Nascimento Fonseca - 16/0008166  (Module creator and maintainer)
  * José Luiz Gomes Nogueira - 16/0032458
  * Victor André Gris Costa - 16/0019311
  */

// Header guard:
#ifndef CONSOLE_H
#define CONSOLE_H

// Compiler includes:
#include <stdio.h>
#include <stdarg.h>

// Macros:

// Text colors:
#define TEXT_COLOR_BLACK   "\x1b[30m"
#define TEXT_COLOR_RED     "\x1b[31m"
#define TEXT_COLOR_GREEN   "\x1b[32m"
#define TEXT_COLOR_YELLOW  "\x1b[33m"
#define TEXT_COLOR_BLUE    "\x1b[34m"
#define TEXT_COLOR_MAGENTA "\x1b[35m"
#define TEXT_COLOR_CYAN    "\x1b[36m"
#define TEXT_COLOR_WHITE   "\x1b[37m"

// Background colors:
#define BKGD_COLOR_BLACK   "\x1b[40m"
#define BKGD_COLOR_RED     "\x1b[41m"
#define BKGD_COLOR_GREEN   "\x1b[42m"
#define BKGD_COLOR_YELLOW  "\x1b[43m"
#define BKGD_COLOR_BLUE    "\x1b[44m"
#define BKGD_COLOR_MAGENTA "\x1b[45m"
#define BKGD_COLOR_CYAN    "\x1b[46m"
#define BKGD_COLOR_WHITE   "\x1b[47m"

// Text blinking colors:
#define TEXT_BCOLOR_BLACK   "\x1b[90m"
#define TEXT_BCOLOR_RED     "\x1b[91m"
#define TEXT_BCOLOR_GREEN   "\x1b[92m"
#define TEXT_BCOLOR_YELLOW  "\x1b[93m"
#define TEXT_BCOLOR_BLUE    "\x1b[94m"
#define TEXT_BCOLOR_MAGENTA "\x1b[95m"
#define TEXT_BCOLOR_CYAN    "\x1b[96m"
#define TEXT_BCOLOR_WHITE   "\x1b[97m"

// Background blinking colors:
#define BKGD_BCOLOR_BLACK   "\x1b[100m"
#define BKGD_BCOLOR_RED     "\x1b[101m"
#define BKGD_BCOLOR_GREEN   "\x1b[102m"
#define BKGD_BCOLOR_YELLOW  "\x1b[103m"
#define BKGD_BCOLOR_BLUE    "\x1b[104m"
#define BKGD_BCOLOR_MAGENTA "\x1b[105m"
#define BKGD_BCOLOR_CYAN    "\x1b[106m"
#define BKGD_BCOLOR_WHITE   "\x1b[107m"

// Bold text:
#define TEXT_BOLD_UNDL   "\x1b[1m"

// Reset colors:
#define RESET   "\x1b[0m"

// Function implementations:
static void success(const char *context, const char *msg_fmt, ...){
    if(context != NULL){
        printf("[%s]: ", context);
    }

    printf(TEXT_COLOR_GREEN "[SUCCESS] ");
    va_list arg_list;
    va_start(arg_list, msg_fmt);
    vprintf(msg_fmt, arg_list);
    va_end(arg_list);
    printf(RESET);
}

static void error(const char *context, const char* msg_fmt,...) {
    if(context != NULL){
        printf("[%s]: ", context);
    }

    printf(TEXT_COLOR_RED "[ERROR] ");
    va_list arg_list;
    va_start(arg_list, msg_fmt);
    vprintf(msg_fmt, arg_list);
    va_end(arg_list);
    printf(RESET);
}

static void info(const char *context, const char* msg_fmt,...) {
    if(context != NULL){
        printf("[%s]: ", context);
    }

    printf(TEXT_COLOR_BLUE "[INFO] ");
    va_list arg_list;
    va_start(arg_list, msg_fmt);
    vprintf(msg_fmt, arg_list);
    va_end(arg_list);
    printf(RESET);
}

static void warning(const char *context, const char* msg_fmt,...) {
    if(context != NULL){
        printf("[%s]: ", context);
    }

    printf(TEXT_BCOLOR_YELLOW "[WARNING] ");
    va_list arg_list;
    va_start(arg_list, msg_fmt);
    vprintf(msg_fmt, arg_list);
    va_end(arg_list);
    printf(RESET);
}

static void message(const char *context, const char* msg_fmt,...){
    if(context != NULL){
        printf("[%s]: ", context);
    }

    va_list arg_list;
    va_start(arg_list, msg_fmt);
    vprintf(msg_fmt, arg_list);
    va_end(arg_list);
}

static void customize_flags(int num_flags, ...){
    va_list arg_list;
    va_start(arg_list, num_flags);
    for(int i = 0; i < num_flags; i++){
        printf("%s", va_arg(arg_list, const char*));
    }
    va_end(arg_list);
}

static void reset_flags(){
    printf(RESET);
}

#endif //CONSOLE_H
