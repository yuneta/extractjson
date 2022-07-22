/****************************************************************************
 *          YSCAPEC.C
 *
 *          Escape File to C String
 *
 *          Copyright (c) 2020 Niyamaka.
 *          All Rights Reserved.
 ****************************************************************************/
#include <stdio.h>
#include <argp.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ghelpers.h>

/***************************************************************************
 *              Constants
 ***************************************************************************/
#define NAME        "extractjson"
#define DOC         "Extract json from FILE or stdin (-) ignoring no json prefix or postfix data. Examples:\nycommand -c 'command-yuno id=gpu service=__yuno__ command=view-config' |extractjson |fx\nycommand -c 'command-yuno id=gpu service=__yuno__ command=view-gobj-tree' |extractjson |fx"

#define VERSION     "1.0.0"
#define SUPPORT     "<niyamaka at yuneta.io>"
#define DATETIME    __DATE__ " " __TIME__

/***************************************************************************
 *              Structures
 ***************************************************************************/
/*
 *  Used by main to communicate with parse_opt.
 */
#define MIN_ARGS 0
#define MAX_ARGS 0
struct arguments
{
    char *args[MAX_ARGS+1];     /* positional args */
    int max_matches;              /* line size */
    int use_file;
    const char *json_file;
};

/***************************************************************************
 *              Prototypes
 ***************************************************************************/
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/***************************************************************************
 *      Data
 ***************************************************************************/
const char *argp_program_version = NAME " " VERSION;
const char *argp_program_bug_address = SUPPORT;

/* Program documentation. */
static char doc[] = DOC;

/* A description of the arguments we accept. */
static char args_doc[] = "";

/*
 *  The options we understand.
 *  See https://www.gnu.org/software/libc/manual/html_node/Argp-Option-Vectors.html
 */
static struct argp_option options[] = {
/*-name-------------key-----arg---------flags---doc-----------------group */
{"file",            'f',    "FILE",     0,      "File with json", 0},
{"max-match",       'm',    "NUMBER",   0,      "Maximum number of matches (0 no limit)",    0},
{0}
};

/* Our argp parser. */
static struct argp argp = {
    options,
    parse_opt,
    args_doc,
    doc
};

/***************************************************************************
 *  Parse a single option
 ***************************************************************************/
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    /*
     *  Get the input argument from argp_parse,
     *  which we know is a pointer to our arguments structure.
     */
    struct arguments *arguments = state->input;

    switch (key) {
    case 'f':
        arguments->json_file = arg;
        arguments->use_file = 1;
        break;
    case 'm':
        arguments->max_matches = atoi(arg);
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num >= MAX_ARGS) {
            /* Too many arguments. */
            argp_usage (state);
        }
        arguments->args[state->arg_num] = arg;
        break;

    case ARGP_KEY_END:
        if (state->arg_num < MIN_ARGS) {
            /* Not enough arguments. */
            argp_usage (state);
        }
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/***************************************************************************
 *
 ***************************************************************************/
int json_stream_callback(
    void *user_data,
    json_t *dict_found  // owned!!
)
{
    print_json(dict_found);
    json_decref(dict_found);
    return 0;
}

/***************************************************************************
 *
 ***************************************************************************/
int your_utility(int use_file, const char *filename, int max_matches)
{
    if(!use_file) {
        filename = "/dev/stdin";
    }
    if(empty_string(filename)) {
        printf("What filename?\n");
        exit(-1);
    }

    stream_json_filename_parser(
        filename,
        json_stream_callback,
        0,  // user_data,
        0   // 1 log, 2 log+dump
    );


    FILE *file = fopen(filename, "r");
    if(!file) {
        printf("Cannot open '%s': %s\n", filename, strerror(errno));
        exit(-1);
    }

    fclose(file);
    return 0;
}

/***************************************************************************
 *                      Main
 ***************************************************************************/
int main(int argc, char *argv[])
{
    struct arguments arguments;

    /*
     *  Default values
     */
    arguments.use_file = 0;
    arguments.json_file = "";
    arguments.max_matches = 0;

    /*
     *  Parse arguments
     */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    /*
     *  Do your work
     */
    return your_utility(
        arguments.use_file,
        arguments.json_file,
        arguments.max_matches
    );
}
