/*************************************************************************/
/*  polygon_2d.cpp                                                       */
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

#include "red_bubble.h"

#include "core/math/geometry.h"
#include "scene/2d/skeleton_2d.h"
#include "red_line.h"

#include "red_line_builder.h"
#include "core/math/math_funcs.h"
#include "core/core_string_names.h"
#include <string>

//Line

void REDBubble::set_width(float p_width) {
	if (p_width < 0.0)
		p_width = 0.0;
	_width = p_width;
	update();

}

float REDBubble::get_width() const {
	return _width;
}

void REDBubble::set_curve(const Ref<Curve> &p_curve) {
	// Cleanup previous connection if any
	if (_curve.is_valid()) {
		_curve->disconnect(CoreStringNames::get_singleton()->changed, this, "_curve_changed");
	}

	_curve = p_curve;

	// Connect to the curve so the line will update when it is changed
	if (_curve.is_valid()) {
		_curve->connect(CoreStringNames::get_singleton()->changed, this, "_curve_changed");
	}

	update();
}

Ref<Curve> REDBubble::get_curve() const {
	return _curve;
}

void REDBubble::set_default_color(Color p_color) {
	_default_color = p_color;
	update();
}

Color REDBubble::get_default_color() const {
	return _default_color;
}

void REDBubble::set_gradient(const Ref<Gradient> &p_gradient) {

	// Cleanup previous connection if any
	if (_gradient.is_valid()) {
		_gradient->disconnect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}

	_gradient = p_gradient;

	// Connect to the gradient so the line will update when the ColorRamp is changed
	if (_gradient.is_valid()) {
		_gradient->connect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}

	update();
}

Ref<Gradient> REDBubble::get_gradient() const {
	return _gradient;
}

void REDBubble::set_line_texture(const Ref<Texture> &p_texture) {
	_texture = p_texture;
	update();
}

Ref<Texture> REDBubble::get_line_texture() const {
	return _texture;
}

void REDBubble::set_texture_mode(const LineTextureMode p_mode) {
	_texture_mode = p_mode;
	update();
}

REDBubble::LineTextureMode REDBubble::get_texture_mode() const {
	return _texture_mode;
}

void REDBubble::set_joint_mode(REDBubble::LineJointMode p_mode) {
	_joint_mode = p_mode;
	update();
}

REDBubble::LineJointMode REDBubble::get_joint_mode() const {
	return _joint_mode;
}

void REDBubble::set_sharp_limit(float p_limit) {
	if (p_limit < 0.f)
		p_limit = 0.f;
	_sharp_limit = p_limit;
	update();
}

float REDBubble::get_sharp_limit() const {
	return _sharp_limit;
}

void REDBubble::set_round_precision(int p_precision) {
	if (p_precision < 1)
		p_precision = 1;
	_round_precision = p_precision;
	update();
}

int REDBubble::get_round_precision() const {
	return _round_precision;
}

