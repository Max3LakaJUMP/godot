#include "red_shape.h"

#include "core/math/math_funcs.h"
#include "core/core_string_names.h"
#include "scene/scene_string_names.h"
#include "core/math/math_funcs.h"

#include "red_engine.h"
#include "red.h"
#include "red_issue.h"
#include "red_shape_renderer.h"
#include "core/math/geometry.h"

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
void REDShape::update_renderers() {
	if(!is_inside_tree())
		return;
	int count = render_nodes.size();
	for (size_t i = 0; i < render_nodes.size(); i++){
		REDShapeRenderer *renderer = render_nodes[i];
		if(renderer){
			if(!renderer->is_inside_tree())
				continue;
			renderer->update();
		}
	}
}

void REDShape::update_polygon_renderers() {
	if(!is_inside_tree())
		return;
	int count = render_nodes.size();
	for (size_t i = 0; i < count; i++){
		REDShapeRenderer *renderer = render_nodes[i];
		if(renderer && (renderer->get_render_mode() == REDShapeRenderer::RENDER_POLYGON)){
			if(!renderer->is_inside_tree())
				continue;
			renderer->update();
		}
	}
}

void REDShape::update_line_renderers() {
	if(!is_inside_tree())
		return;
	int count = render_nodes.size();
	for (size_t i = 0; i < render_nodes.size(); i++){
		REDShapeRenderer *renderer = render_nodes[i];
		if(renderer && (renderer->get_render_mode() == REDShapeRenderer::RENDER_LINE)){
			if(!renderer->is_inside_tree())
				continue;
			renderer->update();
		}
	}
}

void REDShape::update_root() {
	if(!root_shape)
		return;
	if(!is_inside_tree())
		return;
	if(root_shape->is_inside_tree()){
		root_shape->boolean_dirty = true;
		root_shape->width_dirty = true;
		root_shape->update();
	}
}

void REDShape::set_boolean(REDShape::Boolean p_boolean){
	if(boolean == p_boolean)
		return;
	if(bool(root_shape) && is_inside_tree() && root_shape->is_inside_tree()){
		if (p_boolean == BOOLEAN_MAIN){
			root_shape->boolean_nodes.erase(this);
		}else if(boolean == BOOLEAN_MAIN){
			root_shape->boolean_nodes.push_back(this);
		}
	}
	boolean = p_boolean;
	update_root();
}

REDShape::Boolean REDShape::get_boolean() const{
	return boolean;
}

void REDShape::set_polygon(const PoolVector<Vector2> &p_polygon) {
	polygon = p_polygon;
	rect_cache_dirty = true;
	polygon_dirty = true;
	if(get_deformation_enable() && deformation_state == DEFORMATION_ENDED)
		deformation_state = DEFORMATION_NORMAL;
	update();
}

PoolVector<Vector2> REDShape::get_polygon() const {
	return polygon;
}

Vector2 REDShape::get_offset() const {
	return offset;
}

void REDShape::set_offset(const Vector2 &p_offset) {
	offset = p_offset;
	rect_cache_dirty = true;
	polygon_dirty = true;
	update();
	_change_notify("offset");
}

void REDShape::set_smooth(int p_smooth) {
	p_smooth = CLAMP(p_smooth, 0, 32);
	if(smooth == p_smooth)
		return;
    smooth = p_smooth;
	polygon_dirty = true;
    update();
}

int REDShape::get_smooth() const{
    return smooth;
}

void REDShape::set_interpolation(red::TesselateMode p_interpolation){
	if(interpolation == p_interpolation)
		return;
    interpolation = p_interpolation;
	polygon_dirty = true;
    update();
}

red::TesselateMode REDShape::get_interpolation() const{
    return interpolation;
}

void REDShape::set_spikes(float p_spikes) {
    spikes = p_spikes;
	polygon_dirty = true;
    update();
}

float REDShape::get_spikes() const{
    return spikes;
}

void REDShape::set_reorient(int p_reorient){
	reorient = p_reorient;
	boolean_dirty = true;
	width_dirty = true;
	update();
}

