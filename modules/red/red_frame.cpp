#include "red_frame.h"
#include "scene/animation/animation_player.h"
#include "scene/animation/animation_tree.h"
#include "scene/animation/animation_node_state_machine.h"
#include "core/engine.h"

#include "red_page.h"
#include "red_engine.h"
#include "red.h"
#include "red_issue.h"
#include "red_shape.h"
#include "red_polygon.h"

#include "core/math/geometry.h"
#include "red_line.h"

#include "red_line_builder.h"
#include "core/math/math_funcs.h"
#include "core/core_string_names.h"
#include <string>
#include "scene/animation/animation_tree.h"
#include "scene/scene_string_names.h"
#include "red_parallax_folder.h"
#include "red_target.h"

// Cam pos

void REDFrame::unload_target(const NodePath &p_path, StringName function){
	if (has_node(p_path)){
		REDTarget *target = Object::cast_to<REDTarget>(get_node(p_path));
		if (target){
			target->disconnect("_target_moved", this, function);
		}
	}
}

Transform2D REDFrame::load_target(const NodePath &p_path, StringName function){
	if (has_node(p_path)){
		REDTarget *target = Object::cast_to<REDTarget>(get_node(p_path));
		if (target){
			target->connect("_target_moved", this, function);
			return target->get_transform();
		}
	}
	return this->get_transform();
}

void REDFrame::load_targets(){
	set_camera_pos(load_target(camera_pos_path, "set_camera_pos"));
	set_camera_pos_in(load_target(camera_pos_in_path, "set_camera_pos_in"));
	set_camera_pos_out(load_target(camera_pos_out_path, "set_camera_pos_out"));
	set_parallax_pos(load_target(parallax_pos_path, "set_parallax_pos"));
	set_parallax_pos_in(load_target(parallax_pos_in_path, "set_parallax_pos_in"));
	set_parallax_pos_out(load_target(parallax_pos_out_path, "set_parallax_pos_out"));
}
void REDFrame::unload_targets(){
	unload_target(camera_pos_path, "set_camera_pos");
	unload_target(camera_pos_in_path, "set_camera_pos_in");
	unload_target(camera_pos_out_path, "set_camera_pos_out");
	unload_target(parallax_pos_path, "set_parallax_pos");
	unload_target(parallax_pos_in_path, "set_parallax_pos_in");
	unload_target(parallax_pos_out_path, "set_parallax_pos_out");
}



void REDFrame::set_camera_pos_path(const NodePath &p_path){
	if (!is_inside_tree()){
		camera_pos_path = p_path;
		return;
	}
	unload_target(camera_pos_path, "set_camera_pos");
	camera_pos_path = p_path;
	set_camera_pos(load_target(p_path, "set_camera_pos"));
}

void REDFrame::set_camera_pos_in_path(const NodePath &p_path){
	if (!is_inside_tree()){
		camera_pos_in_path = p_path;
		return;
	}
	unload_target(camera_pos_in_path, "set_camera_pos_in");
	camera_pos_in_path = p_path;
	set_camera_pos_in(load_target(p_path, "set_camera_pos_in"));
}

void REDFrame::set_camera_pos_out_path(const NodePath &p_path){
	if (!is_inside_tree()){
		camera_pos_out_path = p_path;
		return;
	}
	unload_target(camera_pos_path, "set_camera_pos_out");
	camera_pos_out_path = p_path;
	set_camera_pos_in(load_target(p_path, "set_camera_pos_out"));
}

void REDFrame::set_parallax_pos_path(const NodePath &p_path){
	if (!is_inside_tree()){
		parallax_pos_path = p_path;
		return;
	}
	unload_target(parallax_pos_path, "set_parallax_pos");
	parallax_pos_path = p_path;
	set_parallax_pos(load_target(p_path, "set_parallax_pos"));
}

