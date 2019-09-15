#include "red_clipper.h"
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
#include "drivers/gles2/shaders/canvas.glsl.gen.h"
#include "scene/animation/animation_tree.h"
#include "scene/scene_string_names.h"

void REDClipper::clip() {
	if (clip_enable && material_objects.size()>0 && is_inside_tree()){
		_update_stencil();
		_send_stencil();
	}
}

void REDClipper::_update_materials() {
	int count = material_objects.size();
	cached_materials.clear();
	for (int i = 0; i < count; i++)
	{
		if (has_node(material_objects[i])){
			CanvasItem *c = (CanvasItem*)get_node(material_objects[i]);
			if (c != nullptr){
				Ref<ShaderMaterial> mat = Ref<ShaderMaterial>(c->get_material());
				if (mat.is_valid())
					cached_materials.push_back(mat);
			}
		}
	}
	second_split_start_material_id = cached_materials.size();
	count = material_objects2.size();
	for (int i = 0; i < count; i++)
	{
		if (has_node(material_objects2[i])){
			CanvasItem *c = (CanvasItem*)get_node(material_objects2[i]);
			if (c != nullptr){
				Ref<ShaderMaterial> mat = Ref<ShaderMaterial>(c->get_material());
				if (mat.is_valid())
					cached_materials.push_back(mat);
			}
		}
	}
}

void REDClipper::_send_stencil() {
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
	}
}

void REDClipper::_update_stencil() {
	int count = cached_materials.size();
	if (count == 0)
		return;
	int polygon_size = get_polygon().size();
	if (polygon_size!=3 && polygon_size!=4 && polygon_size!=8)
		return;

	screen_coords.resize(polygon_size);
	Transform2D tr;
	switch (space)
	{
	case WORLD:
		tr = get_global_transform();
		break;
	case SCREEN:
		tr = get_viewport_transform() * get_global_transform();
		break;
	default:
		tr = get_transform();
	}

	for (int i = 0; i < polygon_size; i++) {
		Vector2 screen_coord = tr.xform(get_polygon()[i]);
		screen_coords.write[i] = screen_coord;
	}
	if (space == SCREEN){
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
		int split_int = (int)split_angle;
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
}

void REDClipper::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_update_materials();
		} break;
		case NOTIFICATION_DRAW: {
			VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), clip_rect_enable);
			clip();
			if (get_use_outline()){
				Vector<Vector2> points;
				int count = get_polygon().size();
				points.resize(count);
				PoolVector<Vector2>::Read polyr = get_polygon().read();
				for (int i = 0; i < count; i++)
					points.write[i] = polyr[i] + get_offset();
				_draw_outline(points);
			}
		} break;
	}
}

void REDClipper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_clip_enable", "clip_enable"), &REDClipper::set_clip_enable);
    ClassDB::bind_method(D_METHOD("get_clip_enable"), &REDClipper::get_clip_enable);
	ClassDB::bind_method(D_METHOD("set_clip_rect_enable", "clip_rect_enable"), &REDClipper::set_clip_rect_enable);
    ClassDB::bind_method(D_METHOD("get_clip_rect_enable"), &REDClipper::get_clip_rect_enable);

	ClassDB::bind_method(D_METHOD("set_split", "split"), &REDClipper::set_split);
    ClassDB::bind_method(D_METHOD("get_split"), &REDClipper::get_split);

	ClassDB::bind_method(D_METHOD("set_split_angle", "split_angle"), &REDClipper::set_split_angle);
    ClassDB::bind_method(D_METHOD("get_split_angle"), &REDClipper::get_split_angle);

	ClassDB::bind_method(D_METHOD("set_split_offset", "split_offset"), &REDClipper::set_split_offset);
    ClassDB::bind_method(D_METHOD("get_split_offset"), &REDClipper::get_split_offset);

	ClassDB::bind_method(D_METHOD("set_space", "space"), &REDClipper::set_space);
    ClassDB::bind_method(D_METHOD("get_space"), &REDClipper::get_space);
	
	ClassDB::bind_method(D_METHOD("set_material_objects", "material_objects"), &REDClipper::set_material_objects);
    ClassDB::bind_method(D_METHOD("get_material_objects"), &REDClipper::get_material_objects);
	
	ClassDB::bind_method(D_METHOD("set_material_objects2", "material_objects2"), &REDClipper::set_material_objects2);
    ClassDB::bind_method(D_METHOD("get_material_objects2"), &REDClipper::get_material_objects2);
	
	ClassDB::bind_method(D_METHOD("update_stencil"), &REDClipper::_update_stencil);
	ClassDB::bind_method(D_METHOD("send_stencil"), &REDClipper::_send_stencil);

	ADD_GROUP("Clipper", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_enable"), "set_clip_enable", "get_clip_enable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_rect_enable"), "set_clip_rect_enable", "get_clip_rect_enable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "split"), "set_split", "get_split");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "split_angle"), "set_split_angle", "get_split_angle");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "split_offset"), "set_split_offset", "get_split_offset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "space", PROPERTY_HINT_ENUM, "World, Local, Screen"), "set_space", "get_space");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "material_objects", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDPolygon"), "set_material_objects", "get_material_objects");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "material_objects2", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDPolygon"), "set_material_objects2", "get_material_objects2");
	BIND_ENUM_CONSTANT(WORLD);
	BIND_ENUM_CONSTANT(LOCAL);
	BIND_ENUM_CONSTANT(SCREEN);
}
void REDClipper::set_split(bool p_split){
	split = p_split;
	_update_materials();
	update();
}
bool REDClipper::get_split() const{
	return split;
}

