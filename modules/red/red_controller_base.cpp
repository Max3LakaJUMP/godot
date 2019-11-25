#include "red_controller_base.h"

#include "core/engine.h"
#include "scene/2d/camera_2d.h"

#include "scene/animation/animation_player.h"
#include "scene/animation/animation_tree.h"
#include "scene/animation/animation_node_state_machine.h"

#include "red_engine.h"
#include "red_frame.h"
#include "red_page.h"
#include "red_issue.h"
#include <string>
#include "scene/scene_string_names.h"
#include "scene/animation/tween.h"
#include "scene/main/timer.h"

void REDControllerBase::_frame_change() {
	bool first_frame = false;
	if (frame != nullptr){
		frame->disconnect("update_camera_zoom", this, "update_camera_zoom");
		frame->disconnect("update_camera_pos", this, "update_camera_pos");
		if (frame_timer_connected){
			frame_timer_connected = false;
			timer->disconnect("timeout", this, "_frame_changed");
		}
		frame->set_active(false);
		tween->stop(frame, "set_parallax");
	}else{
		first_frame = true;
	}

	frame = page->get_frame(page->get_id());
	if (frame == nullptr)
		return;

	//frame->_started();
	frame->connect("update_camera_zoom", this, "update_camera_zoom");
	frame->connect("update_camera_pos", this, "update_camera_pos");
	
	update_camera_to_frame(first_frame);
	if (first_frame){
		camera->reset_smoothing();
	}
}

void REDControllerBase::_frame_changed() {
	frame_timer_connected = false;
	print_line("changed");
	camera_state = STATIC;
	frame->set_active(true);
}

void REDControllerBase::_camera_moved(const Transform2D &p_transform, const Point2 &p_screen_offset) {
	if (frame != nullptr){
		if (frame->is_active()){
			frame->set_parallax(get_target_parallax());
		}
	}
}


void REDControllerBase::set_tween(Tween *p_tween){
	tween = p_tween;
}
Tween *REDControllerBase::get_tween() const{
	return tween;
}

void REDControllerBase::set_camera_smooth(const bool p_smooth){
	camera_smooth = p_smooth;
}
bool REDControllerBase::get_camera_smooth() const{
	return camera_smooth;
}
bool REDControllerBase::to_prev_page() {
	if (!issue){
		return false;
	}
	int new_id = issue->get_id() - 1;
	if (new_id >= 0 && new_id < issue->get_pages_count()){
		set_page(new_id, true);
		return true;
	}
	return false;
}

bool REDControllerBase::to_next_page() {
	if (!issue){
		return false;
	}
	int new_id = issue->get_id() + 1;
	if (new_id >= 0 && new_id < issue->get_pages_count()){
		set_page(new_id);
		return true;
	}
	return false;
}


bool REDControllerBase::to_prev_frame() {
	if (!page){
		return false;
	}
	int new_id = page->get_id() - 1;
	if (new_id >= 0 && new_id < page->get_frames_count()){
		set_frame(new_id, false);
		return true;
	}
	return false;
}

bool REDControllerBase::to_next_frame() {
	if (!page){
		return false;
	}
	int new_id = page->get_id() + 1;
	if (new_id >= 0 && new_id < page->get_frames_count()){
		set_frame(new_id, true);
		return true;
	}
	return false;
}