int REDShape::get_reorient() const{
	return reorient;
}

void REDShape::set_simplify(float p_simplify){
	p_simplify = MAX(0, p_simplify);
	if(simplify == p_simplify)
		return;
    simplify = p_simplify;
	boolean_dirty = true;
    update();
}
float REDShape::get_simplify() const{
	return simplify;
}

void REDShape::add_spikes(Vector<Vector2> &p_points){
	if (spikes != 0){
		int size = p_points.size();
		int last_id = size - 1;
		Vector2 first = p_points[0];
		Vector2 last = p_points[last_id];
		Vector2 prev;
		Vector2 next;
		Vector2 curr;
		for (int i = 0; i < size; ++i){
			if(i == 0){
				prev = last;
				curr = first;
				next = p_points[i + 1];
			}else{
				prev = curr;
				curr = next;
				next = i == last_id ? first : p_points[i + 1];
			}
			Vector2 baca = (prev - next) / 2.0;
			Vector3 normal = Vector3(baca.x, baca.y, 0.0).cross(Vector3(0.0, 0.0, -1.0));
			Vector2 normal2d = Vector2(normal.x, normal.y).normalized();
			if (i % 2)
				p_points.write[i] = curr + normal2d * spikes;
			else
				p_points.write[i] = curr - normal2d * spikes;
		}
	}
}

Vector2  REDShape::get_camera_zoom() const{
	return camera_zoom;
};

void REDShape::update_camera_zoom(Vector2 p_camera_zoom) {
	camera_zoom = p_camera_zoom;
	update_line_renderers();
}

// animation

