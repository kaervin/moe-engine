#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// this code generator is for generating empty function declarations of all library functions, so that they can be added to the dynamically loaded level code, and creating functions so that these empty functions then get assigned at loadtime their code from the main library
// this saves space and in turn speeds up loading and compilation time of the levelcode


char * read_file_into_memory_null_terminate(char *filename) {
	FILE *f = fopen(filename, "r");
	if (!f) return 0;
	fseek(f, 0, SEEK_END);
	size_t filesize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char * result = malloc(filesize+1);
	fread(result, filesize, 1, f);
	result[filesize] = 0;
	fclose(f);
	return result;
}

enum Token_Type {
	TOKEN_EOF,
	TOKEN_IDENTIFIER,
	TOKEN_NK_API,
	TOKEN_ERROR,
	
	TOKEN_OPEN_PAREN,
	TOKEN_CLOSE_PAREN,
	TOKEN_STAR,
};

typedef struct Token {
	enum Token_Type type;
	char *text;
	int length;
} Token;

typedef struct Tokenizer {
	char *at;
} Tokenizer;

typedef struct Out_Files {
	FILE * function_definitions_file;
	FILE * fill_fp_struct_file;
	FILE * receive_fp_file;
	FILE * fp_struct_file;
} Out_Files; 

bool is_whitespace(char *cp) {
	char c = *cp;
	return (((c == ' ') || (c == '\n') || (c == '\t') || (c == '\r')));
}

