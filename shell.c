#include "shell.h"
#include <stddef.h>
#include "string-util.h"

#define NUM_OF_CMD 3

typedef void (*p_cmd_func)(char * arg);
/* define a command desrciption structure*/
typedef struct CMD_DESRCIPTION 
{
	char *name;
	char *des;
	p_cmd_func handler;
	
}cmd;

extern void put(char * str);

void ps(char * arg)
{
/* ps implementation*/
}

void echo(char * arg)
{
/* echo implementation*/
}

void hello(char *arg)
{
        put("hello world!\n\r");
}


static cmd cmd_list [NUM_OF_CMD] = {
	{
		.name = "ps",
		.des = "process information",
		.handler = ps
	},
	{
		.name = "echo",
		.des = "echo string",
		.handler = echo
	},
	{
		.name = "hello",
		.des = "hello world",
		.handler = hello
	}
};

void cmd_arbiter(char * cmd)
{
	/* prepare a char array to contain string from "itoa"*/
	char num_char[I2A_MAX_DIGI];
	num_char[I2A_MAX_DIGI - 1] = '\0';
}