void REDBubble::_draw_outline(Vector<Vector2> &p_points) {
	if (p_points.size() <= 1 || _width == 0.f)
		return;
    Vector<float> t_l;
	
	int len = p_points.size();
	{
        int oldSize = MIN(thickness_list.size(), len);
        t_l.resize(len);
        PoolVector<float>::Read thickness_list_read = thickness_list.read();
        for (int i = 0; i < oldSize; i++) {
            t_l.write[i] = thickness_list_read[i];
        }
        for (int i = oldSize; i < len; i++) {
            t_l.write[i] = 1.f;
        }
	}

	Vector<float> new_thickness;
	{
		int pre_a;
		int b;
		int post_b;
		int end=thickness_list.size()-1;
		float smooth_thickness_iter = len*1.0f/thickness_list.size()-1;

		PoolVector<float>::Read thickness_list_read = thickness_list.read();
		for(int i=0; i<thickness_list.size(); ++i){
			if (i==end)
				b = 0;
			else
				b = i + 1;

			if (i==0)
				pre_a = end;
			else
				pre_a = i - 1;

			if (i==end - 1)
				post_b = 0;
			else
				post_b = b + 1;
			new_thickness.push_back(thickness_list_read[i]);
			for(int j=0; j<smooth_thickness_iter; ++j){
				new_thickness.push_back(thickness_list_read[i]+(j+1)*1.0/(smooth_thickness_iter+1)*(thickness_list_read[b]-thickness_list_read[i]));
			}
		}
	}
	print_line(std::to_string(new_thickness.size()).c_str());
	new_thickness.resize(p_points.size());
	// TODO Maybe have it as member rather than copying parameters and allocating memory?
	REDLineBuilder lb;
	lb.points = p_points;
	lb.default_color = _default_color;
	lb.gradient = *_gradient;
	lb.texture_mode = static_cast<REDLine::LineTextureMode>(_texture_mode);
	lb.joint_mode = static_cast<REDLine::LineJointMode>(_joint_mode);
	lb.begin_cap_mode = static_cast<REDLine::LineCapMode>(_begin_cap_mode);
	lb.end_cap_mode = static_cast<REDLine::LineCapMode>(_end_cap_mode);
	lb.round_precision = _round_precision;
	lb.sharp_limit = _sharp_limit;
	lb.width = _width;
	lb.curve = *_curve;
    lb.is_closed = true;
    lb.thickness_list = new_thickness;

	RID texture_rid;
	if (_texture.is_valid()) {
		texture_rid = _texture->get_rid();

		lb.tile_aspect = _texture->get_size().aspect();
	}

	lb.build();

	VS::get_singleton()->canvas_item_add_triangle_array(
			get_canvas_item(),
			lb.indices,
			lb.vertices,
			lb.colors,
			lb.uvs, Vector<int>(), Vector<float>(),

			texture_rid);
	/*Draw wireframe
		if(lb.indices.size() % 3 == 0) {
			Color col(0,0,0);
			for(int i = 0; i < lb.indices.size(); i += 3) {
				int vi = lb.indices[i];
				int lbvsize = lb.vertices.size();
				Vector2 a = lb.vertices[lb.indices[i]];
				Vector2 b = lb.vertices[lb.indices[i+1]];
				Vector2 c = lb.vertices[lb.indices[i+2]];
				draw_line(a, b, col);
				draw_line(b, c, col);
				draw_line(c, a, col);
			}
			for(int i = 0; i < lb.vertices.size(); ++i) {
				Vector2 p = lb.vertices[i];
				draw_rect(Rect2(p.x-1, p.y-1, 2, 2), Color(0,0,0,0.5));
			}
		}*/
}

void REDBubble::_gradient_changed() {
	update();
}

void REDBubble::_curve_changed() {
	update();
}

void REDBubble::set_thickness_list(const PoolVector<float> &p_thickness_list) {
    thickness_list = p_thickness_list;
    update();
}

PoolVector<float> REDBubble::get_thickness_list() const {
    return thickness_list;
}

Dictionary REDBubble::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["offset"] = offset;
	return state;
}

void REDBubble::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_offset(p_state["offset"]);
}

void REDBubble::_edit_set_pivot(const Point2 &p_pivot) {
	set_position(get_transform().xform(p_pivot));
	set_offset(get_offset() - p_pivot);
}

Point2 REDBubble::_edit_get_pivot() const {
	return Vector2();
}

bool REDBubble::_edit_use_pivot() const {
	return true;
}

Rect2 REDBubble::_edit_get_rect() const {
	if (rect_cache_dirty) {
		int l = polygon.size();
		PoolVector<Vector2>::Read r = polygon.read();
		item_rect = Rect2();
		for (int i = 0; i < l; i++) {
			Vector2 pos = r[i] + offset;
			if (i == 0)
				item_rect.position = pos;
			else
				item_rect.expand_to(pos);
		}
		rect_cache_dirty = false;
	}

	return item_rect;
}

bool REDBubble::_edit_use_rect() const {
	return polygon.size() > 0;
}

bool REDBubble::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {

	Vector<Vector2> REDBubble = Variant(polygon);
	return Geometry::is_point_in_polygon(p_point - get_offset(), REDBubble);
}

void REDBubble::set_spikes(const float p_detail) {
    spikes = p_detail;
    update();
}

