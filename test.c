#include "fri13th.h"

#include <stdio.h>
#include <stdlib.h>

#include <CUnit/CUnit.h>
#include <CUnit/Console.h>

size_t read_file(const char* filename, char** buffer)
{
	FILE* file = fopen(filename, "r");
	if(!file) {
		return 0;
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	*buffer = malloc(size + 1);
	if(!*buffer) {
		fclose(file);
		return 0;
	}

	fread(*buffer, 1, size, file);
	fclose(file);
	(*buffer)[size] = '\0';

	return size;
}

void test_000empty(void)
{
	char * buffer;
	read_file("TestCases/000empty.json", &buffer);
	FRI13TH_Value* value = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_OBJECT);
	CU_ASSERT_EQUAL(value->object->size, 0);
	free(buffer);
	FRI13TH_free(value);
}

void test_001void(void)
{
	char * buffer;
	read_file("TestCases/001void.json", &buffer);
	FRI13TH_Value* value = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NULL(value);
	free(buffer);
	FRI13TH_free(value);
}

void test_002not_object(void)
{
	char * buffer;
	read_file("TestCases/002not_object.json", &buffer);
	FRI13TH_Value* value = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NULL(value);
	free(buffer);
	FRI13TH_free(value);
}

void test_003single_object(void)
{
	char * buffer;
	read_file("TestCases/003single_object.json", &buffer);
	FRI13TH_Value* value = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_OBJECT);
	CU_ASSERT_EQUAL(value->object->size, 1);
	FRI13TH_ObjectIterator iterator = FRI13TH_get_object_iterator(value->object);
	FRI13TH_KeyValue* key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "message");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "hello");
	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NULL(key_value);
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(value, unparse_buffer, sizeof(unparse_buffer), true);
	puts(unparse_buffer);
	CU_ASSERT_TRUE(unparse_result);
	free(buffer);
	FRI13TH_free(value);
}

void test_004nested_object(void)
{
	char * buffer;
	read_file("TestCases/004nested_object.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	CU_ASSERT_EQUAL(root->object->size, 1);
	FRI13TH_Value* value = FRI13TH_get_object_value(root->object, "object");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_OBJECT);
	CU_ASSERT_EQUAL(value->object->size, 2);
	FRI13TH_Value *name = FRI13TH_get_object_value(value->object, "name");
	FRI13TH_Value *age = FRI13TH_get_object_value(value->object, "age");
	CU_ASSERT_PTR_NOT_NULL(name);
	CU_ASSERT_PTR_NOT_NULL(age);
	CU_ASSERT_EQUAL(name->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(name->string, "Takeshi");
	CU_ASSERT_EQUAL(age->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(age->number, 42);
	free(buffer);
	FRI13TH_free(root);
}

void test_005array(void)
{
	char * buffer;
	read_file("TestCases/005array.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	FRI13TH_Value* value = FRI13TH_get_object_value(root->object, "array");
	FRI13TH_Array* array = value->array;
	FRI13TH_ArrayIterator iterator = FRI13TH_get_array_iterator(array);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 5);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 8);
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(root, unparse_buffer, sizeof(unparse_buffer), true);
	puts(unparse_buffer);
	CU_ASSERT_TRUE(unparse_result);	
	free(buffer);
	FRI13TH_free(root);
}

void test_006nested_array(void)
{
	char * buffer;
	read_file("TestCases/006nested_array.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	FRI13TH_Value* value = FRI13TH_get_object_value(root->object, "array");
	FRI13TH_Array* array = value->array;
	FRI13TH_ArrayIterator iterator = FRI13TH_get_array_iterator(array);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_ARRAY);
	FRI13TH_Array* nested_array = value->array;
	FRI13TH_ArrayIterator nested_iterator = FRI13TH_get_array_iterator(nested_array);
	value = FRI13TH_iterate_array(&nested_iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1);
	value = FRI13TH_iterate_array(&nested_iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_ARRAY);
	nested_array = value->array;
	nested_iterator = FRI13TH_get_array_iterator(nested_array);
	value = FRI13TH_iterate_array(&nested_iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2);
	value = FRI13TH_iterate_array(&nested_iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, -3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_ARRAY);
	nested_array = value->array;
	nested_iterator = FRI13TH_get_array_iterator(nested_array);
	value = FRI13TH_iterate_array(&nested_iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, -5);
	value = FRI13TH_iterate_array(&nested_iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 8);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NULL(value);
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(root, unparse_buffer, sizeof(unparse_buffer), true);
	CU_ASSERT_TRUE(unparse_result);
	puts(unparse_buffer);
	free(buffer);
	FRI13TH_free(root);	
}

static void test_007escaped_string(void)
{
	char * buffer;
	read_file("TestCases/007escaped_string.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	FRI13TH_Value* value = FRI13TH_get_object_value(root->object, "string");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(value->string,  "A\n\r\t\b\f\\\"夜露死苦");
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(root, unparse_buffer, sizeof(unparse_buffer), true);
	CU_ASSERT_TRUE(unparse_result);
	puts(unparse_buffer);
	free(buffer);
	FRI13TH_free(root);
}

static void test_008immediate(void)
{
	char * buffer;
	read_file("TestCases/008immediate.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	FRI13TH_Value* value = FRI13TH_get_object_value(root->object, "true");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_BOOL);
	CU_ASSERT_TRUE(value->boolean);
	value = FRI13TH_get_object_value(root->object, "false");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_BOOL);
	CU_ASSERT_FALSE(value->boolean);
	value = FRI13TH_get_object_value(root->object, "null");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NULL);
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(root, unparse_buffer, sizeof(unparse_buffer), true);
	CU_ASSERT_TRUE(unparse_result);
	puts(unparse_buffer);
	free(buffer);
	FRI13TH_free(root);	
}

