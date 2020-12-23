#include "red_clipper.h"
#include "scene/animation/animation_player.h"
#include "scene/animation/animation_tree.h"
#include "scene/animation/animation_node_state_machine.h"
#include "core/engine.h"

#include "red_page.h"
#include "red_engine.h"
#include "red.h"
#include "red_issue.h"
#include "red_shape.h"

#include "core/math/geometry.h"

#include "core/math/math_funcs.h"
#include "core/core_string_names.h"
#include <string>
#include "scene/animation/animation_tree.h"
#include "scene/scene_string_names.h"
#include "red_parallax_folder.h"
#include "red_target.h"

void REDClipper::_send_stencil() {
	if (!(send_stencil_dirty && clip_enable && is_inside_tree()))
		return;
	int pos_count = screen_coords.size();
	switch (pos_count){
	case 8:{

		VS::get_singleton()->clipper_set_points(ci, Vector3(multiple[0], constant[0], screen_coords[0].y),
													Vector3(multiple[1], constant[1], screen_coords[1].y),
													Vector3(multiple[2], constant[2], screen_coords[2].y),
													Vector3(multiple[3], constant[3], screen_coords[3].y));
	} break;
	case 4:{
		VS::get_singleton()->clipper_set_points(ci, Vector3(multiple[0], constant[0], screen_coords[0].y),
													Vector3(multiple[1], constant[1], screen_coords[1].y),
													Vector3(multiple[2], constant[2], screen_coords[2].y),
													Vector3(multiple[3], constant[3], screen_coords[3].y));
	}break;
	case 3:{
		VS::get_singleton()->clipper_set_points(ci, Vector3(multiple[0], constant[0], screen_coords[0].y),
													Vector3(multiple[1], constant[1], screen_coords[1].y),
													Vector3(multiple[2], constant[2], screen_coords[2].y),
													Vector3(multiple[2], constant[2], screen_coords[2].y));
	}break;
	default:
		break;
	}
	send_stencil_dirty = false;
	/*
	int count = cached_materials.size();
	int pos_count = screen_coords.size();
	for (int i = 0; i < count; i++){
		if (!cached_materials[i].is_valid()){
			cached_materials.remove(i);
			if ( i < second_split_start_material_id){
				second_split_start_material_id -= 1;
			}
		}
	}
	switch (pos_count){
	case 8:{
		for (int i = 0; i < second_split_start_material_id; i++){
			cached_materials.write[i]->set_shader_param("clipper_calc0", Vector3(multiple[0], constant[0], screen_coords[0].y));
			cached_materials.write[i]->set_shader_param("clipper_calc1", Vector3(multiple[1], constant[1], screen_coords[1].y));
			cached_materials.write[i]->set_shader_param("clipper_calc2", Vector3(multiple[2], constant[2], screen_coords[2].y));
			cached_materials.write[i]->set_shader_param("clipper_calc3", Vector3(multiple[3], constant[3], screen_coords[3].y));

			//cached_materials.write[i]->set_shader_param("clipper_pos0", Vector2(screen_coords[0].x, screen_coords[0].y));
			//cached_materials.write[i]->set_shader_param("clipper_pos1", Vector2(screen_coords[1].x, screen_coords[1].y));
			//cached_materials.write[i]->set_shader_param("clipper_pos2", Vector2(screen_coords[2].x, screen_coords[2].y));
			//cached_materials.write[i]->set_shader_param("clipper_pos3", Vector2(screen_coords[3].x, screen_coords[3].y));
		}
		for (int i = second_split_start_material_id; i < count; i++){
			cached_materials.write[i]->set_shader_param("clipper_calc0", Vector3(multiple[4], constant[4], screen_coords[4].y));
			cached_materials.write[i]->set_shader_param("clipper_calc1", Vector3(multiple[5], constant[5], screen_coords[5].y));
			cached_materials.write[i]->set_shader_param("clipper_calc2", Vector3(multiple[6], constant[6], screen_coords[6].y));
			cached_materials.write[i]->set_shader_param("clipper_calc3", Vector3(multiple[7], constant[7], screen_coords[7].y));
		
			//cached_materials.write[i]->set_shader_param("clipper_pos0", Vector2(screen_coords[4].x, screen_coords[4].y));
			//cached_materials.write[i]->set_shader_param("clipper_pos1", Vector2(screen_coords[5].x, screen_coords[5].y));
			//cached_materials.write[i]->set_shader_param("clipper_pos2", Vector2(screen_coords[6].x, screen_coords[6].y));
			//cached_materials.write[i]->set_shader_param("clipper_pos3", Vector2(screen_coords[7].x, screen_coords[7].y));
		}
	} break;
	case 4:
		for (int i = 0; i < count; i++){
			cached_materials.write[i]->set_shader_param("clipper_calc3", Vector3(multiple[3], constant[3], screen_coords[3].y));

			//cached_materials.write[i]->set_shader_param("clipper_pos3", Vector2(screen_coords[3].x, screen_coords[3].y));
		}
	case 3:
		for (int i = 0; i < count; i++){
			cached_materials.write[i]->set_shader_param("clipper_calc0", Vector3(multiple[0], constant[0], screen_coords[0].y));
			cached_materials.write[i]->set_shader_param("clipper_calc1", Vector3(multiple[1], constant[1], screen_coords[1].y));
			cached_materials.write[i]->set_shader_param("clipper_calc2", Vector3(multiple[2], constant[2], screen_coords[2].y));

			//cached_materials.write[i]->set_shader_param("clipper_pos0", Vector2(screen_coords[0].x, screen_coords[0].y));
			//cached_materials.write[i]->set_shader_param("clipper_pos1", Vector2(screen_coords[1].x, screen_coords[1].y));
			//cached_materials.write[i]->set_shader_param("clipper_pos2", Vector2(screen_coords[2].x, screen_coords[2].y));
		}
		break;
	default:
		break;
	}*/
}

