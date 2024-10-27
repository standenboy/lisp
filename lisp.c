#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAXVARCOUNT 65536
#define MAXARGCOUNT 8
#define MAXFUNCOUNT 512

typedef enum toktype{ 
	FUNC = 0,
	ARG = 1,
	DEPTHUP = 2,
	DEPTHDOWN = 3,
	NIL = -1,
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
	char *varargumentsnames[MAXARGCOUNT];
	struct lispcodenode *functionargs[MAXARGCOUNT];
} lispcodenode;

lispll *variables[MAXVARCOUNT]; // a vairables id is just its index
char *varnames[MAXVARCOUNT];
int varidptr = 0;

char *builtinfunctions[] = {
	"def", // takes a string litteral varname and a starting value // 0
	"set", // takes a var and a value // 1
	"+", // takes 2 values // 2
	"-", // takes 2 values // 3 
	"printint", // takes 1 value // 4
	"*", // takes 2 values // 5
	"/", // takes 2 values // 6
	"%", // takes 2 values // 7
	"defproc", // takes a string litteral name // 8
	"endproc", // takes nothing // 9
	"printstr" // takes a value that is a string // 10
}; 

lispcodenode *functions[MAXFUNCOUNT]; // a functions id is just its index
int funcidptr = 0;

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

				      
int getvarid(char *name){
	int ptr = 0;
	while (varnames[ptr] != NULL && strcmp(varnames[ptr], name) != 0) ptr++;
	free(name);

	return ptr;
}

lispll *lispexec(lispcodenode *code); // for recursion

lispll *getargasvarat(lispcodenode *code, int index){
	if (code->functionargs[index] != NULL){
		lispll *tmp = lispexec(code->functionargs[index]);
		if (tmp != NULL) return tmp;
		exit(1);
	} else if (code->varargumentsnames[index] != NULL){
		return variables[getvarid(code->varargumentsnames[index])];
	} else {
		return code->literalarguments[index];
	}
}

lispll *lispexec(lispcodenode *code){
	if (code == NULL) return NULL;
	lispll *newvar, *var1, *var2;
	int ptr;
	char *varname1, varname2;
	switch (code->funcid){
		case 0: 
			newvar = malloc(sizeof(lispll));
			variables[varidptr] = newvar;
			varnames[varidptr] = fromlltocstr(getargasvarat(code, 0));
			memcpy(newvar, getargasvarat(code, 1), sizeof(lispll));

			varidptr++;
			break;
		case 1:
			var1 = getargasvarat(code, 0);
			memcpy(var1, getargasvarat(code, 1), sizeof(lispll));

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
		case 5:
			var1 = getargasvarat(code, 0);
			var2 = getargasvarat(code, 1);

			newvar = malloc(sizeof(lispll));
			newvar->data = var1->data * var2->data;
			return newvar;
			break;
		case 6:
			var1 = getargasvarat(code, 0);
			var2 = getargasvarat(code, 1);

			newvar = malloc(sizeof(lispll));
			newvar->data = var1->data / var2->data;
			return newvar;
			break;
		case 7:
			var1 = getargasvarat(code, 0);
			var2 = getargasvarat(code, 1);

			newvar = malloc(sizeof(lispll));
			newvar->data = var1->data % var2->data;
			return newvar;
			break;
		case 8:
			break;
		case 9:
			break;
		case 10:
			newvar = getargasvarat(code, 0);
			printf("%s\n", fromlltocstr(newvar));
			break;
		default:
			exit(3);
			break;
	}
	return NULL;
}

int readlen = 0;
//# returns an allocated string that is cut at the first ocurence of one of the chars from cs, and stores the length it read into readlen
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
		if (builtinfunctions[i] != NULL){
			if (strcmp(func, builtinfunctions[i]) == 0){
				return i;
			}
		} else return -1;
	}
	return -1; 
}


lispll *litteraltoll(char *litteral){
	lispll *ll = malloc(sizeof(lispll));
	if (litteral[0] == '"'){
		lispll *tmp = ll;
loop:
		litteral++;
		if (litteral[0] != '"') {
			tmp->data = litteral[0];		
			tmp->next = malloc(sizeof(lispll));
			tmp = tmp->next;
			goto loop;
		}
		return ll;
	} else {
		char *eptr;
		ll->data = strtol(litteral, &eptr, 10);
		if (eptr != litteral)
			return ll;
		else {
			free(ll);
			return NULL;
		}
	}
}

lispcodenode *lispparse(dictionaryll *tokens){
	dictionaryll *token = tokens;
	if (tokens == NULL) return NULL;

	lispcodenode *code = malloc(sizeof(lispcodenode));
	code->funcid = NIL;
	int argcount = 0;

	while (token != NULL){
		if (token->type == DEPTHDOWN) return code;
		else if (code->funcid == NIL && token->type == FUNC) 
			code->funcid = getfuncid(token->data);
		else if (token->type == ARG){
			code->literalarguments[argcount] = litteraltoll(token->data);
			if (code->literalarguments[argcount] != NULL) {
				code->vararguments[argcount] = -1;
			} else {
				code->varargumentsnames[argcount] = token->data;
			}
			argcount++;
		} else if (code->funcid != NIL && token->type == DEPTHUP){
			code->functionargs[argcount] = lispparse(token->next);
			int depth = 1;
			while (depth != 0){
				token = token->next;
				if (token->type == DEPTHUP) depth++;
				else if (token->type == DEPTHDOWN) depth--;
			}
			argcount++;
		}
		token = token->next;
	}	
	return code;
}

toktype prev;
//# converts an expression into a linked list of tokens
dictionaryll *lisplexer(char *exp){ 
	if (exp[0] == '\0') {
		return NULL;
	}
	while (exp[0] == ' ') exp++;

	char *tmp;
	dictionaryll *dict = malloc(sizeof(dictionaryll));

	if (exp[0] == '(') {
		dict->data = NULL;
		dict->type = DEPTHUP;
		prev = DEPTHUP;
		dict->next = lisplexer(exp+1);
	} else if (exp[0] == ')') {
		dict->data = NULL;
		dict->type = DEPTHDOWN;
		prev = DEPTHDOWN;
		dict->next = lisplexer(exp+1);
	} else if (prev == DEPTHUP){
		dict->data = readuntil(exp, " )");
		dict->type = FUNC;
		prev = FUNC;
		dict->next = lisplexer(exp+readlen+1);
	} else if (prev == FUNC || prev == ARG || prev == DEPTHDOWN){
		dict->data = readuntil(exp, " )");
		dict->type = ARG;
		dict->next = lisplexer(exp+readlen);
	}

	return dict;
}


int main(){
	lispexec(lispparse(lisplexer("(def \"a\" (* 2 5))")));
	lispexec(lispparse(lisplexer("(def \"b\" (* a 5))")));
	lispexec(lispparse(lisplexer("(def \"c\" \"test\")")));
	lispexec(lispparse(lisplexer("(printint b)")));
	lispexec(lispparse(lisplexer("(printstr \"hello\")")));
	lispexec(lispparse(lisplexer("(printstr c)")));
}
