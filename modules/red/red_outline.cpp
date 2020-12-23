/*************************************************************************/
/*  line_2d.cpp                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "red_outline.h"
#include "red_shape.h"
#include "red.h"
#include "scene/2d/line_builder.h"

void REDOutline::_draw() {
	ERR_FAIL_COND_MSG(!shape, "No parent REDshape found!");
	LineBuilder lb;
	lb.points = shape->real_polygon;
	lb.default_color = shape->line_color;
	//lb.gradient = *_gradient;
	lb.texture_mode = shape->texture_mode;
	lb.joint_mode = shape->joint_mode;
	//lb.begin_cap_mode = _begin_cap_mode;
	//lb.end_cap_mode = _end_cap_mode;
	lb.round_precision = 8;
	lb.sharp_limit = 1000.f;
	lb.width = shape->width + shape->outline_width_zoom_const * ((shape->width * shape->get_camera_zoom().x / get_scale().x) - shape->width);
	//lb.curve = *_curve;
    lb.closed = true;
    lb.width_list = shape->real_width_list;
	RID texture_rid;
	if (shape->texture.is_valid()) {
		texture_rid = shape->texture->get_rid();
		lb.tile_aspect = shape->texture->get_size().aspect();
	}
	lb.build();
	VS::get_singleton()->canvas_item_add_triangle_array(
			get_canvas_item(),
			lb.indices,
			lb.vertices,
			lb.colors,
			lb.uvs, Vector<int>(), Vector<float>(),
			texture_rid, -1, RID(),
			shape->antialiased, true);
}

void REDOutline::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			_draw();
		}
		case NOTIFICATION_ENTER_TREE: {
			Node *parent = get_parent();
			for (int i = 0; i < 10; i++){
				shape = Object::cast_to<REDShape>(parent);
				if (shape){
					shape->set_outline(this);
					break;
				}
				parent = get_parent();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (shape){
				shape->set_outline(NULL);
			}
			shape = NULL;
		} break;
	}
}
void REDOutline::_bind_methods() {

}

REDOutline::REDOutline(){	

}