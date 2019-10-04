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

#include "core/math/geometry.h"
#include "red_line.h"

#include "red_line_builder.h"
#include "core/math/math_funcs.h"
#include "core/core_string_names.h"
#include <string>
#include "scene/animation/animation_tree.h"
#include "scene/scene_string_names.h"

void  REDFrame::set_camera_pos(const Vector2 &p_camera_pos){
	camera_pos = p_camera_pos;

	emit_signal("update_camera_pos", p_camera_pos);
};

Vector2  REDFrame::get_camera_pos() const{
	return camera_pos;
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

void REDFrame::set_camera_zoom(const Vector2 &p_camera_zoom){
	if (camera_zoom != p_camera_zoom){
		camera_zoom = p_camera_zoom;
		emit_signal("update_camera_zoom", camera_zoom);
		//if (use_outline)
		//	update();
			//_draw_outline();
	}
};

Vector2  REDFrame::get_camera_zoom() const{
	return camera_zoom;
};

void REDFrame::animation_changed(StringName old_name, StringName new_name){
	if (reinit_tree){
		Ref<AnimationNodeStateMachinePlayback> playback = get_playback();
		if (playback.is_valid() && playback->is_playing()){
			reinit_tree = false;
			playback->start(end_state);
			print_line("REVERSEDERRROR");
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
			Ref<AnimationNodeStateMachinePlayback> playback = get_playback();
			if (playback != nullptr)
				playback->connect("animation_started", this, "animation_changed");
		} break;
	}
}

bool REDFrame::is_pre_starting() const{
	return b_pre_starting;
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

void REDFrame::_pre_starting(){
	b_pre_starting = true;
	b_starting = false;
	b_started = false;
	b_ending = false;
	b_ended = false;
	b_active = false;
	travel();
}

void REDFrame::_starting(){
	b_pre_starting = false;
	b_starting = true;
	b_started = false;
	b_ending = false;
	b_ended = false;
	b_active = true;
	travel();
	print_line("ACTIVATE");
	if (!anim_tree.is_empty()){
		//AnimationTree *at = Object::cast_to<AnimationTree>(get_node(get_anim_tree()));
		//Ref<AnimationNodeStateMachine> machine = at->get_tree_root();
		//if (machine.is_valid()){
			//Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
			//if (machine->has_node(machine->get_start_node())) {
				//playback->start(machine->get_start_node());
			//}
			//if (machine->has_node(get_state(id))) {
			//	playback->travel(get_state(id));
			//} 

			//Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
			//if (machine->has_node(machine->get_start_node()) && playback->get_current_node() != machine->get_start_node()) {
			//	playback->travel(machine->get_start_node());
			//}
		//}
	}
	if (get_script_instance() != NULL) {
		if (get_script_instance()->has_method("_starting"))
			get_script_instance()->call("_starting");
	}
}


void REDFrame::_started(){
	b_pre_starting = false;
	b_starting = false;
	b_started = true;
	b_ending = false;
	b_ended = false;
	b_active = true;

	travel();
	if (get_script_instance() != NULL) {
		if (get_script_instance()->has_method("_started"))
			get_script_instance()->call("_started");
	}
}

void REDFrame::_ending(){
	b_pre_starting = false;
	b_starting = false;
	b_started = false;
	b_ending = true;
	b_ended = false;
	b_active = true;
	travel();

	if (get_script_instance() != NULL) {
		if (get_script_instance()->has_method("_ending"))
			get_script_instance()->call("_ending");
	}
}

void REDFrame::_ended(){
	b_pre_starting = false;
	b_starting = false;
	b_started = false;
	b_ending = false;
	b_ended = true;
	b_active = false;
	travel();
	//REDManager *controller = red::get_controller(this);
	//if (controller!=nullptr){
	//	controller->to_next();
	//}
    if (get_script_instance() != NULL) {
		if (get_script_instance()->has_method("_ended"))
        	get_script_instance()->call("_ended");
	}
}
void REDFrame::_bind_methods() {
	ClassDB::bind_method(D_METHOD("animation_changed", "old_name", "new_name"), &REDFrame::animation_changed);
    //Frame managment
    //ClassDB::bind_method(D_METHOD("run"), &REDFrame::run);

    //ClassDB::bind_method(D_METHOD("recalc_state"), &REDFrame::recalc_state);

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

	ClassDB::bind_method(D_METHOD("set_camera_zoom", "camera_zoom"), &REDFrame::set_camera_zoom);
	ClassDB::bind_method(D_METHOD("get_camera_zoom"), &REDFrame::get_camera_zoom);

	ClassDB::bind_method(D_METHOD("set_camera_pos", "camera_pos"), &REDFrame::set_camera_pos);
	ClassDB::bind_method(D_METHOD("get_camera_pos"), &REDFrame::get_camera_pos);

	ADD_GROUP("Frame", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_pos"), "set_camera_pos", "get_camera_pos");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "camera_zoom"), "set_camera_zoom", "get_camera_zoom");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
	
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

    BIND_VMETHOD(MethodInfo("_starting"));
    BIND_VMETHOD(MethodInfo("_started"));
    BIND_VMETHOD(MethodInfo("_ending"));
    BIND_VMETHOD(MethodInfo("_ended"));

	ADD_SIGNAL(MethodInfo("update_camera_zoom", PropertyInfo(Variant::VECTOR2, "zoom_val")));
	ADD_SIGNAL(MethodInfo("update_camera_pos", PropertyInfo(Variant::VECTOR2, "camera_val")));
    //BIND_VMETHOD(MethodInfo("_state_changed"));
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

void REDFrame::set_state (int p_id){
	if (p_id >= states.size()){
		_ending();
	}
	else if(p_id < 0){
		_pre_starting();
	}
	else{
		set_id(p_id);
		_started();
	}
}

String REDFrame::get_state (int p_id){
	return states[p_id];
}

int REDFrame::get_states_count() const{
    return states.size();
}

void REDFrame::travel(){
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
	Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
	if (playback.is_null())
		return;
	at->set_active(true);
	if (is_active() && playback->is_playing()){
		if (is_ending()){
			if (machine->has_node(end_transition)) {
				playback->travel(end_transition);
			}
		}
		else if (is_starting()){
			if (machine->has_node(start_transition))
				playback->travel(start_transition);
		}
		else if (machine->has_node(states[id])) {
			playback->travel(states[id]);
		}
	} else{
		if (is_ended()){
			if (machine->has_node(end_state)) {
				playback->travel(end_state);
			}
		}
		else if (is_pre_starting()){
			if (machine->has_node(machine->get_start_node())) {
				playback->travel(machine->get_start_node());
			}
		}
	}
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
	if (playback.is_null()){
		return playback;
	}
	return playback;
}


void REDFrame::set_anim_tree(const NodePath &new_anim_tree) {
    anim_tree = new_anim_tree;
}

NodePath REDFrame::get_anim_tree() const {
    return anim_tree;
}

REDFrame::REDFrame() {
	camera_pos = Vector2(0.f, 0.f);
	camera_zoom = Vector2(1.f, 1.f);
    
	b_pre_starting = true;
    b_starting = false;
    b_started = false;
    b_ending = false;
    b_ended = false;

	reinit_tree = false;
    b_active = false;
    end_transition = "pre start";
    end_transition = "pre end";
    end_state = "end";
    id = 0;
}
