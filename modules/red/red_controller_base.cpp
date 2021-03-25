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
#include "scene/scene_string_names.h"
#include "scene/animation/tween.h"
#include "scene/main/timer.h"

// setget
void REDControllerBase::set_orientation(const Orientation p_orientation) {
	if (orientation == p_orientation)
		return;
	orientation = p_orientation;
	if (!camera_mode || Engine::get_singleton()->is_editor_hint())
		return;
	_update_orientation();
}

REDControllerBase::Orientation REDControllerBase::get_orientation() const{
	return orientation;
}

void REDControllerBase::set_camera_smooth(const bool p_smooth){
	camera_smooth = p_smooth;
}

bool REDControllerBase::get_camera_smooth() const{
	return camera_smooth;
}

void REDControllerBase::set_camera_speed(float p_camera_speed){
	camera_speed = p_camera_speed;
}

float REDControllerBase::get_camera_speed() const{
	return camera_speed;
}

void REDControllerBase::set_camera_zoom_min(const Vector2 &p_zoom) {
	camera_zoom_min = p_zoom;
	_update_camera_zoom();
}

Vector2 REDControllerBase::get_camera_zoom_min() const{
	return camera_zoom_min;
}

void REDControllerBase::set_camera_zoom_max(const Vector2 &p_zoom) {
	camera_zoom_max = p_zoom;
	_update_camera_zoom();
}

Vector2 REDControllerBase::get_camera_zoom_max() const{
	return camera_zoom_max;
}

void REDControllerBase::set_mouse_offset_k(const Vector2 &p_max_mouse_offset) {
	if (camera_zoom_max == p_max_mouse_offset)
		return;
	mouse_offset_k = p_max_mouse_offset;
	_update_mouse_pos();
}

Vector2 REDControllerBase::get_mouse_offset_k() const{
	return mouse_offset_k;
}

void REDControllerBase::set_tween(Tween *p_tween){
	tween = p_tween;
}

Tween *REDControllerBase::get_tween() const{
	return tween;
}

void REDControllerBase::_frame_start(){
	if (!frame->is_active()){
		frame->set_active(true);
		frame->connect("_frame_zoom_changed", this, "_update_camera_zoom");
		frame->connect("_target_pos_moved", this, "_update_frame_local_pos");
		frame->connect("_target_parallax_moved", this, "_update_frame_parallax");
	}
	int id = frame->get_id();
	int last_id = frame->get_states_count() - 1;
	if (id == 0 && id < last_id){
		frame->set_current_state_id(id + 1);
	}else if (id == last_id && id > 0){
		frame->set_current_state_id(id - 1);
	}
	_update_camera_zoom();
	_update_frame_local_pos();
	_update_frame_parallax();
}

