#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <t1lib.h>
#include <wchar.h>

#define MAXVARCOUNT 65536
#define MAXARGCOUNT 8
#define MAXFUNCOUNT 512

typedef enum toktype{ 
	FUNC,
	ARG,
	DEPTHUP,
	DEPTHDOWN,
	NIL
} toktype;

typedef struct dictionaryll {
	char *data;
	toktype type;
	struct dictionaryll *next;
} dictionaryll;

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
*/

char *builtinfunctions[] = {
	"def",
	"set",
	"+",
	"-",
	"print",
};

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

lispll *exec(lispcodenode *code); // for recursion

lispll *getargasvarat(lispcodenode *code, int index){
	if (code->functionargs[index] != NULL){
		lispll *tmp = exec(code->functionargs[index]);
		if (tmp != NULL) return tmp;
		exit(1);
	} else if (code->vararguments[index] != -1){
		return variables[code->vararguments[index]];
	} else {
		return code->literalarguments[index];
	}
}

lispll *exec(lispcodenode *code){
	lispll *newvar, *var1, *var2;
	int ptr;
	char *varname1, varname2;
	switch (code->funcid){
		case 0: 
			newvar = malloc(sizeof(lispll));
			variables[varidptr] = newvar;
			varnames[varidptr] = fromlltocstr(getargasvarat(code, 0));
			varidptr++;
			break;
		case 1:
			ptr = 0;
			varname1 = fromlltocstr(getargasvarat(code, 0));
			while (strcmp(varnames[ptr], varname1) != 0) ptr++; 
			free(varname1);
			
			variables[ptr] = getargasvarat(code, 1);

			break;
		case 2:
			var1 = getargasvarat(code, 0);
			var2 = getargasvarat(code, 1);

			newvar = malloc(sizeof(lispll));
			newvar->data = var1->data + var2->data;
			return newvar;
			break;
		case 3:
			var1 = getargasvarat(code, 0);
			var2 = getargasvarat(code, 1);

			newvar = malloc(sizeof(lispll));
			newvar->data = var1->data - var2->data;
			return newvar;
			break;
		case 4:
			newvar = getargasvarat(code, 0);
			printf("%d\n", newvar->data);
			break;
		default:
			exit(3);
			break;
	}
	return NULL;
}

int readlen = 0;
//# returns an alocated string that is cut at the first ocurence of one of the chars from cs, and stores the length it read into readlen
char *readuntil(char *s, char* cs){ 
	char *out = malloc(strlen(s)+1);
	strcpy(out, s);
	char *tmp; 
	for (int i = 0; i < strlen(cs); i++)
		if ((tmp = strchr(out, cs[i])) != NULL) 
			*tmp = '\0';

	readlen = strlen(out);

	return out;
}

int getfuncid(char *func){
	for (int i = 0; i < MAXFUNCOUNT; i++){
		if (strcmp(func, builtinfunctions[i])) return i;
		// should do something for a user defined function here
	}
	return -1; 
}

int getvarid(char *name){
	int ptr = 0;
	while (strcmp(varnames[ptr], name) != 0) ptr++;  // fix this
	free(name);

	return ptr;
}

lispll *litteraltoll(char *litteral){
	lispll *ll = malloc(sizeof(lispll));
	errno = 0;
	ll->data = strtol(litteral, NULL, 10);
	if (errno == 0)
		return ll;
	else {
		free(ll);
		return NULL;
	}
}

lispcodenode *parse(dictionaryll *tokens){
	dictionaryll *token = tokens;
	if (tokens == NULL) return NULL;

	lispcodenode *code = malloc(sizeof(lispcodenode));
	code->funcid = NIL;
	int argcount = 0;

	while (token != NULL){
		if (token->type == DEPTHDOWN) return NULL;
		else if (code->funcid == NIL && token->type == FUNC) 
			code->funcid = getfuncid(token->data);
		else if (token->type == ARG){
			code->vararguments[argcount] = getvarid(token->data);
			code->literalarguments[argcount] = litteraltoll(token->data);
			argcount++;
		} else if (code->funcid != NIL && token->type == DEPTHUP){
			code->functionargs[argcount] = parse(token->next);
			argcount++;
		}
		token = token->next;
	}	
	return code;
}

toktype prev;
//# converts an expression into a linked list of tokens
dictionaryll *lexer(char *exp){ 
	if (exp[0] == '\0') {
		return NULL;
	}

	char *tmp;
	dictionaryll *dict = malloc(sizeof(dictionaryll));

	if (exp[0] == '(') {
		dict->data = NULL;
		dict->type = DEPTHUP;
		prev = DEPTHUP;
		dict->next = lexer(exp+1);
	} else if (exp[0] == ')') {
		dict->data = NULL;
		dict->type = DEPTHDOWN;
		prev = DEPTHDOWN;
		dict->next = lexer(exp+1);
	} else if (prev == DEPTHUP){
		dict->data = readuntil(exp, " )");
		dict->type = FUNC;
		prev = FUNC;
		dict->next = lexer(exp+readlen+1);
	} else if (prev == FUNC || prev == ARG){
		dict->data = readuntil(exp, " )");
		dict->type = ARG;
		dict->next = lexer(exp+readlen+1);
	}

	return dict;
}


int main(){
	parse(lexer("(print (+ (- 2 3) 2))"));
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
	var->data = 2;

	lispll *var2 = malloc(sizeof(lispll));
	var2->data = 1;

	lispcodenode *code1 = malloc(sizeof(lispcodenode));

	// define var 1
	code1->funcid = 0;
	code1->literalarguments[0] = varname1;
	code1->vararguments[0] = -1;

	exec(code1);
	// asign var 1
	code1->funcid = 1;
	code1->literalarguments[0] = varname1;
	code1->literalarguments[1] = var;
	code1->vararguments[0] = -1;
	code1->vararguments[1] = -1;
	exec(code1);

	// define var 2
	code1->funcid = 0;
	code1->literalarguments[0] = varname2;
	code1->vararguments[0] = -1;
	exec(code1);

	// asign var 2
	code1->funcid = 1;
	code1->literalarguments[0] = varname2;
	code1->literalarguments[1] = var2;
	code1->vararguments[0] = -1;
	code1->vararguments[1] = -1;

	exec(code1);


	// define var 3
	code1->funcid = 0;
	code1->literalarguments[0] = varname3;
	code1->vararguments[0] = -1;
	exec(code1);

	// make the add function	
	lispcodenode *code2 = malloc(sizeof(lispcodenode));
	code2->funcid = 2;
	code2->vararguments[0] = 0;
	code2->vararguments[1] = 1;

	// asign var 3
	code1->funcid = 1;
	code1->literalarguments[0] = varname3;
	code1->functionargs[1] = code2;
	code1->vararguments[0] = -1;
	code1->vararguments[1] = -1;
	exec(code1);

	code1->funcid = 4;
	code1->vararguments[0] = 2;
	exec(code1);
}
