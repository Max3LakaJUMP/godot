#include "red_shape.h"
#include "scene/animation/animation_player.h"
#include "scene/animation/animation_tree.h"
#include "scene/animation/animation_node_state_machine.h"
#include "core/engine.h"

#include "red_page.h"
#include "red_engine.h"
#include "red.h"
#include "red_issue.h"
#include "red_outline.h"
#include "red_clipper.h"
#include "core/math/geometry.h"

#include "scene/2d/line_builder.h"
#include "core/math/math_funcs.h"
#include "core/core_string_names.h"
#include <string>
#include "scene/animation/animation_tree.h"
#include "scene/scene_string_names.h"
#include "core/math/math_funcs.h"

#ifdef TOOLS_ENABLED
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
#endif
// main
void REDShape::set_polygon(const PoolVector<Vector2> &p_polygon) {
	polygon = p_polygon;
	rect_cache_dirty = true;
	polygon_dirty = true;
	update();
}

PoolVector<Vector2> REDShape::get_polygon() const {
	return polygon;
}

void REDShape::set_offset(const Vector2 &p_offset) {
	offset = p_offset;
	rect_cache_dirty = true;
	polygon_dirty = true;
	update();
	_change_notify("offset");
}

Vector2 REDShape::get_offset() const {
	return offset;
}

Vector2  REDShape::get_camera_zoom() const{
	return camera_zoom;
};

void REDShape::update_camera_zoom(Vector2 p_camera_zoom) {
	camera_zoom = p_camera_zoom;
	if(outline_node && use_outline){
		outline_node->update();
	}
}
// content
void REDShape::update_content() {
	if(!is_inside_tree())
		return;
	if(!content_node)
		return;
	if(use_content && content_node->is_inside_tree())
		content_node->update();
}

void REDShape::set_content(REDClipper *p_content){
	content_node = p_content;
	update_content();
}

REDClipper *REDShape::get_content(){
	if(!content_node){
		bool create = true;
		for (int i = 0; i < get_child_count(); i++){
			content_node = Object::cast_to<REDClipper>(get_child(i));
			if(content_node){
				bool create = false;
				break;
			}
		}
		if(create){
			content_node = red::create_node<REDClipper>(this, "content");
		}
	}
	return content_node;
}

void REDShape::set_use_content(const bool b) {
    use_content = b;
	update_content();
}

bool REDShape::get_use_content() const{
    return use_content;
}

// line
void REDShape::update_outline() {
	if(!is_inside_tree())
		return;
	if(!outline_node)
		return;
	if(use_outline && outline_node->is_inside_tree())
		outline_node->update();
}

void REDShape::set_use_outline(const bool b) {
    use_outline = b;
	if(outline_node && use_outline)
		outline_node->update();
}

bool REDShape::get_use_outline() const{
    return use_outline;
}

void REDShape::set_outline(REDOutline *p_outline){
	outline_node = p_outline;
	update_outline();
}

REDOutline *REDShape::get_outline(){
	if(!outline_node){
		bool create = true;
		for (int i = 0; i < get_child_count(); i++){
			outline_node = Object::cast_to<REDOutline>(get_child(i));
			if(outline_node){
				bool create = false;
				break;
			}
		}
		if(create)
			outline_node = red::create_node<REDOutline>(this, "outline");
	}
	return outline_node;
}

void REDShape::set_width(float p_width) {
	if (p_width < 0.f)
		p_width = 0.f;
	width = p_width;
	update_outline();
}

float REDShape::get_width() const {
	return width;
}

void REDShape::set_width_list(const PoolVector<float> &p_width_list) {
    width_list = p_width_list;
	width_dirty = true;
	update_outline();
}

PoolVector<float> REDShape::get_width_list() const {
    return width_list;
}

void REDShape::set_outline_width_zoom_const(float p_outline_width_constant){
	outline_width_zoom_const = p_outline_width_constant;
	update_outline();
}

float REDShape::get_outline_width_zoom_const() const {
	return outline_width_zoom_const;
}