void REDClipper::_update_stencil() {
	ERR_FAIL_COND_MSG(!shape, "No parent REDshape found!");
	if (!(stencil_dirty && clip_enable && is_inside_tree()))
		return;
	int polygon_size = shape->real_polygon.size();
	if (polygon_size != 3 && polygon_size != 4 && polygon_size != 8)
		return;

	screen_coords.resize(polygon_size);
	Transform2D tr;
	switch (space)
	{
	case CLIPPER_SPACE_WORLD:
		tr = get_global_transform();
		break;
	case CLIPPER_SPACE_SCREEN:
		tr = get_viewport_transform() * get_global_transform();
		break;
	default:
		tr = get_transform();
	}
	for (int i = 0; i < polygon_size; i++) {
		screen_coords.write[i] = tr.xform(shape->real_polygon[i]);
	}

	if (space == CLIPPER_SPACE_SCREEN){
		Size2 res = get_viewport()->get_size();
		for (int i = 0; i < polygon_size; i++) {
			Vector2 screen_coord = screen_coords[i]/res;
			screen_coord.y = 1.0f - screen_coord.y;
			screen_coords.write[i] = screen_coord;
		}
	}

	if (split && polygon_size == 4){
		Vector<Vector2> screen_coords_new;
		polygon_size = 8;
		screen_coords_new.resize(polygon_size);
		float k = fmod(split_angle + 0.5, 4);
    	if (k < 0)
        	k += 4;
		if (k < 1.0f){
			float offset = split_offset.x*(1-ABS(k*2-1))*0.5;
			screen_coords_new.write[0] = screen_coords[0];
			screen_coords_new.write[1] = Vector2::linear_interpolate(screen_coords[0], screen_coords[1], CLAMP(k + offset, 0, 1));
			screen_coords_new.write[2] = Vector2::linear_interpolate(screen_coords[2], screen_coords[3], CLAMP(k - offset, 0, 1));
			screen_coords_new.write[3] = screen_coords[3];

			screen_coords_new.write[4] = screen_coords[1];
			screen_coords_new.write[5] = screen_coords_new[1];
			screen_coords_new.write[6] = screen_coords_new[2];
			screen_coords_new.write[7] = screen_coords[2];
		}else if (k < 2.0f){
			k = k - 1;
			float offset = -split_offset.y*(1-ABS(k*2-1))*0.5;
			screen_coords_new.write[0] = screen_coords[1];
			screen_coords_new.write[1] = Vector2::linear_interpolate(screen_coords[1], screen_coords[2], CLAMP(k + offset, 0, 1));
			screen_coords_new.write[2] = Vector2::linear_interpolate(screen_coords[3], screen_coords[0], CLAMP(k - offset, 0, 1));
			screen_coords_new.write[3] = screen_coords[0];

			screen_coords_new.write[4] = screen_coords[2];
			screen_coords_new.write[5] = screen_coords_new[1];
			screen_coords_new.write[6] = screen_coords_new[2];
			screen_coords_new.write[7] = screen_coords[3];
		}else if (k < 3.0f){
			k = k - 2;
			float offset = split_offset.x*(1-ABS(k*2-1))*0.5;
			screen_coords_new.write[0] = screen_coords[1];
			screen_coords_new.write[1] = Vector2::linear_interpolate(screen_coords[0], screen_coords[1], CLAMP(k + offset, 0, 1));
			screen_coords_new.write[2] = Vector2::linear_interpolate(screen_coords[2], screen_coords[3], CLAMP(k - offset, 0, 1));
			screen_coords_new.write[3] = screen_coords[2];

			screen_coords_new.write[4] = screen_coords[0];
			screen_coords_new.write[5] = screen_coords_new[1];
			screen_coords_new.write[6] = screen_coords_new[2];
			screen_coords_new.write[7] = screen_coords[3];
		}else{
			k = k - 3;
			float offset = -split_offset.y*(1-ABS(k*2-1))*0.5;
			screen_coords_new.write[0] = screen_coords[2];
			screen_coords_new.write[1] = Vector2::linear_interpolate(screen_coords[1], screen_coords[2], CLAMP(k + offset, 0, 1));
			screen_coords_new.write[2] = Vector2::linear_interpolate(screen_coords[3], screen_coords[0], CLAMP(k - offset, 0, 1));
			screen_coords_new.write[3] = screen_coords[3];

			screen_coords_new.write[4] = screen_coords[1];
			screen_coords_new.write[5] = screen_coords_new[1];
			screen_coords_new.write[6] = screen_coords_new[2];
			screen_coords_new.write[7] = screen_coords[0];
		}
		screen_coords.clear();
		screen_coords = screen_coords_new;
	} if (polygon_size == 6){
		Vector<Vector2> screen_coords_new;
		polygon_size = 8;
		screen_coords_new.resize(polygon_size);
		screen_coords_new.write[0] = screen_coords[0];
		screen_coords_new.write[1] = screen_coords[1];
		screen_coords_new.write[2] = screen_coords[4];
		screen_coords_new.write[3] = screen_coords[5];
		screen_coords_new.write[4] = screen_coords[1];
		screen_coords_new.write[5] = screen_coords[2];
		screen_coords_new.write[6] = screen_coords[3];
		screen_coords_new.write[7] = screen_coords[4];
		screen_coords = screen_coords_new;
	}else if(polygon_size == 8){
		Vector2 temp = screen_coords[2];
		screen_coords.write[2] = screen_coords[6];
		screen_coords.write[6] = temp;
		temp = screen_coords[3];
		screen_coords.write[3] = screen_coords[7];
		screen_coords.write[7] = temp;
	}
	
	if (polygon_size == 8){
		multiple.resize(polygon_size);
		constant.resize(polygon_size);
		int j = 3;

		for(int i=0; i < 4; i++) {
			if(screen_coords[j].y == screen_coords[i].y) {
				multiple.write[i] = 0;
				constant.write[i] = screen_coords[i].x;
			}
			else {
				multiple.write[i] = (screen_coords[j].x - screen_coords[i].x) / (screen_coords[j].y - screen_coords[i].y);
				constant.write[i] = screen_coords[i].x - (screen_coords[i].y * screen_coords[j].x) / (screen_coords[j].y - screen_coords[i].y) +
														(screen_coords[i].y * screen_coords[i].x) / (screen_coords[j].y - screen_coords[i].y);
			}
			j=i; 
		}
		j = 7;
		for(int i=4; i < polygon_size; i++) {
			if(screen_coords[j].y == screen_coords[i].y) {
				multiple.write[i] = 0;
				constant.write[i] = screen_coords[i].x;
			}
			else {
				multiple.write[i] = (screen_coords[j].x - screen_coords[i].x) / (screen_coords[j].y - screen_coords[i].y);
				constant.write[i] = screen_coords[i].x - (screen_coords[i].y * screen_coords[j].x) / (screen_coords[j].y - screen_coords[i].y) +
														(screen_coords[i].y * screen_coords[i].x) / (screen_coords[j].y - screen_coords[i].y);
			}
			j=i; 
		}
	}
	else{
		int j = polygon_size - 1;
		multiple.resize(polygon_size);
		constant.resize(polygon_size);
		for(int i=0; i < polygon_size; i++) {
			if(screen_coords[j].y == screen_coords[i].y) {
				multiple.write[i] = 0;
				constant.write[i] = screen_coords[i].x;
			}
			else {
				multiple.write[i] = (screen_coords[j].x - screen_coords[i].x) / (screen_coords[j].y - screen_coords[i].y);
				constant.write[i] = screen_coords[i].x - (screen_coords[i].y * screen_coords[j].x) / (screen_coords[j].y - screen_coords[i].y) +
														(screen_coords[i].y * screen_coords[i].x) / (screen_coords[j].y - screen_coords[i].y);
			}
			j=i; 
		}
	}
	stencil_dirty = false;
	send_stencil_dirty = true;
}