bool REDControllerBase::to_prev_frame_state(){
	if (frame==nullptr){
		return false;
	}
	if (frame->get_anim_tree().is_empty()){
		return false;
	}
	AnimationTree *at = Object::cast_to<AnimationTree>(frame->get_node(frame->get_anim_tree()));
	Ref<AnimationNodeStateMachine> machine = at->get_tree_root();

	if (machine.is_null()){
		return false;
	}

	Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
	if (playback.is_null()){
		return false;
	}
	int new_id = frame->get_id() - 1;

	if (new_id < frame->get_states_count()){
		if (new_id >= 0){
			frame->set_id(new_id);
			frame->_started();
			//frame->travel();
			/*
			if (machine->has_node(frame->get_state(new_id))) {
				playback->travel(frame->get_state(new_id));
				return true;
			} */
			return true;
		}
		else {
			AnimationPlayer *ap = (AnimationPlayer*)(at->get_node(at->get_animation_player()));
			if (ap != nullptr && !frame->is_starting() && machine->has_node(frame->get_start_transition())) {
				frame->_starting();
				print_line("attaching signal");
				playback->connect(SceneStringNames::get_singleton()->animation_changed, this, "frame_start");
				b_can_control = false;
				print_line("signal attached");
				return true;
			}
			frame->_pre_starting();
			b_can_control = true;
			return false;
		}
	}
	return false;
}


bool REDControllerBase::to_next_frame_state() {
	if (frame==nullptr){
		return false;
	}
	if (frame->get_anim_tree().is_empty()){
		return false;
	}
	AnimationTree *at = Object::cast_to<AnimationTree>(frame->get_node(frame->get_anim_tree()));
	Ref<AnimationNodeStateMachine> machine = at->get_tree_root();

	if (machine.is_null()){
		return false;
	}

	Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
	if (playback.is_null()){
		return false;
	}
	int new_id = frame->get_id() + 1;
	
	if (new_id >= 0){
		if (new_id < frame->get_states_count()){
			frame->set_id(new_id);
			frame->_started();
			//frame->travel();
			/*
			if (machine->has_node(frame->get_state(new_id))) {
				playback->travel(frame->get_state(new_id));
				return true;
			} */
			return true;
		}
		else {
			//machine->has_node(machine->get_end_node()) && playback->get_current_node() != machine->get_end_node();
			AnimationPlayer *ap = (AnimationPlayer*)(at->get_node(at->get_animation_player()));
			if (ap != nullptr && !frame->is_ending() && machine->has_node(frame->get_end_transition())) {
				frame->_ending();
				print_line("attaching signal");
				playback->connect(SceneStringNames::get_singleton()->animation_changed, this, "frame_end");
				b_can_control = false;
				print_line("signal attached");
				return true;
			}
			frame->_ended();
			b_can_control = true;
			return false;
		}
	}
	return false;
}

void REDControllerBase::frame_start(StringName old_name, StringName new_name){
	AnimationTree *at = Object::cast_to<AnimationTree>(frame->get_node(frame->get_anim_tree()));	
	Ref<AnimationNodeStateMachine> machine = at->get_tree_root();
	if (old_name == frame->get_start_transition()){
		Ref<AnimationNodeStateMachinePlayback> playback = frame->get_playback();
		if (playback.is_valid())
			playback->disconnect("animation_changed", this, "frame_start");
		to_prev();
	}
}

void REDControllerBase::frame_end(StringName old_name, StringName new_name){
	if (old_name == frame->get_end_transition()){
		Ref<AnimationNodeStateMachinePlayback> playback = frame->get_playback();
		if (playback.is_valid())
			playback->disconnect("animation_changed", this, "frame_end");
		to_next();
	}
}

void REDControllerBase::to_prev() {
	if(!to_prev_frame_state()){
		if(!to_prev_frame()){
			to_prev_page();
		}
	}
}


void REDControllerBase::to_next() {
	print_line("to_next_frame_state");
	if(!to_next_frame_state()){
		print_line("to_next_frame");
		if(!to_next_frame()){
			print_line("to_next_page");
			to_next_page();
		}
	}
}

void REDControllerBase::set_issue_by_path(const NodePath &p_issue_path) {
	set_issue((REDIssue*)get_node(p_issue_path));
}

void REDControllerBase::set_issue(REDIssue *p_issue) {
	//if (issue != nullptr){
	//	camera->disconnect("send_camera_zoom", issue, "update_camera");
	//}
    issue = p_issue;
	if (issue == nullptr)
		return;
	//connect("send_camera_zoom", issue, "update_camera"));
	set_page(issue->get_id());
}

REDIssue *REDControllerBase::get_issue() const {
    return issue;
}