bool is_alpha(char *cp) {
	char c = *cp;
	return (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') || (c == '.') || (c == '-'));
}

void eat_singleline_comment(Tokenizer *tknzr) {
	tknzr->at += 2;
	while(true) {
		if ((tknzr->at[0] == '\n') || (tknzr->at[0] == 0)) {
			return;
		}
		++tknzr->at;
	}
}

void eat_multiline_comment(Tokenizer *tknzr) {
	tknzr->at += 2;
	while(true) {
		if (tknzr->at[0] == 0) {
			return;
		}
		if ((tknzr->at[0] == '*') && (tknzr->at[1] == '/')) {
			tknzr->at += 2;
			return;
		}
		++tknzr->at;
	}
}

void eat_all_whitespace(Tokenizer *tknzr) {
	
	while(true) {
		if(is_whitespace(tknzr->at)) {
			++tknzr->at;
			continue;
		}
		
		if ((tknzr->at[0] == '/') && (tknzr->at[1] == '/')) {
			eat_singleline_comment(tknzr);
			continue;
		}
		
		if ((tknzr->at[0] == '/') && (tknzr->at[1] == '*')) {
			eat_multiline_comment(tknzr);
			continue;
		}
		
		return;
	}
}

bool does_identifier_match(Token token, char *str) {
	
	int i = 0;
	
	for(; i < token.length; i++) {
		if (str[i] != token.text[i]) {
			return false;
		}
	}
	
	if (str[i] == 0) {
		return true;
	}
	return false;
}

Token parse_alpha(Tokenizer *tknzr) {
	Token token;
	token.type = TOKEN_IDENTIFIER;
	token.length = 0;
	token.text = tknzr->at;
	
	while(is_alpha(tknzr->at)) {
		++token.length;
		++tknzr->at;
	}
	return token;
}

Token get_token(Tokenizer *tknzr) {
	Token token;
	eat_all_whitespace(tknzr);
	
	token.text = tknzr->at;
	token.length = 0;
	
	if (tknzr->at[0] == 0) {
		token.type = TOKEN_EOF;
		return token;
	}
	
	if (is_alpha(tknzr->at)) {
		token = parse_alpha(tknzr);
		
		if (does_identifier_match(token, "FUN")) {
			token.type = TOKEN_NK_API;
		}
		return token;
	}
	
	token.length = 1;
	
	if (tknzr->at[0] == '(') {
		token.type = TOKEN_OPEN_PAREN;
	}
	else if (tknzr->at[0] == ')') {
		token.type = TOKEN_CLOSE_PAREN;
	}
	else if (tknzr->at[0] == '*') {
		token.type = TOKEN_STAR;
	}
	else {
		token.type = TOKEN_ERROR;
	}
	++tknzr->at;
	return token;
}

void parse_function_definition(Tokenizer *tknzr, Out_Files out) {
	
	char string_buffer_return[5000];
	int next_string_return = 0;
	
	char string_buffer_name[5000];
	int next_string_name = 0;
	
	char string_buffer_param[5000];
	int next_string_param = 0;
	
	Token final_ret_token = get_token(tknzr);
	Token ret_type_token = final_ret_token;
	next_string_return += sprintf(&string_buffer_return[next_string_return], "%.*s", ret_type_token.length, ret_type_token.text);
	
	bool parse_return_type = true;
	while(parse_return_type) {
		if (does_identifier_match(ret_type_token, "const") || does_identifier_match(ret_type_token, "struct") || does_identifier_match(ret_type_token, "enum") || does_identifier_match(ret_type_token, "unsigned")) {
			
			ret_type_token = get_token(tknzr);
			next_string_return += sprintf(&string_buffer_return[next_string_return], " %.*s", ret_type_token.length, ret_type_token.text);
		}
		else {
			break;
		}
		
	}
	
	Token fun_name_token = get_token(tknzr);
	if (fun_name_token.type == TOKEN_STAR) {
		next_string_return += sprintf(&string_buffer_return[next_string_return], "* ");
		fun_name_token = get_token(tknzr);
	}
	
	next_string_name += sprintf(&string_buffer_name[next_string_name], "%.*s", fun_name_token.length, fun_name_token.text);
	
	Token open_paren_token = get_token(tknzr);
	if (open_paren_token.type != TOKEN_OPEN_PAREN) {
		return;
	}
	
	Token param_token = get_token(tknzr);
	if (param_token.type == TOKEN_CLOSE_PAREN) {
		next_string_param += sprintf(&string_buffer_param[next_string_param], "void");
	}
	
	int paren_level = 0;
	while(true) {
		if (param_token.type == TOKEN_CLOSE_PAREN && paren_level <= 0) {
			break;
		}
		if (param_token.type == TOKEN_OPEN_PAREN) {
			paren_level++;
		}
		if (param_token.type == TOKEN_CLOSE_PAREN) {
			paren_level--;
		}
		if (does_identifier_match(param_token, "NK_PRINTF_FORMAT_STRING")) {
			param_token = get_token(tknzr);
			continue;
		}
		
		next_string_param += sprintf(&string_buffer_param[next_string_param], "%.*s ", param_token.length, param_token.text);
		
		param_token = get_token(tknzr);
	}
	
	Token close_token = get_token(tknzr);
	if (!does_identifier_match(close_token, ";")) {
		return;
	}
	
	fprintf(out.function_definitions_file, "%s (*%s)(%s) = NULL;\n", string_buffer_return, string_buffer_name, string_buffer_param);
	
	
	fprintf(out.fp_struct_file, "%s (*%s) (%s);\n", string_buffer_return, string_buffer_name, string_buffer_param);
	
	fprintf(out.receive_fp_file, "%s = fp_struct.%s;\n", string_buffer_name, string_buffer_name);
	
	fprintf(out.fill_fp_struct_file, "fp_struct.%s = %s;\n", string_buffer_name, string_buffer_name);
}

int main() {
	char * file_contents = read_file_into_memory_null_terminate("../libs/render_basic.h");
	bool parsing = true;
	Tokenizer tknzr;
	tknzr.at = file_contents;
	
	Out_Files out;
	out.function_definitions_file = fopen("fdef.h", "w");
	if (out.function_definitions_file == NULL) {
		printf("couldn't open\n");
	}
	out.fill_fp_struct_file = fopen("ffill.h", "w");
	if (out.fill_fp_struct_file == NULL) {
		printf("couldn't open\n");
	}
	out.receive_fp_file = fopen("frecv.h", "w");
	if (out.receive_fp_file == NULL) {
		printf("couldn't open\n");
	}
	out.fp_struct_file = fopen("fpstruct.h", "w");
	if (out.fp_struct_file == NULL) {
		printf("couldn't open\n");
	}
	fprintf(out.fp_struct_file, "typedef struct FP_Struct {\n");
	
	fprintf(out.fill_fp_struct_file, "void fill_function_pointers() {\n");
	
	fprintf(out.receive_fp_file, "void receive_function_pointers() {\n");
	
	while(parsing) {
		Token token = get_token(&tknzr);
		
		switch (token.type) {
			case TOKEN_EOF: {
				parsing = false;
			} break;
			
			case TOKEN_IDENTIFIER: {
				//printf("%.*s\n", token.length, token.text);
			} break;
			
			case TOKEN_ERROR: {
				//printf("%.*s", token.length, token.text);
			} break;
			
			case TOKEN_NK_API: {
				printf("xxx  %.*s\n", token.length, token.text);
				parse_function_definition(&tknzr, out);
			} break;
			
			default: {} break;
		}
	}
	
	fprintf(out.fp_struct_file, "} FP_Struct;");
	fprintf(out.fill_fp_struct_file, "}");
	fprintf(out.receive_fp_file, "}");
}