void REDShape::set_line_color(Color p_color) {
	line_color = p_color;
	update_outline();
}

Color REDShape::get_line_color() const {
	return line_color;
}

void REDShape::set_line_texture(const Ref<Texture> &p_texture) {
	texture = p_texture;
	update_outline();
}

Ref<Texture> REDShape::get_line_texture() const {
	return texture;
}

void REDShape::set_texture_mode(const Line2D::LineTextureMode p_mode) {
	texture_mode = p_mode;
	update_outline();
}

Line2D::LineTextureMode REDShape::get_texture_mode() const {
	return texture_mode;
}

void REDShape::set_joint_mode(Line2D::LineJointMode p_mode) {
	joint_mode = p_mode;
	update_outline();
}

Line2D::LineJointMode REDShape::get_joint_mode() const {
	return joint_mode;
}

void REDShape::set_antialiased(bool p_antialiased) {
	antialiased = p_antialiased;
	update_outline();
}

bool REDShape::get_antialiased() const {
	return antialiased;
}

void REDShape::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_INTERNAL_PROCESS: {
			_move_points(get_process_delta_time());
		} break;
		case NOTIFICATION_DRAW: {
			if (!is_inside_tree())
				return;
			bool poly_updated = false;
			bool width_updated = false;
			if (polygon_dirty && (use_content || use_outline)){
				calc_polygon();
				poly_updated = true;
			}
			if(width_dirty && use_outline){
				calc_width_list();
				width_updated = true;
			}
			if (poly_updated){
				update_content();
				update_outline();
			}
			else if (width_updated){
				update_outline();
			}
		} break;
	}
}

// deformation
void REDShape::_move_points(const float deltatime){
	int count = get_polygon().size();
	RandomPCG randbase;
	randbase.randomize();
	int width_offsets_count = width_offsets_state.size();
	if (width_offsets_count == count){
		for (int i = 0; i < count; i++){
			if (deformation_state == DEFORMATION_END){
				width_timer = deltatime;
				width_offsets_old.write[i] = width_offsets[i];
				width_offsets.write[i] = width_offsets_old[i] + width_timer*2.0f * (1.0f - width_offsets_old[i]);
			}else if (deformation_state == DEFORMATION_ENDING){
				width_offsets.write[i] = width_offsets_old[i] + width_timer*2.0f * (1.0f - width_offsets_old[i]);
			}else{
				width_offsets.write[i] = width_offsets_old[i] + width_timer * ((sin(width_offsets_state[i]) + 1) * (deformation_width_max-deformation_width_factor) / 2.0 + deformation_width_factor - width_offsets_old[i]);
			}
		}
	}else{
		width_timer = 0.0f;
		width_offsets.resize(count);
		width_offsets_state.resize(count);
		width_offsets_old.resize(count);
		for (int i = 0; i < count; i++){
			width_offsets_old.write[i] = 1.0f;
			width_offsets.write[i] = 1.0f;
			width_offsets_state.write[i] = i * (1.5708 / count) * 4.0;
		}
	} 

	width_timer += deltatime;
	if (width_timer > 1.0f){
		width_timer = deltatime;
		for (int i = 0; i < count; i++){
			width_offsets_old.write[i] = width_offsets[i];
			width_offsets_state.write[i] = width_offsets_state[i] + (1.5708 / count)*4.0;
		}
	}

	int offsets_count = offsets.size();
	if (offsets_count == count){
		for (int i = 0; i < count; i++){
			if (deformation_state == DEFORMATION_END){
				targets_old.write[i] = offsets[i];
				targets.write[i] = Vector2(0, 0);
				timers.write[i] = deltatime;
				times.write[i] = 0.5f;
				offsets.write[i] = targets_old[i];
			}else if (timers[i] <= times[i]){
				offsets.write[i] = targets_old[i].linear_interpolate(targets[i], timers[i] / times[i]);
				timers.write[i] += deltatime;
			}else if (deformation_state == DEFORMATION_ENDING){
				deformation_state = DEFORMATION_ENDED;
				break;
			} else{
				targets_old.write[i] = targets[i];
				targets.write[i] = Vector2(randbase.random(-deformation_offset, deformation_offset), randbase.random(-deformation_offset, deformation_offset));
				timers.write[i] = deltatime;
				times.write[i] = targets_old[i].distance_to(targets[i]) / deformation_speed;
				offsets.write[i] = targets_old[i];
			}
		}
	}else{
		targets_old.resize(count);
		targets.resize(count);
		timers.resize(count);
		times.resize(count);
		offsets.resize(count);
		for (int i = 0; i < count; i++){
			targets_old.write[i] = Vector2(0, 0);
			targets.write[i] = Vector2(randbase.random(-deformation_offset, deformation_offset), randbase.random(-deformation_offset, deformation_offset));
			timers.write[i] = deltatime;
			times.write[i] = targets_old[i].distance_to(targets[i]) / deformation_speed;
			offsets.write[i] = targets_old[i];
		}
	} 
	if (deformation_state == DEFORMATION_END){
		deformation_state = DEFORMATION_ENDING;
	}else if (deformation_state == DEFORMATION_ENDED){
		for (int i = 0; i < count; i++){
			offsets.write[i] = Vector2(0, 0);
			width_offsets_old.write[i] = 1.0f;
			width_timer = 0.0f;
		}
		set_process_internal(false);
	}
	polygon_dirty = true;
	width_dirty = true;
	update();
}