void REDControllerBase::_frame_end(){
	// _update_camera_zoom();
	// _update_frame_local_pos();
	// _update_mouse_pos();
	// _update_frame_parallax();
	// target_zoom_dirty = true;
	// target_pos_local_dirty = true;
	// target_parallax_offset_dirty = true;

	tween->stop(frame, "set_parallax_zoom");
	tween->stop(frame, "set_parallax_offset");
	// tween->stop(frame, "set_frame_scale");
	tween->interpolate_method(frame, "set_parallax_zoom", frame->get_parallax_zoom(), Vector2(1.0, 1.0), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->interpolate_method(frame, "set_parallax_offset", frame->get_parallax_offset(), Vector2(0.0, 0.0), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	// tween->interpolate_method(frame, "set_frame_scale", frame->get_scale(), Vector2(1.0, 1.0), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
	tween->start();	
	if (frame->is_active()){
		frame->set_active(false);
		frame->disconnect("_frame_zoom_changed", this, "_update_camera_zoom");
		frame->disconnect("_target_pos_moved", this, "_update_frame_local_pos");
		frame->disconnect("_target_parallax_moved", this, "_update_frame_parallax");
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
		mouse_pos = get_viewport()->get_size() * 0.5;
	}else{
		frame->set_focused(false);
		_frame_end();
	}
	frame = page->get_frame(page->get_id());
	ERR_FAIL_NULL(frame);
	if (frame->is_start_immediate()){
		// after animation playback init
		VisualServer::get_singleton()->connect("frame_post_draw", this, "_frame_start", varray(), CONNECT_ONESHOT);
		
		// _frame_start();
	}
	update_camera_to_frame(first_frame);
}

void REDControllerBase::update_camera_to_frame(bool force_immediate) {
    if (!camera_mode || Engine::get_singleton()->is_editor_hint())
		return;
	ERR_FAIL_NULL(frame);
    if (get_camera_path().is_empty())
		return;
	if (reset_camera_on_frame_change){
		zoom_reset();
	}
	if (tween->is_active()){
		tween->stop(camera, "set_position");	
		tween->stop(camera, "set_zoom");
		tween->stop(frame, "set_parallax_zoom");
		tween->stop(frame, "set_parallax_offset");
		// tween->stop(frame, "set_frame_scale");
	}
	
	Size2 frame_size = red::get_rect(frame->get_polygon(), frame->get_offset()).size;
	if(camera_smooth && !force_immediate){
		camera_state = CAMERA_MOVING;
		Tween::TransitionType tween_transition_type = Tween::TRANS_CUBIC;
		Tween::EaseType tween_ease_type = Tween::EASE_OUT;
		float tween_duration = MAX(ABS(camera->get_position().distance_to(get_camera_global_pos())) / camera_speed, 0.5);
		tween->follow_method(camera, "set_position", camera->get_position(), this, "get_camera_global_pos", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(camera, "set_zoom", camera->get_zoom(), this, "get_camera_zoom", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(frame, "set_parallax_zoom", frame->get_parallax_zoom(), this, "get_parallax_zoom", tween_duration, tween_transition_type, tween_ease_type);
		tween->follow_method(frame, "set_parallax_offset", frame->get_parallax_offset(), this, "get_parallax_offset", tween_duration, tween_transition_type, tween_ease_type);
 		// tween->interpolate_method(frame, "set_frame_scale", frame->get_scale(), (Vector2(frame_size.x, frame_size.x)+frame_expand)/(frame_size.x), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		tween->start();	
		frame_timer_connected = true;
		camera_timer->start(tween_duration + 0.01);
	}
	else{
		// tween->interpolate_method(frame, "set_frame_scale", frame->get_scale(), (frame_size+frame_expand)/(frame_size), 0.5f, Tween::TRANS_CUBIC, Tween::EASE_OUT);
		_update_orientation();
		orientation_k.current = orientation_k.get_target();
		_update_camera_zoom();
		camera_zoom.current = camera_zoom.get_target();
		_update_frame_local_pos();
		frame_pos_local.current = frame_pos_local.get_target();
		_update_mouse_pos();
		mouse_offset.current = mouse_offset.get_target();
		_update_frame_parallax();
		frame_parallax.current = frame_parallax.get_target();

		tween->start();	
		camera->set_position(get_camera_global_pos());
		camera->set_zoom(get_camera_zoom());
		camera->reset_smoothing();
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
		// after animation playback init
		VisualServer::get_singleton()->connect("frame_post_draw", this, "_frame_start", varray(), CONNECT_ONESHOT);
		// _frame_start();
	}
}

void REDControllerBase::set_reset_camera_on_frame_change(bool p_reset_camera){
	reset_camera_on_frame_change = p_reset_camera;
}

bool REDControllerBase::is_reset_camera_on_frame_change() const{
	return reset_camera_on_frame_change;
}

void REDControllerBase::_camera_moved() {
	if (frame != nullptr){
		if (frame->is_focused()){
			frame->set_parallax_offset(get_parallax_offset());
			frame->set_parallax_zoom(get_parallax_zoom());
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

bool REDControllerBase::to_prev_page() {
	if(issue == nullptr)
		return false;
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
	if(issue == nullptr)
		return false;
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
		frame->set_current_state_id(new_id);
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
		frame->set_current_state_id(new_id);
		if (new_id == 0 || new_id == frame->get_states_count() - 1)
			return false;
		else
			return true;
	}
	return false;
}

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
	_update_orientation();
}

void REDControllerBase::set_page(int p_id, bool is_prev){
	ERR_FAIL_NULL(issue)  
	issue->set_page(p_id, is_prev, get_camera_zoom());
	page = issue->get_page(p_id);
	ERR_FAIL_NULL(page)  
	set_frame(page->get_id(), !is_prev);
	_update_orientation();
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

Vector2 REDControllerBase::get_camera_global_pos() const{
	return frame_pos_local.current;
	return (frame_pos_local.current + mouse_offset.current) * frame->get_scale() + frame->get_origin_pos_gl();
}

Vector2 REDControllerBase::get_camera_zoom() const{
	return camera_zoom.current * orientation_k.current;
}

Vector2 REDControllerBase::get_parallax_zoom() const{
	return (camera->get_zoom() / (frame->get_camera_zoom() + abs(zoom_k.current) * (Vector2(1.0f, 1.0f) - frame->get_camera_zoom()))) * orientation_k.current;
}

Vector2 REDControllerBase::get_parallax_offset() const{
	return frame_parallax.current;
	if (frame->is_focused()){
		return frame_parallax.current - (camera->get_position() - frame->get_origin_pos_gl()) / frame->get_scale();// + camera->get_offset();
	}
	else{
		return frame_parallax.current - (frame_pos_local.current + mouse_offset.current);// + camera->get_offset();
	}
}

REDFrame *REDControllerBase::get_frame() const{
    return frame;
}

void REDControllerBase::set_camera_path(const NodePath &p_camera_path) {
	if (camera_path == p_camera_path)
		return;
	camera_path = p_camera_path;

	if (is_inside_tree()){
		camera = (Camera2D*) get_node(p_camera_path);
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

Vector2 REDControllerBase::get_global_camera_zoom() const{
	return (frame == nullptr) ? camera_zoom_min + zoom_k.current * (camera_zoom_max - camera_zoom_min) : 
			camera_zoom_min + zoom_k.current * (camera_zoom_max - camera_zoom_min) * frame->get_camera_zoom();
}

bool REDControllerBase::can_control() const {
	return b_can_control;
}

void REDControllerBase::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:{
			if (!Engine::get_singleton()->is_editor_hint()) {
				tween = red::create_node<Tween>(this);
				frame_timer = red::create_node<Timer>(this);
				camera_timer = red::create_node<Timer>(this);
				frame_timer->connect("timeout", this, "_frame_change");
				camera_timer->connect("timeout", this, "_frame_changed");
				group_name = "__cameras_" + itos(get_viewport()->get_viewport_rid().get_id());
				add_to_group(group_name);
				set_process_input(true);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				get_tree()->get_root()->disconnect("size_changed", this, "_update_orientation");
				remove_from_group(group_name);
				tween->queue_delete();
				frame_timer->queue_delete();
				camera_timer->queue_delete();
			}
		} break;
		case NOTIFICATION_READY: {
			camera = (Camera2D*)(get_node(camera_path));
			get_tree()->get_root()->connect("size_changed", this, "_update_orientation");
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			bool stop = true;
			float delta = get_process_delta_time();
			zoom_k.process(delta);
			orientation_k.process(delta);
			camera_zoom.process(delta);
			if (camera_zoom.is_active() || zoom_k.is_active() || orientation_k.is_active()){
				stop = false;
				if (camera_state == CAMERA_STATIC)
					camera->set_zoom(get_camera_zoom());
				_update_mouse_pos();
				_update_frame_local_pos();
			}
			frame_pos_local.process(delta);
			mouse_offset.process(delta);
			if (frame_pos_local.is_active() || mouse_offset.is_active()){
				stop = false;
				if (camera_state == CAMERA_STATIC)
					camera->set_position(get_camera_global_pos());
				_update_frame_parallax();
			}
			frame_parallax.process(delta);
			if (frame_parallax.is_active() || orientation_k.is_active()){
				stop = false;
				if (frame != nullptr && camera_state == CAMERA_STATIC){
					frame->set_parallax_offset(get_parallax_offset());
					frame->set_parallax_zoom(camera->get_zoom() * orientation_k.current);
				}
			}
			if(stop)
				set_process_internal(false);
		} break;
	}
}

void REDControllerBase::_update_frame_parallax(){
	if(frame == nullptr)
		return;
	Vector2 frame_camera_zoom = frame->get_camera_zoom();
	Vector2 target = frame->get_parallax_pos().get_origin();
	if (zoom_k.get_target() < 0){
		target = target.linear_interpolate(frame->get_parallax_pos_out().get_origin(), -zoom_k.get_target()); // 0.5 - cos(zoom_k.get_target() * 3.14) * 0.5);
	}
	else{
		target = target.linear_interpolate(frame->get_parallax_pos_in().get_origin(), zoom_k.get_target()); //  0.5 - cos(zoom_k.get_target() * 3.14) * 0.5);
	}

	if (frame->is_focused()){
		target = target - (camera->get_position() - frame->get_origin_pos_gl()) / frame->get_scale();// + camera->get_offset();
	}
	else{
		target = target - (frame_pos_local.get_target() - frame->get_origin_pos_gl()) / frame->get_scale();// + camera->get_offset();
	}
	if(frame_parallax.set_target(target))
		set_process_internal(true);
}

void REDControllerBase::_update_camera_zoom(){
	if(frame == nullptr)
		return;
	Vector2 frame_camera_zoom = frame->get_camera_zoom();
	Vector2 target;
	if (zoom_k.get_target() < 0){
		Vector2 camera_zoom_max_clamped = Vector2(MAX(camera_zoom_max.x, frame_camera_zoom.x), MAX(camera_zoom_max.y, frame_camera_zoom.y));
		target = frame_camera_zoom.linear_interpolate(camera_zoom_max_clamped, -zoom_k.get_target());
	}else{
		Vector2 camera_zoom_min_clamped = Vector2(MIN(camera_zoom_min.x, frame_camera_zoom.x), MIN(camera_zoom_min.y, frame_camera_zoom.y));
		target = frame_camera_zoom.linear_interpolate(camera_zoom_min_clamped, zoom_k.get_target());
	}
	if(camera_zoom.set_target(target)){
		set_process_internal(true);
	}
}

void REDControllerBase::_update_orientation(){
	_window_resized();
}

void REDControllerBase::_update_frame_local_pos(){
	if(frame == nullptr)
		return;
	float target_k = zoom_k.get_target(); // 0.5 - cos(zoom_k.get_target() * 3.14) * 0.5;
	Vector2 local_target = frame->get_camera_pos().get_origin().linear_interpolate(
								  target_k > 0 ? frame->get_camera_pos_in().get_origin() : frame->get_camera_pos_out().get_origin(), 
								  Math::abs(target_k));
	if(frame_pos_local.set_target((local_target + mouse_offset.get_target()) * frame->get_scale() + frame->get_origin_pos_gl()))
		set_process_internal(true);
}

Size2 REDControllerBase::get_viewport_size(){
	if(camera != NULL && camera != nullptr){
		if(camera->is_rotating())
			return camera->get_global_transform().basis_xform(get_viewport()->get_size()).abs();
	}
	return get_viewport()->get_size();
}

void REDControllerBase::_update_mouse_pos(){
	if(camera == nullptr || frame == nullptr)
		return;
	Vector2 viewport_size = get_viewport()->get_size();
	Vector2 clamped_pos = Vector2(MIN(mouse_pos.x, viewport_size.x), MIN(mouse_pos.y, viewport_size.y));
	Vector2 target = (clamped_pos - Vector2(0.5, 0.5) * viewport_size) * mouse_offset_k * camera->get_zoom() / frame->get_camera_zoom();
	// if (zoom_k.get_target() < 0.f)
	// 	target *= 1 + zoom_k.get_target();
	// lock overpage scroll
	Size2 max_offset = (page->get_size() - viewport_size * camera->get_zoom()) * 0.5;
	max_offset.x = MAX(max_offset.width, 0.0001f);
	max_offset.y = MAX(max_offset.height, 0.0001f);
	{
		float max_offset_k = CLAMP((ABS(target.x) / max_offset.x), 0.f, 1.f);
		target.x = target.x + max_offset_k * (SGN(target.x) * max_offset.x - target.x);
	}
	{
		float max_offset_k = CLAMP((ABS(target.y) / max_offset.y), 0.f, 1.f);
		target.y = target.y + max_offset_k * (SGN(target.y) * max_offset.y - target.y);
	}
	if(camera->is_rotating()){
		Transform2D cam_transform = camera->get_global_transform();
		// max_offset = cam_transform.basis_xform(max_offset);
		target = cam_transform.basis_xform(target);
	}
	// mouse_offset.max_offset = max_offset;
	if(mouse_offset.set_target(target))
		set_process_internal(true);
}

void REDControllerBase::zoom_in(float p_val) {
	if(zoom_k.set_target(CLAMP(zoom_k.get_target() + p_val, -1.0f, 1.0f))){
		_update_camera_zoom();
	}
}

void REDControllerBase::zoom_out(float p_val) {
	if(zoom_k.set_target(CLAMP(zoom_k.get_target() - p_val, -1.0f, 1.0f))){
		_update_camera_zoom();
	}
}

void REDControllerBase::zoom_reset() {
	if(zoom_k.set_target(0.0)){
		_update_camera_zoom();
	}
}

void REDControllerBase::_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseMotion> mb = p_event;
	if (mb.is_valid()) {
		_mouse_moved(mb->get_position());
	}
}

void REDControllerBase::_mouse_moved(const Vector2 &p_mouse_pos){
	mouse_pos = p_mouse_pos;
	_update_mouse_pos();
}

void REDControllerBase::_window_resized(){
	float target = 1.0;
	Vector2 viewport_size = get_viewport()->get_size(); // get_viewport_size();
	if(page == nullptr)
		target =  1.0;
	else{
		switch (orientation)
		{
		case ORIENTATION_MINIMUM:
			target = viewport_size.x < viewport_size.y ? page->get_size().width / viewport_size.width : 
														 page->get_size().height / viewport_size.height;
			break;
		case ORIENTATION_HORIZONTAL:
			target = page->get_size().width / viewport_size.width;
			break;
		case ORIENTATION_VERTICAL:
			target = page->get_size().height / viewport_size.height;
			break;
		case ORIENTATION_HYBRID:
			target = page->get_size().width / viewport_size.height;
			break;
		default:
			target = viewport_size.x > viewport_size.y ? page->get_size().width / viewport_size.width : 
														 page->get_size().height / viewport_size.height;
			break;
		}
	}
	if(orientation_k.set_target(target, true)){
		// orientation_k.current = MIN(orientation_k.current, orientation_k.get_target());
		set_process_internal(true);
	}
	ScriptInstance *script = get_script_instance();
	if (script && script->has_method("_window_resized"))
		script->call("_window_resized");
}

void REDControllerBase::rotate_camera(){
	if(camera != NULL && camera != nullptr){
		int new_rotation = Math::round(camera->get_rotation_degrees()) + 90;
		camera->set_rotation_degrees(new_rotation);
		camera->set_rotating(new_rotation % 360 != 0);
		_window_resized();
	}
}

void REDControllerBase::_bind_methods() { 
	ClassDB::bind_method(D_METHOD("_update_frame_parallax"), &REDControllerBase::_update_frame_parallax);
	ClassDB::bind_method(D_METHOD("_update_camera_zoom"), &REDControllerBase::_update_camera_zoom);
	ClassDB::bind_method(D_METHOD("_update_frame_local_pos"), &REDControllerBase::_update_frame_local_pos);

	ClassDB::bind_method(D_METHOD("rotate_camera"), &REDControllerBase::rotate_camera);
	ClassDB::bind_method(D_METHOD("set_issue_by_path", "issue_path"), &REDControllerBase::set_issue_by_path);
	ClassDB::bind_method(D_METHOD("_frame_start"), &REDControllerBase::_frame_start);
	ClassDB::bind_method(D_METHOD("_frame_end"), &REDControllerBase::_frame_end);
	
	ClassDB::bind_method(D_METHOD("_mouse_moved", "mouse_position"), &REDControllerBase::_mouse_moved);
	ClassDB::bind_method(D_METHOD("zoom_out", "zoom_val"), &REDControllerBase::zoom_out);
	ClassDB::bind_method(D_METHOD("zoom_in", "zoom_val"), &REDControllerBase::zoom_in);
	ClassDB::bind_method(D_METHOD("to_next"), &REDControllerBase::to_next);
	ClassDB::bind_method(D_METHOD("to_prev"), &REDControllerBase::to_prev);

	ClassDB::bind_method(D_METHOD("set_reset_camera_on_frame_change", "reset_camera"), &REDControllerBase::set_reset_camera_on_frame_change);
	ClassDB::bind_method(D_METHOD("is_reset_camera_on_frame_change"), &REDControllerBase::is_reset_camera_on_frame_change);

	ClassDB::bind_method(D_METHOD("set_camera_speed", "camera_speed"), &REDControllerBase::set_camera_speed);
	ClassDB::bind_method(D_METHOD("get_camera_speed"), &REDControllerBase::get_camera_speed);
	ClassDB::bind_method(D_METHOD("set_camera_mode", "camera_mode"), &REDControllerBase::set_camera_mode);
	ClassDB::bind_method(D_METHOD("get_camera_mode"), &REDControllerBase::get_camera_mode);
	ClassDB::bind_method(D_METHOD("set_camera_path", "camera_path"), &REDControllerBase::set_camera_path);
	ClassDB::bind_method(D_METHOD("get_camera_path"), &REDControllerBase::get_camera_path);

	ClassDB::bind_method(D_METHOD("set_camera_zoom_min", "camera_zoom_min"), &REDControllerBase::set_camera_zoom_min);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_min"), &REDControllerBase::get_camera_zoom_min);
	ClassDB::bind_method(D_METHOD("set_camera_zoom_max", "camera_zoom_max"), &REDControllerBase::set_camera_zoom_max);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_max"), &REDControllerBase::get_camera_zoom_max);

	ClassDB::bind_method(D_METHOD("can_control"), &REDControllerBase::can_control);

	ClassDB::bind_method(D_METHOD("set_camera_smooth", "camera_smooth"), &REDControllerBase::set_camera_smooth);
	ClassDB::bind_method(D_METHOD("get_camera_smooth"), &REDControllerBase::get_camera_smooth);

	ClassDB::bind_method(D_METHOD("update_camera_to_frame"), &REDControllerBase::update_camera_to_frame);

	ClassDB::bind_method(D_METHOD("set_mouse_offset_k", "mouse_offset_k"), &REDControllerBase::set_mouse_offset_k);
	ClassDB::bind_method(D_METHOD("get_mouse_offset_k"), &REDControllerBase::get_mouse_offset_k);

	ClassDB::bind_method(D_METHOD("_camera_moved"), &REDControllerBase::_camera_moved);
	ClassDB::bind_method(D_METHOD("_frame_change"), &REDControllerBase::_frame_change);
	ClassDB::bind_method(D_METHOD("_frame_changed"), &REDControllerBase::_frame_changed);
	
	ClassDB::bind_method(D_METHOD("get_camera_global_pos"), &REDControllerBase::get_camera_global_pos);
	ClassDB::bind_method(D_METHOD("get_camera_zoom"), &REDControllerBase::get_camera_zoom);

	ClassDB::bind_method(D_METHOD("get_parallax_offset"), &REDControllerBase::get_parallax_offset);
	ClassDB::bind_method(D_METHOD("get_parallax_zoom"), &REDControllerBase::get_parallax_zoom);

	ClassDB::bind_method(D_METHOD("set_frame_expand", "frame_expand"), &REDControllerBase::set_frame_expand);
	ClassDB::bind_method(D_METHOD("get_frame_expand"), &REDControllerBase::get_frame_expand);
	ClassDB::bind_method(D_METHOD("_input"), &REDControllerBase::_input);
	ClassDB::bind_method(D_METHOD("set_orientation", "orientation"), &REDControllerBase::set_orientation);
	ClassDB::bind_method(D_METHOD("get_orientation"), &REDControllerBase::get_orientation);
	ClassDB::bind_method(D_METHOD("_update_orientation"), &REDControllerBase::_update_orientation);

	ClassDB::add_virtual_method(get_class_static(), MethodInfo("_window_resized"));

	ADD_GROUP("Frame", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "frame_expand"), "set_frame_expand", "get_frame_expand");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reset_camera_on_frame_change"), "set_reset_camera_on_frame_change", "is_reset_camera_on_frame_change");

	ADD_GROUP("Camera", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Camera2D"), "set_camera_path", "get_camera_path");

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom_min"), "set_camera_zoom_min", "get_camera_zoom_min");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom_max"), "set_camera_zoom_max", "get_camera_zoom_max");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "mouse_offset_k"), "set_mouse_offset_k", "get_mouse_offset_k");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_smooth"), "set_camera_smooth", "get_camera_smooth");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_mode"), "set_camera_mode", "get_camera_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "camera_speed"), "set_camera_speed", "get_camera_speed");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "orientation", PROPERTY_HINT_ENUM, "Maximum, Minimum, Horizontal, Vertical, Hybrid"), "set_orientation", "get_orientation");

	BIND_ENUM_CONSTANT(ORIENTATION_MAXIMUM);
	BIND_ENUM_CONSTANT(ORIENTATION_MINIMUM);
	BIND_ENUM_CONSTANT(ORIENTATION_HORIZONTAL);
	BIND_ENUM_CONSTANT(ORIENTATION_VERTICAL);
	BIND_ENUM_CONSTANT(ORIENTATION_HYBRID);
}

