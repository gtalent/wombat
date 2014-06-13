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
#include "sprite.hpp"

namespace wombat {
namespace world {

using core::TaskState;

Sprite::Sprite(models::Sprite model) {
	load(model);
}

TaskState Sprite::run(core::Event) {
	return TaskState::Continue;
}

void Sprite::load(models::Sprite model) {
}

void Sprite::unload() {
}

Sprite *loadSprite(models::Sprite model) {
	switch ((models::SpriteType) model.SpriteType) {
	case models::SpriteType::Inanimate:
		break;
	case models::SpriteType::Person:
		return new Person(model);
	case models::SpriteType::Creature:
		break;
	}
	return 0;
}

}
}