void REDFrame::set_parallax_pos_in_path(const NodePath &p_path){
	if (!is_inside_tree()){
		parallax_pos_in_path = p_path;
		return;
	}
	unload_target(parallax_pos_in_path, "set_parallax_pos_in");
	parallax_pos_in_path = p_path;
	set_parallax_pos_in(load_target(p_path, "set_parallax_pos_in"));
}

void REDFrame::set_parallax_pos_out_path(const NodePath &p_path){
	if (!is_inside_tree()){
		parallax_pos_out_path = p_path;
		return;
	}
	unload_target(parallax_pos_out_path, "set_parallax_pos_out");
	parallax_pos_out_path = p_path;
	set_parallax_pos_out(load_target(p_path, "set_parallax_pos_out"));
}

/////////////////////
NodePath REDFrame::get_camera_pos_path() const{
	return camera_pos_path;
}
NodePath REDFrame::get_camera_pos_in_path() const{
	return camera_pos_in_path;
}
NodePath REDFrame::get_camera_pos_out_path() const{
	return camera_pos_out_path;
}
NodePath REDFrame::get_parallax_pos_path() const{
	return parallax_pos_path;
}
NodePath REDFrame::get_parallax_pos_in_path() const{
	return parallax_pos_in_path;
}
NodePath REDFrame::get_parallax_pos_out_path() const{
	return parallax_pos_out_path;
}
//////////////////////

void REDFrame::_target_moved_parallax(){
	emit_signal("_target_moved_parallax");
};

void REDFrame::_target_moved_pos(){
	emit_signal("_target_moved_pos");
};

void REDFrame::set_camera_pos(const Transform2D &p_pos){
	camera_pos = p_pos;
	//_target_moved_pos();
};
void REDFrame::set_camera_pos_in(const Transform2D &p_pos){
	camera_pos_in = p_pos;
	_target_moved_pos();
};
void REDFrame::set_camera_pos_out(const Transform2D &p_pos){
	camera_pos_out = p_pos;
	_target_moved_pos();
};
void REDFrame::set_parallax_pos(const Transform2D &p_pos){
	parallax_pos = p_pos;
	_target_moved_parallax();
};

void REDFrame::set_parallax_pos_in(const Transform2D &p_pos){
	parallax_pos_in = p_pos;
	_target_moved_parallax();
};

void REDFrame::set_parallax_pos_out(const Transform2D &p_pos){
	parallax_pos_out = p_pos;
	_target_moved_parallax();
};

///////////////////////////////////////////
Transform2D REDFrame::get_camera_pos() const{
	return camera_pos;
};

Transform2D REDFrame::get_camera_pos_in() const{
	return camera_pos_in;
};

Transform2D REDFrame::get_camera_pos_out() const{
	return camera_pos_out;
};

Transform2D REDFrame::get_parallax_pos() const{
	return parallax_pos;
};

Transform2D REDFrame::get_parallax_pos_in() const{
	return parallax_pos_in;
};

Transform2D REDFrame::get_parallax_pos_out() const{
	return parallax_pos_out;
};
///////////////////////////////////////////////////


void REDFrame::update_origin_pos_gl(){
	origin_pos_gl = get_global_position();
};

void REDFrame::set_origin_pos_gl(const Point2 &p_origin_pos_gl){
	origin_pos_gl = p_origin_pos_gl;
};

Point2 REDFrame::get_origin_pos_gl() const{
	return origin_pos_gl;
};


void REDFrame::set_offset_motion_scale(const Vector2 &p_motion_scale){
	Vector2 new_offset = parallax * p_motion_scale;
	if (old_offset != new_offset){
		set_offset(get_offset() - old_offset + new_offset);
		old_offset = new_offset;
	}
	offset_motion_scale = p_motion_scale;
};

Vector2 REDFrame::get_offset_motion_scale() const{
	return offset_motion_scale;
};

void REDFrame::revert_z_index(){
	set_z_index(old_z_index);
};

