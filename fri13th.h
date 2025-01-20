#pragma once

#include <stddef.h>
#include <stdbool.h>

#if __cplusplus
extern "C" {
#endif

enum FRI13TH_ValueType
{
	FRI13TH_TYPE_NULL,
	FRI13TH_TYPE_BOOL,
	FRI13TH_TYPE_NUMBER,
	FRI13TH_TYPE_STRING,
	FRI13TH_TYPE_ARRAY,
	FRI13TH_TYPE_OBJECT
};

typedef struct FRI13TH_Array_s FRI13TH_Array;
typedef struct FRI13TH_Object_s FRI13TH_Object;

typedef struct FRI13TH_Value
{
	enum FRI13TH_ValueType type;
	union
	{
		bool boolean;
		double number;
		char * string;
	 	FRI13TH_Array* array;
		FRI13TH_Object* object;
	};
} FRI13TH_Value;

typedef struct FRI13TH_KeyValue_s
{
	char * key;
	FRI13TH_Value* value;
} FRI13TH_KeyValue;

typedef struct FRI13TH_Object_s
{
	FRI13TH_KeyValue *key_values;
	size_t size;
	size_t capacity;
} FRI13TH_Object;

typedef struct FRI13TH_ObjectIterator_s
{
	FRI13TH_Object *object;
	size_t index;
} FRI13TH_ObjectIterator;

typedef struct FRI13TH_Array_s
{
	FRI13TH_Value** values;
	size_t size;
	size_t capacity;
} FRI13TH_Array;

typedef struct FRI13TH_ArrayIterator_s
{
	FRI13TH_Array *array;
	size_t index;
} FRI13TH_ArrayIterator;

/**
 * @brief JSON文字列をパースする
 * @param json パースするJSON文字列
 * @return 文字列からパースされたJSONオブジェクトを返す、パースに失敗した場合はNULLを返す
 */
extern FRI13TH_Value* FRI13TH_parse(char const* json);
/**
 * @brief JSONオブジェクトのイテレータを作成する
 * @param object イテレータを作成するJSONオブジェクト
 * @return JSONオブジェクトのイテレータ
 */
extern FRI13TH_ObjectIterator FRI13TH_get_object_iterator(FRI13TH_Object * object);
/**
 * @brief オブジェクトイテレータからキーバリューを取得してイテレータを進める
 * @param iterator オブジェクトイテレータ
 * @return イテレータから取得したキーバリュー、末尾に到達した場合はNULLを返す
 */
extern FRI13TH_KeyValue* FRI13TH_iterate_object(FRI13TH_ObjectIterator* iterator);
/**
 * @brief オブジェクトから指定したキーの値を取得する
 * @param object オブジェクト
 * @param key キー
 * @return キーに対応する値、キーが存在しない場合はNULLを返す
 */
extern FRI13TH_Value * FRI13TH_get_object_value(FRI13TH_Object * object, char const* key);
/**
 * @brief オブジェクトの指定したキーに対応する値を設定する。既に設定されている値がある場合はメモリ領域を解放して上書きする
 * @param object オブジェクト
 * @param key 設定先のキー(ヒープに確保された文字列を指定すること)
 * @param value 設定する値
 * @return 設定に成功した場合はtrueを返す
 */
extern bool FRI13TH_set_object_value(FRI13TH_Object * object, char * key, FRI13TH_Value * value);
/**
 * @brief JSON配列のイテレータを作成する
 * @param array イテレータを作成するJSON配列
 * @return JSON配列のイテレータ
 */
extern FRI13TH_ArrayIterator FRI13TH_get_array_iterator(FRI13TH_Array * array);
/**
 * @brief 配列イテレータから値を取得してイテレータを進める
 * @param iterator 配列イテレータ
 * @return イテレータから取得した値、末尾に到達した場合はNULLを返す
 */
extern FRI13TH_Value* FRI13TH_iterate_array(FRI13TH_ArrayIterator* iterator);
/**
 * @brief 配列から指定したインデックスの値を取得する
 * @param array 配列
 * @param index インデックス
 * @return インデックスに対応する値、インデックスが範囲外の場合はNULLを返す
 */
extern FRI13TH_Value * FRI13TH_get_array_value(FRI13TH_Array* array, size_t index);
/**
 * @brief 配列に値を追加する
 * @param array 値を追加する配列
 * @param value 追加する値
 * @return 追加に成功した場合はtrueを返す
 */
extern bool FRI13TH_append_array_value(FRI13TH_Array * array, FRI13TH_Value * value);
/**
 * @brief JSONオブジェクトを解放する
 * @param root 解放するJSONオブジェクト
 */
extern void FRI13TH_free(FRI13TH_Value* root);
/**
 * @brief JSONオブジェクトを文字列に変換する
 * @param root 変換するJSONオブジェクト
 * @param buffer 変換した文字列を格納するバッファ
 * @param size バッファのサイズ
 * @param pretty trueの場合は整形された文字列を返す、falseの場合は最小限の文字列を返す
 * @return 変換に成功した場合はtrueを返す
 */
extern bool FRI13TH_unparse(FRI13TH_Value * root, char* buffer, size_t size, bool pretty);

#if __cplusplus
}
#endif
