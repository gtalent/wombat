/*
 * Copyright 2013-2014 gtalent2@gmail.com
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "person.hpp"

namespace wombat {
namespace world {

Person::Person() {
	m_class = 0;
}

Person::Person(std::string path) {
	models::Person model;
	core::read(model, path);
	m_class = PersonClass::checkout(model.PersonClass);
}

Person::~Person() {
	PersonClass::checkin(m_class);
}

void Person::draw(core::Graphics &gfx, common::Point pt) {
}

}
}