void REDFrame::set_old_z_index(int p_z_index){
	old_z_index = get_z_index();
};

void REDFrame::set_start_delay(float p_delay){
	end_delay = p_delay;
};

float REDFrame::get_start_delay() const{
	return end_delay;
};


void REDFrame::set_end_delay(float p_delay){
	end_delay = p_delay;
};

float REDFrame::get_end_delay() const{
	return end_delay;
};

// Parallax, const Vector2 &p_parallax_pos, const Vector2 &p_scale
void REDFrame::set_parallax(const Vector2 &p_parallax){
	if (!is_inside_tree())
		return;
	if (Engine::get_singleton()->is_editor_hint())
		return;
	int count = get_child_count();

	Vector2 new_offset = p_parallax * offset_motion_scale;
	if (old_offset != new_offset){
		set_offset(get_offset() - old_offset + new_offset);
		old_offset = new_offset;
	}
	//set_scale(Vector2(1, 1) * p_scale * origin_scale);
	for (int i = 0; i < count; i++)
	{
		REDParallaxFolder *folder = Object::cast_to<REDParallaxFolder>(get_child(i));
		if (!folder)
			continue;
		folder->set_base_offset_and_scale(p_parallax, get_scroll_scale(), Point2(0, 0));
	}
	parallax = p_parallax;
	//emit_signal("parallax_changed");
};
/*
void REDFrame::set_position_motion_scale(const Vector2 &p_motion_scale){
	Vector2 new_postion_offset = (scale_parallax_pos - 1.0) * p_motion_scale;
	if (old_position_offset != new_postion_offset){
		set_position(get_position() - old_position_offset + new_postion_offset);
		old_position_offset = new_postion_offset;
	} 
	position_motion_scale = p_motion_scale;
};

Vector2 REDFrame::get_position_motion_scale() const{
	return position_motion_scale;
};*/

void REDFrame::set_scale_pos(const Vector2 &p_scale_pos){
	scale_pos = p_scale_pos;
};

void REDFrame::set_frame_scale(const Vector2 &p_scale_factor){
	
	//Transform2D tr;
	//tr.set_origin(camera_pos);
	//tr
	//set_position(Vector2(0.0,0.0));
	//set_scale(Vector2(1.0,1.0));

	//Transform2D tr = get_transform();
	//tr.set_origin(-camera_pos_zoom_out);
	//tr.set_scale(p_scale_factor);
	////set_transform(tr);
	//set_transform(tr*get_transform());
	if (anchor==FRAME_ANCHOR_TOP_LEFT){
		Vector2 new_scale_offset =(p_scale_factor*_edit_get_rect().size-_edit_get_rect().size)*0.5;
		if (old_scale_offset != new_scale_offset){
			set_position(get_position() + old_scale_offset - new_scale_offset);
			//origin_pos_gl = origin_pos_gl + old_scale_offset- new_scale_offset;
			old_scale_offset = new_scale_offset;
			set_scale(p_scale_factor);
		} 
	}
	else{
		set_scale(p_scale_factor);
	}
};

Vector2 REDFrame::get_scale_offset() const{
	return old_scale_offset;
};

Vector2 REDFrame::get_parallax() const{
	return parallax;
};

float REDFrame::get_scroll_scale() const{
	return 1.0;
};

void REDFrame::set_parallax_pos_current(const Vector2 &p_parallax_pos){
	parallax_pos_current = p_parallax_pos;
	emit_signal("parallax_pos_changed");
};

Vector2 REDFrame::get_parallax_pos_current() const{
	return parallax_pos_current;
};

// Cam zoom
void REDFrame::set_camera_zoom(const Vector2 &p_camera_zoom){
	if (camera_zoom != p_camera_zoom){
		camera_zoom = p_camera_zoom;
		emit_signal("update_camera_zoom");
	}
};

