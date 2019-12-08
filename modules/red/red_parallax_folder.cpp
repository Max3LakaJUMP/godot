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
Vector2 REDParallaxFolder::get_parallax_scale(){
	if (parallax_scale_dirty){
		bool zoom_in = (camera_zoom.x < 1) ? true : false;
		Vector2 scale_offset = zoom_in ? Vector2(1, 1) / camera_zoom : camera_zoom;
		Vector2 invert = Vector2(1, 1);
		if (camera_zoom.x > 0){
			scale_offset.x = 1 - scale_offset.x;
		}else{
			scale_offset.y = 1 + scale_offset.x;
			invert.x = -1;
		}
		if (camera_zoom.y > 0){
			scale_offset.y = 1 - scale_offset.y;
		}else{
			scale_offset.y = 1 + scale_offset.y;
			invert.y = -1;
		}

		if (zoom_in){
			if (zoom_in_scale < 0){
				parallax_scale = Vector2(1, 1) / (Vector2(1, 1) + scale_offset * zoom_in_scale);
			}else{
				parallax_scale = Vector2(1, 1) - scale_offset * zoom_in_scale;
			}
		}else{
			if (zoom_out_scale > 0){
				parallax_scale = Vector2(1, 1) / (Vector2(1, 1) - scale_offset * zoom_out_scale);
			}else{
				parallax_scale = Vector2(1, 1) + scale_offset * zoom_out_scale;
			}
		}
		parallax_scale *= invert;
		parallax_scale_dirty = false;
	}
	return parallax_scale;
}
Point2 REDParallaxFolder::get_parallax_offset(){
	if (parallax_offset_dirty){
		//parallax_offset = (screen_offset + (p_offset - screen_offset) * parallax_factor) + motion_offset * get_parallax_scale() + orig_offset * scale;
		parallax_offset = camera_offset * parallax_factor + motion_offset;
		parallax_offset_dirty = false;
	}
	return parallax_offset;
}

void REDParallaxFolder::apply_parallax() {
	if (!is_inside_tree())
		return;
	if (Engine::get_singleton()->is_editor_hint())
		return;
	Vector2 s = get_parallax_scale();
	if (s != old_parallax_scale){
		set_scale(get_scale() / old_parallax_scale * s);
		old_parallax_scale = s;
	}
	Point2 o = get_parallax_offset();
	if (o != old_parallax_offset){
		set_position(get_position() - old_parallax_offset + o);
		old_parallax_offset = o;
	}
}

void REDParallaxFolder::set_camera_offset(const Vector2 &p_offset){
	if (camera_offset == p_offset)
		return;
	camera_offset = p_offset;
	parallax_offset_dirty = true;
	update();
}

void REDParallaxFolder::set_camera_zoom(const Vector2 &p_zoom){
	if (camera_zoom == p_zoom)
		return;
	camera_zoom = p_zoom;
	parallax_scale_dirty = true;
	update();
}

void REDParallaxFolder::set_parallax_factor(const Size2 &p_scale) {
	if (parallax_factor == p_scale)
		return;
	parallax_factor = p_scale;
	parallax_offset_dirty = true;
	update();
}
void REDParallaxFolder::set_motion_offset(const Size2 &p_offset) {
	if (motion_offset == p_offset)
		return;
	motion_offset = p_offset;
	parallax_offset_dirty = true;
	update();
}

void REDParallaxFolder::set_zoom_in_scale(float p_scale){
	if (zoom_in_scale == p_scale)
		return;
	zoom_in_scale = p_scale;
	parallax_scale_dirty = true;
	parallax_offset_dirty = true;
	update();
}

void REDParallaxFolder::set_zoom_out_scale(float p_scale){
	if (zoom_out_scale == p_scale)
		return;
	zoom_out_scale = p_scale;
	parallax_scale_dirty = true;
	parallax_offset_dirty = true;
	update();
}

Size2 REDParallaxFolder::get_parallax_factor() const {

	return parallax_factor;
}

Size2 REDParallaxFolder::get_motion_offset() const {

	return motion_offset;
}

float REDParallaxFolder::get_zoom_in_scale() const{
	return zoom_in_scale;
}

float REDParallaxFolder::get_zoom_out_scale() const{
	return zoom_out_scale;
}

String REDParallaxFolder::get_configuration_warning() const {

	if (!Object::cast_to<REDFrame>(get_parent())) {
		return TTR("REDParallaxFolder node only works when set as child of a REDFrame node.");
	}

	return String();
}

void REDParallaxFolder::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			apply_parallax();
		} break;
	}
}

void REDParallaxFolder::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_parallax_factor", "scale"), &REDParallaxFolder::set_parallax_factor);
	ClassDB::bind_method(D_METHOD("get_parallax_factor"), &REDParallaxFolder::get_parallax_factor);
	ClassDB::bind_method(D_METHOD("set_zoom_in_scale", "zoom_in_scale"), &REDParallaxFolder::set_zoom_in_scale);
	ClassDB::bind_method(D_METHOD("get_zoom_in_scale"), &REDParallaxFolder::get_zoom_in_scale);
	ClassDB::bind_method(D_METHOD("set_zoom_out_scale", "zoom_out_scale"), &REDParallaxFolder::set_zoom_out_scale);
	ClassDB::bind_method(D_METHOD("get_zoom_out_scale"), &REDParallaxFolder::get_zoom_out_scale);
	ClassDB::bind_method(D_METHOD("set_motion_offset", "offset"), &REDParallaxFolder::set_motion_offset);
	ClassDB::bind_method(D_METHOD("get_motion_offset"), &REDParallaxFolder::get_motion_offset);

	ADD_GROUP("Parallax", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "parallax_factor"), "set_parallax_factor", "get_parallax_factor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "zoom_in_scale"), "set_zoom_in_scale", "get_zoom_in_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "zoom_out_scale"), "set_zoom_out_scale", "get_zoom_out_scale");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "motion_offset"), "set_motion_offset", "get_motion_offset");
}

REDParallaxFolder::REDParallaxFolder() {
	camera_offset = Vector2(0, 0);
	camera_zoom = Vector2(1, 1);

	parallax_offset = Vector2(0, 0);
	old_parallax_offset = Vector2(0, 0);
	parallax_offset_dirty = true;
		
	parallax_scale = Vector2(1, 1);
	old_parallax_scale = Vector2(1, 1);
	parallax_scale_dirty = true;

	zoom_in_scale = 0.0f;
	zoom_out_scale = 0.0f;

	parallax_factor = Size2(0, 0);
}
