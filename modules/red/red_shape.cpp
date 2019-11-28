#include "red_shape.h"
#include "scene/animation/animation_player.h"
#include "scene/animation/animation_tree.h"
#include "scene/animation/animation_node_state_machine.h"
#include "core/engine.h"

#include "red_page.h"
#include "red_engine.h"
#include "red.h"
#include "red_issue.h"

#include "core/math/geometry.h"
#include "red_line.h"

#include "red_line_builder.h"
#include "core/math/math_funcs.h"
#include "core/core_string_names.h"
#include <string>
#include "scene/animation/animation_tree.h"
#include "scene/scene_string_names.h"

void REDShape::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			/*if (use_outline){
				Vector<Vector2> points;
				int count = get_polygon().size();
				points.resize(count);
				PoolVector<Vector2>::Read polyr = polygon.read();
				for (int i = 0; i < count; i++)
					points.write[i] = polyr[i] + get_offset();
				_draw_outline(points);
			}*/
		} break;
	}
}

void REDShape::set_use_outline(const bool b) {
    use_outline = b;
	update_outline = b;
	update();
}

bool REDShape::get_use_outline() const{
    return use_outline;

}

void REDShape::set_outline_width_constant(const bool p_outline_width_constant){
	outline_width_constant = p_outline_width_constant;
}
bool REDShape::is_outline_width_constant() const{
	return outline_width_constant;
}

void REDShape::set_width(float p_width) {
	if (p_width < 0.0)
		p_width = 0.0;
	width = p_width;
	update_outline = true;
	update();

}

float REDShape::get_width() const {
	return width;
}

void REDShape::set_width_curve(const Ref<Curve> &p_width_curve) {
	// Cleanup previous connection if any
	if (width_curve.is_valid()) {
		width_curve->disconnect(CoreStringNames::get_singleton()->changed, this, "_width_curve_changed");
	}

	width_curve = p_width_curve;

	// Connect to the width_curve so the line will update when it is changed
	if (width_curve.is_valid()) {
		width_curve->connect(CoreStringNames::get_singleton()->changed, this, "_width_curve_changed");
	}
	update_outline = true;
	update();
}

Ref<Curve> REDShape::get_width_curve() const {
	return width_curve;
}

void REDShape::set_default_color(Color p_color) {
	default_color = p_color;
	update_outline = true;
	update();
}

Color REDShape::get_default_color() const {
	return default_color;
}

void REDShape::set_gradient(const Ref<Gradient> &p_gradient) {

	// Cleanup previous connection if any
	if (gradient.is_valid()) {
		gradient->disconnect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}

	gradient = p_gradient;

	// Connect to the gradient so the line will update when the ColorRamp is changed
	if (gradient.is_valid()) {
		gradient->connect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}
	update_outline = true;
	update();
}

Ref<Gradient> REDShape::get_gradient() const {
	return gradient;
}

void REDShape::set_line_texture(const Ref<Texture> &p_texture) {
	texture = p_texture;
	update_outline = true;
	update();
}

Ref<Texture> REDShape::get_line_texture() const {
	return texture;
}

void REDShape::set_texture_mode(const LineTextureMode p_mode) {
	texture_mode = p_mode;
	update_outline = true;
	update();
}

REDShape::LineTextureMode REDShape::get_texture_mode() const {
	return texture_mode;
}

void REDShape::set_joint_mode(REDShape::LineJointMode p_mode) {
	joint_mode = p_mode;
	update_outline = true;
	update();
}

REDShape::LineJointMode REDShape::get_joint_mode() const {
	return joint_mode;
}

void REDShape::set_sharp_limit(float p_limit) {
	if (p_limit < 0.f)
		p_limit = 0.f;
	sharp_limit = p_limit;
	update_outline = true;
	update();
}

float REDShape::get_sharp_limit() const {
	return sharp_limit;
}

void REDShape::set_round_precision(int p_precision) {
	if (p_precision < 1)
		p_precision = 1;
	round_precision = p_precision;
	update_outline = true;
	update();
}

int REDShape::get_round_precision() const {
	return round_precision;
}

void REDShape::update_camera_zoom(Vector2 p_camera_zoom) {
	camera_zoom = p_camera_zoom;
	if (use_outline)
		update();
		//_draw_outline();
}

