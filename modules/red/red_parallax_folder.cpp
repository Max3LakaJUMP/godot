/*************************************************************************/
/*  parallax_layer.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "red_parallax_folder.h"

#include "core/engine.h"
#include "red_frame.h"

void REDParallaxFolder::set_motion_scale(const Size2 &p_scale) {

	motion_scale = p_scale;

	REDFrame *pb = Object::cast_to<REDFrame>(get_parent());
	if (pb && is_inside_tree()) {
		Vector2 ofs = pb->get_parallax();
		float scale = pb->get_scroll_scale();
		set_base_offset_and_scale(ofs, scale, screen_offset);
	}
}

Size2 REDParallaxFolder::get_motion_scale() const {

	return motion_scale;
}

void REDParallaxFolder::set_motion_offset(const Size2 &p_offset) {

	motion_offset = p_offset;

	REDFrame *pb = Object::cast_to<REDFrame>(get_parent());
	if (pb && is_inside_tree()) {
		Vector2 ofs = pb->get_parallax();
		float scale = pb->get_scroll_scale();
		set_base_offset_and_scale(ofs, scale, screen_offset);
	}
}

Size2 REDParallaxFolder::get_motion_offset() const {

	return motion_offset;
}

void REDParallaxFolder::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_ENTER_TREE: {

			orig_offset = get_position();
			orig_scale = get_scale();
		} break;
		case NOTIFICATION_EXIT_TREE: {

			set_position(orig_offset);
			set_scale(orig_scale);
		} break;
	}
}

void REDParallaxFolder::set_base_offset_and_scale(const Point2 &p_offset, float p_scale, const Point2 &p_screen_offset) {
	screen_offset = p_screen_offset;
	if (!is_inside_tree())
		return;
	if (Engine::get_singleton()->is_editor_hint())
		return;
	Point2 new_ofs = (screen_offset + (p_offset - screen_offset) * motion_scale) + motion_offset * p_scale + orig_offset * p_scale;

	set_position(new_ofs);
	set_scale(Vector2(1, 1) * p_scale * orig_scale);
}

String REDParallaxFolder::get_configuration_warning() const {

	if (!Object::cast_to<REDFrame>(get_parent())) {
		return TTR("REDParallaxFolder node only works when set as child of a REDFrame node.");
	}

	return String();
}

void REDParallaxFolder::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_motion_scale", "scale"), &REDParallaxFolder::set_motion_scale);
	ClassDB::bind_method(D_METHOD("get_motion_scale"), &REDParallaxFolder::get_motion_scale);
	ClassDB::bind_method(D_METHOD("set_motion_offset", "offset"), &REDParallaxFolder::set_motion_offset);
	ClassDB::bind_method(D_METHOD("get_motion_offset"), &REDParallaxFolder::get_motion_offset);

	ADD_GROUP("Motion", "motion_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "motion_scale"), "set_motion_scale", "get_motion_scale");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "motion_offset"), "set_motion_offset", "get_motion_offset");
}

REDParallaxFolder::REDParallaxFolder() {
	motion_scale = Size2(0, 0);
}
