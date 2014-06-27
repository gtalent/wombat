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
#include <core/core.hpp>

namespace wombat {
namespace world {

extern bool _debug;

extern int TileWidth;
extern int TileHeight;

extern std::map<core::TaskProcessor*, class Zone*> m_zoneMap;

class ZoneProcessor: public core::TaskProcessor {
	private:
		Zone *m_zone = nullptr;

	public:
		/**
		 * Constructor
		 * @param zone the Zone that owns this ZoneProcessor
		 */
		ZoneProcessor(class Zone *zone);

		/**
		 * Gets the Zone that owns this ZoneProcessor.
		 * @return the Zone that owns this ZoneProcessor
		 */
		class Zone *zone();
};

common::Point addrToPt(common::Point);

common::Point ptToAddr(common::Point);

}
}