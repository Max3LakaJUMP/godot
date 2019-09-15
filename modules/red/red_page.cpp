#include "red_page.h"

#include "core/math/geometry.h"
#include "red_frame.h"
#include "scene/2d/skeleton_2d.h"
#include "scene/main/node.h"
#include "scene/resources/packed_scene.h"
#include <iostream>
#include <string>
#include "core/class_db.h"
#include "core/object.h"
#include "red_engine.h"
#include "red_issue.h"
#include "core/print_string.h"
#include "scene/animation/animation_tree.h"
#include "core/engine.h"
#include "scene/scene_string_names.h"
/*
void REDPage::set_start_frame_path(const NodePath &n) {
if (start_frame_path == n)
return;
start_frame_path = n;
}

NodePath REDPage::get_start_frame_path() const {
return start_frame_path;
}

void REDPage::set_next_page(const Ref<PackedScene> &s) {
next_page = s;
}

Ref<PackedScene> REDPage::get_next_page() const {
return next_page;
}

REDFrame *REDPage::get_start_frame() const {
if (this->get_start_frame_path().is_empty()) {
if (get_child_count() > 0) {
return Object::cast_to<REDFrame>(this->get_child(0));
} else
return nullptr;
}
else {
return Object::cast_to<REDFrame>(get_node(start_frame_path));
}
}

void REDPage::_notification(int p_notification) {
switch (p_notification) {
case NOTIFICATION_READY: {

if (start_frame_path.is_empty()) {
    int cc = get_child_count();
    Array arr;
    arr.resize(cc);
    for (int i = 0; i < cc; i++) {
        arr[i] = get_child(i);
        print_line(arr[i].get_type());
        if (arr[i].get_type() == "REDFrame") {
            start_frame = arr[i];
            break;
        }
    }
}


    if (get_child_count() > 0) {
        Node *w = get_child(0);
        NodePath *path = &w->get_path();
        start_frame_path = *path;
        print_line(w->get_name());
    }
}

Array arr;
int cc = get_child_count();
arr.resize(cc);
for (int i = 0; i < cc; i++)
    arr[i] = get_child(i);
print_line("Page has childs");
print_line(std::to_string(cc).c_str());

		} break;
	}
}


void REDPage::run(bool is_prev=false) {
    RED *r = red::get_red(this);
    r->set_current_page_path(r->get_path_to(this));
    
    if(frames.size()>0){
        pause_frames();
        if (!frames[id].is_empty()){
            set_current(frames[id]);
            REDFrame *child_frame = Object::cast_to<REDFrame>(get_node(current));
            if (!child_frame->get_anim_tree().is_empty())
            {
                AnimationTree *at = Object::cast_to<AnimationTree>(child_frame->get_node(child_frame->get_anim_tree()));
                at->set_active(true);
                child_frame->run(false);
            }
        }
    }
}

void REDPage::update_camera() const {
    Node* node = get_node(get_current());
    RED *r = red::get_red(node);
    if (r->get_camera_mode() && !r->get_camera().is_empty()) {
        auto *cam = Object::cast_to<Node2D>(r->get_node(r->get_camera()));
        REDFrame* node2d = Object::cast_to<REDFrame>(node);
        Vector2 offset = node2d->_edit_get_rect().get_size();
        Vector2 new_pos = node2d->get_global_transform().xform(Vector2(0, 0))+offset/2.0f;
        cam->set_position(new_pos);
    }
}
void REDPage::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
            break;
		} 
	}
}
*/



void REDPage::pause_frames() {
    if (is_inside_tree()){
        int count = frames.size();
        for (int i = 0; i < count; i++)
        {
            REDFrame *child_frame = get_frame(i);
            if (child_frame != nullptr){
                child_frame->set_active(false);
                print_line("set_active");
            }
        }
    }
}

