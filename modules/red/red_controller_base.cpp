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

void REDControllerBase::set_tween_path(const NodePath &p_tween_path){
	if (tween_path == p_tween_path)
		return;
	tween_path = p_tween_path;
	if (is_inside_tree()){
		tween = (Tween*)(get_node(NodePath(p_tween_path)));
	}
}
NodePath REDControllerBase::get_tween_path() const{
	return tween_path;
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

void REDControllerBase::update_camera_pos() {
    if (!camera_mode || Engine::get_singleton()->is_editor_hint()){
		return;
	}
    if (get_camera_path().is_empty()){
		return;
	}

    if (frame == nullptr){
		return;
	}
	Vector2 new_pos;
	{
		if (camera_zoom < 0.5){
			new_pos = frame->get_camera_pos();
		}
		else{
			new_pos = frame->get_camera_pos() + (camera_zoom - 0.5) * 2 * (frame->get_camera_pos_zoom_out() - frame->get_camera_pos());
		}
		new_pos = new_pos + frame->get_global_position();
	}
	if(camera_smooth && !tween_path.is_empty()){
		if (tween->is_active()){
			tween->stop(camera, "position");	
		}		
		Tween::TransitionType tween_transition_type = Tween::TRANS_CUBIC;
		Tween::EaseType tween_ease_type = Tween::Tween::EASE_IN_OUT;
		float tween_time = 0.5f;

		tween->interpolate_property(camera, NodePath("position"), camera->get_position(), new_pos, tween_time, tween_transition_type, tween_ease_type);
	}else{
		camera->set_position(new_pos);
	}
	b_camera_can_update = false;
}
void REDControllerBase::update_camera_zoom() {
    if (!camera_mode || !b_camera_can_update || Engine::get_singleton()->is_editor_hint()){
		return;
	}
    if (get_camera_path().is_empty()){
		return;
	}
    if (frame == nullptr){
		return;
	}
	Vector2 new_pos;
	Vector2 new_camera_zoom;
	{
		Vector2 frame_camera_zoom = frame->get_camera_zoom();
		Vector2 camera_zoom_min_clamped = Vector2(MIN(camera_zoom_min.x, frame_camera_zoom.x), MIN(camera_zoom_min.y, frame_camera_zoom.y));
		Vector2 camera_zoom_max_clamped = Vector2(MAX(camera_zoom_max.x, frame_camera_zoom.x), MAX(camera_zoom_max.y, frame_camera_zoom.y));
		if (camera_zoom < 0.5){
			new_pos = frame->get_camera_pos();
			new_camera_zoom = camera_zoom_min_clamped + camera_zoom * 2.0 * (frame_camera_zoom - camera_zoom_min_clamped);
		}
		else{
			new_pos = frame->get_camera_pos() + (camera_zoom - 0.5) * 2 * (frame->get_camera_pos_zoom_out() - frame->get_camera_pos());
			new_camera_zoom = frame_camera_zoom + (camera_zoom - 0.5) * 2 * (camera_zoom_max_clamped - frame_camera_zoom);
		}
		new_pos = new_pos + frame->get_global_position();
		new_camera_zoom *= page->get_size().width / get_viewport()->get_size().width;
	}

	if(!tween_path.is_empty()){
		if (tween->is_active()){
			tween->stop(camera, "position");	
			tween->stop(camera, "zoom");
			if (issue != nullptr){
				tween->stop(issue, "update_camera_zoom");	
			}
			else if (page != nullptr){
				tween->stop(page, "update_camera_zoom");	
			}
		}
		Tween::TransitionType tween_transition_type = Tween::TRANS_CUBIC;
		Tween::EaseType tween_ease_type = (camera->get_zoom()) < new_camera_zoom ? Tween::EASE_OUT : Tween::EASE_OUT;
		float tween_time = 0.5f;

		tween->interpolate_property(camera, NodePath("position"), camera->get_position(), new_pos, tween_time, tween_transition_type, tween_ease_type);
		tween->interpolate_property(camera, NodePath("zoom"), camera->get_zoom(), new_camera_zoom, tween_time, tween_transition_type, tween_ease_type);
		if (issue != nullptr){
			tween->interpolate_method(issue, "update_camera_zoom", camera->get_zoom(), new_camera_zoom, tween_time, tween_transition_type, tween_ease_type);
		}
		else if (page != nullptr){
			tween->interpolate_method(page, "update_camera_zoom", camera->get_zoom(), new_camera_zoom, tween_time, tween_transition_type, tween_ease_type);
		}
		tween->start();
	}else{
		camera->set_position(new_pos);
		camera->set_zoom(new_camera_zoom);
		if (issue != nullptr){
			issue->update_camera_zoom(new_camera_zoom);
		}
		else if (page != nullptr){
			page->update_camera_zoom(new_camera_zoom);
		}
	}
	b_camera_can_update = false;
}

void REDControllerBase::update_camera() {
    if (!camera_mode || !b_camera_can_update || Engine::get_singleton()->is_editor_hint()){
		return;
	}
    if (get_camera_path().is_empty()){
		return;
	}

    if (frame == nullptr){
		return;
	}
	Vector2 new_pos;
	Vector2 new_camera_zoom;
	{
		Vector2 frame_camera_zoom = frame->get_camera_zoom();
		Vector2 camera_zoom_min_clamped = Vector2(MIN(camera_zoom_min.x, frame_camera_zoom.x), MIN(camera_zoom_min.y, frame_camera_zoom.y));
		Vector2 camera_zoom_max_clamped = Vector2(MAX(camera_zoom_max.x, frame_camera_zoom.x), MAX(camera_zoom_max.y, frame_camera_zoom.y));
		if (camera_zoom < 0.5){
			new_pos = frame->get_camera_pos();
			new_camera_zoom = camera_zoom_min_clamped + camera_zoom * 2.0 * (frame_camera_zoom - camera_zoom_min_clamped);
		}
		else{
			new_pos = frame->get_camera_pos() + (camera_zoom - 0.5) * 2 * (frame->get_camera_pos_zoom_out() - frame->get_camera_pos());
			new_camera_zoom = frame_camera_zoom + (camera_zoom - 0.5) * 2 * (camera_zoom_max_clamped - frame_camera_zoom);
		}
		new_pos = new_pos + frame->get_global_position();
		new_camera_zoom *= page->get_size().width / get_viewport()->get_size().width;
	}

	if(camera_smooth && !tween_path.is_empty()){
		if (tween->is_active()){
			tween->stop(camera, "position");	
			tween->stop(camera, "zoom");
			if (issue != nullptr){
				tween->stop(issue, "update_camera_zoom");	
			}
			else if (page != nullptr){
				tween->stop(page, "update_camera_zoom");	
			}
		}
		Tween::TransitionType tween_transition_type = Tween::TRANS_CUBIC;
		Tween::EaseType tween_ease_type = (camera->get_zoom()) < new_camera_zoom ? Tween::EASE_OUT : Tween::EASE_IN;
		float tween_time = 1.0f;

		tween->interpolate_property(camera, NodePath("position"), camera->get_position(), new_pos, tween_time, tween_transition_type, tween_ease_type);
		tween->interpolate_property(camera, NodePath("zoom"), camera->get_zoom(), new_camera_zoom, tween_time, tween_transition_type, tween_ease_type);
		if (issue != nullptr){
			tween->interpolate_method(issue, "update_camera_zoom", camera->get_zoom(), new_camera_zoom, tween_time, tween_transition_type, tween_ease_type);
		}
		else if (page != nullptr){
			tween->interpolate_method(page, "update_camera_zoom", camera->get_zoom(), new_camera_zoom, tween_time, tween_transition_type, tween_ease_type);
		}
		tween->start();
	}
	else{
		camera->set_position(new_pos);
		camera->set_zoom(new_camera_zoom);
		if (issue != nullptr){
			issue->update_camera_zoom(new_camera_zoom);
		}
		else if (page != nullptr){
			page->update_camera_zoom(new_camera_zoom);
		}
	}
	b_camera_can_update = false;
}
/*
void REDControllerBase::unload_page(REDPage *page) const {
	if (page != nullptr) {
		page->queue_delete();
        page = nullptr;
	}
}

REDPage *REDControllerBase::load_page(const int &i, bool is_prev, int state) {
	if (i<0 || i >= issue->get_pages_count())
		return nullptr;
	Ref<PackedScene> scene = issue->get_page_scene(i);
	if (scene.is_null())
		return nullptr;

	Node2D *node2d = (Node2D*)scene->instance();
	add_child(node2d);
	node2d->set_owner(this);

	

	Node *child = node2d->get_child(0);
	if (child->get_class() != "REDPage")
		return nullptr;
	REDPage *page = (REDPage*)(node2d->get_child(0));
	
	Array y_offset = issue->get_y_offset();

	if (y_offset.size() <= i + 1)
		y_offset.resize(i+2);
	if (i==0)
		y_offset[i] = 0.0;
	if (!is_prev)
		y_offset[i+1] = y_offset[i] + Vector2(0.0, page->get_height());
	issue->set_y_offset(y_offset[i]);


	int count = page->get_frames_count();
	if (is_prev){
		page->set_id(page->get_frames_count()-1);
	}
	if (state==0){
		prev_initialized = true;
	} else if (state==1){
		current_initialized = true;
	} else if (state==2){
		next_initialized = true;
	}
	for (int i = 0; i < count; i++) {
		REDFrame *child_frame = page->get_frame(i);
		if (child_frame!=nullptr){
			if (!child_frame->get_anim_tree().is_empty())
			{
				((AnimationTree*)(child_frame->get_node(child_frame->get_anim_tree())))->set_active(false);
			}
		}
	}
	return page;
}

void REDControllerBase::unload_pages() {
	if (prev_initialized){
		prev_initialized = false;
		unload_page(prev_page);
	}

	if (next_initialized){
		next_initialized = false;
		unload_page(next_page);
	}

	if (current_initialized){
		current_initialized = false;
		unload_page(page);
	}
}

void REDControllerBase::load_pages() {
	//int i = issue->get_id();
    //set_page(load_page(i, false, 1));
    //prev_page = load_page(i - 1, false, 0);
	//next_page = load_page(i + 1, false, 2);
}
*/
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
	/*
	int new_id = issue->get_id() + 1;
	if (new_id < issue->get_pages_count()) {
		issue->set_id(new_id);
        unload_page(prev_page);
        prev_page = page;
        set_page(next_page);

        next_page = load_page(new_id + 1, false, 2);
		return true;
	}*/
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
    issue = p_issue;
	if (issue == nullptr)
		return;
	set_page(issue->get_id());
}
REDIssue *REDControllerBase::get_issue() const {
    return issue;
}

