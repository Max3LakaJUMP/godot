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
/*
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
}*/
void REDControllerBase::_frame_start(){
	if (!frame->is_active()){
		frame->set_active(true);
		frame->connect("update_camera_zoom", this, "update_camera");
		frame->connect("_target_moved_pos", this, "update_camera_pos");
		frame->connect("_target_moved_parallax", this, "update_camera_parallax");

		int id = frame->get_id();
		int last_id = frame->get_states_count() - 1;
		if (id == 0 && id < last_id){
			set_state(id + 1);
		}else if (id == last_id && id > 0){
			set_state(id - 1);
		}
	}
}

void REDControllerBase::_frame_end(){
	tween->stop(frame, "set_parallax");
	tween->stop(frame, "set_frame_scale");
	tween->interpolate_method(frame, "set_parallax", frame->get_parallax(), Vector2(0.0, 0.0), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->interpolate_method(frame, "set_frame_scale", frame->get_scale(), Vector2(1.0, 1.0), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
	if (frame->is_active()){
		frame->set_active(false);
		frame->disconnect("update_camera_zoom", this, "update_camera");
		frame->disconnect("_target_moved_pos", this, "update_camera_pos");
		frame->disconnect("_target_moved_parallax", this, "update_camera_parallax");
	}
}


void REDControllerBase::set_frame(int p_id, bool to_next){
	bool force_immediate = false;
	frame_changing = true;
	page->set_frame(p_id, true, to_next);

	float total_delay = 0.0;
	if (!force_immediate){
		if (frame != nullptr){
			if (frame->is_focused()){
				if (to_next)
					total_delay += frame->get_end_delay();
				else
					total_delay += frame->get_start_delay();
			}
		}
	}

	if (total_delay > 0.0f){
		if (to_next)
			frame_next_timer_connected = true;
		else
			frame_prev_timer_connected = true;
		frame_timer->start(total_delay);
	}else{
		_frame_change();
	}
}

void REDControllerBase::_frame_change() {
	frame_timer->stop();
	frame_changing = false;

	frame_next_timer_connected = false;
	frame_prev_timer_connected = false;
	bool first_frame = false;
	if (frame == nullptr){
		first_frame = true;
	}else{
		frame->set_focused(false);
		_frame_end();
	}
	frame = page->get_frame(page->get_id());
	if (frame == nullptr)
		return;
	if (frame->is_start_immediate()){
		_frame_start();
	}
	update_camera_to_frame();
	if (first_frame){
		camera->reset_smoothing();
	}
}

void REDControllerBase::update_camera_to_frame(bool force_immediate) {
    if (!camera_mode || Engine::get_singleton()->is_editor_hint()){
		return;
	}
    if (get_camera_path().is_empty()){
		return;
	}
    if (frame == nullptr){
		return;
	}
	if (reset_camera_on_frame_change){
		zoom_k_target = 0.5;
	}
	if (tween->is_active()){
		tween->stop(camera, "set_position");	
		tween->stop(camera, "set_zoom");
		tween->stop(frame, "set_parallax");
		tween->stop(frame, "set_frame_scale");
	}
	Size2 frame_size = frame->_edit_get_rect().size;
	if(camera_smooth && !force_immediate){
		camera_state = CAMERA_MOVING;
		Tween::TransitionType tween_transition_type = Tween::TRANS_CUBIC;
		Tween::EaseType tween_ease_type = (camera->get_zoom()) <= frame_zoom ? Tween::EASE_OUT : Tween::EASE_OUT;
		float tween_duration = MAX(ABS(camera->get_position().distance_to(get_target_pos()))/4000.0f, 0.5);
		tween->follow_method(camera, "set_position", camera->get_position(), this, "get_target_pos", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(camera, "set_zoom", camera->get_zoom(), this, "get_target_zoom", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(frame, "set_parallax", frame->get_parallax(), this, "get_target_parallax", tween_duration, tween_transition_type, tween_ease_type);
 		tween->interpolate_method(frame, "set_frame_scale", frame->get_scale(), (Vector2(frame_size.x, frame_size.x)+frame_expand)/(frame_size.x), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		tween->start();	
		frame_timer_connected = true;
		camera_timer->start(tween_duration + 0.01);
	}
	else{
		//tween->interpolate_method(this, "set_frame_scale", frame->get_scale(), (frame_size+frame_expand)/(frame_size), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		tween->start();	
		camera->set_position(get_target_pos());
		camera->set_zoom(get_target_zoom());
		_frame_changed();
	}
}

void REDControllerBase::_frame_changed() {
	camera_state = CAMERA_STATIC;
	last_dirrection = dirrection;
	dirrection = DIRRECTION_NONE;
	camera_timer->stop();
	if (frame == nullptr)
		return;
	//frame_timer_connected = false;
	//frame->_started();
	frame->set_focused(true);
	if (!frame->is_start_immediate()){
		_frame_start();
	}

}

void REDControllerBase::set_reset_camera_on_frame_change(bool p_reset_camera){
	reset_camera_on_frame_change = p_reset_camera;
}
bool REDControllerBase::is_reset_camera_on_frame_change() const{
	return reset_camera_on_frame_change;
}


void REDControllerBase::_camera_moved(const Transform2D &p_transform, const Point2 &p_screen_offset) {
	if (frame != nullptr){
		if (frame->is_focused()){
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
	dirrection = DIRRECTION_BACKWARD;
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
	if (new_id > -1 && new_id < issue->get_pages_count()){
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
	if (new_id > -1 && new_id < page->get_frames_count()){
		set_frame(new_id, false);
		return true;
	}
	return false;
}

bool REDControllerBase::to_next_frame() {
	if (!page){
		return false;
	}
	/*
	if (frame_next_timer_connected){
		timer->disconnect("timeout", this, "_frame_change");
		timer->stop();
		_frame_change();
		return true;
	}else if (frame_prev_timer_connected){
		timer->disconnect("timeout", this, "_frame_change");
		timer->stop();
	}*/
	int new_id = page->get_id() + 1;
	if (new_id >= 0 && new_id < page->get_frames_count()){
		set_frame(new_id, true);
		return true;
	}
	return false;
}


bool REDControllerBase::to_prev_frame_state(){
	int new_id = frame->get_id() - 1;
	if (new_id > -1 && new_id < frame->get_states_count()){
		set_state(new_id);
		if (new_id == 0 || new_id == frame->get_states_count() - 1)
			return false;
		else
			return true;
	}
	return false;
}


bool REDControllerBase::to_next_frame_state() {
	int new_id = frame->get_id() + 1;
	if (new_id > -1 && new_id < frame->get_states_count()){
		set_state(new_id);
		if (new_id == 0 || new_id == frame->get_states_count() - 1)
			return false;
		else
			return true;
	}
	return false;
}
	/*
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
	if (new_id == 0 || new_id == frame->get_states_count() - 1){
		set_state(new_id);
		return false;
	}
	if (new_id > 0){
		if (new_id < frame->get_states_count()-2){
			set_state(new_id);
			//frame->travel();

			if (machine->has_node(frame->get_state(new_id))) {
				playback->travel(frame->get_state(new_id));
				return true;
			} 
			return true;
		}
		/*else {
			//frame->set_active(false);
			//frame->_end_loop();
			//frame->travel();
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

		}
	}
	return false;
}
*/

void REDControllerBase::to_prev() {
	if (frame_changing){
		if (dirrection == DIRRECTION_BACKWARD){
			frame_timer->stop();
			_frame_change();
			return;
		}
	}
	dirrection = DIRRECTION_BACKWARD;
	if(!to_prev_frame_state()){
		if(!to_prev_frame()){
			to_prev_page();
		}
	}
}


void REDControllerBase::to_next() {
	if (frame_changing){
		if (dirrection == DIRRECTION_FORWARD){
			frame_timer->stop();
			_frame_change();
			return;
		}
	}
	dirrection = DIRRECTION_FORWARD;
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
	Vector2 z = camera_zoom_min + zoom_k * (camera_zoom_max - camera_zoom_min);
	issue->set_page(p_id, is_prev, z);
	page = issue->get_page(p_id);
	if (page != nullptr){
		set_frame(page->get_id(), !is_prev);
	}
}

REDPage *REDControllerBase::get_page() const{
	return page;
}



void REDControllerBase::set_frame_expand(const Vector2 &p_frame_expand){
	frame_expand = p_frame_expand;
};

Vector2 REDControllerBase::get_frame_expand() const{
	return frame_expand;
};


void REDControllerBase::set_frame_scale(const Vector2 &p_scale_factor){
	//frame->set_frame_scale(p_scale_factor);
};

Vector2 REDControllerBase::get_target_mouse(){
	mouse_offset_target = get_viewport()->get_size() * mouse_offset * mouse_offset_k;
	if (zoom_k > 0.5)
		mouse_offset_target *= 2.0 - 2 * zoom_k;
	//mouse_offset_target*=frame->get_scale();
	return mouse_offset_target;
}

Vector2 REDControllerBase::get_target_pos(){
	if (zoom_k < 0.5)
		frame_pos_local = frame->get_camera_pos_in().get_origin().linear_interpolate(frame->get_camera_pos().get_origin(), zoom_k * 2.0);
	else
		frame_pos_local = frame->get_camera_pos().get_origin().linear_interpolate(frame->get_camera_pos_out().get_origin(), 2 * zoom_k - 1.0);
	

	frame_pos_local = (frame_pos_local + get_target_mouse());
	frame_pos_global = frame_pos_local*frame->get_scale() + frame->get_origin_pos_gl(); //frame->get_origin_pos_gl()
	return frame_pos_global;
}

Vector2 REDControllerBase::get_target_zoom(){
	Vector2 frame_camera_zoom = frame->get_camera_zoom();
	Vector2 camera_zoom_min_clamped = Vector2(MIN(camera_zoom_min.x, frame_camera_zoom.x), MIN(camera_zoom_min.y, frame_camera_zoom.y));
	Vector2 camera_zoom_max_clamped = Vector2(MAX(camera_zoom_max.x, frame_camera_zoom.x), MAX(camera_zoom_max.y, frame_camera_zoom.y));
	if (zoom_k < 0.5){
		frame_zoom = camera_zoom_min_clamped + zoom_k * 2.0 * (frame_camera_zoom - camera_zoom_min_clamped);
	}
	else{
		frame_zoom = frame_camera_zoom + (zoom_k - 0.5) * 2 * (camera_zoom_max_clamped - frame_camera_zoom);
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

Vector2 REDControllerBase::get_target_parallax(){
	if (zoom_k < 0.5){
		frame_parallax_pos = frame->get_parallax_pos_in().get_origin().linear_interpolate(frame->get_parallax_pos().get_origin(), zoom_k * 2.0);
	}
	else{
		frame_parallax_pos = frame->get_parallax_pos().get_origin().linear_interpolate(frame->get_parallax_pos_out().get_origin(), 2 * zoom_k - 1.0);
	}
	//frame_parallax_pos = frame_parallax_pos*frame->get_scale();
	if (frame->is_focused()){
		frame_parallax = (camera->get_position() - frame->get_origin_pos_gl())/frame->get_scale() - frame_parallax_pos;// + camera->get_offset();
	}
	else{
		frame_parallax = frame_pos_local - frame_parallax_pos;// + camera->get_offset();
	}
	return frame_parallax;
}

void REDControllerBase::set_state(int p_id){
	frame->set_id(p_id);
	frame->travel_state();
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

void REDControllerBase::set_zoom_k_target(const float p_val) {
	zoom_k_target = CLAMP(p_val, 0.0f, 1.0f);
	if (zoom_k == zoom_k_target)
		return;
	tween->stop(this, "set_zoom_k");	
	tween->interpolate_method(this, "set_zoom_k", zoom_k, zoom_k_target, ABS(zoom_k-zoom_k_target), Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
}

void REDControllerBase::zoom_in(const float &p_val) {
	set_zoom_k_target(zoom_k_target - p_val);
}

void REDControllerBase::zoom_out(const float &p_val) {
	set_zoom_k_target(zoom_k_target + p_val);
}

void REDControllerBase::zoom_reset() {
	set_zoom_k_target(0.5f);
}
void REDControllerBase::set_mouse_offset(const Vector2 &p_mouse_offset) {
	mouse_offset = p_mouse_offset;
	update_camera_pos();

	//if(camera_state == CAMERA_STATIC){
		//tween->stop(camera, "set_position");	
		//tween->follow_method(camera, "set_position", camera->get_position(), this, "get_target_pos", 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		//tween->start();	
		//tween->stop(camera, "set_offset");
		//tween->interpolate_method(camera, "set_offset", camera->get_offset(), mouse_offset_target, 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		//tween->start();
	//}
}


void REDControllerBase::set_mouse_offset_k(const Vector2 &p_max_mouse_offset) {
	if (camera_zoom_max == p_max_mouse_offset)
		return;
	mouse_offset_k = p_max_mouse_offset;
	update_camera_pos();

	//if(camera_state == CAMERA_STATIC){
		//tween->stop(camera, "set_position");	
		//tween->follow_method(camera, "set_position", camera->get_position(), this, "get_target_pos", 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		//tween->start();	
	//}
}

Vector2 REDControllerBase::get_mouse_offset_k() const{
	return mouse_offset_k;
}

void REDControllerBase::set_zoom_k(const float p_zoom) {
	if(zoom_k == p_zoom){
		return;
	}
	zoom_k = p_zoom;
	update_camera();
}

float REDControllerBase::get_zoom_k() const{
	return zoom_k;
}

void REDControllerBase::update_camera() {
	if (frame == nullptr)
		return;
	if (camera_state == CAMERA_STATIC){
		camera->set_position(get_target_pos());
		camera->set_zoom(get_target_zoom());
		//camera->set_offset(get_target_mouse());
		//Size2 frame_size = frame->_edit_get_rect().size;
		//frame->set_frame_scale((Vector2(frame_size.x, frame_size.x)+frame_expand)/(frame_size.x));
	}
}

void REDControllerBase::update_camera_pos() {
	if (frame == nullptr)
		return;
	if (camera_state == CAMERA_STATIC){
		camera->set_position(get_target_pos());
	}
}

void REDControllerBase::update_camera_parallax() {
	if (frame == nullptr)
		return;
	if (camera_state == CAMERA_STATIC){
		frame->set_parallax(get_target_parallax());
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
	return (frame == nullptr) ? camera_zoom_min + zoom_k * (camera_zoom_max - camera_zoom_min) : camera_zoom_min + zoom_k * (camera_zoom_max - camera_zoom_min) * frame->get_camera_zoom();
}



Vector2 REDControllerBase::get_mouse_offset() const{
	return mouse_offset;
}

bool REDControllerBase::can_control() const {
	return b_can_control;
}

void REDControllerBase::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:{
			if (!Engine::get_singleton()->is_editor_hint()) {
				set_process_input(true);
				frame_timer = memnew(Timer);
				add_child(frame_timer);
				frame_timer->set_owner(this);
				frame_timer->connect("timeout", this, "_frame_change");

				camera_timer = memnew(Timer);
				add_child(camera_timer);
				camera_timer->set_owner(this);
				camera_timer->connect("timeout", this, "_frame_changed");

				tween = memnew(Tween);
				add_child(tween);


				group_name = "__cameras_" + itos(get_viewport()->get_viewport_rid().get_id());
				add_to_group(group_name);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			get_tree()->get_root()->disconnect("size_changed", this, "update_camera");
			remove_from_group(group_name);
		} break;
		case NOTIFICATION_READY: {
			camera = (Camera2D*)(get_node(camera_path));
			if (!camera){
				camera = memnew(Camera2D);
				add_child(camera);
				camera->set_owner(this);
			}
			get_tree()->get_root()->connect("size_changed", this, "update_camera");
		} break;
	}
}

void REDControllerBase::mouse_move(const Vector2 &p_mouse_pos){
	Vector2 target = p_mouse_pos/get_viewport()->get_size() - Vector2(0.5, 0.5);
	if (mouse_offset == target)
		return;
	tween->stop(this, "set_mouse_offset");	
	tween->interpolate_method(this, "set_mouse_offset", mouse_offset, target, mouse_offset.distance_to(target), Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
}

void REDControllerBase::_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseMotion> mb = p_event;
	if (mb.is_valid()) {
		mouse_move(mb->get_position());
		//tween->stop(this, "set_mouse_offset");	
		//tween->interpolate_method(this, "set_mouse_offset", mouse_offset, 
		//						  mb->get_position()/get_viewport()->get_size() - Vector2(0.5, 0.5), 
		//						  ABS(mouse_offset-camera_mouse_offset_target), Tween::TRANS_CUBIC, Tween::EASE_OUT);
		//tween->start();	
		//set_mouse_offset(mb->get_position()/get_viewport()->get_size() - Vector2(0.5, 0.5));
	}
}

void REDControllerBase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_issue_by_path", "issue_path"), &REDControllerBase::set_issue_by_path);
	ClassDB::bind_method(D_METHOD("_frame_start"), &REDControllerBase::_frame_start);
	ClassDB::bind_method(D_METHOD("_frame_end"), &REDControllerBase::_frame_end);
	
	ClassDB::bind_method(D_METHOD("mouse_move", "mouse_offset"), &REDControllerBase::mouse_move);
	ClassDB::bind_method(D_METHOD("zoom_out", "zoom_val"), &REDControllerBase::zoom_out);
	ClassDB::bind_method(D_METHOD("zoom_in", "zoom_val"), &REDControllerBase::zoom_in);
	ClassDB::bind_method(D_METHOD("to_next"), &REDControllerBase::to_next);
	ClassDB::bind_method(D_METHOD("to_prev"), &REDControllerBase::to_prev);

	ClassDB::bind_method(D_METHOD("set_reset_camera_on_frame_change", "reset_camera"), &REDControllerBase::set_reset_camera_on_frame_change);
	ClassDB::bind_method(D_METHOD("is_reset_camera_on_frame_change"), &REDControllerBase::is_reset_camera_on_frame_change);

	ClassDB::bind_method(D_METHOD("set_frame_scale", "frame_scale"), &REDControllerBase::set_frame_scale);
	ClassDB::bind_method(D_METHOD("set_camera_mode", "camera_mode"), &REDControllerBase::set_camera_mode);
	ClassDB::bind_method(D_METHOD("get_camera_mode"), &REDControllerBase::get_camera_mode);
	ClassDB::bind_method(D_METHOD("set_camera_path", "camera_path"), &REDControllerBase::set_camera_path);
	ClassDB::bind_method(D_METHOD("get_camera_path"), &REDControllerBase::get_camera_path);

	ClassDB::bind_method(D_METHOD("set_zoom_k", "zoom_k"), &REDControllerBase::set_zoom_k);
	ClassDB::bind_method(D_METHOD("get_zoom_k"), &REDControllerBase::get_zoom_k);
	ClassDB::bind_method(D_METHOD("set_camera_zoom_min", "camera_zoom_min"), &REDControllerBase::set_camera_zoom_min);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_min"), &REDControllerBase::get_camera_zoom_min);
	ClassDB::bind_method(D_METHOD("set_camera_zoom_max", "camera_zoom_max"), &REDControllerBase::set_camera_zoom_max);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_max"), &REDControllerBase::get_camera_zoom_max);

	ClassDB::bind_method(D_METHOD("can_control"), &REDControllerBase::can_control);

	ClassDB::bind_method(D_METHOD("set_camera_smooth", "camera_smooth"), &REDControllerBase::set_camera_smooth);
	ClassDB::bind_method(D_METHOD("get_camera_smooth"), &REDControllerBase::get_camera_smooth);

	ClassDB::bind_method(D_METHOD("update_camera_pos"), &REDControllerBase::update_camera_pos);
	ClassDB::bind_method(D_METHOD("update_camera"), &REDControllerBase::update_camera);
	ClassDB::bind_method(D_METHOD("update_camera_to_frame"), &REDControllerBase::update_camera);
	ClassDB::bind_method(D_METHOD("update_camera_parallax()"), &REDControllerBase::update_camera_parallax);

	ClassDB::bind_method(D_METHOD("set_mouse_offset", "mouse_offset"), &REDControllerBase::set_mouse_offset);
	ClassDB::bind_method(D_METHOD("get_mouse_offset"), &REDControllerBase::get_mouse_offset);
	ClassDB::bind_method(D_METHOD("set_mouse_offset_k", "mouse_offset_k"), &REDControllerBase::set_mouse_offset_k);
	ClassDB::bind_method(D_METHOD("get_mouse_offset_k"), &REDControllerBase::get_mouse_offset_k);

	ClassDB::bind_method(D_METHOD("_camera_moved"), &REDControllerBase::_camera_moved);
	ClassDB::bind_method(D_METHOD("_frame_change"), &REDControllerBase::_frame_change);
	ClassDB::bind_method(D_METHOD("_frame_changed"), &REDControllerBase::_frame_changed);
	
	ClassDB::bind_method(D_METHOD("get_target_pos"), &REDControllerBase::get_target_pos);
	ClassDB::bind_method(D_METHOD("get_target_zoom"), &REDControllerBase::get_target_zoom);
	ClassDB::bind_method(D_METHOD("get_target_mouse"), &REDControllerBase::get_target_mouse);
	ClassDB::bind_method(D_METHOD("get_target_parallax"), &REDControllerBase::get_target_parallax);
	
	ClassDB::bind_method(D_METHOD("set_frame_expand", "frame_expand"), &REDControllerBase::set_frame_expand);
	ClassDB::bind_method(D_METHOD("get_frame_expand"), &REDControllerBase::get_frame_expand);
	ClassDB::bind_method(D_METHOD("_input"), &REDControllerBase::_input);

	ADD_GROUP("Frame", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "frame_expand"), "set_frame_expand", "get_frame_expand");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reset_camera_on_frame_change"), "set_reset_camera_on_frame_change", "is_reset_camera_on_frame_change");

	ADD_GROUP("Camera", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Camera2D"), "set_camera_path", "get_camera_path");

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "zoom_k"), "set_zoom_k", "get_zoom_k");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom_min"), "set_camera_zoom_min", "get_camera_zoom_min");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom_max"), "set_camera_zoom_max", "get_camera_zoom_max");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "mouse_offset_k"), "set_mouse_offset_k", "get_mouse_offset_k");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_smooth"), "set_camera_smooth", "get_camera_smooth");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_mode"), "set_camera_mode", "get_camera_mode");
}

REDControllerBase::REDControllerBase() {
	reset_camera_on_frame_change = true;
	frame_expand = Vector2(100.0, 100.0);
	dirrection = DIRRECTION_FORWARD;
	issue = nullptr;
    page = nullptr;
	frame = nullptr;
	frame_timer_connected = false;
	frame_changing = false;

	camera_mode = true;
	b_can_control = true;
	zoom_k = 0.5;
	zoom_k_target = 0.5;
	camera_zoom_min = Vector2(0.5f, 0.5f);
	camera_zoom_max = Vector2(4, 4);
	mouse_offset_k = Vector2(1, 1);
	mouse_offset = Vector2(0, 0);
}
