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
#include "scene/animation/animation_tree.h"
#include "scene/scene_string_names.h"
#include "core/math/random_pcg.h"



void REDClipper::_move_points(const float deltatime){
	int count = get_polygon().size();
	int offsets_count = offsets.size();
	RandomPCG randbase;
	randbase.randomize();
	if (offsets_count == count){
		for (int i = 0; i < count; i++){
			if (deformation_state = DEFORMATION_END){
				deformation_state = DEFORMATION_ENDING;
				targets_old.write[i] = targets[i].linear_interpolate(targets_old[i], (timers[i]) / times[i]);
				targets.write[i] = Vector2(0.0, 0.0);
				timers.write[i] = deltatime;
				times.write[i] = 0.5f;
				offsets.write[i] = targets_old[i];
			}else if (timers[i] <= times[i]){
				offsets.write[i] = targets_old[i].linear_interpolate(targets[i], (timers[i]) / times[i]);
				timers.write[i] += deltatime;
			}else{
				if (deformation_state = DEFORMATION_ENDING){
					deformation_state = DEFORMATION_ENDED;
					break;
				}
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
	
	if (deformation_state = DEFORMATION_ENDED){
		for (int i = 0; i < count; i++){
			offsets.write[i] = Vector2(0, 0);
		}
		deformation_shape = false;
	}
	outline_dirty = true;
	stencil_dirty = true;
	update();
}

void REDClipper::set_deformation_speed(float p_deformation_speed){
	if (deformation_speed == p_deformation_speed)
		return;
	deformation_speed = p_deformation_speed;
	if (deformation_shape){
		outline_dirty = true;
		stencil_dirty = true;
		update();
	}
}

float REDClipper::get_deformation_speed() const{
	return deformation_speed;
}

void REDClipper::set_deformation_offset(float p_deformation_offset){
	if (deformation_offset == p_deformation_offset)
		return;
	deformation_offset = p_deformation_offset;
	if (deformation_shape){
		outline_dirty = true;
		stencil_dirty = true;
		update();
	}
}

float REDClipper::get_deformation_offset() const{
	return deformation_offset;
}

void REDClipper::set_deformation_shape(bool p_deformate){
	if (deformation_shape == p_deformate)
		return;
	if (p_deformate){
		deformation_state = DEFORMATION_NORMAL;
		deformation_shape = true;
	}else{
		deformation_state = DEFORMATION_END;
	}
	outline_dirty = true;
	stencil_dirty = true;
	update();
}

bool REDClipper::get_deformation_shape() const{
	return deformation_shape;
}

Vector<Vector2> REDClipper::get_offsets(){
	return offsets;
}

void REDClipper::get_points(Vector<Vector2> &p_points) const{
	PoolVector<Vector2>::Read polyr = get_polygon().read();
	int count = get_polygon().size();
	p_points.resize(count);
	if (deformation_state){
		int offsets_count = offsets.size();
		if (offsets_count == count){
			for (int i = 0; i < count; i++)
				p_points.write[i] = polyr[i] + get_offset() + offsets[i];
			return;
		}
	}
	for (int i = 0; i < count; i++)
		p_points.write[i] = polyr[i] + get_offset();
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
	if (!(send_stencil_dirty&&clip_enable&&material_objects.size()>0 && is_inside_tree()))
		return;
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


void REDClipper::set_rotation1(const Vector2 &p_rotation){
	rotation1 = p_rotation;
	update();
	if (rotation_enable){
		send_rotation_dirty = true;
		update();
	}
}
Vector2 REDClipper::get_rotation1() const{
	return rotation1;
}
void REDClipper::set_rotation2(const Vector2 &p_rotation){
	rotation2 = p_rotation;
	if (rotation_enable){
		send_rotation_dirty = true;
		update();
	}
}
Vector2 REDClipper::get_rotation2() const{
	return rotation2;
}

void REDClipper::_send_rotation() {
	if (!(send_rotation_dirty && rotation_enable && material_objects.size()>0 && is_inside_tree())){
		return;
	}
	send_rotation_dirty = false;
	int count = cached_materials.size();
	for (int i = 0; i < count; i++){
		if (!cached_materials[i].is_valid()){
			cached_materials.remove(i);
			if ( i < second_split_start_material_id){
				second_split_start_material_id -= 1;
			}
		}
	}
	for (int i = 0; i < second_split_start_material_id; i++){
		cached_materials.write[i]->set_shader_param("global_rotation", rotation1);
	}
	for (int i = second_split_start_material_id; i < count; i++){
		cached_materials.write[i]->set_shader_param("global_rotation", rotation2);
	}
}

void REDClipper::_update_stencil(const Vector<Vector2> &p_points) {
	if (!(stencil_dirty&&clip_enable&&material_objects.size()>0 && is_inside_tree()))
		return;
	stencil_dirty = false;
	send_stencil_dirty = true;
	int count = cached_materials.size();
	if (count == 0)
		return;
	int polygon_size = p_points.size();
	if (polygon_size != 3 && polygon_size != 4 && polygon_size != 8)
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
	int offsets_count = offsets.size();
	for (int i = 0; i < polygon_size; i++) {
		screen_coords.write[i] = tr.xform(p_points[i]);
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
		case NOTIFICATION_PROCESS: {
			if (deformation_shape){
				_move_points(get_process_delta_time());
			}
		} break;
		case NOTIFICATION_TRANSFORM_CHANGED:{
			if (material_objects.size()>0 && is_inside_tree()){
				if (clip_enable){
					Vector<Vector2> points;
					get_points(points);
					_update_stencil(points);
					_send_stencil();
				}
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			set_notify_transform(clip_enable);
		} break;
		case NOTIFICATION_READY: {
			_update_materials();
		} break;
		case NOTIFICATION_DRAW: {
			VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), clip_rect_enable);
			if ((stencil_dirty && clip_enable) || (outline_dirty && get_use_outline())){
				Vector<Vector2> points;
				get_points(points);
				if (material_objects.size()>0 && is_inside_tree()){
					_update_stencil(points);
					_send_stencil();
				}
				_draw_outline(points);
			}
			_send_rotation();	
		} break;
	}
}

void REDClipper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_rotation_enable", "rotation_enable"), &REDClipper::set_rotation_enable);
    ClassDB::bind_method(D_METHOD("get_rotation_enable"), &REDClipper::get_rotation_enable);
	ClassDB::bind_method(D_METHOD("set_clip_enable", "clip_enable"), &REDClipper::set_clip_enable);
    ClassDB::bind_method(D_METHOD("get_clip_enable"), &REDClipper::get_clip_enable);
	ClassDB::bind_method(D_METHOD("set_clip_rect_enable", "clip_rect_enable"), &REDClipper::set_clip_rect_enable);
    ClassDB::bind_method(D_METHOD("get_clip_rect_enable"), &REDClipper::get_clip_rect_enable);

	ClassDB::bind_method(D_METHOD("set_rotation1", "rotation1"), &REDClipper::set_rotation1);
    ClassDB::bind_method(D_METHOD("get_rotation1"), &REDClipper::get_rotation1);
	ClassDB::bind_method(D_METHOD("set_rotation2", "rotation2"), &REDClipper::set_rotation2);
    ClassDB::bind_method(D_METHOD("get_rotation2"), &REDClipper::get_rotation2);

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

	ClassDB::bind_method(D_METHOD("set_deformation_shape", "deformation_shape"), &REDClipper::set_deformation_shape);
    ClassDB::bind_method(D_METHOD("get_deformation_shape"), &REDClipper::get_deformation_shape);
	ClassDB::bind_method(D_METHOD("set_deformation_offset", "deformation_offset"), &REDClipper::set_deformation_offset);
    ClassDB::bind_method(D_METHOD("get_deformation_offset"), &REDClipper::get_deformation_offset);
	ClassDB::bind_method(D_METHOD("set_deformation_speed", "deformation_speed"), &REDClipper::set_deformation_speed);
    ClassDB::bind_method(D_METHOD("get_deformation_speed"), &REDClipper::get_deformation_speed);
	
	ADD_GROUP("Deformation", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "deformation_shape"), "set_deformation_shape", "get_deformation_shape");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_offset"), "set_deformation_offset", "get_deformation_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_speed"), "set_deformation_speed", "get_deformation_speed");

	ADD_GROUP("Rotator", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rotation_enable"), "set_rotation_enable", "get_rotation_enable");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "rotation1"), "set_rotation1", "get_rotation1");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "rotation2"), "set_rotation2", "get_rotation2");
	ADD_GROUP("Clipper", "");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_enable"), "set_clip_enable", "get_clip_enable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_rect_enable"), "set_clip_rect_enable", "get_clip_rect_enable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "split"), "set_split", "get_split");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "split_angle"), "set_split_angle", "get_split_angle");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "split_offset"), "set_split_offset", "get_split_offset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "space", PROPERTY_HINT_ENUM, "World, Local, Screen"), "set_space", "get_space");
	ADD_GROUP("Materials", "");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "material_objects", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDPolygon"), "set_material_objects", "get_material_objects");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "material_objects2", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDPolygon"), "set_material_objects2", "get_material_objects2");


	BIND_ENUM_CONSTANT(WORLD);
	BIND_ENUM_CONSTANT(LOCAL);
	BIND_ENUM_CONSTANT(SCREEN);
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