void REDClipper::set_split_angle(float p_split_angle){
	split_angle = p_split_angle;
	update();
}
float REDClipper::get_split_angle() const{
	return split_angle;
}

void REDClipper::set_split_offset(const Vector2 &p_split_offset){
	split_offset = p_split_offset;
	update();
}

Vector2 REDClipper::get_split_offset() const{
	return split_offset;
}

void REDClipper::set_clip_enable(bool p_clip){
	clip_enable = p_clip;
	update();
}

bool REDClipper::get_clip_enable() const{
	return clip_enable;
}

void REDClipper::set_clip_rect_enable(bool p_clip_rect_enable){
	clip_rect_enable = p_clip_rect_enable;
	update();
}

bool REDClipper::get_clip_rect_enable() const{
	return clip_rect_enable;
}

void REDClipper::set_space(REDClipper::Space p_space){
	space = p_space;
	clip();
}

REDClipper::Space REDClipper::get_space() const{
	return space;
}

void REDClipper::set_material_objects(const Array &p_material_objects) {
	int old_count = cached_materials.size();
	int count = p_material_objects.size();
	material_objects.resize(count);
	for (int i = 0; i < count; i++) {
		material_objects.write[i] = p_material_objects[i];
	}
	if (is_inside_tree() && count>0){
		_update_materials();
		if (old_count == 0)
			_update_stencil();
		_send_stencil();
	}
}

Array REDClipper::get_material_objects() const {
    Array material_objects_temp;
	int count = material_objects.size();
    material_objects_temp.resize(count);
    for (int i = 0; i < count; i++) {
        material_objects_temp[i] = material_objects[i];
    }
    return material_objects_temp;
}

void REDClipper::set_material_objects2(const Array &p_material_objects2) {
	int old_count = cached_materials.size();
	int count = p_material_objects2.size();
	material_objects2.resize(count);
	for (int i = 0; i < count; i++) {
		material_objects2.write[i] = p_material_objects2[i];
	}
	if (is_inside_tree() && count>0){
		_update_materials();
		if (old_count == 0)
			_update_stencil();
		_send_stencil();
	}
}
Array REDClipper::get_material_objects2() const {
    Array material_objects_temp;
	int count = material_objects2.size();
    material_objects_temp.resize(count);
    for (int i = 0; i < count; i++) {
        material_objects_temp[i] = material_objects2[i];
    }
    return material_objects_temp;
}

Vector<Ref<ShaderMaterial> > REDClipper::get_cached_materials() const {
    return cached_materials;
}

REDClipper::REDClipper() {
	clip_enable = true;
	clip_rect_enable = true;
	split = false;
	split_angle = 0;
	split_offset = Vector2(0, 0);
	space = WORLD;
}