float REDBubble::get_spikes() const{
    return spikes;
}
void REDBubble::set_smooth(const int p_detail) {
    smooth = p_detail;
    update();
}

int REDBubble::get_smooth() const{
    return smooth;
}
void REDBubble::set_interpolation(REDBubble::Interpolation p_interpolation){
    interpolation = p_interpolation;

    if (smooth>0) {
		update();
    }
}

REDBubble::Interpolation REDBubble::get_interpolation() const{
    return interpolation;
}

void REDBubble::set_tail_interpolation(REDBubble::Interpolation p_tale_interpolation){
    tale_interpolation = p_tale_interpolation;

    if (smooth>0) {
		update();
    }
}

REDBubble::Interpolation REDBubble::get_tail_interpolation() const{
    return tale_interpolation;
}

void REDBubble::set_edit_tail(const bool p_edit_tail) {
    edit_tail = p_edit_tail;
	update();
}

bool REDBubble::get_edit_tail() const{
    return edit_tail;
}

void REDBubble::set_polygon_tail(const PoolVector<Vector2> &p_polygon) {
	polygon_tail = p_polygon;
	rect_cache_dirty = true;
	update();
}

PoolVector<Vector2> REDBubble::get_polygon_tail() const {
	return polygon_tail;
}
void REDBubble::set_use_outline(const bool b) {
    use_outline = b;
	update();
}

bool REDBubble::get_use_outline() const{
    return use_outline;

}
void REDBubble::set_reorient(const bool b) {
    reorient = b;
	update();
}

bool REDBubble::get_reorient() const{
    return reorient;
}

Vector2 _cubic_bezier(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, float t){
    Vector2 q0 = p0.linear_interpolate(p1, t);
    Vector2 q1 = p1.linear_interpolate(p2, t);
    Vector2 q2 = p2.linear_interpolate(p3, t);

    Vector2 r0 = q0.linear_interpolate(q1, t);
    Vector2 r1 = q1.linear_interpolate(q2, t);

    Vector2 r = r0.linear_interpolate(r1, t);
    return r;
} 

Vector2 _quadratic_bezier(Vector2 p0, Vector2 p1, Vector2 p2, float t){
    Vector2 q0 = p0.linear_interpolate(p1, t);
    Vector2 q1 = p1.linear_interpolate(p2, t);
    Vector2 r = q0.linear_interpolate(q1, t);
    return r;
}

Vector<Vector2> REDBubble::_tesselate(Vector<Vector2> &low_res, const REDBubble::Interpolation &p_interpolation) const{
	int pre_a;
	int b;
	int post_b;
	int end = low_res.size()-1;
	Vector<Vector2> new_points;
	switch(p_interpolation){
		case(CUBIC): {
			for(int i=0; i<low_res.size(); ++i){
				if (i==end)
					b = 0;
				else
					b = i + 1;

				if (i==0)
					pre_a = end;
				else
					pre_a = i - 1;

				if (i==end - 1)
					post_b = 0;
				else
					post_b = b + 1;
				new_points.push_back(low_res[i]);
				for(int j=0; j<smooth; ++j){
					new_points.push_back(low_res[i].cubic_interpolate(low_res[b], low_res[pre_a], low_res[post_b], (j+1)*1.0/(smooth+1)));
				}
			}
		} break;
		case(QUADRATIC_BEZIER): {
			for(int i=0; i<low_res.size(); i+=2){
				if (i==end)
					b = 0;
				else
					b = i + 1;
				if (i==0)
					pre_a = end;
				else
					pre_a = i - 1;
				if (i==end - 1)
					post_b = 0;
				else
					post_b = b + 1;
				new_points.push_back(low_res[i]);
				for(int j=0; j<smooth; ++j){
					new_points.push_back(_quadratic_bezier(low_res[i], low_res[b], low_res[post_b], (j+1)*1.0/(smooth+1)));
				}
			}
		} break;
		case(CUBIC_BEZIER): {
			for(int i=0; i<low_res.size(); i+=3){
				if (i==end)
					b = 0;
				else
					b = i + 1;
				if (i==0)
					pre_a = end;
				else
					pre_a = i - 1;
				if (i==end - 1)
					post_b = 0;
				else
					post_b = b + 1;
				new_points.push_back(low_res[pre_a]);
				for(int j=0; j<smooth; ++j){
					new_points.push_back(_cubic_bezier(low_res[pre_a], low_res[i], low_res[b], low_res[post_b], (j+1)*1.0/(smooth+1)));
				}
			}
		} break;
	}
	return new_points;
}