void REDShape::_draw_outline(Vector<Vector2> &p_points) {
	int len = p_points.size();

	/*Vector<Vector2> p_points;
	
	p_points.resize(len);
	{
		PoolVector<Vector2>::Read polyr = polygon.read();
		for (int i = 0; i < len; i++)
			p_points.write[i] = polyr[i] + offset;
	}
	*/
	if (p_points.size() <= 1 || width == 0.f)
		return;
	
	Vector<float> new_width_list;
	if (width_list.size() < 2){
        new_width_list.resize(len);
        for (int i = 0; i < len; i++) {
			new_width_list.write[i] = 1.f;
        }
	} else if (width_list.size() != len){
		int pre_a;
		int b;
		int post_b;
		int end=width_list.size()-1;
		float smooth_thickness_iter = len * 1.0f / width_list.size() - 1;
		PoolVector<float>::Read width_list_read = width_list.read();
		for(int i=0; i<width_list.size(); ++i){
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
			new_width_list.push_back(width_list_read[i]);
			for(int j=0; j<smooth_thickness_iter; ++j){
				new_width_list.push_back(width_list_read[i]+(j+1)*1.0/(smooth_thickness_iter+1)*(width_list_read[b]-width_list_read[i]));
			}
		}
	} else{
        new_width_list.resize(len);
        PoolVector<float>::Read width_list_read = width_list.read();
        for (int i = 0; i < len; i++) {
            new_width_list.write[i] = width_list_read[i];
        }
	}

	// TODO Maybe have it as member rather than copying parameters and allocating memory?
	REDLineBuilder lb;
	
	lb.points = p_points;
	lb.default_color = default_color;
	lb.gradient = *gradient;
	lb.texture_mode = static_cast<REDLine::LineTextureMode>(texture_mode);
	lb.joint_mode = static_cast<REDLine::LineJointMode>(joint_mode);
	lb.round_precision = round_precision;
	lb.sharp_limit = sharp_limit;
	if (outline_width_constant){
		lb.width = width * camera_zoom.x;
	}
	else{
		lb.width = width;
	}
	lb.width_curve = *width_curve;
    lb.is_closed = closed;
    lb.width_list = new_width_list;

	RID texture_rid;
	if (texture.is_valid()) {
		texture_rid = texture->get_rid();

		lb.tile_aspect = texture->get_size().aspect();
	}

	lb.build();

	VS::get_singleton()->canvas_item_add_triangle_array(
			get_canvas_item(),
			lb.indices,
			lb.vertices,
			lb.colors,
			lb.uvs, Vector<int>(), Vector<float>(),

			texture_rid);
	update_outline = false;
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

void REDShape::_gradient_changed() {
	update_outline = true;
	update();

}

void REDShape::_width_curve_changed() {
	update_outline = true;
	update();
}

void REDShape::set_width_list(const PoolVector<float> &p_width_list) {
    width_list = p_width_list;
	update_outline = true;
    update();
}

PoolVector<float> REDShape::get_width_list() const {
    return width_list;
}

Dictionary REDShape::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["offset"] = offset;
	return state;
}

void REDShape::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_offset(p_state["offset"]);
}

void REDShape::_edit_set_pivot(const Point2 &p_pivot) {
	set_position(get_transform().xform(p_pivot));
	set_offset(get_offset() - p_pivot);

	//PoolVector<Vector2>::Write w = polygon.write();
	//for (int i = 0; i < polygon.size(); i++)
	//{
	//	w[i] -= p_pivot;
	//}
	//set_position(get_transform().xform(p_pivot));
	//for (int i = 0; i < get_child_count(); i++)
	//{
	//	Node2D *node = Object::cast_to<Node2D>(get_child(i));
	//	if (node)
	//		node->set_position(node->get_position()-p_pivot);
	//}
}

Point2 REDShape::_edit_get_pivot() const {
	return Vector2();
}


bool REDShape::_edit_use_pivot() const {
	return true;
}

