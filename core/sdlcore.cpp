/*
 * Copyright 2013 gtalent2@gmail.com
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
#include <vector>

#include <SDL.h>

#include "sdlglobs.hpp"
#include "core.hpp"

namespace wombat {
namespace core {

std::vector<Drawer*> drawers;
std::vector<Graphics*> graphicsInstances;
SDL_Window *display = 0;
SDL_Thread *drawThread = 0;
SDL_Renderer *renderer = 0;
extern std::vector<std::function<void(Event)>> eventListeners;
const auto Event_DrawEvent = SDL_RegisterEvents(1);

void draw() {
	SDL_Event ev;
	SDL_zero(ev);
	ev.type = Event_DrawEvent;
	SDL_PushEvent(&ev);
}

void main() {
	// handle events
	SDL_Event sev;
	for (auto running = true; running;) {
		SDL_WaitEvent(&sev);
		const auto t = sev.type;
		if (t == Event_DrawEvent) {
			for (int i = 0; i < drawers.size(); i++) {
				drawers[i]->draw(graphicsInstances[i]);
			}
			SDL_RenderPresent(renderer);
		} else {
			Event ev;
			ev.type = toEventType(sev.type);
			for (auto f : eventListeners) {
				f(ev);
			}

			if (ev.type == Quit) {
				running = false;
			}
		}
	}
}

int init(bool fullscreen, int w, int h) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) == -1) {
		return -1;
	}

	display = SDL_CreateWindow("Wombat", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL);
	if (!display)
		return -3;
	renderer = SDL_CreateRenderer(display, -1, SDL_RENDERER_ACCELERATED);

	return 0;
}

void addDrawer(Drawer *d) {
	graphicsInstances.push_back(new Graphics());
	drawers.push_back(d);
}

int thread(void *func) {
	if (func) {
		auto f = (std::function<void()>*) func;
		(*f)();
		delete f;
	}
}

void startThread(std::function<void()> f) {
	static int threadCount = 0;

	char thrdNm[10];
	snprintf(thrdNm, 10, "Thread %d", threadCount);


	auto fp = new std::function<void()>(f);
	SDL_CreateThread(thread, thrdNm, (void*) fp);

	threadCount++;
}

void sleep(uint64 ms) {
	SDL_Delay(ms);
}

}
}

#endif