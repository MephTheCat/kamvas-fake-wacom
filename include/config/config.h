#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <tablet_common.h>
#include <emulated/emulated.h>

#define CONFIG_MAX_LINE_LEN 256

typedef enum {
	CONFIG_NAME,
	CONFIG_MAPPED_RES,
	
	CONFIG_NOTHING, // There wasn't anything on that line
	CONFIG_INVALID, // Whatever was on that line wasn't valid input
	CONFIG_END      // Triggers the end of the config list
} config_command_t;

typedef enum {
	CONFIG_ARG_STRING,
	CONFIG_ARG_NUM,
} config_arg_type_t;

typedef struct {
	char             *line;
	int               length;
	config_command_t  cmd;
	config_arg_type_t argtype;
	
	union {
		char *string_arg;
		long numeric_args[8];
	};
} config_line_t;

int config_parse_line(const char *line, config_line_t *config);
int config_interpret(const char *line, emulated_tablet_config_t *emu_config, tablet_device_t *tab_config);
int config_interpret_multiple(const char *buffer, emulated_tablet_config_t *emu_config, tablet_device_t *tab_config);

#endif