Rect2 REDShape::_edit_get_rect() const {
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

bool REDShape::_edit_use_rect() const {
	return polygon.size() > 0;
}

bool REDShape::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {

	Vector<Vector2> REDShape = Variant(polygon);
	return Geometry::is_point_in_polygon(p_point - get_offset(), REDShape);
}

void REDShape::set_polygon(const PoolVector<Vector2> &p_polygon) {
	polygon = p_polygon;
	rect_cache_dirty = true;
	update_outline = true;
	update();
}

PoolVector<Vector2> REDShape::get_polygon() const {

	return polygon;
}

void REDShape::set_antialiased(bool p_antialiased) {

	antialiased = p_antialiased;
	update_outline = true;
	update();
}
bool REDShape::get_antialiased() const {

	return antialiased;
}

void REDShape::set_closed(const bool p_closed) {
	closed = p_closed;
	update_outline = true;
	update();
}

bool REDShape::is_closed() const {
	return closed;
}

void REDShape::set_offset(const Vector2 &p_offset) {

	offset = p_offset;
	rect_cache_dirty = true;
	update_outline = true;
	update();
	_change_notify("offset");
}

Vector2 REDShape::get_offset() const {

	return offset;
}






void REDShape::_bind_methods() {
	//Line
	ClassDB::bind_method(D_METHOD("set_use_outline", "use_outline"), &REDShape::set_use_outline);
    ClassDB::bind_method(D_METHOD("get_use_outline"), &REDShape::get_use_outline);
	
	ClassDB::bind_method(D_METHOD("set_closed", "closed"), &REDShape::set_closed);
	ClassDB::bind_method(D_METHOD("is_closed"), &REDShape::is_closed);
	
	ClassDB::bind_method(D_METHOD("set_width_list", "width_list"), &REDShape::set_width_list);
    ClassDB::bind_method(D_METHOD("get_width_list"), &REDShape::get_width_list);

	ClassDB::bind_method(D_METHOD("set_width", "width"), &REDShape::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &REDShape::get_width);

	ClassDB::bind_method(D_METHOD("set_width_curve", "width_curve"), &REDShape::set_width_curve);
	ClassDB::bind_method(D_METHOD("get_width_curve"), &REDShape::get_width_curve);
	ClassDB::bind_method(D_METHOD("_width_curve_changed"), &REDShape::_width_curve_changed);
	
	ClassDB::bind_method(D_METHOD("set_default_color", "color"), &REDShape::set_default_color);
	ClassDB::bind_method(D_METHOD("get_default_color"), &REDShape::get_default_color);

	ClassDB::bind_method(D_METHOD("set_gradient", "color"), &REDShape::set_gradient);
	ClassDB::bind_method(D_METHOD("get_gradient"), &REDShape::get_gradient);
	ClassDB::bind_method(D_METHOD("_gradient_changed"), &REDShape::_gradient_changed);

	ClassDB::bind_method(D_METHOD("set_line_texture", "line_texture"), &REDShape::set_line_texture);
	ClassDB::bind_method(D_METHOD("get_line_texture"), &REDShape::get_line_texture);

	ClassDB::bind_method(D_METHOD("set_texture_mode", "mode"), &REDShape::set_texture_mode);
	ClassDB::bind_method(D_METHOD("get_texture_mode"), &REDShape::get_texture_mode);

	ClassDB::bind_method(D_METHOD("set_joint_mode", "mode"), &REDShape::set_joint_mode);
	ClassDB::bind_method(D_METHOD("get_joint_mode"), &REDShape::get_joint_mode);

	ClassDB::bind_method(D_METHOD("set_sharp_limit", "limit"), &REDShape::set_sharp_limit);
	ClassDB::bind_method(D_METHOD("get_sharp_limit"), &REDShape::get_sharp_limit);

	ClassDB::bind_method(D_METHOD("set_round_precision", "precision"), &REDShape::set_round_precision);
	ClassDB::bind_method(D_METHOD("get_round_precision"), &REDShape::get_round_precision);

	ClassDB::bind_method(D_METHOD("set_polygon", "polygon"), &REDShape::set_polygon);
	ClassDB::bind_method(D_METHOD("get_polygon"), &REDShape::get_polygon);

	ClassDB::bind_method(D_METHOD("set_antialiased", "antialiased"), &REDShape::set_antialiased);
	ClassDB::bind_method(D_METHOD("get_antialiased"), &REDShape::get_antialiased);

	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &REDShape::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &REDShape::get_offset);


	ADD_GROUP("Shape", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "polygon"), "set_polygon", "get_polygon");
	//Line
	ADD_GROUP("Line", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_outline"), "set_use_outline", "get_use_outline");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "closed"), "set_closed", "is_closed");
	//ADD_PROPERTY(PropertyInfo(Variant::BOOL, "antialiased"), "set_antialiased", "get_antialiased");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_color"), "set_default_color", "get_default_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "width_list"), "set_width_list", "get_width_list");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "width_width_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_width_curve", "get_width_curve");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "joint_mode", PROPERTY_HINT_ENUM, "Sharp,Bevel,Round"), "set_joint_mode", "get_joint_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharp_limit"), "set_sharp_limit", "get_sharp_limit");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round_precision"), "set_round_precision", "get_round_precision");

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "line_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_line_texture", "get_line_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_mode", PROPERTY_HINT_ENUM, "None,Tile,Stretch"), "set_texture_mode", "get_texture_mode");
    

	
	BIND_ENUM_CONSTANT(LINE_JOINT_SHARP);
	BIND_ENUM_CONSTANT(LINE_JOINT_BEVEL);
	BIND_ENUM_CONSTANT(LINE_JOINT_ROUND);

	BIND_ENUM_CONSTANT(LINE_TEXTURE_NONE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_TILE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_STRETCH);
}

REDShape::REDShape() {
	camera_zoom = Vector2(1.f, 1.0f);

	use_outline = true;
	update_outline = true;
	outline_width_constant = true;
	closed = true;

	antialiased = false;
	rect_cache_dirty = true;

	joint_mode = LINE_JOINT_SHARP;
	width = 5.0f;

	default_color = Color(0.0, 0.0, 0.0);
	texture_mode = LINE_TEXTURE_STRETCH;
	//texture = ResourceLoader::load("res://redot/textures/outline16.tres", "Texture");
	sharp_limit = 100.f;
	round_precision = 8;
}