void REDControllerBase::set_page(REDPage *p_page){
	set_issue(nullptr);
	page = p_page;
	if (page == nullptr)
		return;
	set_frame(page->get_id());
}

void REDControllerBase::set_page(int p_id, bool is_prev){
	if (issue == nullptr)
		return;
	Vector2 z = camera_zoom_min + camera_zoom * (camera_zoom_max - camera_zoom_min);
	issue->set_page(p_id, is_prev, z);
	if (issue->get_instanced_list(p_id)){
		REDPage *p_page = issue->get_page(p_id);
		page = p_page;
		set_frame(page->get_id(), !is_prev);
	}
}

REDPage *REDControllerBase::get_page() const{
	return page;
}

void REDControllerBase::set_frame(int p_id, bool ended){
	page->set_frame(p_id, true, ended);
	bool reset_zoom = true;

	float zoom_reset_duration = -0.25f;
	float total_delay = 0.0;

	if (frame != nullptr){
		if (frame->is_active()){
			if (zoom_reset_duration >= 0 && camera_zoom != 0.5f){
				total_delay += zoom_reset_duration;
				zoom_reset(zoom_reset_duration);
			}
		}
	}
	if (total_delay > 0.0f){
		timer->connect("timeout", this, "_frame_change", varray(), CONNECT_ONESHOT);
		timer->start(total_delay);
	}else{
		_frame_change();
	}
}


