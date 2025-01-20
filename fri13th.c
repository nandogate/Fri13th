#include "fri13th.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char const* parse_value(char const* s, FRI13TH_Value** out_value);
static char const* parse_object(char const* s, FRI13TH_Object** out_object);

#ifdef DEBUG_OUTPUT
#define DEBUG_PRINT_CUR printf( "FRI13TH L.%d :  %p : %02x\n", __LINE__, p , *p)
#define DEBUG_PRINTF(...) printf( "FRI13TH L.%d : ", __LINE__); printf(__VA_ARGS__)
#else
#define DEBUG_PRINT_CUR
#define DEBUG_PRINTF(...)
#endif

static char const* skip_space(char const* p)
{
	while(*p != '\0' && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' )) ++p;
	return p;
}

static bool get_escaped(char source_char, char* out)
{
	switch(source_char)
	{
	case '"':
		*out = '\"';
		return true;
	case '\\':
		*out = '\\';
		return true;
	case '/':
		*out = '/';
		return true;
	case 'b':
		*out = '\b';
		return true;
	case 'f':
		*out = '\f';
		return true;
	case 'n':
		*out = '\n';
		return true;
	case 'r':
		*out = '\r';
		return true;
	case 't':
		*out = '\t';
		return true;
	default:
		*out = '\0';
		return false;
	}
}

static size_t append_string(char** out, size_t out_len, char const* in, size_t in_len, size_t *out_capacity)
{
	size_t new_len = out_len + in_len;
	while(new_len > *out_capacity) {
		*out_capacity = new_len * 2;
		*out = realloc(*out, *out_capacity);
		if(!*out) return 0;
	}
	memcpy(*out + out_len, in, in_len);
	return new_len;
}

static size_t append_char(char** out, size_t out_len, char in, size_t *out_capacity)
{
	size_t new_len = out_len + 1;
	if(new_len > *out_capacity) {
		*out_capacity = new_len * 2;
		*out = realloc(*out, *out_capacity);
		if(!*out) return 0;
	}
	(*out)[out_len] = in;
	return new_len;
}

static size_t convert_and_append_utf8(int codepoint, char **out, size_t out_len, size_t *out_capacity)
{
	char utf8[4] = {};
	size_t utf8_len = 0;
	if(codepoint <= 0x7F) {
		utf8[0] = codepoint;
		utf8_len = 1;
	} else if(codepoint <= 0x7FF) {
		utf8[0] = 0xC0 | (codepoint >> 6);
		utf8[1] = 0x80 | (codepoint & 0x3F);
		utf8_len = 2;
	} else if(codepoint <= 0xFFFF) {
		utf8[0] = 0xE0 | (codepoint >> 12);
		utf8[1] = 0x80 | ((codepoint >> 6) & 0x3F);
		utf8[2] = 0x80 | (codepoint & 0x3F);
		utf8_len = 3;
	} else if(codepoint <= 0x10FFFF) {
		utf8[0] = 0xF0 | (codepoint >> 18);
		utf8[1] = 0x80 | ((codepoint >> 12) & 0x3F);
		utf8[2] = 0x80 | ((codepoint >> 6) & 0x3F);
		utf8[3] = 0x80 | (codepoint & 0x3F);
		utf8_len = 4;
	} else {
		return 0;
	}
	return append_string(out, out_len, utf8, utf8_len, out_capacity);
}

static char const*  parse_string(char const* s, char ** out_string)
{
	enum Mode
	{
		NORMAL,
		ESCAPE,
		CODEPOINT
	} mode = NORMAL;
	char const* p = s;

	DEBUG_PRINT_CUR;

	int current_codepoint = 0;
	int digit_count = 0;

	size_t total_out = 0;
	size_t out_buf_size = 64;
	char* out_buf = NULL;
	
	if(*p != '"') goto FAILURE;

	++p;
	out_buf =  malloc(sizeof(char) * out_buf_size);
	if(!out_buf) goto FAILURE;

	while(*p != '\0')
	{
		DEBUG_PRINT_CUR;
		switch(mode)
		{
		case NORMAL:
			if(*p == '\\') {
				mode = ESCAPE;
			} else if(*p == '"') {
				total_out = append_char(&out_buf, total_out, '\0', &out_buf_size);
				if(!total_out) goto FAILURE;
				*out_string = out_buf;
				return p + 1;
			} else {
				total_out = append_char(&out_buf, total_out, *p, &out_buf_size);
				if(!total_out) goto FAILURE;
			}
			break;
		case ESCAPE:
			if(*p == 'u') {
				mode = CODEPOINT;
			} else {
				char escaped;
				if(!get_escaped(*p, &escaped)) goto FAILURE;
				total_out = append_char(&out_buf, total_out, escaped, &out_buf_size);
				if(!total_out) goto FAILURE;
				mode = NORMAL;
			}
			break;
		case CODEPOINT:
			if('0' <= *p && *p <= '9') current_codepoint = current_codepoint * 16 + (*p - '0');
			else if('A' <= *p && *p <= 'F') current_codepoint = current_codepoint * 16 + (*p - 'A' + 10);
			else if('a' <= *p && *p <= 'f') current_codepoint = current_codepoint * 16 + (*p - 'a' + 10);
			else goto FAILURE;
			++digit_count;
			if(digit_count == 4)
			{
				total_out = convert_and_append_utf8(current_codepoint, &out_buf, total_out, &out_buf_size);
				if(!total_out) goto FAILURE;
				mode = NORMAL;
				digit_count = 0;
				current_codepoint = 0;
			}
			break;
		}
		++p;
	}
FAILURE:
	free(out_buf);
	*out_string = NULL;
	return NULL;
}

static char const* parse_number(char const* s, double* out)
{
	if(sscanf(s, "%lf", out) != 1) return NULL;
	return s + strspn(s, "0123456789+-.eE") ;
}

static const char* parse_immediate(char const* s, enum FRI13TH_ValueType *type, bool *boolean_value)
{
	if(strncmp(s, "true", 4) == 0) {
		*type = FRI13TH_TYPE_BOOL;
		*boolean_value = true;
		return s + 4;
	}
	if(strncmp(s, "false", 5) == 0) {
		*type = FRI13TH_TYPE_BOOL;
		*boolean_value = false;
		return s + 5;
	}
	if(strncmp(s, "null", 4) == 0) {
		*type = FRI13TH_TYPE_NULL;
		*boolean_value = false;
		return s + 4;
	}
	return NULL;
}

static char const* parse_array(char const* s, FRI13TH_Array** out_array)
{
	char const* p = s;
	FRI13TH_Array* this_array;
	FRI13TH_Value* value;
	DEBUG_PRINT_CUR;
	if(*p != '[') return NULL;
	++p;
	DEBUG_PRINT_CUR;

	p = skip_space(p);
	DEBUG_PRINT_CUR;

	if(*p == '\0') return NULL;

	this_array = malloc(sizeof(FRI13TH_Array));
	if(!this_array) return NULL;

	this_array->size = 0;
	this_array->capacity = 16;
	this_array->values = malloc(sizeof(FRI13TH_Value*) * 16);

	if(!this_array->values) {
		free(this_array);
		return NULL;
	}

	bool first = true;
	bool found_comma = false;

	while(*p != '\0')
	{
		DEBUG_PRINT_CUR;
		switch(*p)
		{
		case ']':
			if(!first && found_comma) goto FAILURE;
			*out_array = this_array;
			return p + 1;
		case ',':
			if(first || found_comma) goto FAILURE;
			++p;
			p = skip_space(p);
			DEBUG_PRINT_CUR;
			found_comma = true;
			break;
		default:
			if(!first && !found_comma) goto FAILURE;
			p = parse_value(p, &value);
			if(!p) goto FAILURE;
			DEBUG_PRINT_CUR;
			p = skip_space(p);
			DEBUG_PRINT_CUR;
			FRI13TH_append_array_value(this_array, value);
			found_comma = false;
			break;
		}
		first = false;
	}

FAILURE:
	for(size_t i = 0; i < this_array->size; ++i)
	{
		FRI13TH_free(this_array->values[i]);
	}
	free(this_array->values);
	free(this_array);
	*out_array = NULL;
	return NULL;
}

static char const* parse_value(char const* s, FRI13TH_Value **out_value)
{
	const char* p = s;
	FRI13TH_Value* this_value;
	char* string_value;
	FRI13TH_Array* array_value;
	FRI13TH_Object* object_value;

	DEBUG_PRINT_CUR;
	p = skip_space(p);

	if(*p == '\0') {
		*out_value = NULL;
		return NULL;
	}

	this_value = malloc(sizeof(FRI13TH_Value));
	if(!this_value) return NULL;

	DEBUG_PRINT_CUR;
	if(*p == '"') {
		p = parse_string(p, &string_value);
		if(!p) return NULL;
		DEBUG_PRINT_CUR;
		this_value->type = FRI13TH_TYPE_STRING;
		this_value->string = string_value;
	}else if(*p == '[') {
		p = parse_array(p, &array_value);
		if(!p) goto FAILURE;
		DEBUG_PRINT_CUR;
		this_value->type = FRI13TH_TYPE_ARRAY;
		this_value->array = array_value;
	}else if(*p == '{') {
		p = parse_object(p, &object_value);
		if(!p) goto FAILURE;
		DEBUG_PRINT_CUR;
		this_value->type = FRI13TH_TYPE_OBJECT;
		this_value->object = object_value;
	}else if(*p == 't' || *p == 'f' || *p == 'n') {
		enum FRI13TH_ValueType type;
		bool boolean_value;
		p = parse_immediate(p, &type, &boolean_value);
		if(!p) return NULL;
		DEBUG_PRINT_CUR;
		this_value->type = type;
		this_value->boolean = boolean_value;
	}else if(('0' <= *p && *p <= '9') || *p == '-') {
		double number;
		p = parse_number(p, &number);
		DEBUG_PRINTF("number %f\n", number);
		if(!p) return NULL;
		DEBUG_PRINT_CUR;
		this_value->type = FRI13TH_TYPE_NUMBER;
		this_value->number = number;
	}
	p = skip_space(p);
	DEBUG_PRINT_CUR;
	*out_value = this_value;
	return p;

FAILURE:
	free(this_value);
	*out_value = NULL;
	return NULL;
}

char const*  parse_object(char const* json, FRI13TH_Object **out_object)
{
	const char* p = json;
	FRI13TH_Object* this_obj = NULL;
	char* key = NULL;
	FRI13TH_Value* value = NULL;

	DEBUG_PRINT_CUR;
	if(*p != '{') return NULL;
	++p;

	DEBUG_PRINT_CUR;
	p = skip_space(p);

	if(*p == '\0') return NULL;
	DEBUG_PRINT_CUR;

	this_obj = malloc(sizeof(FRI13TH_Object));
	if(!this_obj) return NULL;

	this_obj->size = 0;
	this_obj->capacity = 16; 
	this_obj->key_values = malloc(sizeof(FRI13TH_KeyValue) * 16);
	if(!this_obj->key_values) {
		free(this_obj);
		return NULL;
	}
	bool found_comma = false;
	bool first = true;

	while(*p != '\0')
	{
		switch(*p)
		{
		case '}':
			if(!first && found_comma) goto FAILURE;

			*out_object = this_obj;
			return p + 1;
		case '"':
			if(!first && !found_comma) goto FAILURE;

			p = parse_string(p, &key);
			if(!p) goto FAILURE;
			DEBUG_PRINT_CUR;
			
			p = skip_space(p);
			DEBUG_PRINT_CUR;
			
			if(*p != ':') goto FAILURE;
			++p;
			DEBUG_PRINT_CUR;
			
			p = parse_value(p, &value);
			if(!p) goto FAILURE;
			DEBUG_PRINT_CUR;
			
			FRI13TH_set_object_value(this_obj, key, value);
			p = skip_space(p);
			DEBUG_PRINT_CUR;
			if(*p == ',')  {
				found_comma = true;
				p = skip_space(p + 1);
			} else {
				found_comma = false;
			}
			DEBUG_PRINT_CUR;
			break;
		}
		first = false;
	}
FAILURE:
	for(size_t i = 0; i < this_obj->size; ++i)
	{
		free(this_obj->key_values[i].key);
		FRI13TH_free(this_obj->key_values[i].value);
	}
	free(this_obj->key_values);
	free(this_obj);
	free(key);
	free(value);
	return NULL;
}

static FRI13TH_KeyValue* get_object_key_value(FRI13TH_Object* object, char const* key)
{
	for(size_t i = 0; i < object->size; ++i) {
		DEBUG_PRINTF("i : %ld\n", i);
		DEBUG_PRINTF("key : %s, %s\n", key, object->key_values[i].key);
		if(strcmp(object->key_values[i].key, key) == 0) return &object->key_values[i];
	}
	return NULL;
}

bool unparse(FRI13TH_Value* root, char* buffer, size_t size, bool pretty, int depth)
{
	FRI13TH_Value* value = root;
	FRI13TH_ObjectIterator object_iterator;
	FRI13TH_ArrayIterator array_iterator;

	size_t out_count = 0;
	switch(value->type)
	{
	case FRI13TH_TYPE_STRING:
		if(out_count + strlen(value->string) + 2 >= size) return false;
		buffer[out_count++] = '"';
		for(size_t i = 0; i < strlen(value->string); ++i)
		{
			switch(value->string[i])
			{
			case '"':
				buffer[out_count++] = '\\';
				buffer[out_count++] = '"';
				break;
			case '\\':
				buffer[out_count++] = '\\';
				buffer[out_count++] = '\\';
				break;
			case '/':
				buffer[out_count++] = '\\';
				buffer[out_count++] = '/';
				break;
			case '\b':
				buffer[out_count++] = '\\';
				buffer[out_count++] = 'b';
				break;
			case '\f':
				buffer[out_count++] = '\\';
				buffer[out_count++] = 'f';
				break;
			case '\n':
				buffer[out_count++] = '\\';
				buffer[out_count++] = 'n';
				break;
			case '\r':
				buffer[out_count++] = '\\';
				buffer[out_count++] = 'r';
				break;
			case '\t':
				buffer[out_count++] = '\\';
				buffer[out_count++] = 't';
				break;
			default:
				buffer[out_count++] = value->string[i];
				break;
			}
		}
		buffer[out_count++] = '"';
		break;
	case FRI13TH_TYPE_NUMBER:
		if(out_count + 64 >= size) return false;
		sprintf(buffer + out_count, "%.16g", value->number);
		out_count += strlen(buffer + out_count);
		break;
	case FRI13TH_TYPE_BOOL:
		if(out_count + 6 >= size) return false;
		if(value->boolean) {
			strcpy(buffer + out_count, "true");
			out_count += 4;
		} else {
			strcpy(buffer + out_count, "false");
			out_count += 5;
		}
		break;
	case FRI13TH_TYPE_NULL:
		if(out_count + 6 >= size) return false;
		strcpy(buffer + out_count, "null");
		out_count += 4;
		break;
	case FRI13TH_TYPE_ARRAY:
		if(out_count + 1 >= size) return false;
		buffer[out_count++] = '[';
		array_iterator = FRI13TH_get_array_iterator(value->array);
		FRI13TH_Value* next_child = FRI13TH_iterate_array(&array_iterator);
		depth++;
		while(true)
		{
			FRI13TH_Value* child = next_child;
			if(pretty)
			{
				if(out_count + 1 >= size) return false;
				buffer[out_count++] = '\n';
				for(int i = 0; i < depth; ++i)
				{
					if(out_count + 1 >= size) return false;
					buffer[out_count++] = '\t';
				}
			}
			if(!unparse(child, buffer + out_count, size - out_count, pretty, depth)) return false;
			out_count += strlen(buffer + out_count);
			next_child = FRI13TH_iterate_array(&array_iterator);
			if(!next_child) {
				break;
			}
			if(out_count + 1 >= size) return false;
			buffer[out_count++] = ',';
		}
		depth--;
		if(pretty) {
			if(out_count + 1 >= size) return false;
			buffer[out_count++] = '\n';
			for(int i = 0; i < depth ; ++i)
			{
				if(out_count + 1 >= size) return false;
				buffer[out_count++] = '\t';
			}
		}
		if(out_count + 1 >= size) return false;
		buffer[out_count++] = ']';
		break;
	case FRI13TH_TYPE_OBJECT:
		if(out_count + 1 >= size) return false;
		buffer[out_count++] = '{';
		object_iterator = FRI13TH_get_object_iterator(value->object);
		FRI13TH_KeyValue* next_key_value = FRI13TH_iterate_object(&object_iterator);
		depth++;
		while(true)
		{
			FRI13TH_KeyValue* key_value = next_key_value;
			if(pretty)
			{
				if(out_count + 1 >= size) return false;
				buffer[out_count++] = '\n';
				for(int i = 0; i < depth; ++i)
				{
					if(out_count + 1 >= size) return false;
					buffer[out_count++] = '\t';
				}
			}
			snprintf(buffer + out_count, size - out_count, "\"%s\":", key_value->key);
			out_count += strlen(buffer + out_count);
			if(out_count + 1 >= size) return false;
			if(!unparse(key_value->value, buffer + out_count, size - out_count, pretty, depth)) return false;
			out_count += strlen(buffer + out_count);
			next_key_value = FRI13TH_iterate_object(&object_iterator);
			if(!next_key_value) {
				break;
			}
			if(out_count + 1 >= size) return false;
			buffer[out_count++] = ',';
		}
		depth--;

		if(pretty) {
			buffer[out_count++] = '\n';
			for(int i = 0; i < depth - 1; ++i)
			{
				if(out_count + 1 >= size) return false;
				buffer[out_count++] = '\t';
			}
		}
	
		if(out_count + 1 >= size) return false;
		buffer[out_count++] = '}';
		break;
	}
	return true;
}

FRI13TH_Value* FRI13TH_parse(char const* json)
{
	FRI13TH_Value *value;
	if(parse_value(json, &value) && value->type != FRI13TH_TYPE_OBJECT) {
		FRI13TH_free(value);
		return NULL;
	}
	return value;
}

FRI13TH_ObjectIterator FRI13TH_get_object_iterator(FRI13TH_Object* object)
{
	FRI13TH_ObjectIterator iterator;
	iterator.object = object;
	iterator.index = 0;
	return iterator;
}

FRI13TH_KeyValue* FRI13TH_iterate_object(FRI13TH_ObjectIterator* iterator)
{
	if(iterator->index == iterator->object->size) return NULL;
	return &iterator->object->key_values[iterator->index++];
}

FRI13TH_Value* FRI13TH_get_object_value(FRI13TH_Object* object, char const* key)
{
	FRI13TH_KeyValue* key_value = get_object_key_value(object, key);
	if(key_value) {
		return key_value->value;
	}
	return NULL;
}

bool FRI13TH_set_object_value(FRI13TH_Object * object, char * key, FRI13TH_Value * value)
{
	DEBUG_PRINTF("key : %s, value : %p\n", key, value);

	FRI13TH_KeyValue* key_value = get_object_key_value(object, key);

	if(key_value) {
		DEBUG_PRINTF("found and replace.\n");
		FRI13TH_free(key_value->value);
		key_value->value = value;
		return true;
	}

	if(object->size == object->capacity) {
		object->capacity *= 2;
		object->key_values = realloc(object->key_values, sizeof(FRI13TH_KeyValue) * object->capacity);
		DEBUG_PRINTF("realloc %ld\n", object->capacity);
		if(!object->key_values) return false;
	}

	DEBUG_PRINTF("not found and append. index: %ld\n", object->size);
	object->key_values[object->size].key = key;
	object->key_values[object->size].value = value;
	++object->size;
	return true;
}
FRI13TH_ArrayIterator FRI13TH_get_array_iterator(FRI13TH_Array* array)
{
	FRI13TH_ArrayIterator iterator;
	iterator.array = array;
	iterator.index = 0;
	return iterator;
}

FRI13TH_Value* FRI13TH_iterate_array(FRI13TH_ArrayIterator* iterator)
{
	if(iterator->index == iterator->array->size) return NULL;
	return iterator->array->values[iterator->index++];
}

FRI13TH_Value* FRI13TH_get_array_value(FRI13TH_Array* array, size_t index)
{
	if(index >= array->size) return NULL;
	return array->values[index];
}

bool FRI13TH_append_array_value(FRI13TH_Array* array, FRI13TH_Value* value)
{
	if(array->size == array->capacity) {
		array->capacity *= 2;
		array->values = realloc(array->values, sizeof(FRI13TH_Value*) * array->capacity);
		if(!array->values) return false;
	}
	array->values[array->size++] = value;
	return true;
}

void FRI13TH_free(FRI13TH_Value* root)
{
	FRI13TH_Value* value = root;
	FRI13TH_ObjectIterator object_iterator;
	FRI13TH_ArrayIterator array_iterator;

	if(!value) return;

	switch(value->type)
	{
	case FRI13TH_TYPE_STRING:
		free(value->string);
		break;
	case FRI13TH_TYPE_ARRAY:
		array_iterator = FRI13TH_get_array_iterator(value->array);
		while(true)
		{
			FRI13TH_Value* child = FRI13TH_iterate_array(&array_iterator);
			if(!child) break;
			FRI13TH_free(child);
		}
		free(value->array->values);
		free(value->array);
		break;
	case FRI13TH_TYPE_OBJECT:
		object_iterator = FRI13TH_get_object_iterator(value->object);
		while(true)
		{
			FRI13TH_KeyValue* child = FRI13TH_iterate_object(&object_iterator);
			if(!child) break;
			FRI13TH_free(child->value);
			free(child->key);
		}
		free(value->object->key_values);
		free(value->object);
		break;
	default:
		break;
	}
	free(value);
}

bool FRI13TH_unparse(FRI13TH_Value* root, char* buffer, size_t size, bool pretty)
{
	return unparse(root, buffer, size, pretty, 0);
}
