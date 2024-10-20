#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <t1lib.h>

#define MAXVARCOUNT 65536
#define MAXARGCOUNT 8
#define MAXFUNCOUNT 512

typedef enum toktype{ 
	FUNC,
	LITTERAL,
	STRLITTERAL,
	VARUSE,
	DEPTHUP,
	DEPTHDOWN
} toktype;

typedef struct dictionary {
	void *data;
	toktype type;
} dictionary;

typedef struct lispll {
	int data;
	struct lispll *next;
} lispll;

typedef struct lispcodenode {
	int funcid;
	lispll *literalarguments[MAXARGCOUNT];
	int vararguments[MAXARGCOUNT];
	struct lispcodenode *functionargs[MAXARGCOUNT];
} lispcodenode;

lispll *variables[MAXVARCOUNT]; // a vairables id is just its index
char *varnames[MAXVARCOUNT];
int varidptr = 0;

/* function id's
 * 0 - def 
 * 1 - set
 * 2 - add
 * 3 - sub
 * 4 - print
 *
*/

lispcodenode *functions[MAXFUNCOUNT]; // a functions id is just its index
int funcidptr = 5;

size_t lllen(lispll *ll){
	if (ll == NULL) return 0;
	else return 1 + lllen(ll->next);
}

char *fromlltocstr(lispll *ll){
	int ptr = 0;
	char *str = malloc(lllen(ll));
	lispll *tmp = malloc(sizeof(lispll));
	tmp = ll;	
	while (tmp != NULL){
		str[ptr] = tmp->data;
		tmp = tmp->next;
		ptr++;
	}
	free(tmp);

	return str;
}

lispll *exec(lispcodenode *code){
	lispll *newvar, *var1, *var2;
	int ptr;
	char *varname1, varname2;
	switch (code->funcid){
		case 0: 
			newvar = malloc(sizeof(lispll));
			variables[varidptr] = newvar;
			varnames[varidptr] = fromlltocstr(code->literalarguments[0]);
			varidptr++;
			break;
		case 1:
			ptr = 0;
			varname1 = fromlltocstr(code->literalarguments[0]);
			while (strcmp(varnames[ptr], varname1) != 0) ptr++; 
			free(varname1);

			if (code->functionargs[1] != NULL){
				if ((variables[ptr] = exec(code->functionargs[1])) == NULL) {
					exit(1);	
				}
			} else if (code->vararguments[1] != -1){
				variables[ptr] = variables[code->vararguments[1]];
			} else {
				variables[ptr] = code->literalarguments[1];
			}

			break;
		case 2:
			if (code->functionargs[0] != NULL){
				if ((var1 = exec(code->functionargs[1])) == NULL) exit(1);	
			} else if (code->vararguments[0] != -1){
				var1 = variables[code->vararguments[0]];
			} else {
				var1 = code->literalarguments[0];
			}

			if (code->functionargs[1] != NULL){
				if ((var2 = exec(code->functionargs[1])) == NULL) exit(1);	
			} else if (code->vararguments[1] != -1){
				var2 = variables[code->vararguments[1]];
			} else {
				var2 = code->literalarguments[1];
			}

			newvar = malloc(sizeof(lispll));
			newvar->data = var1->data + var2->data;
			return newvar;
			break;
		case 3:
			if (code->functionargs[0] != NULL){
				if ((var1 = exec(code->functionargs[1])) == NULL) exit(1);	
			} else if (code->vararguments[0] != -1){
				var1 = variables[code->vararguments[1]];
			} else {
				var1 = code->literalarguments[1];
			}

			if (code->functionargs[1] != NULL){
				if ((var2 = exec(code->functionargs[1])) == NULL) exit(1);	
			} else if (code->vararguments[1] != -1){
				var2 = variables[code->vararguments[1]];
			} else {
				var2 = code->literalarguments[1];
			}

			newvar = malloc(sizeof(lispll));
			newvar->data = var1->data - var2->data;
			return newvar;
			break;
		case 4:

			if (code->functionargs[0] != NULL){
				if ((var2 = exec(code->functionargs[0])) == NULL) exit(1);	
				printf("%d\n", exec(code->functionargs[0])->data);
			} else if (code->vararguments[0] != -1){
				var1 = variables[code->vararguments[0]];
				printf("%d\n", var1->data);
			} else {
				printf("%d\n", code->literalarguments[0]->data);
			}

			break;
		default:
			break;
	}
	return NULL;
}

int readlen = 0;
char *readuntil(char *s, char c){
	char *out = malloc(strlen(s)+1);
	strcpy(out, s);
	char *tmp = out;
	*strchr(out, c) = '\0';

	readlen = out - tmp;

	return out;
}