void REDClipper::set_split(bool p_split){
	split = p_split;
	if (clip_enable){
		stencil_dirty = true;
		update();
	}
}

bool REDClipper::get_split() const{
	return split;
}

void REDClipper::set_split_angle(float p_split_angle){
	split_angle = p_split_angle;
	if (clip_enable){
		stencil_dirty = true;
		update();
	}
}

float REDClipper::get_split_angle() const{
	return split_angle;
}

void REDClipper::set_split_offset(const Vector2 &p_split_offset){
	split_offset = p_split_offset;
	if (clip_enable){
		stencil_dirty = true;
		update();
	}
}

Vector2 REDClipper::get_split_offset() const{
	return split_offset;
}

void REDClipper::set_clip_enable(bool p_clip){
	clip_enable = p_clip;
	set_notify_transform(p_clip);
	if (clip_enable){
		stencil_dirty = true;
		update();
	}
}

bool REDClipper::get_clip_enable() const{
	return clip_enable;
}

void REDClipper::set_clip_rect_enable(bool p_clip_rect_enable){
	if (clip_rect_enable == p_clip_rect_enable)
		return;
	clip_rect_enable = p_clip_rect_enable;
	VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), clip_rect_enable);
}

bool REDClipper::get_clip_rect_enable() const{
	return clip_rect_enable;
}

