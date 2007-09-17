#include "basictype.h"


bool_t isWide(BType_t type) {
	return (type == T_LONG || type == T_DOUBLE);
}

bool_t isPrimitive(BType_t type) {
	return getChar(type) == 'L';
}

bool_t isArray(BType_t type) {
	return getChar(type) == '[';
}

char getChar(BType_t type) {
	switch (type) {
		case T_BOOLEAN: return 'Z';
		case T_BYTE: return 'B';
		case T_CHAR: return 'C';
		case T_DOUBLE: return 'D';
		case T_FLOAT: return 'F';
		case T_INT: return 'I';
		case T_LONG: return 'J';
		case T_OBJECT: return 'L';
		case T_SHORT: return 'S';
		default: return '\0';
	}
}

int    getSize(BType_t type) {
	switch (type) {
		case T_BOOLEAN: 
		case T_BYTE: 
			return 1;
			
		case T_CHAR:
		case T_SHORT:
			return 2;
			
		case T_FLOAT:
		case T_INT:
		case T_OBJECT:
			return 4;
		
		case T_DOUBLE:
		case T_LONG:
			return 8;
		
		default: return 0;
	}
}