Vector2  REDFrame::get_camera_zoom() const{
	return camera_zoom;
};
void REDFrame::update_camera_zoom_and_child(const Vector2 &p_camera_zoom){
	update_camera_zoom(p_camera_zoom);
	int count = get_child_count();
	for (int i = 0; i < count; i++)
	{
		Node *child_shape = (Node*)get_child(i);
		if (child_shape->is_class("REDShape")){
			((REDShape*)child_shape)->update_camera_zoom(p_camera_zoom);
		}
	}
};


void REDFrame::animation_changed(StringName old_name, StringName new_name){
	if (reinit_tree){
		Ref<AnimationNodeStateMachinePlayback> playback = get_playback();
		if (playback.is_valid() && playback->is_playing()){
			reinit_tree = false;
			playback->start(end_state);
			playback->disconnect("animation_changed", this, "animation_changed");
			if (get_script_instance() != NULL) {
				if (get_script_instance()->has_method("_ended"))
					get_script_instance()->call("_ended");
			}
		}
	}
}	

void REDFrame::set_reinit_tree(bool p_reinit_tree){
	reinit_tree = p_reinit_tree;
}
void REDFrame::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			load_targets();

			Ref<AnimationNodeStateMachinePlayback> playback = get_playback();
			if (playback != nullptr)
				playback->connect("animation_started", this, "animation_changed");
			origin_pos_gl = get_camera_pos().get_origin();
			origin_scale = get_scale();
			origin_pos = get_position();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			unload_targets();
		} break;
	}
}


void REDFrame::set_anchor(REDFrame::Anchor p_anchor){
	anchor = p_anchor;
}

REDFrame::Anchor REDFrame::get_anchor() const{
	return anchor;
}


void REDFrame::set_start_transition (const String &p_start_transition){
	start_transition = p_start_transition;
}

String REDFrame::get_start_transition (){
	return start_transition;
}


void REDFrame::set_end_transition (const String &p_end_transition){
	end_transition = p_end_transition;
}

String REDFrame::get_end_transition (){
	return end_transition;
}

void REDFrame::set_end_state (const String &p_end_state){
	end_state = p_end_state;
}

String REDFrame::get_end_state (){
	return end_state;
}
/*
void REDFrame::set_state (int p_id){
	id = p_id;
	if (p_id >= states.size()){
		_ending();
	}
	else if(p_id < 0){
		_start_loop();
	}
	else{
		set_id(p_id);
		_started();
	}
}*/

String REDFrame::get_state (int p_id){
	if (p_id > -1 && p_id < states.size())
		return "Null";
	return states[p_id];
}

String REDFrame::get_state(){
	if (id > -1 && id < states.size())
		return "Null";
	return states[id];
}

int REDFrame::get_states_count() const{
    return states.size();
}



/*
void REDFrame::_starting(){
	state = FRAME_STATE_STARTING;

	if (get_script_instance() != NULL) {
		if (get_script_instance()->has_method("_starting"))
			get_script_instance()->call("_starting");
	}
}


void REDFrame::_started(){
	state = FRAME_STATE_STARTED;

	if (get_script_instance() != NULL) {
		if (get_script_instance()->has_method("_started"))
			get_script_instance()->call("_started");
	}
}

void REDFrame::_ending(){
	state = FRAME_STATE_ENDING;
	//travel(end_state);
	//travel(end_transition);

	if (get_script_instance() != NULL) {
		if (get_script_instance()->has_method("_ending"))
			get_script_instance()->call("_ending");
	}
}

void REDFrame::_ended(){
	state = FRAME_STATE_ENDED
	//travel(states[id]);
	//REDManager *controller = red::get_controller(this);
	//if (controller!=nullptr){
	//	controller->to_next();
	//}
    if (get_script_instance() != NULL) {
		if (get_script_instance()->has_method("_ended"))
        	get_script_instance()->call("_ended");
	}
}


bool REDFrame::is_start_loop() const{
	return b_start_loop;
}

bool REDFrame::is_starting() const{
	return b_starting;
}

bool REDFrame::is_started() const{
	return b_started;
}

bool REDFrame::is_ending() const{
	return b_ending;
}

bool REDFrame::is_ended() const{
	return b_ended;
}

bool REDFrame::is_end_loop() const{
	return b_end_loop;
}
*/