void REDShape::set_deformation_width_max(const float p_deformation_width_max){
	if (p_deformation_width_max < deformation_width_factor)
		return;
	deformation_width_max = p_deformation_width_max;
}

float REDShape::get_deformation_width_max() const{
	return deformation_width_max;
}

void REDShape::set_deformation_width_factor(const float p_deformation_width_factor){
	if (p_deformation_width_factor < 0.0 || p_deformation_width_factor > deformation_width_max)
		return;
	deformation_width_factor = p_deformation_width_factor;
}

float REDShape::get_deformation_width_factor() const{
	return deformation_width_factor;
}

void REDShape::set_deformation_speed(const float p_deformation_speed){
	if (deformation_speed == p_deformation_speed)
		return;
	deformation_speed = p_deformation_speed;
	if (get_deformation_enable()){
		polygon_dirty = true;
		width_dirty = true;
		update();
	}
}

float REDShape::get_deformation_speed() const{
	return deformation_speed;
}

void REDShape::set_deformation_offset(float p_deformation_offset){
	if (deformation_offset == p_deformation_offset)
		return;
	deformation_offset = p_deformation_offset;
	if (get_deformation_enable()){
		polygon_dirty = true;
		width_dirty = true;
		update();
	}
}

float REDShape::get_deformation_offset() const{
	return deformation_offset;
}

void REDShape::set_deformation_enable(bool p_deformate){
	if (p_deformate){
		deformation_state = DEFORMATION_NORMAL;
		set_process_internal(true);
	}else{
		deformation_state = DEFORMATION_END;
	}
}

bool REDShape::get_deformation_enable() const{
	return (deformation_state == DEFORMATION_NORMAL);
}

void REDShape::calc_width_list(){
	if (real_polygon.size() <= 1 || width == 0.f)
		return;
	int len = real_polygon.size();
	if (width_offsets.size() != len){
		width_offsets.resize(len);
		for (int i = 0; i < len; i++) {
			width_offsets.write[i] = 1.0f;
		}
	}
	if (width_list.size() < 2){
		real_width_list.resize(len);
		for (int i = 0; i < len; i++) {
			real_width_list.write[i] = 1.f * width_offsets[i];
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
			real_width_list.push_back(width_list_read[i]);
			for(int j=0; j<smooth_thickness_iter; ++j){
				real_width_list.push_back((width_list_read[i]+(j+1)*1.0/(smooth_thickness_iter+1)*(width_list_read[b]-width_list_read[i])) * width_offsets[real_width_list.size()]);
			}
		}
	} else{
		real_width_list.resize(len);
		PoolVector<float>::Read width_list_read = width_list.read();
		for (int i = 0; i < len; i++) {
			real_width_list.write[i] = width_list_read[i] * width_offsets[i];
		}
	}
}

