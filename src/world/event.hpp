/*
 * Copyright 2013-2015 gtalent2@gmail.com
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef WOMBAT_WORLD_EVENT_HPP
#define WOMBAT_WORLD_EVENT_HPP

#include <core/event.hpp>

namespace wombat {
namespace world {

enum WorldEvent {
	SpriteHandover = core::Event::AppEvent + 1,
	SpriteHandoverAck,
	StartMoving,
	StopMoving
};

}
}

#endif