void REDControllerBase::set_page(REDPage *p_page){
	issue = nullptr;
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
	bool first_frame = false;
	if (frame!=nullptr){
		frame->disconnect("update_camera_zoom", this, "update_camera_zoom");
		frame->disconnect("update_camera_pos", this, "update_camera_pos");
	}else{
		first_frame = true;
	}

	page->set_frame(p_id, true, ended);
	frame = page->get_frame(p_id);
	if (frame == nullptr)
		return;

	frame->_started();
	frame->connect("update_camera_zoom", this, "update_camera_zoom");
	frame->connect("update_camera_pos", this, "update_camera_pos");

	if (first_frame){
		if (camera_smooth){
			camera_smooth = false;
			update_camera();
			camera_smooth = true;
		} else{
			update_camera();
		}
		camera->reset_smoothing();
	}else{
		update_camera();
	}
	//update_camera_pos();
	//Camera2D *camera = (Camera2D*)(get_node(camera_path));
	//set_camera_zoom(camera->get_zoom()*frame->get_camera_zoom());
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
	set_camera_zoom(camera_zoom - p_val);
}

void REDControllerBase::zoom_out(const float &p_val) {
	set_camera_zoom(camera_zoom + p_val);
}

void REDControllerBase::set_camera_pos(const Vector2 &p_pos) {
	if (camera_pos != p_pos){
		camera_pos = p_pos;
		update_camera_pos();
	}
}