void REDControllerBase::zoom_reset(const float reset_duration) {
	if (!frame->is_active())
		return;
	tween->stop(this, "set_camera_zoom");	
	tween->interpolate_method(this, "set_camera_zoom", camera_zoom, 0.5f, reset_duration, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
}


void REDControllerBase::update_camera_to_frame(bool first_frame) {
    if (!camera_mode || Engine::get_singleton()->is_editor_hint()){
		return;
	}
    if (get_camera_path().is_empty()){
		return;
	}
    if (frame == nullptr){
		return;
	}
	camera_zoom = 0.5;
	if (tween->is_active()){
		tween->stop(camera, "set_position");	
		tween->stop(camera, "set_zoom");
		tween->stop(camera, "set_offset");	
		tween->stop(frame, "set_parallax");
	}
	if(camera_smooth && !first_frame){
		Tween::TransitionType tween_transition_type = Tween::TRANS_CUBIC;
		Tween::EaseType tween_ease_type = (camera->get_zoom()) <= frame_zoom ? Tween::EASE_OUT : Tween::EASE_OUT;
		float tween_duration = ABS(camera->get_position().distance_to(get_target_pos()))/4000.0f;
		tween->follow_method(camera, "set_position", camera->get_position(), this, "get_target_pos", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(camera, "set_zoom", camera->get_zoom(), this, "get_target_zoom", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(camera, "set_offset", camera->get_offset(), this, "get_target_offset", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(frame, "set_parallax", frame->get_parallax(), this, "get_target_parallax", tween_duration, tween_transition_type, tween_ease_type);
		camera_state = MOVING;
		tween->start();	
		frame_timer_connected = true;
		timer->connect("timeout", this, "_frame_changed", varray(), CONNECT_ONESHOT);
		timer->start(tween_duration + 0.01);
	}
	else{
		get_target_pos();
		camera->set_position(Vector2(1000, -10000.0));
		camera->set_zoom(get_target_zoom());
		camera->set_offset(get_target_offset());
		_frame_changed();
	}
}


Vector2 REDControllerBase::get_target_pos(){
	if (camera_zoom < 0.5)
		frame_pos_local = frame->get_camera_pos_zoom_in().linear_interpolate(frame->get_camera_pos(), camera_zoom * 2.0);
	else
		frame_pos_local = frame->get_camera_pos().linear_interpolate(frame->get_camera_pos_zoom_out(), 2 * camera_zoom - 1.0);
	frame_pos_global = frame_pos_local + frame->get_global_position();
	return frame_pos_global;
}

Vector2 REDControllerBase::get_target_zoom(){
	Vector2 frame_camera_zoom = frame->get_camera_zoom();
	Vector2 camera_zoom_min_clamped = Vector2(MIN(camera_zoom_min.x, frame_camera_zoom.x), MIN(camera_zoom_min.y, frame_camera_zoom.y));
	Vector2 camera_zoom_max_clamped = Vector2(MAX(camera_zoom_max.x, frame_camera_zoom.x), MAX(camera_zoom_max.y, frame_camera_zoom.y));
	if (camera_zoom < 0.5){
		frame_zoom = camera_zoom_min_clamped + camera_zoom * 2.0 * (frame_camera_zoom - camera_zoom_min_clamped);
	}
	else{
		frame_zoom = frame_camera_zoom + (camera_zoom - 0.5) * 2 * (camera_zoom_max_clamped - frame_camera_zoom);
	}
	frame_zoom *= page->get_size().width / get_viewport()->get_size().width;
	if (issue != nullptr){
		issue->update_camera_zoom(camera->get_zoom());
	}
	else if (page != nullptr){
		page->update_camera_zoom(camera->get_zoom());
	}
	return frame_zoom;
}

Vector2 REDControllerBase::get_target_offset(){
	if (camera_zoom > 0.5){
		mouse_pos_final = mouse_pos * (2.0 - 2 * camera_zoom);
	}
	else{
		mouse_pos_final = mouse_pos;
	}
	return mouse_pos_final;
}

Vector2 REDControllerBase::get_target_parallax(){
	if (camera_zoom < 0.5){
		frame_parallax = frame->get_parallax_pos_zoom_in().linear_interpolate(frame->get_parallax_pos(), camera_zoom * 2.0);
	}
	else{
		frame_parallax = frame->get_parallax_pos().linear_interpolate(frame->get_parallax_pos_zoom_out(), 2 * camera_zoom - 1.0);
	}
	if (frame->is_active()){
		frame_parallax = camera->get_position() - frame->get_global_position() - frame_parallax + camera->get_offset();
	}
	else{
		frame_parallax = frame_pos_local - frame_parallax + camera->get_offset();
	}
	return frame_parallax;
}

void REDControllerBase::set_state(int p_id){
	frame->set_state(p_id);
}

REDFrame *REDControllerBase::get_frame() const{
    return frame;
}

void REDControllerBase::set_camera_path(const NodePath &p_camera_path) {
	if (camera_path == p_camera_path)
		return;
	camera_path = p_camera_path;

	if (is_inside_tree()){
		camera = (Camera2D*)(get_node(p_camera_path));
	}
}

NodePath REDControllerBase::get_camera_path() const {
	return camera_path;
}
void REDControllerBase::set_camera(Camera2D *p_camera){
	camera = p_camera;
}
Camera2D *REDControllerBase::get_camera() const{
	return camera;
}
void REDControllerBase::set_camera_mode(bool b) {
	camera_mode = b;
}

bool REDControllerBase::get_camera_mode() const {
	return camera_mode;
}

void REDControllerBase::zoom_in(const float &p_val) {
	if (frame == nullptr)
		return;

	camera_zoom_target = CLAMP(camera_zoom_target - p_val, 0.0f, 1.0f);
	if (camera_zoom == camera_zoom_target)
		return;
	
	tween->stop(this, "set_camera_zoom");	
	tween->interpolate_method(this, "set_camera_zoom", camera_zoom, camera_zoom_target, ABS(camera_zoom-camera_zoom_target), Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
}

void REDControllerBase::zoom_out(const float &p_val) {
	if (frame == nullptr)
		return;
	camera_zoom_target = CLAMP(camera_zoom_target + p_val, 0.0f, 1.0f);
	if (camera_zoom == camera_zoom_target)
		return;
	
	tween->stop(this, "set_camera_zoom");	
	tween->interpolate_method(this, "set_camera_zoom", camera_zoom, camera_zoom_target, ABS(camera_zoom-camera_zoom_target), Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
}

void REDControllerBase::set_camera_zoom(const float p_zoom) {
	if(camera_zoom == p_zoom){
		return;
	}
	camera_zoom = p_zoom;
	update_camera();
}

float REDControllerBase::get_camera_zoom() const{
	return camera_zoom;
}

void REDControllerBase::update_camera() {
	if (camera_state == STATIC){
		camera->set_position(get_target_pos());
		camera->set_zoom(get_target_zoom());
		camera->set_offset(get_target_offset());
	}
}

void REDControllerBase::update_camera_pos() {
	if (camera_state == STATIC){
		camera->set_position(get_target_pos());
	}
}

void REDControllerBase::update_camera_zoom() {
	if (camera_state == STATIC){
		camera->set_zoom(get_target_zoom());
	}
}

void REDControllerBase::set_camera_zoom_min(const Vector2 &p_zoom) {
	if (camera_zoom_min == p_zoom)
		return;
	camera_zoom_min = p_zoom;
	update_camera();
}

Vector2 REDControllerBase::get_camera_zoom_min() const{
	return camera_zoom_min;
}

void REDControllerBase::set_camera_zoom_max(const Vector2 &p_zoom) {
	if (camera_zoom_max == p_zoom)
		return;
	camera_zoom_max = p_zoom;
	update_camera();
}

Vector2 REDControllerBase::get_camera_zoom_max() const{
	return camera_zoom_max;
}

Vector2 REDControllerBase::get_global_camera_zoom() const{
	return (frame == nullptr) ? camera_zoom_min + camera_zoom * (camera_zoom_max - camera_zoom_min) : camera_zoom_min + camera_zoom * (camera_zoom_max - camera_zoom_min) * frame->get_camera_zoom();
}

Vector2 REDControllerBase::get_mouse_pos() const{
	return mouse_pos;
}

void REDControllerBase::set_mouse_pos(const Vector2 &p_mouse_pos) {
	if (frame == nullptr)
		return;
	mouse_pos = p_mouse_pos;
	if(frame->is_active()){
		tween->stop(camera, "set_offset");	
		tween->follow_method(camera, "set_offset", camera->get_offset(), this, "get_target_offset", 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		tween->start();	

		//recalc_mouse();
		//tween->stop(camera, "set_offset");
		//tween->interpolate_method(camera, "set_offset", camera->get_offset(), mouse_pos_final, 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		//tween->start();
	}
}

bool REDControllerBase::can_control() const {
	return b_can_control;
}

void REDControllerBase::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:{
			if (!Engine::get_singleton()->is_editor_hint()) {
				timer = memnew(Timer);
				add_child(timer);
				timer->set_owner(this);
				timer->set_one_shot(true);

				tween = memnew(Tween);
				add_child(tween);
				timer->set_owner(this);

				group_name = "__cameras_" + itos(get_viewport()->get_viewport_rid().get_id());
				add_to_group(group_name);
				//mouse_pos = get_viewport()->get_size() / 2.0;
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			
			remove_from_group(group_name);
		} break;
		case NOTIFICATION_READY: {
			camera = (Camera2D*)(get_node(camera_path));
			get_tree()->get_root()->connect("size_changed", this, "update_camera");
		} break;
	}
}
void REDControllerBase::_input(Ref<InputEvent> p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		mouse_pos = mb->get_position();
		//update_camera_pos();
	}
}

void REDControllerBase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_issue_by_path", "issue_path"), &REDControllerBase::set_issue_by_path);
	ClassDB::bind_method(D_METHOD("frame_start", "old_name", "new_name"), &REDControllerBase::frame_start);
	ClassDB::bind_method(D_METHOD("frame_end", "old_name", "new_name"), &REDControllerBase::frame_end);

	ClassDB::bind_method(D_METHOD("zoom_out", "zoom_val"), &REDControllerBase::zoom_out);
	ClassDB::bind_method(D_METHOD("zoom_in", "zoom_val"), &REDControllerBase::zoom_in);
	ClassDB::bind_method(D_METHOD("to_next"), &REDControllerBase::to_next);
	ClassDB::bind_method(D_METHOD("to_prev"), &REDControllerBase::to_prev);


	ClassDB::bind_method(D_METHOD("set_camera_mode", "camera_mode"), &REDControllerBase::set_camera_mode);
	ClassDB::bind_method(D_METHOD("get_camera_mode"), &REDControllerBase::get_camera_mode);
	ClassDB::bind_method(D_METHOD("set_camera_path", "camera_path"), &REDControllerBase::set_camera_path);
	ClassDB::bind_method(D_METHOD("get_camera_path"), &REDControllerBase::get_camera_path);

	ClassDB::bind_method(D_METHOD("set_camera_zoom", "camera_zoom"), &REDControllerBase::set_camera_zoom);
	ClassDB::bind_method(D_METHOD("get_camera_zoom"), &REDControllerBase::get_camera_zoom);
	ClassDB::bind_method(D_METHOD("set_camera_zoom_min", "camera_zoom_min"), &REDControllerBase::set_camera_zoom_min);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_min"), &REDControllerBase::get_camera_zoom_min);
	ClassDB::bind_method(D_METHOD("set_camera_zoom_max", "camera_zoom_max"), &REDControllerBase::set_camera_zoom_max);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_max"), &REDControllerBase::get_camera_zoom_max);

	ClassDB::bind_method(D_METHOD("can_control"), &REDControllerBase::can_control);

	ClassDB::bind_method(D_METHOD("set_camera_smooth", "camera_smooth"), &REDControllerBase::set_camera_smooth);
	ClassDB::bind_method(D_METHOD("get_camera_smooth"), &REDControllerBase::get_camera_smooth);

	//ClassDB::bind_method(D_METHOD("update_camera_pos"), &REDControllerBase::update_camera_pos);
	ClassDB::bind_method(D_METHOD("update_camera"), &REDControllerBase::update_camera);
	ClassDB::bind_method(D_METHOD("update_camera_to_frame"), &REDControllerBase::update_camera);

	ClassDB::bind_method(D_METHOD("set_mouse_pos", "mouse_pos"), &REDControllerBase::set_mouse_pos);
	ClassDB::bind_method(D_METHOD("get_mouse_pos"), &REDControllerBase::get_mouse_pos);
	
	ClassDB::bind_method(D_METHOD("_camera_moved"), &REDControllerBase::_camera_moved);
	ClassDB::bind_method(D_METHOD("_frame_change"), &REDControllerBase::_frame_change);
	ClassDB::bind_method(D_METHOD("_frame_changed"), &REDControllerBase::_frame_changed);
	
	ClassDB::bind_method(D_METHOD("get_target_pos"), &REDControllerBase::get_target_pos);
	ClassDB::bind_method(D_METHOD("get_target_zoom"), &REDControllerBase::get_target_zoom);
	ClassDB::bind_method(D_METHOD("get_target_offset"), &REDControllerBase::get_target_offset);
	ClassDB::bind_method(D_METHOD("get_target_parallax"), &REDControllerBase::get_target_parallax);
	ADD_GROUP("Camera", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Camera2D"), "set_camera_path", "get_camera_path");

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "camera_zoom"), "set_camera_zoom", "get_camera_zoom");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom_min"), "set_camera_zoom_min", "get_camera_zoom_min");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom_max"), "set_camera_zoom_max", "get_camera_zoom_max");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_smooth"), "set_camera_smooth", "get_camera_smooth");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_mode"), "set_camera_mode", "get_camera_mode");
}

REDControllerBase::REDControllerBase() {
	issue = nullptr;
    page = nullptr;
	frame = nullptr;
	frame_timer_connected = false;

	camera_mode = true;
	b_can_control = true;
	camera_zoom = 0.5;
	camera_zoom_target = 0.5;
	camera_zoom_min = Vector2(0.5f, 0.5f);
	camera_zoom_max = Vector2(4, 4);

	mouse_pos = Vector2(0, 0);

}
