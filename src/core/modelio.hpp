/*
 * Copyright 2013-2014 gtalent2@gmail.com
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef WOMBAT_CORE_MODELIO_HPP
#define WOMBAT_CORE_MODELIO_HPP

#include <map>
#include <string>
#include <functional>
#include "sync.hpp"
#include "models/models.hpp"

namespace wombat {
namespace core {

typedef std::string Path;
const Path NullPath = "";

/**
 * Prepends to the path to load models from.
 * Unless another path is prepended to the path, this will be the first
 * directory that files are looked for in.
 * @param path the new path
 */
void prependPath(std::string h);

/**
 * Adds to the path to load models from.
 * This will be the last directory the files are looked for in.
 * @param path the new path
 */
void appendPath(std::string path);

/**
 * Returns the given path with the wombat_home path prepended to it.
 * @param path the of the file to refer to within wombat_home
 * @return the given path with the wombat_home path prepended to it.
 */
std::string path(Path path);

/**
 * Reads the file at the given path within the path into the given model.
 * @param model the model to load the file into
 * @prarm path the path within the path to read from
 */
models::cyborgbear::Error read(models::cyborgbear::Model &model, Path path);

/**
 * Manages Model IO, preventing redundancies in memory.
 */
template <class Model>
class ModelFlyweight {
	public:
		class Value {
			friend class ModelFlyweight;
			protected:
				int dependents;
			public:
				virtual ~Value() {};
				virtual std::string key() = 0;
		};
		class GenericValue: public Value {
			friend class ModelFlyweight;
			protected:
				std::string m_key;
			public:
				virtual ~GenericValue() {};
				virtual std::string key() {
					return m_key;
				};
		};
		typedef std::function<Value*(Model&)> FlyweightNodeBuilder;

	private:
		std::map<std::string, Value*> m_cache;
		FlyweightNodeBuilder m_build;
		core::Mutex m_lock;

	public:
		ModelFlyweight(FlyweightNodeBuilder build) {
			m_build = build;
		}

		/**
		 * Takes a Model's path as the key.
		 * @param modelPath the path to the model
		 * @return the value associated with the given key
		 */
		Value* checkout(Path modelPath) {
			m_lock.lock();
			Value *v = m_cache[modelPath];
			if (!v) {
				Model key;
				read(key, modelPath);
				m_cache[modelPath] = v = m_build(key);
			}
			if (v) {	
				v->dependents++;
			}
			m_lock.unlock();
			return v;
		}

		Value* checkout(Model &key) {
			m_lock.lock();
			std::string keyStr = key.toJson();
			Value *v = m_cache[keyStr];
			if (!v) {
				m_cache[keyStr] = v = m_build(key);
			}
			if (v) {	
				v->dependents++;
			}
			m_lock.unlock();
			return v;
		}

		void checkin(Path key) {
			m_lock.lock();
			Value *v = m_cache[key];
			checkin(v);
			m_lock.unlock();
		}

		void checkin(Value *v) {
			if (v) {
				m_lock.lock();
				v->dependents--;
				if (!v->dependents) {
					m_cache.erase(v->key());
					delete v;
				}
				m_lock.unlock();
			}
		}
};

}
}

#endif