Vector2 REDControllerBase::get_camera_pos() const{
	return camera_pos;
}

void REDControllerBase::set_camera_zoom(const float p_zoom) {
	//Vector2 global_camera_zoom = p_zoom * frame->get_camera_zoom();
	//Vector2 global_camera_zoom_clamped = clamp_camera_zoom(global_camera_zoom);
	float new_zoom = CLAMP(p_zoom, 0.0f, 1.0f);// + global_camera_zoom_clamped - global_camera_zoom;

	if (camera_zoom == new_zoom)
		return;
	camera_zoom = new_zoom;
	update_camera_zoom();
}

float REDControllerBase::get_camera_zoom() const{
	return camera_zoom;
}
void REDControllerBase::set_camera_zoom_min(const Vector2 &p_zoom) {
	if (camera_zoom_min == p_zoom)
		return;
	camera_zoom_min = p_zoom;
	update_camera_zoom();
}

Vector2 REDControllerBase::get_camera_zoom_min() const{
	return camera_zoom_min;
}

void REDControllerBase::set_camera_zoom_max(const Vector2 &p_zoom) {
	if (camera_zoom_max == p_zoom)
		return;
	camera_zoom_max = p_zoom;
	update_camera_zoom();
}

Vector2 REDControllerBase::get_camera_zoom_max() const{
	return camera_zoom_max;
}


