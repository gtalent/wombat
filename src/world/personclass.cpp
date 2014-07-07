/*
 * Copyright 2013-2014 gtalent2@gmail.com
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "personclass.hpp"

namespace wombat {
namespace world {

using models::SpriteDirection;
using models::SpriteMotion;

core::Flyweight<models::PersonClass> PersonClass::c_personClasses(
	[](models::PersonClass model) {
		if (model.Import != "") {
			core::read(model, model.Import);
		}
		return new PersonClass(model);
	}
);

PersonClass::PersonClass(models::PersonClass model) {
	m_title = model.Title[core::getLanguage()];

	m_animations.resize(model.Overhead.size());
	for (uint i = 0; i < model.Overhead.size(); i++) {
		m_animations[i].resize(model.Overhead[i].size());
		for (uint ii = 0; ii < model.Overhead[i].size(); ii++) {
			m_animations[i][ii].load(model.Overhead[i][ii]);
		}
	}
}

void PersonClass::draw(core::Graphics &gfx, common::Point pt, SpriteDirection facing, SpriteMotion motion) {
	auto anim = this->anim(facing, motion);
	if (anim.animation) {
		gfx.draw(anim.animation->getImage(), pt + anim.point);
	}
}

AnimLayer PersonClass::anim(models::SpriteDirection facing, models::SpriteMotion motion) {
	if ((uint) facing < m_animations.size() && (uint) motion < m_animations[(uint) facing].size()) {
		return m_animations[(uint) facing][(uint) motion];
	}
	return AnimLayer();
}

PersonClass *PersonClass::checkout(models::PersonClass model) {
	return dynamic_cast<PersonClass*>(c_personClasses.checkout(model));
}

PersonClass *PersonClass::checkout(std::string path) {
	return dynamic_cast<PersonClass*>(c_personClasses.checkout(path));
}

void PersonClass::checkin(PersonClass *pc) {
	c_personClasses.checkin(pc);
}

}
}
