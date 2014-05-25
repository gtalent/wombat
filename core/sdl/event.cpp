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
#ifdef WITH_SDL
#include <SDL.h>
#include "../event.hpp"

namespace wombat {
namespace core {

Key toWombatKey(SDL_Event t) {
	switch (t.key.keysym.sym) {
	case SDLK_a:
		return Key_A;
	case SDLK_q:
		return Key_Q;
	case SDLK_ESCAPE:
		return Key_Escape;
	}
	return Key_Unknown;
}

}
}
#endif