char *readexpr(char *exp) { // reads an exp from the bracket at exp[0] till its closing
	if (exp[0] != '(') return NULL;
	stack *s = initstack(128);
	char *str = malloc(strlen(exp)+2);
	for (int i = 0; i < strlen(exp); i++){
		if (exp[i] == '(') push(s, '(');
		else if (exp[i] == ')') pop(s);
		str[i] = exp[i];
		if (isempty(s)) {
			i++;
			str[i] = '\0';
			deinitstack(s);
			return str;
		}
	}
	deinitstack(s);	
	exit(2);
	return NULL;
}

dictionary **lexer(char *exp){ // (print (+ (- 2 4) 2))
	dictionary **dict;
	char *tmp;
	int ptr = 0;
	for (int i = 0; i < 512; i++)
		dict[i] = malloc(sizeof(dictionary));

	for (int i = 0; i < strlen(exp); i++){
		if (exp[i] == '(') {dict[ptr]->type = DEPTHUP; ptr++; }
		else if (exp[i] == ')') {dict[ptr]->type = DEPTHDOWN; ptr++; }
		else if (dict[ptr-1]->type == DEPTHUP){
			dict[ptr]->data = readuntil(&exp[i], ' ');
			i += readlen;
			dict[ptr]->type = FUNC;
		}else if (dict[ptr-1]->type == FUNC){
			// read args here
		}
	}

	return dict;
}


int main(){
	lispll *varname1 = malloc(sizeof(lispll));
	varname1->data = 'a';
	varname1->next = malloc(sizeof(lispll));
	varname1->next->data = '\0';

	lispll *varname2 = malloc(sizeof(lispll));
	varname2->data = 'b';
	varname2->next = malloc(sizeof(lispll));
	varname2->next->data = '\0';

	lispll *varname3 = malloc(sizeof(lispll));
	varname3->data = 'c';
	varname3->next = malloc(sizeof(lispll));
	varname3->next->data = '\0';

	lispll *var = malloc(sizeof(lispll));
	var->data = 1;

	lispcodenode *code1 = malloc(sizeof(lispcodenode));

	// define var 1
	code1->funcid = 0;
	code1->literalarguments[0] = varname1;
	code1->functionargs[0] = NULL;
	code1->vararguments[0] = -1;
	exec(code1);
	// asign var 1
	code1->funcid = 1;
	code1->literalarguments[0] = varname1;
	code1->functionargs[0] = NULL;
	code1->vararguments[0] = -1;

	code1->literalarguments[1] = var;
	code1->functionargs[1] = NULL;
	code1->vararguments[1] = -1;
	exec(code1);

	// define var 2
	code1->funcid = 0;
	code1->literalarguments[0] = varname2;
	code1->functionargs[0] = NULL;
	code1->vararguments[0] = -1;
	exec(code1);

	// asign var 2
	code1->funcid = 1;
	code1->literalarguments[0] = varname2;
	code1->functionargs[0] = NULL;
	code1->vararguments[0] = -1;

	code1->literalarguments[1] = var;
	code1->functionargs[1] = NULL;
	code1->vararguments[1] = -1;
	exec(code1);



	// define var 3
	code1->funcid = 0;
	code1->literalarguments[0] = varname3;
	code1->functionargs[0] = NULL;
	code1->vararguments[0] = -1;
	exec(code1);

	lispcodenode *code3 = malloc(sizeof(lispcodenode));

	// make the add function	
	lispcodenode *code2 = malloc(sizeof(lispcodenode));
	code2->funcid = 2;
	code2->vararguments[0] = 0;
	code2->vararguments[1] = -1;
	code2->literalarguments[0] = NULL;
	code2->literalarguments[1] = NULL;
	code2->functionargs[0] = NULL;
	code2->functionargs[1] = code3;

	code3->funcid = 2;
	code3->vararguments[0] = 0;
	code3->vararguments[1] = 1;
	code3->literalarguments[0] = NULL;
	code3->literalarguments[1] = NULL;
	code3->functionargs[0] = NULL;
	code3->functionargs[1] = NULL;

	// asign var 3
	code1->funcid = 1;
	code1->literalarguments[0] = varname3;
	code1->functionargs[0] = NULL;
	code1->vararguments[0] = -1;

	code1->literalarguments[1] = NULL;
	code1->functionargs[1] = code2;
	code1->vararguments[1] = -1;
	exec(code1);

	code1->funcid = 4;
	code1->vararguments[0] = 2;
	exec(code1);

	code1->funcid = 4;
	code1->functionargs[0] = code2;
	exec(code1);
}
