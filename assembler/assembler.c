#include "assembler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "list.h"
#include "objects.h"
#include "readline.h"
#include "stringop.h"
#include "instructions.h"
#include "match.h"
#include "errors.h"

#define ERROR(ERROR_CODE, COLUMN) add_error(errors, ERROR_CODE, line_number, _line, COLUMN, file_name);

object_t *assemble(FILE *file, const char *file_name, instruction_set_t *set, list_t *errors, list_t *warnings) {
	object_t *object = create_object();
	area_t *current_area = create_area("CODE");
	list_add(object->areas, current_area);
	uint8_t *instruction_buffer = malloc(64 / 8);
	int line_number = 0;
	while (!feof(file)) {
		++line_number;
		char *line = read_line(file);
		char *_line = malloc(strlen(line) + 1);
		strcpy(_line, line);
		int trimmed_start;
		line = strip_whitespace(line, &trimmed_start);
		line = strip_comments(line);
		if (strlen(line) == 0) {
			continue;
		}
		instruction_match_t *match = match_instruction(set, line);
		if (match == NULL) {
			ERROR(ERROR_INVALID_INSTRUCTION, trimmed_start);
		} else {
			uint64_t instruction = match->instruction->value;
			int i;
			for (i = 0; i < match->operands->length; ++i) {
				operand_ref_t *ref = match->operands->items[i];
				instruction |= ref->op->value << (match->instruction->width - ref->shift - ref->op->width);
			}
			int bytes_width = match->instruction->width / 8;
			for (i = 0; i < bytes_width; ++i) {
				instruction_buffer[bytes_width - i - 1] = instruction & 0xFF;
				instruction >>= 8;
			}
			/* Add completed instruction */
			append_to_area(current_area, instruction_buffer, bytes_width);
		}

		free(_line);
		free(line);
	}
	return object;
}
