#include "shell.h"
#include <stddef.h>
#include "string-util.h"

extern void put(char * str);

void cmd_arbiter(char * cmd)
{
}

void hello()
{
	put("hello world!\n\r");
}