void REDBubble::_add_spikes(Vector<Vector2> &target, const Vector<Vector2> &source){
	if (spikes>0)
		for (int i = 0; i < source.size(); ++i)
			if (i%2)
				target.push_back(source[i] + source[i].normalized() * spikes);
			else
				target.push_back(source[i] - source[i].normalized() * spikes);
}

void REDBubble::_add_spikes(Vector<Vector2> &p_points){
	if (spikes!=0)
		for (int i = 0; i < p_points.size(); ++i)
			if (i%2)
				p_points.write[i] = p_points[i] + p_points[i].normalized() * spikes;
			else
				p_points.write[i] = p_points[i] - p_points[i].normalized() * spikes;
}

void REDBubble::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_DRAW: {
			int len = polygon.size();
			if (len < 3)
				return;
			// VS::get_singleton()->canvas_item_attach_skeleton(get_canvas_item(), RID());
			Vector<Vector2> points;
			Vector<Vector2> uvs;
			points.resize(len);
			{
				PoolVector<Vector2>::Read polyr = polygon.read();
				for (int i = 0; i < len; i++)
					points.write[i] = polyr[i] + offset;
			}

			if (texture.is_valid()) {

				Transform2D texmat(tex_rot, tex_ofs);
				texmat.scale(tex_scale);
				Size2 tex_size = texture->get_size();

				uvs.resize(len);

				if (points.size() == uv.size()) {

					PoolVector<Vector2>::Read uvr = uv.read();

					for (int i = 0; i < len; i++) {
						uvs.write[i] = texmat.xform(uvr[i]) / tex_size;
					}

				} else {
					for (int i = 0; i < len; i++) {
						uvs.write[i] = texmat.xform(points[i]) / tex_size;
					}
				}
			}
			Vector<Color> colors;
			if (vertex_colors.size() == points.size()) {
				colors.resize(len);
				PoolVector<Color>::Read color_r = vertex_colors.read();
				for (int i = 0; i < len; i++) {
					colors.write[i] = color_r[i];
				}
			} else {
				colors.push_back(color);
			}
			if (smooth>0)
				points = _tesselate(points, interpolation);
				_add_spikes(points);
			if (polygon_tail.size()>0){
				Vector<Vector2>points_tail;
				int len_tail = polygon_tail.size();
				points_tail.resize(len_tail);
				PoolVector<Vector2>::Read polyr = polygon_tail.read();
				for (int i = 0; i < len_tail; i++) {
					points_tail.write[i] = polyr[i] + offset;
				}
				if (smooth>0)
					points_tail = _tesselate(points_tail, tale_interpolation);
				Vector<Vector<Vector2> > merged_points = Geometry::merge_polygons_2d(points, points_tail);
				int merged_points_len = merged_points[0].size();
				if (merged_points_len>0){
					if (reorient){
						Vector2 origin = points_tail[0];
						int origin_i=0;
						for (int i = 0; i < merged_points_len; ++i)
						{
							if (merged_points[0][i]==origin){
								origin_i = i;
								break;
							}
						}
						if (origin_i>0){
							points.clear();
							for (int i = origin_i; i < merged_points_len; i++)
								points.push_back(merged_points[0][i]);
							for (int i = 0; i < origin_i; i++)
								points.push_back(merged_points[0][i]);
						} else 
							points = merged_points[0];
					}
					else
						points = merged_points[0];
				}
			}
			VS::get_singleton()->canvas_item_add_polygon(get_canvas_item(), points, colors, uvs, texture.is_valid() ? texture->get_rid() : RID(), RID(), antialiased);
			if (use_outline)
				_draw_outline(points);
		} break;
	}
}

