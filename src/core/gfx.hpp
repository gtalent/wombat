/*
 * Copyright 2013-2014 gtalent2@gmail.com
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef WOMBAT_CORE_GFX_HPP
#define WOMBAT_CORE_GFX_HPP

#include <string>

#include "_cliprectstack.hpp"
#include "modelio.hpp"
#include "core.hpp"

namespace wombat {
namespace core {

int displayWidth();

int displayHeight();


class Image: public Flyweight<models::Image>::Value {
	friend class Graphics;
	private:
		void *m_img = nullptr;
		models::Size m_defaultSize;
		std::string m_key;

	protected:
		/**
		 * The bounds to cut out of the source image.
		 */
		common::Bounds m_bounds;

	public:
		/**
		 * Constructor
		 * @param model Model to build the Image from
		 */
		explicit Image(models::Image);

		/**
		 * Destructor
		 */
		virtual ~Image();

		/**
		 * Returns the width of the source image.
		 */
		int width();

		/**
		 * Returns the height of the source image.
		 */
		int height();

		/**
		 * Returns the default draw width.
		 */
		int defaultWidth();

		/**
		 * Returns the default draw width.
		 */
		int defaultHeight();

		bool loaded();

		std::string key();
};

class Animation: public Flyweight<models::Animation>::Value {
	private:
		std::vector<Image*> m_imgs;
		uint64 m_lastUpdate;
		uint64 m_interval;
		int m_slide;
		std::string m_key;

	public:
		explicit Animation(models::Animation);

		~Animation();

		void add(Image*);

		Image *getImage();

		bool loaded();

		std::string key();

		/**
		 * Gets the number of Images in the Animation.
		 * @return the number of Images in the Animation
		 */
		int size();
};

class Graphics {
	friend void _draw();
	private:
		common::Point m_origin;

	protected:
		ClipRectStack m_cliprect;

	public:
		void drawLine(int x1, int y1, int x2, int y2);

		void draw(Image *img, int x, int y, int w, int h);

		void draw(Image *img, int x, int y);

		void draw(Image *img, common::Point pt);

		/**
		 * Sets the color for primitives to draw with.
		 * @param r red value (0-255)
		 * @param g green value (0-255)
		 * @param b blue value (0-255)
		 * @param a alpha value (0-255)
		 */
		void setRGBA(int r, int g, int b, int a);

		/**
		 * Sets the color for primitives to draw with.
		 * @param r red value (0-255)
		 * @param g green value (0-255)
		 * @param b blue value (0-255)
		 */
		void setRGB(int r, int g, int b);

		/**
		 * Pushs the given view port the viewports stack.
		 * This will get cleared out at the end of this draw iteration regardless
		 * of whether or not it is manually popped.
		 * @param bnds the bounds of the desired viewport 
		 */
		void pushClipRect(common::Bounds bnds);

		/**
		 * Pushs the given view port the viewports stack.
		 * This will get cleared out at the end of this draw iteration regardless
		 * of whether or not it is manually popped.
		 * @param x the left border of the clip rect
		 * @param y the right border of the clip rect
		 * @param w the width of the clip rect
		 * @param h the height of the clip rect
		 */
		void pushClipRect(int x, int y, int w, int h);

		/**
		 * Pop the current clip rect.
		 */
		void popClipRect();

	protected:
		void resetViewport();
};

class Drawer {
	public:
		virtual ~Drawer() {};
		virtual void draw(Graphics&) = 0;
};

void addDrawer(Drawer*);

Animation *checkoutAnimation(Path path);

Animation *checkoutAnimation(models::Animation &anim);

void checkinAnimation(Animation *i);

Image *checkoutImage(models::Image img);

void checkinImage(Image *img);

}
}

#endif