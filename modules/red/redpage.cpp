#include "redpage.h"

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
#include "redissue.h"
#include "core/print_string.h"
#include "scene/animation/animation_tree.h"
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
*/

int REDPage::get_frames_count() {
    return frames.size();
}

void REDPage::update_camera() const {
    Node* node = get_node(get_current());
    RED *r = red::get_red(*node);
    if (r->get_camera_mode() && !r->get_camera().is_empty()) {
        auto *cam = Object::cast_to<Node2D>(r->get_node(r->get_camera()));
        REDFrame* node2d = Object::cast_to<REDFrame>(node);
        Vector2 offset = node2d->_edit_get_rect().get_size();
        Vector2 new_pos = node2d->get_global_transform().xform(Vector2(0, 0))+offset/2.0f;
        cam->set_position(new_pos);
    }
}

void REDPage::run(bool is_prev=false) {
    if(frames.size()>0){
        set_current(frames[id]);
        if (!frames[id].is_empty()){
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

void REDPage::pause_frames() {
    Node* node = get_node(get_current());
    if(node!=nullptr){
        RED *r = red::get_red(*node);
        for (int i = 0; i < frames.size(); i++) {
            REDFrame *child_frame = Object::cast_to<REDFrame>(get_node(frames[i]));
            if (!child_frame->get_anim_tree().is_empty())
            {
                AnimationTree *at = Object::cast_to<AnimationTree>(child_frame->get_node(child_frame->get_anim_tree()));
                at->set_active(false);
            }
        }
    }
}

void REDPage::to_prev() {
    int old_id = get_id();
    set_id(old_id - 1);
    int new_id = get_id();
    RED *r = red::get_red(*this);
    if (old_id != new_id) {
        set_current(frames[new_id]);
        r->set_current_frame_path(get_current());
        Object::cast_to<REDFrame>(get_node(get_current()))->run(true);
    }
    else{
        REDIssue *issue = Object::cast_to<REDIssue>(r->get_node(r->get_current_issue_path()));
        issue->to_prev();
    }
}

void REDPage::to_next() {
    int old_id = get_id();
    set_id(old_id + 1);
    int new_id = get_id();
    RED *r = red::get_red(*this);
    if (old_id != new_id) {
        print_line("bug in page file");
        set_current(frames[new_id]);
        Object::cast_to<REDFrame>(get_node(get_current()))->run(false);

    }
    else{
        REDIssue *issue = Object::cast_to<REDIssue>(r->get_node(r->get_current_issue_path()));
        issue->to_next();
    }
}

void REDPage::_bind_methods() {

    ClassDB::bind_method(D_METHOD("set_prev", "prev_page"), &REDPage::set_prev);
    ClassDB::bind_method(D_METHOD("get_prev"), &REDPage::get_prev);
    ClassDB::bind_method(D_METHOD("set_current", "current"), &REDPage::set_current);
    ClassDB::bind_method(D_METHOD("get_current"), &REDPage::get_current);
    ClassDB::bind_method(D_METHOD("set_next", "next"), &REDPage::set_next);
    ClassDB::bind_method(D_METHOD("get_next"), &REDPage::get_next);

    ClassDB::bind_method(D_METHOD("set_frames", "frames"), &REDPage::set_frames);
    ClassDB::bind_method(D_METHOD("get_frames"), &REDPage::get_frames);
    ClassDB::bind_method(D_METHOD("set_id", "id"), &REDPage::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &REDPage::get_id);
    ClassDB::bind_method(D_METHOD("set_height", "height"), &REDPage::set_height);
    ClassDB::bind_method(D_METHOD("get_height"), &REDPage::get_height);

    ClassDB::bind_method(D_METHOD("to_prev"), &REDPage::to_prev);
    ClassDB::bind_method(D_METHOD("to_next"), &REDPage::to_next);
    ClassDB::bind_method(D_METHOD("update_camera"), &REDPage::update_camera);

    BIND_VMETHOD(MethodInfo("_frame_changed"));


    ADD_GROUP("Main", "");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "frames", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDFrame"), "set_frames", "get_frames");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "height"), "set_height", "get_height");

    ADD_GROUP("Data", "");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "prev", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDFrame"), "set_prev", "get_prev");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "current", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDFrame"), "set_current", "get_current");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "next", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDFrame"), "set_next", "get_next");

}

void REDPage::set_prev(const NodePath &frame){
    prev = frame;
}
void REDPage::set_current(const NodePath &frame){
    current = frame;
    if (is_inside_tree()){
        RED *r = red::get_red(*this);
        r->set_current_frame_path(frame);
    }
}
void REDPage::set_next(const NodePath &frame){
    next = frame;
}
NodePath REDPage::get_prev() const{
    return prev;
}
NodePath REDPage::get_current() const{
    return current;
}
NodePath REDPage::get_next() const{
    return next;
}


void REDPage::set_frames(const Array &frames_new) {
	frames.resize(frames_new.size());
	for (int i = 0; i < frames_new.size(); i++) {
		frames.write[i] = frames_new[i];
	}
}

void REDPage::set_id(const int &num) {
    if (num < 0) {
        id = 0;
    } else if (num >= frames.size()) {
        id = frames.size() - 1;
    } else {
        id = num;
    }
}
void REDPage::set_height(const float f) {
    height = f;
}

Array REDPage::get_frames() const {
    Array frames_temp;
    frames_temp.resize(frames.size());
    for (int i = 0; i < frames.size(); i++) {
        frames_temp[i] = frames[i];
    }
    return frames_temp;
}

int REDPage::get_id() const {
    return id;
}



float REDPage::get_height() const {
    return height;
}

REDPage::REDPage() {
	id = 0;
    if (frames.size()>0){
        set_current(frames[0]);
    }
}