void REDBubble::set_polygon(const PoolVector<Vector2> &p_polygon) {
	polygon = p_polygon;
	rect_cache_dirty = true;
	update();
}

PoolVector<Vector2> REDBubble::get_polygon() const {

	return polygon;
}

void REDBubble::set_uv(const PoolVector<Vector2> &p_uv) {

	uv = p_uv;
	update();
}

PoolVector<Vector2> REDBubble::get_uv() const {

	return uv;
}

void REDBubble::set_color(const Color &p_color) {

	color = p_color;
	update();
}
Color REDBubble::get_color() const {

	return color;
}

void REDBubble::set_vertex_colors(const PoolVector<Color> &p_colors) {

	vertex_colors = p_colors;
	update();
}
PoolVector<Color> REDBubble::get_vertex_colors() const {

	return vertex_colors;
}

void REDBubble::set_texture(const Ref<Texture> &p_texture) {

	texture = p_texture;
	update();
}
Ref<Texture> REDBubble::get_texture() const {

	return texture;
}

void REDBubble::set_texture_offset(const Vector2 &p_offset) {

	tex_ofs = p_offset;
	update();
}
Vector2 REDBubble::get_texture_offset() const {

	return tex_ofs;
}

void REDBubble::set_texture_rotation(float p_rot) {

	tex_rot = p_rot;
	update();
}
float REDBubble::get_texture_rotation() const {

	return tex_rot;
}

void REDBubble::set_texture_rotation_degrees(float p_rot) {

	set_texture_rotation(Math::deg2rad(p_rot));
}
float REDBubble::get_texture_rotation_degrees() const {

	return Math::rad2deg(get_texture_rotation());
}

void REDBubble::set_texture_scale(const Size2 &p_scale) {

	tex_scale = p_scale;
	update();
}
Size2 REDBubble::get_texture_scale() const {

	return tex_scale;
}

void REDBubble::set_antialiased(bool p_antialiased) {

	antialiased = p_antialiased;
	update();
}
bool REDBubble::get_antialiased() const {

	return antialiased;
}

void REDBubble::set_offset(const Vector2 &p_offset) {

	offset = p_offset;
	rect_cache_dirty = true;
	update();
	_change_notify("offset");
}

Vector2 REDBubble::get_offset() const {

	return offset;
}