static void test_009long_string(void)
{
	char * buffer;
	read_file("TestCases/009long_string.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	FRI13TH_Value* value = FRI13TH_get_object_value(root->object, "string");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(value->string, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+{}|:\"<>?`-=[]\\;',./~");
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(root, unparse_buffer, sizeof(unparse_buffer), true);
	CU_ASSERT_TRUE(unparse_result);
	puts(unparse_buffer);
	free(buffer);
	FRI13TH_free(root);
}

static void test_010many_keys(void)
{
	char* buffer;
	read_file("TestCases/010many_keys.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	FRI13TH_ObjectIterator iterator = FRI13TH_get_object_iterator(root->object);
	FRI13TH_KeyValue* key_value;
	key_value = FRI13TH_iterate_object(&iterator);

	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "English");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hello");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "French");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Bonjour");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Spanish");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hola");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "German");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hallo");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Japanese");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "こんにちは");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Chinese");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "你好");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Korean");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "안녕하세요");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Russian");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Здравствуйте");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Arabic");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "مرحبا");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Hindi");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "नमस्ते");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Italian");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Ciao");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Portuguese");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Olá");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Dutch");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hallo");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Thai");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "สวัสดี");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Vietnamese");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Xin chào");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Indonesian");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Halo");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Turkish");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Merhaba");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Polish");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Cześć");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Swedish");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hej");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Danish");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hej");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Norwegian");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hei");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Finnish");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hei");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Greek");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Γειά σας");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Swahili");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Hujambo");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Hebrew");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "שלום");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Hungarian");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Helló");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Czech");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Ahoj");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Slovak");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Ahoj");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Romanian");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Salut");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Bulgarian");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Здравейте");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Ukrainian");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Привіт");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Latin");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Salve");

	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NOT_NULL(key_value);
	CU_ASSERT_STRING_EQUAL(key_value->key, "Esperanto");
	CU_ASSERT_EQUAL(key_value->value->type, FRI13TH_TYPE_STRING);
	CU_ASSERT_STRING_EQUAL(key_value->value->string, "Saluton");
	key_value = FRI13TH_iterate_object(&iterator);
	CU_ASSERT_PTR_NULL(key_value);
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(root, unparse_buffer, sizeof(unparse_buffer), true);
	CU_ASSERT_TRUE(unparse_result);
	puts(unparse_buffer);
	free(buffer);
	FRI13TH_free(root);
}