void REDShape::calc_polygon(){
	PoolVector<Vector2>::Read polyr = polygon.read(); 
	int count = polygon.size();
	real_polygon.resize(count);
	if (deformation_state == DEFORMATION_ENDED){
		for (int i = 0; i < count; i++)
			real_polygon.write[i] = polyr[i] + get_offset();
	}else{
		int offsets_count = offsets.size();
		if (offsets_count == count){
			for (int i = 0; i < count; i++)
				real_polygon.write[i] = polyr[i] + get_offset() + offsets[i];
		}else{
			for (int i = 0; i < count; i++)
				real_polygon.write[i] = polyr[i] + get_offset();
		}
	}
}

void REDShape::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_polygon", "polygon"), &REDShape::set_polygon);
	ClassDB::bind_method(D_METHOD("get_polygon"), &REDShape::get_polygon);
	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &REDShape::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &REDShape::get_offset);
	ClassDB::bind_method(D_METHOD("set_use_content", "use_content"), &REDShape::set_use_content);
    ClassDB::bind_method(D_METHOD("get_use_content"), &REDShape::get_use_content);
	//Line
	ClassDB::bind_method(D_METHOD("set_use_outline", "use_outline"), &REDShape::set_use_outline);
    ClassDB::bind_method(D_METHOD("get_use_outline"), &REDShape::get_use_outline);
	ClassDB::bind_method(D_METHOD("set_width", "width"), &REDShape::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &REDShape::get_width);
	ClassDB::bind_method(D_METHOD("set_width_list", "width_list"), &REDShape::set_width_list);
    ClassDB::bind_method(D_METHOD("get_width_list"), &REDShape::get_width_list);
	ClassDB::bind_method(D_METHOD("set_outline_width_zoom_const", "outline_width_zoom_const"), &REDShape::set_outline_width_zoom_const);
    ClassDB::bind_method(D_METHOD("get_outline_width_zoom_const"), &REDShape::get_outline_width_zoom_const);
	// ClassDB::bind_method(D_METHOD("set_width_curve", "width_curve"), &REDShape::set_width_curve);
	// ClassDB::bind_method(D_METHOD("get_width_curve"), &REDShape::get_width_curve);
	// ClassDB::bind_method(D_METHOD("_width_curve_changed"), &REDShape::_width_curve_changed);
	ClassDB::bind_method(D_METHOD("set_line_color", "color"), &REDShape::set_line_color);
	ClassDB::bind_method(D_METHOD("get_line_color"), &REDShape::get_line_color);
	// ClassDB::bind_method(D_METHOD("set_gradient", "color"), &REDShape::set_gradient);
	// ClassDB::bind_method(D_METHOD("get_gradient"), &REDShape::get_gradient);
	ClassDB::bind_method(D_METHOD("set_line_texture", "line_texture"), &REDShape::set_line_texture);
	ClassDB::bind_method(D_METHOD("get_line_texture"), &REDShape::get_line_texture);
	ClassDB::bind_method(D_METHOD("set_texture_mode", "mode"), &REDShape::set_texture_mode);
	ClassDB::bind_method(D_METHOD("get_texture_mode"), &REDShape::get_texture_mode);
	ClassDB::bind_method(D_METHOD("set_joint_mode", "mode"), &REDShape::set_joint_mode);
	ClassDB::bind_method(D_METHOD("get_joint_mode"), &REDShape::get_joint_mode);
	// ClassDB::bind_method(D_METHOD("set_sharp_limit", "limit"), &REDShape::set_sharp_limit);
	// ClassDB::bind_method(D_METHOD("get_sharp_limit"), &REDShape::get_sharp_limit);
	// ClassDB::bind_method(D_METHOD("set_round_precision", "precision"), &REDShape::set_round_precision);
	// ClassDB::bind_method(D_METHOD("get_round_precision"), &REDShape::get_round_precision);
	// ClassDB::bind_method(D_METHOD("_gradient_changed"), &REDShape::_gradient_changed);
	ClassDB::bind_method(D_METHOD("set_antialiased", "antialiased"), &REDShape::set_antialiased);
	ClassDB::bind_method(D_METHOD("get_antialiased"), &REDShape::get_antialiased);
	// deformation
	ClassDB::bind_method(D_METHOD("set_deformation_enable", "deformation_enable"), &REDShape::set_deformation_enable);
    ClassDB::bind_method(D_METHOD("get_deformation_enable"), &REDShape::get_deformation_enable);
	ClassDB::bind_method(D_METHOD("set_deformation_offset", "deformation_offset"), &REDShape::set_deformation_offset);
    ClassDB::bind_method(D_METHOD("get_deformation_offset"), &REDShape::get_deformation_offset);
	ClassDB::bind_method(D_METHOD("set_deformation_speed", "deformation_speed"), &REDShape::set_deformation_speed);
    ClassDB::bind_method(D_METHOD("get_deformation_speed"), &REDShape::get_deformation_speed);
	ClassDB::bind_method(D_METHOD("set_deformation_width_factor", "deformation_width_factor"), &REDShape::set_deformation_width_factor);
    ClassDB::bind_method(D_METHOD("get_deformation_width_factor"), &REDShape::get_deformation_width_factor);
	ClassDB::bind_method(D_METHOD("set_deformation_width_max", "deformation_width_max"), &REDShape::set_deformation_width_max);
    ClassDB::bind_method(D_METHOD("get_deformation_width_max"), &REDShape::get_deformation_width_max);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_content"), "set_use_content", "get_use_content");
	ADD_GROUP("Deformation", "deformation_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "deformation_enable"), "set_deformation_enable", "get_deformation_enable");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_offset"), "set_deformation_offset", "get_deformation_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_speed"), "set_deformation_speed", "get_deformation_speed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_width_factor"), "set_deformation_width_factor", "get_deformation_width_factor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_width_max"), "set_deformation_width_max", "get_deformation_width_max");
	ADD_GROUP("Shape", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "polygon"), "set_polygon", "get_polygon");
	//Line
	ADD_GROUP("Line", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_outline"), "set_use_outline", "get_use_outline");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "width_list"), "set_width_list", "get_width_list");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "outline_width_zoom_const"), "set_outline_width_zoom_const", "get_outline_width_zoom_const");
	//ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "width_width_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_width_curve", "get_width_curve");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "line_color"), "set_line_color", "get_line_color");
	//ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "line_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_line_texture", "get_line_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_mode", PROPERTY_HINT_ENUM, "None,Tile,Stretch"), "set_texture_mode", "get_texture_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "joint_mode", PROPERTY_HINT_ENUM, "Sharp,Bevel,Round"), "set_joint_mode", "get_joint_mode");
	// ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharp_limit"), "set_sharp_limit", "get_sharp_limit");
	// ADD_PROPERTY(PropertyInfo(Variant::INT, "round_precision"), "set_round_precision", "get_round_precision");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "antialiased"), "set_antialiased", "get_antialiased");
}

REDShape::REDShape() {
	// main
	polygon_dirty = true;
	rect_cache_dirty = true;
	// content
	content_node = NULL;
	use_content = true;
	// line
	outline_node = NULL;
	use_outline = true;
	width_dirty = true;
	width = 5.0f;
	line_color = Color(0.0, 0.0, 0.0);
	texture_mode = Line2D::LINE_TEXTURE_STRETCH;
	joint_mode = Line2D::LINE_JOINT_SHARP;
	// sharp_limit = 100.f;
	// round_precision = 8;
	outline_width_zoom_const = 1.0f;
	antialiased = false;
	// deformation
	deformation_state= DEFORMATION_ENDED;
	deformation_offset = 20.0f;
	deformation_speed = 20.0f;
	deformation_enable = false;
	deformation_width_factor = 0.5f;
	deformation_width_max = 1.25f;
	camera_zoom = Vector2(1.f, 1.f);
	width = 5.0f;
}
