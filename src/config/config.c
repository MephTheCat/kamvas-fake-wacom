#include <ctype.h>
#include <config/config.h>

typedef struct {
	const char        *token;
	config_command_t   command;
	config_arg_type_t  argtype;
	uint8_t            num_args;
} config_expects_t;

config_expects_t config_expects[] = {
	{ .token = "tablet_name", .command = CONFIG_NAME, .argtype = CONFIG_ARG_STRING, .num_args = 1 },
	{ .token = "mapped_resolution", .command = CONFIG_MAPPED_RES, .argtype = CONFIG_ARG_NUM, .num_args = 2 },
	{ .token = "", .command = CONFIG_END, .argtype = CONFIG_ARG_NUM, .num_args = 0 }
};

const char *config_delim = " \t";

int find_config_format(const char *name, config_expects_t *conf) {
	for (int i = 0; config_expects[i].command != CONFIG_END; i++) {
		if (strncmp(name, config_expects[i].token, 31) == 0) {
			if (config_expects[i].command == CONFIG_END)
				return -1;
			*conf = config_expects[i];
			return 0;
		}
	}
	
	return -1;
}

int config_interpret_multiple(const char *buffer, emulated_tablet_config_t *emu_config, tablet_device_t *tab_config) {
	const char *buffer_end = buffer + strlen(buffer);
	int err;
	
	while (buffer < buffer_end) {
		err = config_interpret(buffer, emu_config, tab_config);
		
		if (err != 0)
			printf("RETVAL %d\n", err);
		
		if (err < 0)
			return err;
		
		buffer += err + 1;
	}
	
	return 0;
}

int config_interpret(const char *line, emulated_tablet_config_t *emu_config, tablet_device_t *tab_config) {
	(void)(emu_config);
	(void)(tab_config);
	
	config_line_t config;
	int err = config_parse_line(line, &config);
	
	if (err < 0)
		return err;
	
	return config.length;
	
	/*printf("%d \"%s\" (%d) %d\n", err, config.line, config.length, config.cmd);
	
	if (config.argtype == CONFIG_ARG_STRING) {
		printf("\"%s\"\n", config.string_arg);
	} else {
		for (int i = 0; i < 8; i++)
			printf("%ld%c", config.numeric_args[i], (i == 7) ? '\n' : ' ');
	}
	
	if (err == 0) {
		if (config.argtype == CONFIG_ARG_STRING) {
			free(config.string_arg);
		}
		
		free(config.line);
		
		return config.length + 1; // Add on to allow for the newline or null
	}
	
	return err;*/
}

int config_parse_line(const char *line, config_line_t *config) {
	int               err = 0;
	config_expects_t  fmt = {};
	char              command[32];
	const char       *line_end;
	
	if (!config)
		return -1;
	
	config->length = 0;
	
	line_end = strchr(line, '\n');
	if (line_end) {
		config->length = (line_end - line);
	} else {
		config->length = strlen(line);
		line_end       = line + config->length;
	}
	
	sscanf(line, "%31s", command);
	
	err = find_config_format(command, &fmt);
	if (err < 0)
		return err;
	
	//printf("Format found: %s\n", fmt.token);
	
	return 0;
}

int config_parse_line_old(const char *line, config_line_t *config) {
	if (!config)
		return -1;
	
	// A config line ends at either a newline or a null byte, whichever is first
	config->length = 0;
	const char *line_end = strchr(line, '\n');
	
	if (line_end) {
		config->length = line_end - line;
	} else {
		config->length = strlen(line);
		line_end       = line + config->length;
	}
	
	// Consume any leading characters that aren't alphabetic
	while (!isalpha(*line) && (line != line_end))
		line++;
	
	if (line == line_end) {
		config->cmd = CONFIG_NOTHING;
		return 0;
	}
	
	// Anything left should alphabetic, consume until we hit whitespace
	const char *token_start = line;
	const char *token_end   = NULL;
	while (isprint(*line) && !isblank(*line) && line != line_end)
		line++;
	token_end = line-1;
	
	// Whatever we consumed should be a token name
	int   token_length = (token_end - token_start) + 1;
	char *token_value  = (char *)malloc(token_length + 1);
	memset(token_value, 0, token_length + 1);
	strncpy(token_value, token_start, token_length);
	
	config_expects_t fmt;
	if (find_config_format(token_value, &fmt)) {
		free(token_value);
		return -2;
	}
	
	config->line = token_value;
	
	// Consume whitespace again, require that there be at least one space
	if (!isblank(*line)) {
		config->cmd = CONFIG_INVALID;
		return -3;
	}
	
	while (isblank(*line) && (line != line_end))
		line++;
	
	const char *args_start  = line;
	const char *args_end    = line_end - 1;
	int         args_length = (args_end - args_start) + 1;
	
	if (args_start == args_end) {
		config->cmd = CONFIG_INVALID;
		return -3;
	}
	
	config->cmd     = fmt.command;
	config->argtype = fmt.argtype;
	if (fmt.argtype == CONFIG_ARG_STRING) {
		// For string arguments, read everything up to the end of the string
		// Therefore, string config lines may only have one argument
		
		config->string_arg = (char *)malloc(args_length + 1);
		memset(config->string_arg, 0, args_length + 1);
		strncpy(config->string_arg, args_start, args_length);
	} else if (fmt.argtype == CONFIG_ARG_NUM) {
		for (int i = 0; i < 8; i++)
			config->numeric_args[i] = 0;
		
		int num_parsed_args = 0;
		while (num_parsed_args < 8 && args_start < args_end) {
			long val = 0;
			int  off = 0;
			int  num = sscanf(args_start, "%ld%n", &val, &off);
			
			args_start += off;
			
			if (num <= 0) {
				config->cmd = CONFIG_INVALID;
				return -4;
			}
			
			config->numeric_args[num_parsed_args] = val;
			num_parsed_args++;
		}
		
		if (num_parsed_args != fmt.num_args) {
			config->cmd = CONFIG_INVALID;
			return -5;
		}
	}
	
	return 0;
}