void REDFrame::travel_start(){
	target_loop = FRAME_LOOP_START;
	id = 0;
	_travel(get_state());
}

void REDFrame::travel_state(){
	target_loop = FRAME_LOOP_MAIN;
	_travel(get_state());
}

void REDFrame::travel_end(){
	target_loop = FRAME_LOOP_END;
	int states_count = states.size();
	id = states_count - 1;
	_travel(get_state());
}


void REDFrame::_travel(const StringName &p_state){
	print_line("TRAVEL");
    print_line(std::to_string(id).c_str());
	if (get_anim_tree().is_empty())
		return;
	AnimationTree *at = (AnimationTree*)(get_node(get_anim_tree()));	
	if (!at)
		return;
	Ref<AnimationNodeStateMachine> machine = at->get_tree_root();

	if (machine.is_null())
		return;
	if (!machine->has_node(p_state))
		return;
	Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
	if (playback.is_null())
		return;
	at->set_active(true);
	if(!playback->is_playing()){
		return;
	}
	playback->travel(p_state);
	if (get_script_instance() != NULL)
		if (get_script_instance()->has_method("_travel"))
			get_script_instance()->call("_travel");
}
/*

		if (is_ending()){
			if (machine->has_node(end_transition)) {
				playback->travel(end_transition);
			}
		} else if (is_starting()){
			if (machine->has_node(start_transition)){
				playback->travel(start_transition);
			}

		} else if (machine->has_node(states[id])) {
			playback->travel(states[id]);
		}
	} else{
		if (is_end_loop()){
			if (machine->has_node(end_state)) {
				playback->travel(end_state);
			}
		}
		else if (is_start_loop()){
			if (machine->has_node(machine->get_start_node())) {
				playback->travel(machine->get_start_node());
			}
		}
	*/

void REDFrame::set_start_immediate(bool p_start_immediate){
	start_immediate = p_start_immediate;
}
bool REDFrame::is_start_immediate() const{
	return start_immediate;
}

void REDFrame::set_focused(bool p_focused){
	focused = p_focused;
}

bool REDFrame::is_focused() const{
	return focused;
}

void REDFrame::set_active(bool p_active){
	b_active = p_active;
}

bool REDFrame::is_active() const{
	return b_active;
}

void REDFrame::set_id(int p_id){
	id = p_id;
/*
    if (!anim_tree.is_empty()){
        int new_id;
        if (num < 0) {
            is_starting = true;
            new_id = 0;
        } else if (num >= get_states_count()) {
            b_ending = true;
            new_id = get_states_count() - 1;
            if (new_id<0)
                new_id=0;
        } else {
            new_id = num;
        }
        if (id != new_id){
            id = new_id;
        }
    }*/
}

int REDFrame::get_id() const{
    return id;
}

void REDFrame::set_states(const Array &new_states) {
    states.resize(new_states.size());
    for (int i = 0; i < states.size(); i++) {
        states.write[i] = new_states[i];
    }
    set_id(id);
}

Array REDFrame::get_states() const {
    Array states_temp;
    states_temp.resize(states.size());
    for (int i = 0; i < states_temp.size(); i++) {
        states_temp[i] = states[i];
    }
    return states_temp;
}

Ref<AnimationNodeStateMachinePlayback> REDFrame::get_playback() const {
	Ref<AnimationNodeStateMachinePlayback> playback;
	if (get_anim_tree().is_empty()){
		return playback;
	}
	AnimationTree *at = Object::cast_to<AnimationTree>(get_node(get_anim_tree()));
	if (at==nullptr){
		return playback;
	}
	Ref<AnimationNodeStateMachine> machine = at->get_tree_root();
	if (machine.is_null()){
		return playback;
	}
	playback = at->get("parameters/playback");
	return playback;
}


