/*
 * Copyright 2013-2015 gtalent2@gmail.com
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef WOMBAT_MODELS_JSON_HPP
#define WOMBAT_MODELS_JSON_HPP

#include <iostream>
#include <sstream>
#include <jansson.h>
#include "types.hpp"
#include "ptr.hpp"
#include "unknown.hpp"

namespace wombat {
namespace models {

Error parseJson(string json, json_t **out);

Error parseJsonFile(string path, json_t **out);

Error readVal(json_t *jv, int *v);

Error readVal(json_t *jv, double *v);

Error readVal(json_t *jv, bool *v);

Error readVal(json_t *jv, string *v);

Error readVal(json_t *jv, ptr *v);

Error readVal(json_t *jv, unknown *v);

template<typename T>
Error readVal(json_t *jv, std::vector<T> *v);

template<typename Key, typename Value>
Error readVal(json_t *jv, std::map<Key, Value> *v);

template<typename Model>
Error readVal(json_t *jv, Model *v);

template<typename Model>
Error fromJson(Model *model, string json);

template<typename T>
Error readVal(json_t *jo, const char *field, T *v);


// template definitions

template<typename T>
Error readVal(json_t *jv, std::vector<T> *v) {
	auto err = Error::Ok;
	if (json_is_array(jv)) {
		auto &list = *v;
		size_t i;
		json_t *value;
		list.resize(json_array_size(jv));
		json_array_foreach(jv, i, value) {
			err |= readVal(value, &list[i]);
		}
	} else {
		err = Error::TypeMismatch;
	}
	return err;
}

template<typename Key, typename Value>
Error readVal(json_t *jv, std::map<Key, Value> *v) {
	auto err = Error::Ok;
	if (json_is_object(jv)) {
		auto &list = *v;
		const char *i;
		json_t *value;
		json_object_foreach(jv, i, value) {
			std::stringstream conv;
			Key key;
			conv << i;
			conv >> key;
			err |= readVal(value, &list[key]);
		}
	} else {
		err = Error::TypeMismatch;
	}
	return err;
}

template<typename Model>
Error readVal(json_t *jv, Model *v) {
	if (json_is_object(jv)) {
		return fromJson(v, jv);
	}
	return Error::TypeMismatch;
}

template<typename Model>
Error fromJson(Model *model, string json) {
	json_t *jv;
	auto err = parseJson(json, &jv);
	err |= fromJson(model, jv);
	json_decref(jv);
	return err;
}

template<typename Model>
Error readJsonFile(Model *model, string path) {
	json_t *jv;
	auto err = parseJsonFile(path, &jv);
	err |= fromJson(model, jv);
	json_decref(jv);
	return err;
}

template<typename T>
Error readVal(json_t *jo, const char *field, T *v) {
	auto jv = json_object_get(jo, field);
	if (jv) {
		return readVal(jv, v);
	} else {
		return Error::MissingField;
	}
}

}
}

#endif