REDControllerBase::REDControllerBase() {
	orientation_k.current = 1.0;
	frame_pos_local.current = Vector2(0, 0);
	zoom_k.current = 0.0;
	mouse_offset.current = Vector2(0, 0);
	camera_zoom.current = Vector2(1, 1);
	frame_parallax.current = Vector2(0, 0);

	orientation_k.bounces = 0.0;
	frame_pos_local.bounces = 0.0;
	zoom_k.bounces = 0.0;
	mouse_offset.bounces = 0.0;
	camera_zoom.bounces = 0.0;
	frame_parallax.bounces = 0.0;

	orientation_k.speed = 2.0;
	frame_pos_local.speed = 4.0;
	zoom_k.speed = 2.0;
	mouse_offset.speed = 4.0;
	camera_zoom.speed = 2.0;
	frame_parallax.speed = 4.0;

	camera_speed = 4000;
	orientation = ORIENTATION_MAXIMUM;

	screen_multiplayer_start = 1.0;

	reset_camera_on_frame_change = true;
	frame_expand = Vector2(250.0, 250.0);
	dirrection = DIRRECTION_NONE;
	tween = nullptr;
	camera = nullptr;
	issue = nullptr;
    page = nullptr;
	frame = nullptr;
	frame_timer_connected = false;
	frame_changing = false;

	camera_mode = true;
	b_can_control = true;

	camera_zoom_min = Vector2(0.5f, 0.5f);
	camera_zoom_max = Vector2(4, 4);
	mouse_offset_k = Vector2(1, 1);
	screen_multiplayer = 1.0;
}