void REDClipper::set_space(REDClipper::Space p_space){
	space = p_space;
	if (clip_enable){
		stencil_dirty = true;
		update();
	}
}

REDClipper::Space REDClipper::get_space() const{
	return space;
}

RID REDClipper::get_ci() const{
	return ci;
}

void REDClipper::_draw() {
	VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), clip_rect_enable);
	ERR_FAIL_COND_MSG(!shape, "No parent REDshape found!");
	Vector<Color> colors;
	if (colors.size() == shape->real_polygon.size()) {
		colors.push_back(Color(1, 1, 1, 1));
	} else {
		colors.push_back(Color(1, 1, 1, 1));
	}
	VS::get_singleton()->canvas_item_add_polygon(get_canvas_item(), shape->real_polygon, colors);
	if (clip_enable){
		stencil_dirty = true;
		_update_stencil();
		_send_stencil();
	}
}

void REDClipper::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			_draw();
		}
		case NOTIFICATION_ENTER_TREE: {
			Node *parent = get_parent();
			for (int i = 0; i < 10; i++){
				shape = Object::cast_to<REDShape>(parent);
				if (shape){
					shape->set_content(this);
					break;
				}
				parent = get_parent();
			}
			call_deferred("_update_points");
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (shape){
				shape->set_content(NULL);
			}
			shape = NULL;
		} break;
		case NOTIFICATION_TRANSFORM_CHANGED:{
			if (is_inside_tree()){
				stencil_dirty = true;
				send_stencil_dirty = true;
				if (clip_enable){
					_update_stencil();
					_send_stencil();
				}
			}
		} break;
	}
}

