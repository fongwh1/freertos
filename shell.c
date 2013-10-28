#include "shell.h"
#include <stddef.h>
#include "string-util.h"

#include "FreeRTOS.h"
#include "task.h"

#define NUM_OF_CMD 3

typedef void (*p_cmd_func)(char * arg);
/* define a command desrciption structure*/
typedef struct CMD_DESRCIPTION 
{
	char *name;
	char *des;
	p_cmd_func handler;
	
}cmd;

extern void puts(char * str);

void ps(char * arg)
{
	int numberOfTask;

	int charcheck;

	numberOfTask = uxTaskGetNumberOfTasks();

	signed portCHAR psInfo[numberOfTask * 40];

	vTaskList(psInfo);

	puts(psInfo);

	puts("\n\r\0");


}

void echo(char * arg)
{
	puts(arg);
	puts("\n\r\0");
}

void hello(char *arg)
{
        puts("hello world!\n\r");
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

	int curr_cmd = 0;
	int cmd_len;
	char * arg;

	while(curr_cmd < NUM_OF_CMD)
	{
		cmd_len = strlen(cmd_list[curr_cmd].name);
		if(bounded_strcmp(cmd_list[curr_cmd].name,cmd,cmd_len))
		{
			arg = &cmd[cmd_len];
			cmd_list[curr_cmd].handler(arg);
			break;
		}
		curr_cmd++;
	}

}