void REDPage::update_camera_zoom(const Vector2 &p_zoom) {
    if (is_inside_tree()){
        int count = frames.size();
        for (int i = 0; i < count; i++)
        {
            REDFrame *child_frame = get_frame(i);
            if (child_frame != nullptr)
                child_frame->update_camera_zoom_and_child(p_zoom);
        }
    }
}



void REDPage::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_frame", "id", "is_prev"), &REDPage::set_frame);
    ClassDB::bind_method(D_METHOD("get_frame", "id"), &REDPage::get_frame);
    ClassDB::bind_method(D_METHOD("set_frames", "frames"), &REDPage::set_frames);
    ClassDB::bind_method(D_METHOD("get_frames"), &REDPage::get_frames);

    ClassDB::bind_method(D_METHOD("set_id", "id"), &REDPage::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &REDPage::get_id);
    ClassDB::bind_method(D_METHOD("set_size", "size"), &REDPage::set_size);
    ClassDB::bind_method(D_METHOD("get_size"), &REDPage::get_size);
    //BIND_VMETHOD(MethodInfo("_frame_changed"));

    ADD_GROUP("Main", "");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "frames", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDFrame"), "set_frames", "get_frames");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size"), "set_size", "get_size");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
}

int REDPage::get_frames_count() {
    return frames.size();
}

void REDPage::set_frame(int p_id, bool force_inactive, bool ended) {
    REDFrame *frame = get_frame(id);
    /*
    if (force_inactive){
        if (frame != nullptr){

            if (ended)
                frame->_ended();
            else {
                frame->_pre_starting();
            }
        }
    }*/
    int last_id = frames.size() - 1;
    p_id = MAX(MIN(p_id, last_id), 0);

    if (id == p_id){
        if (frame != nullptr){
            //frame->_starting();
        }
        return;
    }
    set_id(p_id);
    /*if (!force_inactive){
        if (frame != nullptr){
            if (ended)
                frame->_ended();
            else {
                frame->_pre_starting();
            }
        }
    }*/
    frame = get_frame(id);
    if (frame != nullptr){
        //frame->_starting();
    }
	return;
}

REDFrame *REDPage::get_frame (int p_id){
    int last_id = frames.size() - 1;
    p_id = MAX(MIN(p_id, last_id), 0);
    NodePath frame_path = frames[p_id];
    if (has_node(frame_path)){
        Node *node = get_node(frame_path);
        if (node->is_class("REDFrame")){
            return (REDFrame *)node;
        }
        node = node->find_node(frame_path.get_name(0));
        if (node != NULL){
            if (node->is_class("REDFrame")){
                return (REDFrame *)node;
            }
        }
    }
    return nullptr;
}

void REDPage::set_frames(const Array &frames_new) {
	frames.resize(frames_new.size());
	for (int i = 0; i < frames_new.size(); i++) {
		frames.write[i] = frames_new[i];
	}
}

Array REDPage::get_frames() const {
    Array frames_temp;
    frames_temp.resize(frames.size());
    for (int i = 0; i < frames.size(); i++) {
        frames_temp[i] = frames[i];
    }
    return frames_temp;
}

void REDPage::set_id(int p_id) {
    print_line("SET id");
    print_line(std::to_string(p_id).c_str());
    id = p_id;
    /*
    int last_id = frames.size() - 1;
    p_id = MAX(MIN(p_id, last_id), 0);
    
    if (id == p_id)
        return -1;
    
    if (p_id < 0){
        if (id == 0){
            return -1;
        }
        p_id = 0;
    }
    int last_id = frames.size() - 1;
    if (p_id >= last_id){
        if (id == last_id){
            return -1;
        }
        p_id = last_id;
    }
    id = p_id;
    return id;*/
}

int REDPage::get_id() const {
    return id;
}

void REDPage::set_size(const Size2 &p_size) {
    size = p_size;
}

Size2 REDPage::get_size() const {
    return size;
}

REDPage::REDPage() {
    id = 0;
}