Vector2 REDControllerBase::get_global_camera_zoom() const{
	return (frame == nullptr) ? camera_zoom_min + camera_zoom * (camera_zoom_max - camera_zoom_min) : camera_zoom_min + camera_zoom * (camera_zoom_max - camera_zoom_min) * frame->get_camera_zoom();
}


Vector2 REDControllerBase::clamp_camera_zoom(const Vector2 &p_zoom) const{
	//if (frame == nullptr){
	//	if (frame->get_camera_zoom() != Vector2(0, 0)){
	//		return CLAMP(p_zoom - frame->get_camera_zoom(), camera_zoom_min, camera_zoom_max) + frame->get_camera_zoom();
	//	}
	//}

	return Vector2(CLAMP(p_zoom.x, camera_zoom_min.x, camera_zoom_max.x), 
				   CLAMP(p_zoom.y, camera_zoom_min.y, camera_zoom_max.y));
}

bool REDControllerBase::can_control() const {
	return b_can_control;
}

void REDControllerBase::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:{
			if (Engine::get_singleton()->is_editor_hint()) {
				set_process_internal(false);
			} else {
				set_process_internal(true);
			}
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			b_camera_can_update = true;
		} break;
		case NOTIFICATION_READY: {
			camera = (Camera2D*)(get_node(camera_path));
			tween = (Tween*)(get_node(tween_path));
			get_tree()->get_root()->connect("size_changed", this, "update_camera_zoom");
		} break;
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
	ClassDB::bind_method(D_METHOD("set_tween_path", "tween_path"), &REDControllerBase::set_tween_path);
	ClassDB::bind_method(D_METHOD("get_tween_path"), &REDControllerBase::get_tween_path);

	ClassDB::bind_method(D_METHOD("set_camera_pos", "camera_pos"), &REDControllerBase::set_camera_pos);
	ClassDB::bind_method(D_METHOD("get_camera_pos"), &REDControllerBase::get_camera_pos);
	ClassDB::bind_method(D_METHOD("set_camera_zoom", "camera_zoom"), &REDControllerBase::set_camera_zoom);
	ClassDB::bind_method(D_METHOD("get_camera_zoom"), &REDControllerBase::get_camera_zoom);
	ClassDB::bind_method(D_METHOD("set_camera_zoom_min", "camera_zoom_min"), &REDControllerBase::set_camera_zoom_min);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_min"), &REDControllerBase::get_camera_zoom_min);
	ClassDB::bind_method(D_METHOD("set_camera_zoom_max", "camera_zoom_max"), &REDControllerBase::set_camera_zoom_max);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_max"), &REDControllerBase::get_camera_zoom_max);

	ClassDB::bind_method(D_METHOD("can_control"), &REDControllerBase::can_control);

	ClassDB::bind_method(D_METHOD("set_camera_smooth", "camera_smooth"), &REDControllerBase::set_camera_smooth);
	ClassDB::bind_method(D_METHOD("get_camera_smooth"), &REDControllerBase::get_camera_smooth);

	ClassDB::bind_method(D_METHOD("update_camera_pos"), &REDControllerBase::update_camera_pos);
	ClassDB::bind_method(D_METHOD("update_camera_zoom"), &REDControllerBase::update_camera_zoom);

	ADD_GROUP("Camera", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Camera2D"), "set_camera_path", "get_camera_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "tween_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Tween"), "set_tween_path", "get_tween_path");

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "camera_zoom"), "set_camera_zoom", "get_camera_zoom");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom_min"), "set_camera_zoom_min", "get_camera_zoom_min");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom_max"), "set_camera_zoom_max", "get_camera_zoom_max");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_smooth"), "set_camera_smooth", "get_camera_smooth");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_mode"), "set_camera_mode", "get_camera_mode");
	//ClassDB::bind_method(D_METHOD("set_zoom", "zoom"), &RED::set_zoom);
	//ClassDB::bind_method(D_METHOD("get_zoom"), &RED::get_zoom);
	
	//ADD_GROUP("Current", "");
    //ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "issue", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDPage"), "set_current_issue_path", "get_current_issue_path");
}

REDControllerBase::REDControllerBase() {
	issue = nullptr;
    page = nullptr;
	frame = nullptr;

    prev_page = nullptr;
    next_page = nullptr;

	camera_mode = true;
	b_can_control = true;
	b_camera_can_update = true;
	camera_pos = Vector2(0, 0);
	camera_zoom = 0.5;
	camera_zoom_min = Vector2(0.5f, 0.5f);
	camera_zoom_max = Vector2(4, 4);
}