void REDClipper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update_stencil"), &REDClipper::_update_stencil);
	ClassDB::bind_method(D_METHOD("send_stencil"), &REDClipper::_send_stencil);
	ClassDB::bind_method(D_METHOD("set_clip_enable", "clip_enable"), &REDClipper::set_clip_enable);
    ClassDB::bind_method(D_METHOD("get_clip_enable"), &REDClipper::get_clip_enable);
	ClassDB::bind_method(D_METHOD("set_clip_rect_enable", "clip_rect_enable"), &REDClipper::set_clip_rect_enable);
    ClassDB::bind_method(D_METHOD("get_clip_rect_enable"), &REDClipper::get_clip_rect_enable);
	ClassDB::bind_method(D_METHOD("set_space", "space"), &REDClipper::set_space);
    ClassDB::bind_method(D_METHOD("get_space"), &REDClipper::get_space);

	ClassDB::bind_method(D_METHOD("set_split", "split"), &REDClipper::set_split);
    ClassDB::bind_method(D_METHOD("get_split"), &REDClipper::get_split);
	ClassDB::bind_method(D_METHOD("set_split_angle", "split_angle"), &REDClipper::set_split_angle);
    ClassDB::bind_method(D_METHOD("get_split_angle"), &REDClipper::get_split_angle);
	ClassDB::bind_method(D_METHOD("set_split_offset", "split_offset"), &REDClipper::set_split_offset);
    ClassDB::bind_method(D_METHOD("get_split_offset"), &REDClipper::get_split_offset);
	
	ADD_GROUP("Clipper", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_enable"), "set_clip_enable", "get_clip_enable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_rect_enable"), "set_clip_rect_enable", "get_clip_rect_enable");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "space", PROPERTY_HINT_ENUM, "World, Local, Screen"), "set_space", "get_space");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "split"), "set_split", "get_split");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "split_angle"), "set_split_angle", "get_split_angle");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "split_offset"), "set_split_offset", "get_split_offset");
	BIND_ENUM_CONSTANT(CLIPPER_SPACE_WORLD);
	BIND_ENUM_CONSTANT(CLIPPER_SPACE_LOCAL);
	BIND_ENUM_CONSTANT(CLIPPER_SPACE_SCREEN);
}

REDClipper::REDClipper() {
	// Clipper
	ci = VS::get_singleton()->clipper_create();
	clip_enable = true;
	clip_rect_enable = false;
	space = CLIPPER_SPACE_WORLD;
	split = false;
	split_angle = 0;
	split_offset = Vector2(0, 0);
	stencil_dirty = true;
	send_stencil_dirty = true;
	set_notify_transform(true);
}
REDClipper::~REDClipper() {
	VS::get_singleton()->free(ci);
}