void REDClipper::set_rotation_enable(bool p_rotation){
	rotation_enable = p_rotation;
	if(rotation_enable){
		send_rotation_dirty = true;
		update();
	}
}

bool REDClipper::get_rotation_enable() const{
	return rotation_enable;
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
	clip_rect_enable = p_clip_rect_enable;
	update();
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

void REDClipper::set_material_objects(const Array &p_material_objects) {
	int old_count = cached_materials.size();
	int count = p_material_objects.size();
	material_objects.resize(count);
	for (int i = 0; i < count; i++) {
		material_objects.write[i] = p_material_objects[i];
	}
	
	if (is_inside_tree() && count>0){
		send_stencil_dirty = true;
		send_rotation_dirty = true;
		_update_materials();
		if (old_count == 0)
			stencil_dirty = true;
		update();
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
		send_stencil_dirty = true;
		send_rotation_dirty = true;
		_update_materials();
		if (old_count == 0)
			stencil_dirty = true;
		update();
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
	deformation_state= DEFORMATION_NORMAL;
	deformation_offset = 200.0f;
	deformation_speed = 200.0f;
	deformation_shape = false;
	outline_dirty = true;
	send_stencil_dirty = true;
	send_rotation_dirty = true;
	materials_dirty = true;
	stencil_dirty = true;
	deformation_shape = true;
	rotation1 = Vector2(0, 0);
	rotation2 = Vector2(0, 0);
	rotation_enable = true;
	clip_enable = true;
	clip_rect_enable = true;
	split = false;
	split_angle = 0;
	split_offset = Vector2(0, 0);
	space = WORLD;
}