void REDBubble::_bind_methods() {
	//Line

	ClassDB::bind_method(D_METHOD("set_thickness_list", "thickness_list"), &REDBubble::set_thickness_list);
    ClassDB::bind_method(D_METHOD("get_thickness_list"), &REDBubble::get_thickness_list);

	ClassDB::bind_method(D_METHOD("set_width", "width"), &REDBubble::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &REDBubble::get_width);

	ClassDB::bind_method(D_METHOD("set_curve", "curve"), &REDBubble::set_curve);
	ClassDB::bind_method(D_METHOD("get_curve"), &REDBubble::get_curve);

	ClassDB::bind_method(D_METHOD("set_default_color", "color"), &REDBubble::set_default_color);
	ClassDB::bind_method(D_METHOD("get_default_color"), &REDBubble::get_default_color);

	ClassDB::bind_method(D_METHOD("set_gradient", "color"), &REDBubble::set_gradient);
	ClassDB::bind_method(D_METHOD("get_gradient"), &REDBubble::get_gradient);

	ClassDB::bind_method(D_METHOD("set_line_texture", "line_texture"), &REDBubble::set_line_texture);
	ClassDB::bind_method(D_METHOD("get_line_texture"), &REDBubble::get_line_texture);

	ClassDB::bind_method(D_METHOD("set_texture_mode", "mode"), &REDBubble::set_texture_mode);
	ClassDB::bind_method(D_METHOD("get_texture_mode"), &REDBubble::get_texture_mode);

	ClassDB::bind_method(D_METHOD("set_joint_mode", "mode"), &REDBubble::set_joint_mode);
	ClassDB::bind_method(D_METHOD("get_joint_mode"), &REDBubble::get_joint_mode);

	ClassDB::bind_method(D_METHOD("set_sharp_limit", "limit"), &REDBubble::set_sharp_limit);
	ClassDB::bind_method(D_METHOD("get_sharp_limit"), &REDBubble::get_sharp_limit);

	ClassDB::bind_method(D_METHOD("set_round_precision", "precision"), &REDBubble::set_round_precision);
	ClassDB::bind_method(D_METHOD("get_round_precision"), &REDBubble::get_round_precision);

	//Red
	ClassDB::bind_method(D_METHOD("set_spikes", "spike"), &REDBubble::set_spikes);
    ClassDB::bind_method(D_METHOD("get_spikes"), &REDBubble::get_spikes);
	ClassDB::bind_method(D_METHOD("set_reorient", "reorient"), &REDBubble::set_reorient);
    ClassDB::bind_method(D_METHOD("get_reorient"), &REDBubble::get_reorient);
	ClassDB::bind_method(D_METHOD("set_use_outline", "use_outline"), &REDBubble::set_use_outline);
    ClassDB::bind_method(D_METHOD("get_use_outline"), &REDBubble::get_use_outline);
	ClassDB::bind_method(D_METHOD("set_polygon_tail", "polygon_tail"), &REDBubble::set_polygon_tail);
    ClassDB::bind_method(D_METHOD("get_polygon_tail"), &REDBubble::get_polygon_tail);
	ClassDB::bind_method(D_METHOD("set_edit_tail", "edit_tail"), &REDBubble::set_edit_tail);
    ClassDB::bind_method(D_METHOD("get_edit_tail"), &REDBubble::get_edit_tail);
    ClassDB::bind_method(D_METHOD("set_smooth", "smooth"), &REDBubble::set_smooth);
    ClassDB::bind_method(D_METHOD("get_smooth"), &REDBubble::get_smooth);
    ClassDB::bind_method(D_METHOD("set_interpolation", "interpolation"), &REDBubble::set_interpolation);
    ClassDB::bind_method(D_METHOD("get_interpolation"), &REDBubble::get_interpolation);
	ClassDB::bind_method(D_METHOD("set_tail_interpolation", "tail_interpolation"), &REDBubble::set_tail_interpolation);
    ClassDB::bind_method(D_METHOD("get_tail_interpolation"), &REDBubble::get_tail_interpolation);

	ClassDB::bind_method(D_METHOD("set_polygon", "polygon"), &REDBubble::set_polygon);
	ClassDB::bind_method(D_METHOD("get_polygon"), &REDBubble::get_polygon);

	ClassDB::bind_method(D_METHOD("set_uv", "uv"), &REDBubble::set_uv);
	ClassDB::bind_method(D_METHOD("get_uv"), &REDBubble::get_uv);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &REDBubble::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &REDBubble::get_color);

	ClassDB::bind_method(D_METHOD("set_vertex_colors", "vertex_colors"), &REDBubble::set_vertex_colors);
	ClassDB::bind_method(D_METHOD("get_vertex_colors"), &REDBubble::get_vertex_colors);

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &REDBubble::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &REDBubble::get_texture);

	ClassDB::bind_method(D_METHOD("set_texture_offset", "texture_offset"), &REDBubble::set_texture_offset);
	ClassDB::bind_method(D_METHOD("get_texture_offset"), &REDBubble::get_texture_offset);

	ClassDB::bind_method(D_METHOD("set_texture_rotation", "texture_rotation"), &REDBubble::set_texture_rotation);
	ClassDB::bind_method(D_METHOD("get_texture_rotation"), &REDBubble::get_texture_rotation);

	ClassDB::bind_method(D_METHOD("set_texture_rotation_degrees", "texture_rotation"), &REDBubble::set_texture_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_texture_rotation_degrees"), &REDBubble::get_texture_rotation_degrees);

	ClassDB::bind_method(D_METHOD("set_texture_scale", "texture_scale"), &REDBubble::set_texture_scale);
	ClassDB::bind_method(D_METHOD("get_texture_scale"), &REDBubble::get_texture_scale);

	ClassDB::bind_method(D_METHOD("set_antialiased", "antialiased"), &REDBubble::set_antialiased);
	ClassDB::bind_method(D_METHOD("get_antialiased"), &REDBubble::get_antialiased);

	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &REDBubble::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &REDBubble::get_offset);
	
	ADD_GROUP("Polygon", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "antialiased"), "set_antialiased", "get_antialiased");

	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "polygon"), "set_polygon", "get_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "uv"), "set_uv", "get_uv");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_COLOR_ARRAY, "vertex_colors"), "set_vertex_colors", "get_vertex_colors");

	ADD_GROUP("Texture", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_GROUP("Texture", "texture_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "texture_offset"), "set_texture_offset", "get_texture_offset");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "texture_scale"), "set_texture_scale", "get_texture_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "texture_rotation_degrees", PROPERTY_HINT_RANGE, "-1080,1080,0.1,or_lesser,or_greater"), "set_texture_rotation_degrees", "get_texture_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "texture_rotation", PROPERTY_HINT_NONE, "", 0), "set_texture_rotation", "get_texture_rotation");
	
	//Red
	ADD_GROUP("Balloon", "");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "smooth"), "set_smooth", "get_smooth");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolation", PROPERTY_HINT_ENUM, "Cubic, Quadratic Bezier, Cubic Bezier"), "set_interpolation", "get_interpolation");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "tail_interpolation", PROPERTY_HINT_ENUM, "Cubic, Quadratic Bezier, Cubic Bezier"), "set_tail_interpolation", "get_tail_interpolation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "spikes"), "set_spikes", "get_spikes");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "polygon_tail"), "set_polygon_tail", "get_polygon_tail");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reorient"), "set_reorient", "get_reorient");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_tail"), "set_edit_tail", "get_edit_tail");

	//Line
	ADD_GROUP("Line", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_outline"), "set_use_outline", "get_use_outline");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_color"), "set_default_color", "get_default_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "thickness_list"), "set_thickness_list", "get_thickness_list");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "width_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_curve", "get_curve");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "joint_mode", PROPERTY_HINT_ENUM, "Sharp,Bevel,Round"), "set_joint_mode", "get_joint_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharp_limit"), "set_sharp_limit", "get_sharp_limit");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round_precision"), "set_round_precision", "get_round_precision");

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "line_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_line_texture", "get_line_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_mode", PROPERTY_HINT_ENUM, "None,Tile,Stretch"), "set_texture_mode", "get_texture_mode");

	BIND_ENUM_CONSTANT(CUBIC);
    BIND_ENUM_CONSTANT(QUADRATIC_BEZIER);
    BIND_ENUM_CONSTANT(CUBIC_BEZIER);

	BIND_ENUM_CONSTANT(LINE_JOINT_SHARP);
	BIND_ENUM_CONSTANT(LINE_JOINT_BEVEL);
	BIND_ENUM_CONSTANT(LINE_JOINT_ROUND);

	BIND_ENUM_CONSTANT(LINE_CAP_NONE);
	BIND_ENUM_CONSTANT(LINE_CAP_BOX);
	BIND_ENUM_CONSTANT(LINE_CAP_ROUND);

	BIND_ENUM_CONSTANT(LINE_TEXTURE_NONE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_TILE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_STRETCH);

	
	ClassDB::bind_method(D_METHOD("_gradient_changed"), &REDBubble::_gradient_changed);
	ClassDB::bind_method(D_METHOD("_curve_changed"), &REDBubble::_curve_changed);
}

REDBubble::REDBubble() {
	spikes = 0;
	_joint_mode = LINE_JOINT_SHARP;
	_begin_cap_mode = LINE_CAP_NONE;
	_end_cap_mode = LINE_CAP_NONE;
	_width = 10;
	_default_color = Color(0.4, 0.5, 1);
	_texture_mode = LINE_TEXTURE_NONE;
	_sharp_limit = 2.f;
	_round_precision = 8;
	
	reorient = false;
	use_outline = false;
	edit_tail = false;
	smooth = 0;
	interpolation = CUBIC;
	tale_interpolation = QUADRATIC_BEZIER;
	
	antialiased = false;
	tex_rot = 0;
	tex_tile = true;
	tex_scale = Vector2(1, 1);
	color = Color(1, 1, 1);
	rect_cache_dirty = true;
}