void REDShape::_animate_width(const float deltatime){
	if(!is_visible())
		return;
	if(deformation_width_min == 1.f && deformation_width_max == 1.f){
		width_state = DEFORMATION_END;
	}
	int count = 4;
	float timer_target = 1.f;
	float ending_timer_target = 0.5f;
	int width_offsets_count = width_offsets_state.size();
	if (width_offsets_count == count){
		for (int i = 0; i < count; i++){
			if (width_state == DEFORMATION_END){
				width_timer = deltatime;
				width_offsets_old.write[i] = width_offsets[i];
				width_offsets.write[i] = width_offsets_old[i] + width_timer / ending_timer_target * (1.f - width_offsets_old[i]);
			}else if (width_state == DEFORMATION_ENDING){
				width_offsets.write[i] = width_offsets_old[i] + width_timer / ending_timer_target * (1.f - width_offsets_old[i]);
			}else{
				width_offsets.write[i] = width_offsets_old[i] + width_timer / timer_target * ((sin(width_offsets_state[i]) + 1) * (deformation_width_max - deformation_width_min) / 2.0 + deformation_width_min - width_offsets_old[i]);
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
	if (width_state == DEFORMATION_NORMAL && width_timer >= timer_target){
		width_timer = deltatime;
		for (int i = 0; i < count; i++){
			width_offsets_old.write[i] = width_offsets[i];
			width_offsets_state.write[i] = width_offsets_state[i] + (1.5708 / count) * 4.0;
		}
	}else if (width_state == DEFORMATION_ENDING && width_timer >= ending_timer_target){
		width_state = DEFORMATION_ENDED;
		width_timer = 0.f;
	}
	if (width_state == DEFORMATION_END){
		width_state = DEFORMATION_ENDING;
	}else if (width_state == DEFORMATION_ENDED){
		for (int i = 0; i < count; i++){
			width_offsets_old.write[i] = 1.0f;
			width_timer = 0.0f;
		}
	}
	width_dirty = true;
	update();
}

void REDShape::_animate_offset(const float deltatime){
	int count = polygon.size();
	if (count < 3){
		deformation_state = DEFORMATION_ENDED;
		return;
	}
	if(!is_visible())
		return;
	float ending_timer_target = 0.5f;
	RandomPCG randbase;
	randbase.randomize();
	int offsets_count = offsets.size();
	if (offsets_count == count){
		for (int i = 0; i < count; i++){
			if (deformation_state == DEFORMATION_END){
				Vector2 target_old = offsets[i];
				targets_old.write[i] = target_old;
				targets.write[i] = Vector2(0, 0);
				timers.write[i] = deltatime;
				times.write[i] = ending_timer_target;
				offsets.write[i] = target_old;
			}else if (timers[i] <= times[i]){
				offsets.write[i] = targets_old[i].linear_interpolate(targets[i], timers[i] / times[i]);
				timers.write[i] += deltatime;
			}else if (deformation_state == DEFORMATION_ENDING){
				deformation_state = DEFORMATION_ENDED;
				break;
			} else{
				Vector2 target = Vector2(randbase.random(-deformation_offset, deformation_offset), randbase.random(-deformation_offset, deformation_offset));
				Vector2 target_old = targets[i];
				targets_old.write[i] = target_old;
				targets.write[i] = target;
				timers.write[i] = deltatime;
				times.write[i] = target_old.distance_to(target) / deformation_speed;
				offsets.write[i] = target_old;
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
		}
	}
	polygon_dirty = true;
	update();
}

void REDShape::set_deformation_width_max(const float p_deformation_width_max){
	if (p_deformation_width_max < deformation_width_min)
		return;
	deformation_width_max = p_deformation_width_max;
	if(deformation_width_max != 1.f && get_deformation_enable())
		width_state = DEFORMATION_NORMAL;
}

float REDShape::get_deformation_width_max() const{
	return deformation_width_max;
}

void REDShape::set_deformation_width_min(float p_deformation_width_min){
	if (p_deformation_width_min < 0.0 || p_deformation_width_min > deformation_width_max)
		return;
	deformation_width_min = p_deformation_width_min;
	if(deformation_width_min != 1.f && get_deformation_enable())
		width_state = DEFORMATION_NORMAL;
}

float REDShape::get_deformation_width_min() const{
	return deformation_width_min;
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

void REDShape::set_deformation_enable(bool p_deformate){
	if (p_deformate){
		deformation_state = DEFORMATION_NORMAL;
		width_state = DEFORMATION_NORMAL;
		set_process_internal(true);
	}else{
		deformation_state = DEFORMATION_END;
		width_state = DEFORMATION_END;
	}
}

bool REDShape::get_deformation_enable() const{
	return (deformation_state == DEFORMATION_NORMAL || width_state == DEFORMATION_NORMAL);
}

Vector<float> REDShape::resize_vector(const Vector<float> &in, int size) const{
	Vector<float> out;
	out.resize(size);
	int l = in.size();
	int pre_a;
	int b;
	int post_b;
	int end = l - 1;
	float smooth_thickness_iter = size * 1.0f / l - 1;
	int num = 0;
	for(int i = 0; i < in.size(); ++i){
		if (num == size)
			break;
		if (i == end)
			b = 0;
		else
			b = i + 1;
		if (i == 0)
			pre_a = end;
		else
			pre_a = i - 1;
		if (i == end - 1)
			post_b = 0;
		else
			post_b = b + 1;
		out.write[num] = in[i];
		num += 1;
		for(int j = 0; j < smooth_thickness_iter; ++j){
			if (num == size)
				break;
			out.write[num] = (in[i] + (j + 1) * 1.0 / (smooth_thickness_iter + 1) * (in[b] - in[i]));
			num += 1;
		}
	}
	return out;
}

void REDShape::calc_width_list(){
	int len = boolean_polygon.size() > 0 ? boolean_polygon.size() : real_polygon.size();
	if (len <= 1)
		return;
	Vector<float> real_width_offsets;
	if (width_offsets.size() < 2){
		real_width_offsets.resize(len);
		for (int i = 0; i < len; i++) {
			real_width_offsets.write[i] = 1.f;
		}
	} else if (width_offsets.size() != len){
		real_width_offsets = resize_vector(width_offsets, len);
	}else{
		real_width_offsets = width_offsets;
	}
	real_width_list = real_width_offsets;
	// if (width_list.size() < 2){
	// 	real_width_list = real_width_offsets;
	// } else if (width_list.size() != len){
	// 	real_width_list.clear();
	// 	int l = 0;
	// 	int pre_a;
	// 	int b;
	// 	int post_b;
	// 	int end = width_list.size() - 1;
	// 	float smooth_thickness_iter = len * 1.0f / width_list.size() - 1;
	// 	PoolVector<float>::Read width_list_read = width_list.read();
	// 	for(int i = 0; i < width_list.size(); ++i){
	// 		if (l == len)
	// 			break;
	// 		if (i == end)
	// 			b = 0;
	// 		else
	// 			b = i + 1;
	// 		if (i == 0)
	// 			pre_a = end;
	// 		else
	// 			pre_a = i - 1;
	// 		if (i == end - 1)
	// 			post_b = 0;
	// 		else
	// 			post_b = b + 1;
	// 		real_width_list.push_back(width_list_read[i]);
	// 		l = real_width_list.size();
	// 		for(int j = 0; j < smooth_thickness_iter; ++j){
	// 			if (l == len)
	// 				break;
	// 			float new_val = (width_list_read[i] + (j + 1) * 1.0 / (smooth_thickness_iter + 1) * (width_list_read[b] - width_list_read[i])) * real_width_offsets[l];
	// 			real_width_list.push_back(new_val);
	// 			l = real_width_list.size();
	// 		}
	// 	}
	// } else{
	// 	real_width_list.resize(len);
	// 	PoolVector<float>::Read width_list_read = width_list.read();
	// 	for (int i = 0; i < len; i++) {
	// 		real_width_list.write[i] = width_list_read[i] * real_width_offsets[i];
	// 	}
	// }
}

void REDShape::calc_polygon(){
	PoolVector<Vector2>::Read polyr = polygon.read(); 
	int count = polygon.size();
	real_polygon.resize(count);
	Transform2D transform = get_transform();
	
	int offsets_count = offsets.size();
	if(boolean == BOOLEAN_MAIN){
		if (deformation_state == DEFORMATION_ENDED || offsets_count != count){
			for (int i = 0; i < count; i++)
				real_polygon.write[i] = polyr[i] + get_offset();
		}else{
			for (int i = 0; i < count; i++)
				real_polygon.write[i] = polyr[i] + get_offset() + offsets[i];
		}		
	}else{
		// todo buggy
		if(root_shape){
			if(root_shape->boolean != BOOLEAN_MAIN){
				transform = root_shape->get_transform() * get_transform();
			}
		}
		if (deformation_state == DEFORMATION_ENDED || offsets_count != count){
			for (int i = 0; i < count; i++)
				real_polygon.write[i] = transform.xform(polyr[i] + get_offset());
		}else{
			for (int i = 0; i < count; i++)
				real_polygon.write[i] = transform.xform(polyr[i] + get_offset() + offsets[i]);
		}	
	}
	if(real_polygon.size() > 2){
		if (smooth > 0)
			real_polygon = red::tesselate(real_polygon, smooth, interpolation);
		add_spikes(real_polygon);
	}
	polygon_dirty = false;
	boolean_dirty = true;
}

void REDShape::calc_boolean(){
	boolean_polygon.clear();
	int count;
	count = boolean_nodes.size();
	for (int i = 0; i < count; i++){
		Vector<Vector<Vector2> > p;
		REDShape *shape = boolean_nodes[i];
		bool use_child_boolean = shape->boolean_polygon.size() > 0;
		bool use_boolean = boolean_polygon.size() > 0;
		bool override = !use_boolean && real_polygon.size() == 0;
		if(!shape->is_visible())
			continue;
		switch (shape->boolean)
		{
		case BOOLEAN_OVERRIDE:
			boolean_polygon = use_child_boolean ? shape->boolean_polygon : shape->real_polygon;
			continue;
		case BOOLEAN_MERGE:
			if(get_boolean()){
				boolean_polygon = use_child_boolean ? shape->boolean_polygon : shape->real_polygon;
				continue;
			}
			else{
				p = Geometry::merge_polygons_2d(use_boolean ? boolean_polygon : real_polygon, use_child_boolean ? shape->boolean_polygon : shape->real_polygon);
			}
			break;
		case BOOLEAN_CLIP:
			if(get_boolean()){
				continue;
			}
			else{
				p = Geometry::clip_polygons_2d(use_boolean ? boolean_polygon : real_polygon, use_child_boolean ? shape->boolean_polygon : shape->real_polygon);
			}
			break;
		case BOOLEAN_INTERSECT:
			if(get_boolean()){
				boolean_polygon = use_child_boolean ? shape->boolean_polygon : shape->real_polygon;
				continue;
			}
			else{
				p = Geometry::intersect_polygons_2d(use_boolean ? boolean_polygon : real_polygon, use_child_boolean ? shape->boolean_polygon : shape->real_polygon);
			}
			break;
		default:
			continue;
		}
		if(p.size() > 0 && p[0].size() > 0)
			boolean_polygon = p[0];
	}
	if (reorient > -1 && reorient < polygon.size() && boolean_polygon.size() > 0){
		int boolean_polygon_size = boolean_polygon.size();
		if (boolean_polygon_size > 0){
			Vector2 origin = real_polygon[reorient];
			int origin_i = -1;
			for (int i = 0; i < boolean_polygon.size(); i++) {
				if (boolean_polygon[i].is_equal_approx(origin)) {
					origin_i = i;
					break;
				}
			}
			if (origin_i > 0){
				Vector<Vector2> boolean_polygon_new;
				for (int i = origin_i; i < boolean_polygon_size; i++)
					boolean_polygon_new.push_back(boolean_polygon[i]);
				for (int i = 0; i < origin_i; i++)
					boolean_polygon_new.push_back(boolean_polygon[i]);
				boolean_polygon = boolean_polygon_new;
			}
		}
	}
	if (simplify > 0){
		if(boolean_polygon.size() > 0){
			boolean_polygon = red::ramer_douglas_peucker(boolean_polygon, simplify, false);
		}else{
			real_polygon = red::ramer_douglas_peucker(real_polygon, simplify, false);
		}
	}
	boolean_dirty = false;
}


void REDShape::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			root_shape = red::find_parent_type<REDShape>((Node2D*)this, 1);
			if(root_shape && boolean != BOOLEAN_MAIN){
				root_shape->boolean_nodes.push_back(this);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if(root_shape && boolean != BOOLEAN_MAIN){
				root_shape->boolean_nodes.erase(this);
				update_root();
			}
			root_shape = NULL;
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			_animate_offset(get_process_delta_time());
			_animate_width(get_process_delta_time());
			if (deformation_state == DEFORMATION_ENDED && width_state == DEFORMATION_ENDED)
				set_process_internal(false);
		} break;
		case NOTIFICATION_DRAW: {
			if (!is_inside_tree())
				break;
			bool poly_updated = false;
			bool width_updated = false;
			if (polygon_dirty){
				calc_polygon();
				poly_updated = true;
			}
			if (boolean_dirty){
				calc_boolean();
				poly_updated = true;
			}
			if(width_dirty){
				calc_width_list();
				width_updated = true;
			}
			if (poly_updated){
				update_root();
				update_renderers();
			}
			else if (width_updated){
				update_line_renderers();
			}
		} break;
	}
}

void REDShape::_bind_methods() {
	// main
	ClassDB::bind_method(D_METHOD("set_polygon", "polygon"), &REDShape::set_polygon);
	ClassDB::bind_method(D_METHOD("get_polygon"), &REDShape::get_polygon);
	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &REDShape::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &REDShape::get_offset);
    ClassDB::bind_method(D_METHOD("set_boolean", "boolean"), &REDShape::set_boolean);
    ClassDB::bind_method(D_METHOD("get_boolean"), &REDShape::get_boolean);
    ClassDB::bind_method(D_METHOD("set_smooth", "smooth"), &REDShape::set_smooth);
    ClassDB::bind_method(D_METHOD("get_smooth"), &REDShape::get_smooth);
    ClassDB::bind_method(D_METHOD("set_interpolation", "interpolation"), &REDShape::set_interpolation);
    ClassDB::bind_method(D_METHOD("get_interpolation"), &REDShape::get_interpolation);
	ClassDB::bind_method(D_METHOD("set_spikes", "spike"), &REDShape::set_spikes);
    ClassDB::bind_method(D_METHOD("get_spikes"), &REDShape::get_spikes);
	ClassDB::bind_method(D_METHOD("set_reorient", "reorient"), &REDShape::set_reorient);
	ClassDB::bind_method(D_METHOD("get_reorient"), &REDShape::get_reorient);
    ClassDB::bind_method(D_METHOD("set_simplify", "simplify"), &REDShape::set_simplify);
    ClassDB::bind_method(D_METHOD("get_simplify"), &REDShape::get_simplify);
	// animation
	ClassDB::bind_method(D_METHOD("set_deformation_enable", "deformation_enable"), &REDShape::set_deformation_enable);
    ClassDB::bind_method(D_METHOD("get_deformation_enable"), &REDShape::get_deformation_enable);
	ClassDB::bind_method(D_METHOD("set_deformation_offset", "deformation_offset"), &REDShape::set_deformation_offset);
    ClassDB::bind_method(D_METHOD("get_deformation_offset"), &REDShape::get_deformation_offset);
	ClassDB::bind_method(D_METHOD("set_deformation_speed", "deformation_speed"), &REDShape::set_deformation_speed);
    ClassDB::bind_method(D_METHOD("get_deformation_speed"), &REDShape::get_deformation_speed);
	ClassDB::bind_method(D_METHOD("set_deformation_width_min", "deformation_width_min"), &REDShape::set_deformation_width_min);
    ClassDB::bind_method(D_METHOD("get_deformation_width_min"), &REDShape::get_deformation_width_min);
	ClassDB::bind_method(D_METHOD("set_deformation_width_max", "deformation_width_max"), &REDShape::set_deformation_width_max);
    ClassDB::bind_method(D_METHOD("get_deformation_width_max"), &REDShape::get_deformation_width_max);
	ADD_GROUP("Shape", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "boolean", PROPERTY_HINT_ENUM, "Main, Merge, Clip, Intersect, Override"), "set_boolean", "get_boolean");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "smooth"), "set_smooth", "get_smooth");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolation", PROPERTY_HINT_ENUM, "Cubic, Quadratic Bezier, Cubic Bezier"), "set_interpolation", "get_interpolation");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "polygon"), "set_polygon", "get_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "spikes"), "set_spikes", "get_spikes");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "reorient"), "set_reorient", "get_reorient");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "simplify"), "set_simplify", "get_simplify");
	ADD_GROUP("Deformation", "deformation_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "deformation_enable"), "set_deformation_enable", "get_deformation_enable");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_offset"), "set_deformation_offset", "get_deformation_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_speed"), "set_deformation_speed", "get_deformation_speed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_width_min"), "set_deformation_width_min", "get_deformation_width_min");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deformation_width_max"), "set_deformation_width_max", "get_deformation_width_max");
}

REDShape::REDShape() {
	// main
	root_shape = NULL;
	boolean = BOOLEAN_MAIN;
	smooth = 0;
	interpolation = red::CUBIC;
	spikes = 0.f;
	reorient = -1;
	simplify = 0.f;
	polygon_dirty = true;
	boolean_dirty = true;
	width_dirty = true;
	rect_cache_dirty = true;
	// animation
	deformation_state= DEFORMATION_ENDED;
	deformation_enable = false;
	deformation_offset = 20.0f;
	deformation_speed = 20.0f;
	deformation_width_min = 0.5f;
	deformation_width_max = 1.25f;
	camera_zoom = Vector2(1.f, 1.f);
}
