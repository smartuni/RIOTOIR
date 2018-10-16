/**
 * \file
 * Riot Test-Application for CRC-8-bit check functions.
 *
 */

#include "crc8.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "shell.h"

int crc(int argc, char **argv)
{
	if(argc > 0){
		uint8_t crc;

		crc = getCRC8((uint8_t*) argv[1], strlen(argv[1]));

		printf("%d\n", crc);

		return crc;
	} else {
		printf("no arguments!!\n");
		return 0;
	}
}

int checkCRC(int argc, char **argv)
{
	if(argc > 0){
		bool equal;

		uint8_t crc8 = atoi(argv[1]);

		equal = checkCRC8(crc8, (uint8_t*) argv[2], strlen(argv[2]));

		printf("%s\n", equal ? "true" : "false");

		return 0;
	} else {
		printf("no arguments!!\n");
		return 0;
	}
}

static const shell_command_t shell_commands[] = {
    { "crc8", "checksum with 8 bit return", crc},
	{ "checkCrc8", "checks the equality of delivered crc and calculated crc", checkCRC},
    { NULL, NULL, NULL }
};

int main(void) {

	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
}
