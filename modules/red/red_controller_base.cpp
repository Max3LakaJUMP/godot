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

void REDControllerBase::size_changed(){
	_window_resized();
	size_dirty = true;
	target_zoom_dirty = true;
	if (b_tween_created){
		screen_multiplayer_start = screen_multiplayer;
		orientation_k = 0.0;
		tween->stop(this, "set_orientation_k");
		tween->interpolate_method(this, "set_orientation_k", orientation_k, 1.0, 1.0, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		tween->start();	
	}
}
void REDControllerBase::_window_resized(){
	if (get_script_instance() && get_script_instance()->has_method("_window_resized")) {
		get_script_instance()->call("_window_resized");
	}
}

void REDControllerBase::_frame_zoom_changed(){
	size_dirty = true;
	target_zoom_dirty = true;
	update_camera_zoom();
};

void REDControllerBase::_target_pos_moved(){
	target_pos_local_dirty = true;
	update_camera_pos();
};

void REDControllerBase::_target_parallax_moved(){
	target_parallax_dirty = true;
	update_camera_parallax();
};

void REDControllerBase::_frame_start(){
	if (!frame->is_active()){
		frame->set_active(true);
		frame->connect("_frame_zoom_changed", this, "_frame_zoom_changed");
		frame->connect("_target_pos_moved", this, "_target_pos_moved");
		frame->connect("_target_parallax_moved", this, "_target_parallax_moved");
	}
	int id = frame->get_id();
	int last_id = frame->get_states_count() - 1;
	if (id == 0 && id < last_id){
		set_state(id + 1);
	}else if (id == last_id && id > 0){
		set_state(id - 1);
	}
}


void REDControllerBase::_frame_end(){
	target_zoom_dirty = true;
	target_pos_local_dirty = true;
	target_parallax_dirty = true;
	tween->stop(frame, "set_parallax_zoom");
	tween->stop(frame, "set_parallax_offset");
	tween->stop(frame, "set_frame_scale");
	tween->interpolate_method(frame, "set_parallax_zoom", frame->get_parallax_zoom(), Vector2(1.0, 1.0), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->interpolate_method(frame, "set_parallax_offset", frame->get_parallax_offset(), Vector2(0.0, 0.0), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->interpolate_method(frame, "set_frame_scale", frame->get_scale(), Vector2(1.0, 1.0), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
	if (frame->is_active()){
		frame->set_active(false);
		frame->disconnect("_frame_zoom_changed", this, "_frame_zoom_changed");
		frame->disconnect("_target_pos_moved", this, "_target_pos_moved");
		frame->disconnect("_target_parallax_moved", this, "_target_parallax_moved");
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
	ERR_FAIL_NULL(frame);
	if (frame->is_start_immediate()){
		_frame_start();
	}
	update_camera_to_frame();
	if (first_frame){
		camera->reset_smoothing();
	}
}

void REDControllerBase::update_camera_to_frame(bool force_immediate) {
    if (!camera_mode || Engine::get_singleton()->is_editor_hint())
		return;
	ERR_FAIL_NULL(frame);
    if (get_camera_path().is_empty())
		return;
	if (reset_camera_on_frame_change)
		set_zoom_k_target(0.0);
	if (tween->is_active()){
		tween->stop(camera, "set_position");	
		tween->stop(camera, "set_zoom");
		tween->stop(frame, "set_parallax_zoom");
		tween->stop(frame, "set_parallax_offset");
		tween->stop(frame, "set_frame_scale");
	}
	Size2 frame_size = frame->_edit_get_rect().size;
	if(camera_smooth && !force_immediate){
		camera_state = CAMERA_MOVING;
		Tween::TransitionType tween_transition_type = Tween::TRANS_CUBIC;
		Tween::EaseType tween_ease_type = Tween::EASE_OUT;
		float tween_duration = MAX(ABS(camera->get_position().distance_to(get_target_pos_global()))/4000.0f, 0.5);
		tween->follow_method(camera, "set_position", camera->get_position(), this, "get_target_pos_global", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(camera, "set_zoom", camera->get_zoom(), this, "get_target_camera_zoom", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(frame, "set_parallax_zoom", frame->get_parallax_zoom(), this, "get_target_parallax_zoom", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(frame, "set_parallax_offset", frame->get_parallax_offset(), this, "get_target_parallax", tween_duration, tween_transition_type, tween_ease_type);
 		tween->interpolate_method(frame, "set_frame_scale", frame->get_scale(), (Vector2(frame_size.x, frame_size.x)+frame_expand)/(frame_size.x), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		tween->start();	
		frame_timer_connected = true;
		camera_timer->start(tween_duration + 0.01);
	}
	else{
		tween->interpolate_method(this, "set_frame_scale", frame->get_scale(), (frame_size+frame_expand)/(frame_size), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		tween->start();	
		camera->set_position(get_target_pos_global());
		camera->set_zoom(get_target_camera_zoom());
		_frame_changed();
	}
}

void REDControllerBase::_frame_changed() {
	ERR_FAIL_NULL(frame);
	camera_state = CAMERA_STATIC;
	last_dirrection = dirrection;
	dirrection = DIRRECTION_NONE;
	camera_timer->stop();
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
			frame->set_parallax_offset(get_target_parallax());
			frame->set_parallax_zoom(get_target_parallax_zoom());
		}
	}
	Vector2 cz = camera->get_zoom();
	if (old_camera_zoom != cz){
		if (issue != nullptr){
			old_camera_zoom = cz;
			issue->update_camera_zoom(cz);
		}
		else if (page != nullptr){
			page->update_camera_zoom(cz);
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
	ERR_FAIL_NULL_V(issue, false)
	dirrection = DIRRECTION_BACKWARD;
	int new_id = issue->get_id() - 1;
	int pages_count = issue->get_pages_count();
	for (int i = new_id; i >= 0 && i < pages_count; i--){
		if (issue->get_page(i)->get_frames_count() > 0){
			set_page(i, true);
			return true;
		}else{
			ERR_PRINTS("Skiping page: no frames");
		}
	}
	return false;
}

bool REDControllerBase::to_next_page() {
	ERR_FAIL_NULL_V(issue, false)
	int new_id = issue->get_id() + 1;
	int pages_count = issue->get_pages_count();
	for (int i = new_id; i >= 0 && i < pages_count; i++){
		if (issue->get_page(i)->get_frames_count() > 0){
			set_page(i);
			return true;
		}else{
			ERR_PRINTS("Skiping page: no frames");
		}
	}
	return false;
}


bool REDControllerBase::to_prev_frame() {
	ERR_FAIL_NULL_V(page, false)
	int new_id = page->get_id() - 1;
	if (new_id > -1 && new_id < page->get_frames_count()){
		set_frame(new_id, false);
		return true;
	}
	return false;
}

bool REDControllerBase::to_next_frame() {
	ERR_FAIL_NULL_V(page, false)
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
	ERR_FAIL_NULL_V(frame, false)
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
	ERR_FAIL_NULL_V(frame, false)
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
	ERR_FAIL_NULL(p_issue);
    issue = p_issue;
	set_page(issue->get_id());
}

REDIssue *REDControllerBase::get_issue() const {
    return issue;
}

void REDControllerBase::set_page(REDPage *p_page){
	ERR_FAIL_NULL(p_page)  
	page = p_page;
	set_frame(page->get_id());
}

void REDControllerBase::set_page(int p_id, bool is_prev){
	ERR_FAIL_NULL(issue)  
	issue->set_page(p_id, is_prev, get_target_camera_zoom());
	page = issue->get_page(p_id);
	ERR_FAIL_NULL(page)  
	set_frame(page->get_id(), !is_prev);
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

Vector2 REDControllerBase::get_target_mouse() const{
	if (target_mouse_dirty){
		//mouse_offset_target*=frame->get_scale();
		target_mouse_dirty = false;
	}
	//Size2 v_size = get_viewport()->get_size();

	mouse_offset_target = mouse_offset * mouse_offset_k * camera->get_zoom() / frame->get_camera_zoom();
	if (zoom_k < 0.0)
		mouse_offset_target *= 1 + zoom_k;
	mouse_offset_target *= get_viewport()->get_size();
	return mouse_offset_target;
}

Vector2 REDControllerBase::get_target_pos_local() const{
	if(target_pos_local_dirty){
		if (zoom_k < 0)
			frame_pos_local = frame->get_camera_pos().get_origin().linear_interpolate(frame->get_camera_pos_out().get_origin(), zoom_k * -1);
		else
			frame_pos_local = frame->get_camera_pos().get_origin().linear_interpolate(frame->get_camera_pos_in().get_origin(), zoom_k);
		target_pos_local_dirty = false;
	}
	return frame_pos_local + get_target_mouse();
}

Vector2 REDControllerBase::get_target_pos_global() const{
	return get_target_pos_local() * frame->get_scale() + frame->get_origin_pos_gl();
}

float REDControllerBase::get_screen_multiplayer() const{
	if (orientation_k == 1.0 && !size_dirty){
		return screen_multiplayer;
	}
	float screen_multiplayer_next;
	if (orientation == ORIENTATION_HORIZONTAL)
		screen_multiplayer_next = page->get_size().width / get_viewport()->get_size().width;
	else if (orientation == ORIENTATION_VERTICAL)
		screen_multiplayer_next = page->get_size().height / get_viewport()->get_size().height;
	else
		screen_multiplayer_next = page->get_size().width / get_viewport()->get_size().height;
	screen_multiplayer = (screen_multiplayer_start + orientation_k * (screen_multiplayer_next - screen_multiplayer_start));
	if (orientation_k == 1.0){
		size_dirty = false;
	}
	return screen_multiplayer;
}

Vector2 REDControllerBase::get_target_zoom() const{
	if(target_zoom_dirty){
		Vector2 frame_camera_zoom = frame->get_camera_zoom();
		if (zoom_k < 0){
			Vector2 camera_zoom_max_clamped = Vector2(MAX(camera_zoom_max.x, frame_camera_zoom.x), MAX(camera_zoom_max.y, frame_camera_zoom.y));
			frame_parallax_zoom = frame_camera_zoom + zoom_k * -1 * (camera_zoom_max_clamped - frame_camera_zoom);
		}else{
			Vector2 camera_zoom_min_clamped = Vector2(MIN(camera_zoom_min.x, frame_camera_zoom.x), MIN(camera_zoom_min.y, frame_camera_zoom.y));
			frame_parallax_zoom = frame_camera_zoom + zoom_k * (camera_zoom_min_clamped - frame_camera_zoom);
		}
		target_zoom_dirty = false;
	}
	return frame_parallax_zoom;
}
Vector2 REDControllerBase::get_target_camera_zoom() const{
	if (page){
		return get_target_zoom() * get_screen_multiplayer();
	}
	return Vector2(1, 1);
}
/*
Vector2 REDControllerBase::get_target_camera_zoom() const{
	if (page){
		if (orientation == ORIENTATION_HORIZONTAL)
			return get_target_zoom() * page->get_size().width / get_viewport()->get_size().width;
		else if (orientation == ORIENTATION_VERTICAL)
			return get_target_zoom() * page->get_size().height / get_viewport()->get_size().height;
		else
			return get_target_zoom() * page->get_size().width / get_viewport()->get_size().height;
	}
	return Vector2(1, 1);
}*/

Vector2 REDControllerBase::get_target_parallax_zoom() const{
	return (camera->get_zoom() / (frame->get_camera_zoom() + abs(zoom_k) * (Vector2(1.0f, 1.0f) - frame->get_camera_zoom()))) * get_screen_multiplayer();
}

Vector2 REDControllerBase::get_target_parallax() const{
	if (target_parallax_dirty){
		if (zoom_k < 0){
			frame_parallax = frame->get_parallax_pos().get_origin().linear_interpolate(frame->get_parallax_pos_out().get_origin(), zoom_k * -1);
		}
		else{
			frame_parallax = frame->get_parallax_pos().get_origin().linear_interpolate(frame->get_parallax_pos_in().get_origin(), zoom_k);
		}
		target_parallax_dirty = false;
		//frame_parallax_pos = frame_parallax_pos*frame->get_scale();
	}
	if (frame->is_focused()){
		return frame_parallax - (camera->get_position() - frame->get_origin_pos_gl()) / frame->get_scale();// + camera->get_offset();
	}
	else{
		return frame_parallax - get_target_pos_local();// + camera->get_offset();
	}
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
	zoom_k_target = CLAMP(p_val, -1.0f, 1.0f);
	if (zoom_k == zoom_k_target)
		return;
	tween->stop(this, "set_zoom_k");	
	tween->interpolate_method(this, "set_zoom_k", zoom_k, zoom_k_target, ABS(zoom_k - zoom_k_target), Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
}

void REDControllerBase::zoom_in(const float &p_val) {
	set_zoom_k_target(zoom_k_target + p_val);
}

void REDControllerBase::zoom_out(const float &p_val) {
	set_zoom_k_target(zoom_k_target - p_val);
}

void REDControllerBase::zoom_reset() {
	set_zoom_k_target(0.0f);
}

void REDControllerBase::set_mouse_offset(const Vector2 &p_mouse_offset) {
	if (mouse_offset == p_mouse_offset)
		return;
	mouse_offset = p_mouse_offset;
	target_mouse_dirty = true;
	update_camera_pos();
}


void REDControllerBase::set_mouse_offset_k(const Vector2 &p_max_mouse_offset) {
	if (camera_zoom_max == p_max_mouse_offset)
		return;
	mouse_offset_k = p_max_mouse_offset;
	target_mouse_dirty = true;
	update_camera_pos();
}

Vector2 REDControllerBase::get_mouse_offset_k() const{
	return mouse_offset_k;
}

void REDControllerBase::set_zoom_k(const float p_zoom) {
	if(zoom_k == p_zoom){
		return;
	}
	zoom_k = p_zoom;
	target_pos_local_dirty = true;
	target_zoom_dirty = true;
	target_parallax_dirty= true;
	target_mouse_dirty = true;
	update_camera();
}

float REDControllerBase::get_zoom_k() const{
	return zoom_k;
}

void REDControllerBase::update_camera() {
	if (frame == nullptr)
		return;
	if (camera_state == CAMERA_STATIC){
		camera->set_position(get_target_pos_global());
		camera->set_zoom(get_target_camera_zoom());
		//camera->set_offset(get_target_mouse());
		//Size2 frame_size = frame->_edit_get_rect().size;
		//frame->set_frame_scale((Vector2(frame_size.x, frame_size.x)+frame_expand)/(frame_size.x));
	}
}

void REDControllerBase::update_camera_zoom() {
	if (frame == nullptr)
		return;
	if (camera_state == CAMERA_STATIC){
		camera->set_zoom(get_target_camera_zoom());
	}
}

void REDControllerBase::update_camera_pos() {
	if (frame == nullptr)
		return;
	if (camera_state == CAMERA_STATIC){
		camera->set_position(get_target_pos_global());
	}
}

void REDControllerBase::update_camera_parallax() {
	if (frame == nullptr)
		return;
	if (camera_state == CAMERA_STATIC){
		frame->set_parallax_offset(get_target_parallax());
		frame->set_parallax_zoom(camera->get_zoom() * get_screen_multiplayer());
	}
}

void REDControllerBase::set_camera_zoom_min(const Vector2 &p_zoom) {
	if (camera_zoom_min == p_zoom)
		return;
	camera_zoom_min = p_zoom;
	target_zoom_dirty = true;
	update_camera_zoom();
}

Vector2 REDControllerBase::get_camera_zoom_min() const{
	return camera_zoom_min;
}

void REDControllerBase::set_camera_zoom_max(const Vector2 &p_zoom) {
	if (camera_zoom_max == p_zoom)
		return;
	camera_zoom_max = p_zoom;
	target_zoom_dirty = true;
	update_camera_zoom();
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
				group_name = "__cameras_" + itos(get_viewport()->get_viewport_rid().get_id());
				add_to_group(group_name);
				set_process_input(true);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				get_tree()->get_root()->disconnect("size_changed", this, "size_changed");
				remove_from_group(group_name);
				tween->queue_delete();
				frame_timer->queue_delete();
				camera_timer->queue_delete();
			}
		} break;
		case NOTIFICATION_READY: {
			camera = (Camera2D*)(get_node(camera_path));
			if (!Engine::get_singleton()->is_editor_hint()) {
				tween = red::create_node<Tween>(this);
				frame_timer = red::create_node<Timer>(this);
				camera_timer = red::create_node<Timer>(this);
				frame_timer->connect("timeout", this, "_frame_change");
				camera_timer->connect("timeout", this, "_frame_changed");
				b_tween_created = true;
			}
			get_tree()->get_root()->connect("size_changed", this, "size_changed");
		} break;
	}
}

void REDControllerBase::_mouse_moved(const Vector2 &p_mouse_pos){
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
		_mouse_moved(mb->get_position());
		//tween->stop(this, "set_mouse_offset");	
		//tween->interpolate_method(this, "set_mouse_offset", mouse_offset, 
		//						  mb->get_position()/get_viewport()->get_size() - Vector2(0.5, 0.5), 
		//						  ABS(mouse_offset-camera_mouse_offset_target), Tween::TRANS_CUBIC, Tween::EASE_OUT);
		//tween->start();	
		//set_mouse_offset(mb->get_position()/get_viewport()->get_size() - Vector2(0.5, 0.5));
	}
}

void REDControllerBase::set_orientation_k(float p_orientation_k) {
	if (orientation_k == p_orientation_k)
		return;
	orientation_k = p_orientation_k;
	update_camera_zoom();
	update_camera_pos();
}


void REDControllerBase::set_orientation(const Orientation p_orientation) {
	if (orientation == p_orientation)
		return;
	orientation = p_orientation;
	orientation_k = 0.0;
	if (!camera_mode || Engine::get_singleton()->is_editor_hint())
		return;
	size_dirty = true;
	screen_multiplayer_start = screen_multiplayer;
	if (b_tween_created){
		tween->stop(this, "set_orientation_k");
		tween->interpolate_method(this, "set_orientation_k", orientation_k, 1.0, 1.0, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		tween->start();	
	}
	else{
		set_orientation_k(1.0);
	}
}

REDControllerBase::Orientation REDControllerBase::get_orientation() const{
	return orientation;
}

void REDControllerBase::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_issue_by_path", "issue_path"), &REDControllerBase::set_issue_by_path);
	ClassDB::bind_method(D_METHOD("_frame_start"), &REDControllerBase::_frame_start);
	ClassDB::bind_method(D_METHOD("_frame_end"), &REDControllerBase::_frame_end);
	
	ClassDB::bind_method(D_METHOD("_mouse_moved", "mouse_offset"), &REDControllerBase::_mouse_moved);
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

	ClassDB::bind_method(D_METHOD("_frame_zoom_changed"), &REDControllerBase::_frame_zoom_changed);
	ClassDB::bind_method(D_METHOD("_target_parallax_moved"), &REDControllerBase::_target_parallax_moved);
	ClassDB::bind_method(D_METHOD("_target_pos_moved"), &REDControllerBase::_target_pos_moved);

	ClassDB::bind_method(D_METHOD("update_camera_pos"), &REDControllerBase::update_camera_pos);
	ClassDB::bind_method(D_METHOD("update_camera"), &REDControllerBase::update_camera);
	ClassDB::bind_method(D_METHOD("update_camera_to_frame"), &REDControllerBase::update_camera);
	ClassDB::bind_method(D_METHOD("update_camera_parallax"), &REDControllerBase::update_camera_parallax);

	ClassDB::bind_method(D_METHOD("set_mouse_offset", "mouse_offset"), &REDControllerBase::set_mouse_offset);
	ClassDB::bind_method(D_METHOD("get_mouse_offset"), &REDControllerBase::get_mouse_offset);
	ClassDB::bind_method(D_METHOD("set_mouse_offset_k", "mouse_offset_k"), &REDControllerBase::set_mouse_offset_k);
	ClassDB::bind_method(D_METHOD("get_mouse_offset_k"), &REDControllerBase::get_mouse_offset_k);

	ClassDB::bind_method(D_METHOD("_camera_moved"), &REDControllerBase::_camera_moved);
	ClassDB::bind_method(D_METHOD("_frame_change"), &REDControllerBase::_frame_change);
	ClassDB::bind_method(D_METHOD("_frame_changed"), &REDControllerBase::_frame_changed);
	
	ClassDB::bind_method(D_METHOD("get_target_pos_local"), &REDControllerBase::get_target_pos_local);
	ClassDB::bind_method(D_METHOD("get_target_pos_global"), &REDControllerBase::get_target_pos_global);
	ClassDB::bind_method(D_METHOD("get_target_zoom"), &REDControllerBase::get_target_zoom);
	ClassDB::bind_method(D_METHOD("get_target_camera_zoom"), &REDControllerBase::get_target_camera_zoom);
	ClassDB::bind_method(D_METHOD("get_target_mouse"), &REDControllerBase::get_target_mouse);
	ClassDB::bind_method(D_METHOD("get_target_parallax"), &REDControllerBase::get_target_parallax);
	ClassDB::bind_method(D_METHOD("get_target_parallax_zoom"), &REDControllerBase::get_target_parallax_zoom);

	ClassDB::bind_method(D_METHOD("set_frame_expand", "frame_expand"), &REDControllerBase::set_frame_expand);
	ClassDB::bind_method(D_METHOD("get_frame_expand"), &REDControllerBase::get_frame_expand);
	ClassDB::bind_method(D_METHOD("_input"), &REDControllerBase::_input);
	ClassDB::bind_method(D_METHOD("set_orientation_k", "orientation_k"), &REDControllerBase::set_orientation_k);
	ClassDB::bind_method(D_METHOD("set_orientation", "orientation"), &REDControllerBase::set_orientation);
	ClassDB::bind_method(D_METHOD("get_orientation"), &REDControllerBase::get_orientation);
	ClassDB::bind_method(D_METHOD("size_changed"), &REDControllerBase::size_changed);

	ClassDB::add_virtual_method(get_class_static(), MethodInfo("_window_resized"));
	
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
	ADD_PROPERTY(PropertyInfo(Variant::INT, "orientation", PROPERTY_HINT_ENUM, "ORIENTATION_HORIZONTAL, ORIENTATION_VERTICAL, ORIENTATION_HYBRID"), "set_orientation", "get_orientation");
	
	BIND_ENUM_CONSTANT(ORIENTATION_HORIZONTAL);
	BIND_ENUM_CONSTANT(ORIENTATION_VERTICAL);
	BIND_ENUM_CONSTANT(ORIENTATION_HYBRID);
}

REDControllerBase::REDControllerBase() {
	orientation = ORIENTATION_HYBRID;
	orientation_k = 1.0;
	screen_multiplayer_start = 1.0;
	size_dirty = true;
	target_pos_local_dirty = true;
	target_zoom_dirty = true;
	target_mouse_dirty = true;
	target_parallax_dirty = true;

	reset_camera_on_frame_change = true;
	frame_expand = Vector2(250.0, 250.0);
	dirrection = DIRRECTION_NONE;
	issue = nullptr;
    page = nullptr;
	frame = nullptr;
	frame_timer_connected = false;
	frame_changing = false;

	camera_mode = true;
	b_can_control = true;
	b_tween_created = false;
	zoom_k = 0.0;
	zoom_k_target = 0.0;
	frame_parallax_zoom = Vector2(1, 1);
	camera_zoom_min = Vector2(0.5f, 0.5f);
	camera_zoom_max = Vector2(4, 4);
	mouse_offset_k = Vector2(1, 1);
	mouse_offset = Vector2(0, 0);
	screen_multiplayer = 1.0;
}