void test_011long_array(void)
{
	char* buffer;
	read_file("TestCases/011long_array.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	FRI13TH_Value* value = FRI13TH_get_object_value(root->object, "array");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_ARRAY);
	FRI13TH_Array* array = value->array;
	FRI13TH_ArrayIterator iterator = FRI13TH_get_array_iterator(array);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 4);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 5);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 9);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 6);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 5);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 5);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 8);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 9);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 7);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 9);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 8);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 4);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 6);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 6);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 4);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 8);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 7);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 9);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 5);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 0);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 8);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 8);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 4);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 9);
	value = FRI13TH_iterate_array(&iterator);
	CU_ASSERT_PTR_NULL(value);
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(root, unparse_buffer, sizeof(unparse_buffer), true);
	CU_ASSERT_TRUE(unparse_result);
	puts(unparse_buffer);
	free(buffer);
	FRI13TH_free(root);
}

void test_012prcision(void)
{
	char* buffer;
	read_file("TestCases/012precision.json", &buffer);
	FRI13TH_Value* root = FRI13TH_parse(buffer);
	CU_ASSERT_PTR_NOT_NULL(root);
	CU_ASSERT_EQUAL(root->type, FRI13TH_TYPE_OBJECT);
	FRI13TH_Value* value = FRI13TH_get_object_value(root->object, "pi");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 3.141592653589793);
	value = FRI13TH_get_object_value(root->object, "double_max");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1.7976931348623157e+308);
	value = FRI13TH_get_object_value(root->object, "double_min");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2.2250738585072014e-308);
	value = FRI13TH_get_object_value(root->object, "double_epsilon");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 2.220446049250313e-16);
	value = FRI13TH_get_object_value(root->object, "over_precision");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, 1.2345678901234567);
	value = FRI13TH_get_object_value(root->object, "negative_zero");
	CU_ASSERT_PTR_NOT_NULL(value);
	CU_ASSERT_EQUAL(value->type, FRI13TH_TYPE_NUMBER);
	CU_ASSERT_EQUAL(value->number, -0.0);
	char unparse_buffer[1024] = {};
	bool unparse_result = FRI13TH_unparse(root, unparse_buffer, sizeof(unparse_buffer), true);
	CU_ASSERT_TRUE(unparse_result);
	puts(unparse_buffer);
	free(buffer);
	FRI13TH_free(root);
}

int main(int /* argc */, char** /* argv */)
{
	CU_pSuite suite = NULL;

	if(CUE_SUCCESS != CU_initialize_registry()) {
		return CU_get_error();
	}

	suite = CU_add_suite("fri13th", NULL, NULL);
	if(NULL == suite) {
		CU_cleanup_registry();
		return CU_get_error();
	}
	CU_ADD_TEST(suite, test_000empty);
	CU_ADD_TEST(suite, test_001void);
	CU_ADD_TEST(suite, test_002not_object);
	CU_ADD_TEST(suite, test_003single_object);
	CU_ADD_TEST(suite, test_004nested_object);
	CU_ADD_TEST(suite, test_005array);
	CU_ADD_TEST(suite, test_006nested_array);
	CU_ADD_TEST(suite, test_007escaped_string);
	CU_ADD_TEST(suite, test_008immediate);
	CU_ADD_TEST(suite, test_009long_string);
	CU_ADD_TEST(suite, test_010many_keys);
	CU_ADD_TEST(suite, test_011long_array);
	CU_ADD_TEST(suite, test_012prcision);

	CU_console_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}