void REDFrame::set_anim_tree(const NodePath &new_anim_tree) {
    anim_tree = new_anim_tree;
}

NodePath REDFrame::get_anim_tree() const {
    return anim_tree;
}



void REDFrame::_bind_methods() {
	ClassDB::bind_method(D_METHOD("animation_changed", "old_name", "new_name"), &REDFrame::animation_changed);
    //Frame managment
    //ClassDB::bind_method(D_METHOD("run"), &REDFrame::run);

    //ClassDB::bind_method(D_METHOD("_started"), &REDFrame::_started);
    //ClassDB::bind_method(D_METHOD("_ended"), &REDFrame::_ended);


    ClassDB::bind_method(D_METHOD("get_states_count"), &REDFrame::get_states_count);

    ClassDB::bind_method(D_METHOD("set_anim_tree", "anim_tree"), &REDFrame::set_anim_tree);
    ClassDB::bind_method(D_METHOD("get_anim_tree"), &REDFrame::get_anim_tree);
    ClassDB::bind_method(D_METHOD("set_id", "id"), &REDFrame::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &REDFrame::get_id);
    ClassDB::bind_method(D_METHOD("set_states", "states"), &REDFrame::set_states);
    ClassDB::bind_method(D_METHOD("get_states"), &REDFrame::get_states);

    ClassDB::bind_method(D_METHOD("set_start_transition", "start_transition"), &REDFrame::set_start_transition);
    ClassDB::bind_method(D_METHOD("get_start_transition"), &REDFrame::get_start_transition);
    ClassDB::bind_method(D_METHOD("set_end_state", "end_state"), &REDFrame::set_end_state);
    ClassDB::bind_method(D_METHOD("get_end_state"), &REDFrame::get_end_state);
    ClassDB::bind_method(D_METHOD("set_end_transition", "end_transition"), &REDFrame::set_end_transition);
    ClassDB::bind_method(D_METHOD("get_end_transition"), &REDFrame::get_end_transition);


	ClassDB::bind_method(D_METHOD("set_origin_pos_gl", "origin_pos_gl"), &REDFrame::set_origin_pos_gl);
	ClassDB::bind_method(D_METHOD("get_origin_pos_gl"), &REDFrame::get_origin_pos_gl);
	ClassDB::bind_method(D_METHOD("update_origin_pos_gl"), &REDFrame::update_origin_pos_gl);
	ClassDB::bind_method(D_METHOD("set_camera_zoom", "camera_zoom"), &REDFrame::set_camera_zoom);
	ClassDB::bind_method(D_METHOD("get_camera_zoom"), &REDFrame::get_camera_zoom);
	ClassDB::bind_method(D_METHOD("set_start_delay", "start_delay"), &REDFrame::set_start_delay);
	ClassDB::bind_method(D_METHOD("get_start_delay"), &REDFrame::get_start_delay);
	ClassDB::bind_method(D_METHOD("set_end_delay", "end_delay"), &REDFrame::set_end_delay);
	ClassDB::bind_method(D_METHOD("get_end_delay"), &REDFrame::get_end_delay);

	ClassDB::bind_method(D_METHOD("set_camera_pos_path", "camera_pos_path"), &REDFrame::set_camera_pos_path);
	ClassDB::bind_method(D_METHOD("get_camera_pos_path"), &REDFrame::get_camera_pos_path);
	ClassDB::bind_method(D_METHOD("set_camera_pos_in_path", "camera_pos_in_path"), &REDFrame::set_camera_pos_in_path);
	ClassDB::bind_method(D_METHOD("get_camera_pos_in_path"), &REDFrame::get_camera_pos_in_path);
	ClassDB::bind_method(D_METHOD("set_camera_pos_out_path", "camera_pos_out_path"), &REDFrame::set_camera_pos_out_path);
	ClassDB::bind_method(D_METHOD("get_camera_pos_out_path"), &REDFrame::get_camera_pos_out_path);
	ClassDB::bind_method(D_METHOD("set_parallax_pos_path", "parallax_pos_path"), &REDFrame::set_parallax_pos_path);
	ClassDB::bind_method(D_METHOD("get_parallax_pos_path"), &REDFrame::get_parallax_pos_path);
	ClassDB::bind_method(D_METHOD("set_parallax_pos_in_path", "parallax_pos_in_path"), &REDFrame::set_parallax_pos_in_path);
	ClassDB::bind_method(D_METHOD("get_parallax_pos_in_path"), &REDFrame::get_parallax_pos_in_path);
	ClassDB::bind_method(D_METHOD("set_parallax_pos_out_path", "parallax_pos_out_path"), &REDFrame::set_parallax_pos_out_path);
	ClassDB::bind_method(D_METHOD("get_parallax_pos_out_path"), &REDFrame::get_parallax_pos_out_path);

	ClassDB::bind_method(D_METHOD("set_camera_pos", "camera_pos"), &REDFrame::set_camera_pos);
	ClassDB::bind_method(D_METHOD("get_camera_pos"), &REDFrame::get_camera_pos);
	ClassDB::bind_method(D_METHOD("set_camera_pos_in", "camera_pos_in"), &REDFrame::set_camera_pos_in);
	ClassDB::bind_method(D_METHOD("get_camera_pos_in"), &REDFrame::get_camera_pos_in);
	ClassDB::bind_method(D_METHOD("set_camera_pos_out", "camera_pos_out"), &REDFrame::set_camera_pos_out);
	ClassDB::bind_method(D_METHOD("get_camera_pos_out"), &REDFrame::get_camera_pos_out);
	ClassDB::bind_method(D_METHOD("set_parallax_pos", "parallax_pos"), &REDFrame::set_parallax_pos);
	ClassDB::bind_method(D_METHOD("get_parallax_pos"), &REDFrame::get_parallax_pos);
	ClassDB::bind_method(D_METHOD("set_parallax_pos_in", "parallax_pos_in"), &REDFrame::set_parallax_pos_in);
	ClassDB::bind_method(D_METHOD("get_parallax_pos_in"), &REDFrame::get_parallax_pos_in);
	ClassDB::bind_method(D_METHOD("set_parallax_pos_out", "parallax_pos_out"), &REDFrame::set_parallax_pos_out);
	ClassDB::bind_method(D_METHOD("get_parallax_pos_out"), &REDFrame::get_parallax_pos_out);

	ClassDB::bind_method(D_METHOD("update_camera_zoom"), &REDFrame::update_camera_zoom);
	ClassDB::bind_method(D_METHOD("_target_moved_parallax"), &REDFrame::_target_moved_parallax);
	ClassDB::bind_method(D_METHOD("_target_moved_pos"), &REDFrame::_target_moved_pos);

	ClassDB::bind_method(D_METHOD("set_frame_scale", "frame_scale"), &REDFrame::set_frame_scale);
	ClassDB::bind_method(D_METHOD("set_parallax", "parallax"), &REDFrame::set_parallax);
	ClassDB::bind_method(D_METHOD("get_parallax"), &REDFrame::get_parallax);
	ClassDB::bind_method(D_METHOD("set_offset_motion_scale", "offset_motion_scale"), &REDFrame::set_offset_motion_scale);
	ClassDB::bind_method(D_METHOD("get_offset_motion_scale"), &REDFrame::get_offset_motion_scale);
	ClassDB::bind_method(D_METHOD("set_anchor", "anchor"), &REDFrame::set_anchor);
    ClassDB::bind_method(D_METHOD("get_anchor"), &REDFrame::get_anchor);

	ClassDB::bind_method(D_METHOD("set_start_immediate", "start_immediate"), &REDFrame::set_start_immediate);
    ClassDB::bind_method(D_METHOD("is_start_immediate"), &REDFrame::is_start_immediate);
	ADD_GROUP("Frame", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "start_immediate"), "set_start_immediate", "is_start_immediate");	
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "start_delay"), "set_start_delay", "get_start_delay");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "end_delay"), "set_end_delay", "get_end_delay");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "Anchor", PROPERTY_HINT_ENUM, "Center, Top left"), "set_space", "get_space");

	ADD_GROUP("Camera", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_pos_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDTarget"), "set_camera_pos_path", "get_camera_pos_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_pos_in_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDTarget"), "set_camera_pos_in_path", "get_camera_pos_in_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_pos_out_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDTarget"), "set_camera_pos_out_path", "get_camera_pos_out_path");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom"), "set_camera_zoom", "get_camera_zoom");

	ADD_GROUP("Parallax", "");

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "parallax_pos_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDTarget"), "set_parallax_pos_path", "get_parallax_pos_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "parallax_pos_in_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDTarget"), "set_parallax_pos_in_path", "get_parallax_pos_in_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "parallax_pos_out_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDTarget"), "set_parallax_pos_out_path", "get_parallax_pos_out_path");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset_motion_scale"), "set_offset_motion_scale", "get_offset_motion_scale");
	
	ADD_GROUP("Animation", "");
 	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "anim_tree", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "AnimationTree"), "set_anim_tree", "get_anim_tree");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "states"), "set_states", "get_states");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "start_transition"), "set_start_transition", "get_start_transition");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "end_transition"), "set_end_transition", "get_end_transition");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "end_state"), "set_end_state", "get_end_state");

	BIND_ENUM_CONSTANT(LINE_JOINT_SHARP);
	BIND_ENUM_CONSTANT(LINE_JOINT_BEVEL);
	BIND_ENUM_CONSTANT(LINE_JOINT_ROUND);

	BIND_ENUM_CONSTANT(LINE_TEXTURE_NONE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_TILE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_STRETCH);

    BIND_VMETHOD(MethodInfo("_travel"));
    /*
	BIND_VMETHOD(MethodInfo("_end_loop"));
    BIND_VMETHOD(MethodInfo("_starting"));
    BIND_VMETHOD(MethodInfo("_started"));
    BIND_VMETHOD(MethodInfo("_ending"));
    BIND_VMETHOD(MethodInfo("_ended"));
	*/
	ADD_SIGNAL(MethodInfo("update_camera_zoom", PropertyInfo(Variant::VECTOR2, "zoom_val")));
	ADD_SIGNAL(MethodInfo("_target_moved_parallax", PropertyInfo(Variant::VECTOR2, "camera_val")));
	ADD_SIGNAL(MethodInfo("_target_moved_pos", PropertyInfo(Variant::VECTOR2, "camera_val")));
    //BIND_VMETHOD(MethodInfo("_state_changed"));

	BIND_ENUM_CONSTANT(FRAME_ANCHOR_CENTER);
	BIND_ENUM_CONSTANT(FRAME_ANCHOR_TOP_LEFT);
}

REDFrame::REDFrame() {
	start_immediate = false;
	anchor = FRAME_ANCHOR_CENTER;
	start_delay = 0.0f;
	end_delay = 0.0f;
	old_scale_offset = Vector2(0.f, 0.f);
	old_offset = Vector2(0.f, 0.f);
	position_motion_scale = Vector2(0.f, 0.f);
	offset_motion_scale = Vector2(0.f, 0.f);

	camera_zoom = Vector2(1.f, 1.f);
    
	b_start_loop = false;
    b_starting = false;
    b_started = false;
    b_ending = false;
    b_ended = false;

	reinit_tree = false;
    b_active = false;
	focused = false;
    end_transition = "pre start";
    end_transition = "pre end";
    end_state = "end";
    id = 0;
}
