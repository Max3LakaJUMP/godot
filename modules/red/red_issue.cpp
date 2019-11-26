#include "red_issue.h"
#include "red.h"
#include "red_engine.h"
#include "red_frame.h"
#include "red_page.h"
#include "red_controller_base.h"

#include "core/engine.h"
#include "core/class_db.h"
#include "core/object.h"
#include "core/string_name.h"
#include <string>
#include "scene/animation/animation_node_state_machine.h"

void REDIssue::set_invert_pages(bool p_invert_pages) {
	invert_pages = p_invert_pages;
    page_scenes.invert();
    _change_notify("page_scenes");
}

bool REDIssue::get_invert_pages() const {
	return invert_pages;
}

void REDIssue::set_autostart(bool p_autostart) {
	autostart = p_autostart;
}

bool REDIssue::get_autostart() const {
	return autostart;
}



REDPage *REDIssue::get_page (int p_id){
    if (pages.size() == 0)
        return nullptr;
    int last_id = pages.size() - 1;
    p_id = MAX(MIN(p_id, last_id), 0);
    return pages[p_id];
}

void REDIssue::resize_instanced_list(int p_size){
    int old_size = pages.size();
    pages.resize(p_size);
    for (int i = old_size; i < p_size; i++){
        pages.write[i] = nullptr;
    }
}

void REDIssue::set_page (int p_id, bool is_prev, const Vector2 &p_zoom){
    int count = get_page_scenes_count();
    int last_id = count - 1;
    p_id = MAX(MIN(p_id, last_id), 0);
    resize_instanced_list(count);

    int start = 0;
    int end = p_id - instance_count;
    for (int i = 0; i < end; i++){
        unload_page(i);
    }
    start = p_id + instance_count + 1;
    end = count;
    for (int i = start; i < end; i++){
        unload_page(i);
    }

    start = MAX(p_id - instance_count, 0);
    end = MIN(p_id + instance_count + 1, count);
    for (int i = start; i < end; i++){
        load_page(i, is_prev, p_zoom);
    }
    set_id(p_id);
}

Ref<PackedScene> REDIssue::get_page_scene (int p_id){
    return page_scenes[p_id];
}

void REDIssue::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
            if(autostart)
                run();
		} break;
	}
}

void REDIssue::run() {
    if (!Engine::get_singleton()->is_editor_hint()) {
        REDControllerBase *controller = red::get_controller(this);
        if (controller != nullptr){
            controller->set_issue(this);
        }
    }
    else{
        //set_page (0, false);
    }
}

void REDIssue::update_camera_pos(const Vector2 &p_camera_pos) const {
	//RED *r = red::get_red(this);
	//if (r->get_camera_mode()) {
		//Object::cast_to<Node2D>(get_node(r->get_camera()))->
		//        set_position(get_node(get_current())->get_global_transform().xform(Vector2(0, 0)));
	//}
}

void REDIssue::update_camera_zoom(const Vector2 &p_camera_zoom) {
    if (!Engine::get_singleton()->is_editor_hint()) {
        if (is_inside_tree()){
            int count = get_pages_count();
            resize_instanced_list(MAX(count, pages.size()));
            for (int i = 0; i < count; i++){
                if (pages[i] != nullptr){
                    pages.write[i]->update_camera_zoom(p_camera_zoom);
                }
            }
        }
    }
}

int REDIssue::get_pages_count() {
    return pages.size();
}

int REDIssue::get_page_scenes_count() {
    return page_scenes.size();
}

void REDIssue::load_page(int p_id, bool is_prev, const Vector2 &p_camera_zoom) {
    REDIssue *issue = this;
    if (pages[p_id] != nullptr) {
        return;
    }
	if (p_id < 0 || p_id >= issue->get_pages_count())
		return;

	Ref<PackedScene> scene = issue->get_page_scene(p_id);
	if (scene.is_null())
    {
        return;
    }

	Node2D *node2d = (Node2D*)scene->instance();
	add_child(node2d);
    if (!Engine::get_singleton()->is_editor_hint()) {
	    node2d->set_owner(this);
    }
	

	Node *child = node2d->get_child(0);
	if (child->get_class() != "REDPage"){
        return;
    }
		
	REDPage *page = (REDPage*)(child);
	
	//Array pages_pos = issue->get_pages_pos();

	if (pages_pos.size() < p_id + 2)
		pages_pos.resize(p_id + 2);
	if (p_id==0)
		pages_pos.write[p_id] = Vector2(0.0f, 0.0f);
	if (!is_prev)
		pages_pos.write[p_id+1] = pages_pos[p_id] + Vector2(0, page->get_size().y) + Vector2(0, pages_margin.y);
	//issue->set_pages_pos(pages_pos[p_id]);
    node2d->set_position(pages_pos[p_id]);

	int count = page->get_frames_count();

	for (int i = 0; i < count; i++) {
		REDFrame *child_frame = page->get_frame(i);
		if (child_frame!=nullptr){
            child_frame->set_active(false);
		}
	}

	if (is_prev){
		page->set_id(page->get_frames_count()-1);
        for (int i = 0; i < count; i++) {
            REDFrame *child_frame = page->get_frame(i);
            if (child_frame!=nullptr){
                Ref<AnimationNodeStateMachinePlayback> playback = child_frame->get_playback();
                if (playback.is_null())
                    continue;
                child_frame->set_id(child_frame->get_states_count()-1);
                child_frame->set_reinit_tree(true);
            }
	    }
	}

    page->update_camera_zoom(p_camera_zoom);
    page->pause_frames();
    
    pages.write[p_id] = page;
}

void REDIssue::unload_page(int p_id) {
	if (pages[p_id] != nullptr) {
        pages[p_id]->get_parent()->queue_delete();
        pages.write[p_id] = nullptr;
        print_line("UnLoaded id");
        print_line(std::to_string(p_id).c_str());
	}
}

void REDIssue::load_pages() {
	int id = get_id();
    int count = get_page_scenes_count();
    pages.resize(count);
    for (int i = 0; i < count; i++){
        if (i < id - (instance_count + 1) / 2 || i > id + (instance_count + 1) / 2)
            continue;
        load_page(i);
    }
}


void REDIssue::unload_pages() {
    int count = pages.size();
    for (int i = 0; i < count; i++){
        unload_page(i);
    }
}


void REDIssue::to_prev() {
        /*
	int old_id = get_id();
    set_id(old_id - 1);
	int new_id = get_id();
	if (old_id != new_id) {
		if (!next.is_empty()) {
			unload_page(get_node(next));
		}
        set_next(current);
        set_current(get_prev());

        RED *r = red::get_red(this);
        r->set_current_page_path(get_current());
        set_prev(load_page(new_id-1, true));
        Object::cast_to<REDPage>(get_node(current))->run(true);
	}
    */
}

void REDIssue::to_next() {
    /*
	int old_id = get_id();
    set_id(old_id + 1);
	int new_id = get_id();
	if (old_id != new_id) {
        if (!prev.is_empty()) {
            unload_page(get_node(prev));
        }
        set_prev(current);
        set_current(get_next());

        RED *r = red::get_red(this);
        r->set_current_page_path(get_current());
        set_next(load_page(new_id+1));
        Object::cast_to<REDPage>(get_node(current))->run(false);
	}*/
}

void REDIssue::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_invert_pages", "invert_pages"), &REDIssue::set_invert_pages);
    ClassDB::bind_method(D_METHOD("get_invert_pages"), &REDIssue::get_invert_pages);
    ClassDB::bind_method(D_METHOD("set_autostart", "autostart"), &REDIssue::set_autostart);
    ClassDB::bind_method(D_METHOD("get_autostart"), &REDIssue::get_autostart);

    ClassDB::bind_method(D_METHOD("set_pages_margin", "pages_margin"), &REDIssue::set_pages_margin);
    ClassDB::bind_method(D_METHOD("get_pages_margin"), &REDIssue::get_pages_margin);

    ClassDB::bind_method(D_METHOD("set_instance_count", "instance_count"), &REDIssue::set_instance_count);
    ClassDB::bind_method(D_METHOD("get_instance_count"), &REDIssue::get_instance_count);

    ClassDB::bind_method(D_METHOD("set_page_scenes", "page_scenes"), &REDIssue::set_page_scenes);
    ClassDB::bind_method(D_METHOD("get_page_scenes"), &REDIssue::get_page_scenes);
    ClassDB::bind_method(D_METHOD("set_id", "id"), &REDIssue::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &REDIssue::get_id);
    ClassDB::bind_method(D_METHOD("set_pages_pos", "pages_pos"), &REDIssue::set_pages_pos);
    ClassDB::bind_method(D_METHOD("get_pages_pos"), &REDIssue::get_pages_pos);

    ClassDB::bind_method(D_METHOD("to_prev"), &REDIssue::to_prev);
    ClassDB::bind_method(D_METHOD("to_next"), &REDIssue::to_next);

	ClassDB::bind_method(D_METHOD("load_page", "i"), &REDIssue::load_page);
	ClassDB::bind_method(D_METHOD("unload_page", "node"), &REDIssue::unload_page);
	ClassDB::bind_method(D_METHOD("load_pages"), &REDIssue::load_pages);
	ClassDB::bind_method(D_METHOD("unload_pages"), &REDIssue::unload_pages);

	ClassDB::bind_method(D_METHOD("update_camera_pos"), &REDIssue::update_camera_pos);
	ClassDB::bind_method(D_METHOD("update_camera_zoom"), &REDIssue::update_camera_zoom);

    ADD_GROUP("Main", "");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "page_scenes", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_page_scenes", "get_page_scenes");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "invert_pages"), "set_invert_pages", "get_invert_pages");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "pages_pos"), "set_pages_pos", "get_pages_pos");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "pages_margin"), "set_pages_margin", "get_pages_margin");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "instance_count"), "set_instance_count", "get_instance_count");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autostart"), "set_autostart", "get_autostart");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
}

Array REDIssue::get_page_scenes() const {
    Array pages_temp;
    pages_temp.resize(page_scenes.size());
    for (int i = 0; i < pages_temp.size(); i++) {
        pages_temp[i] = page_scenes[i];
    }
    return pages_temp;
}

void REDIssue::set_page_scenes(const Array &p_page_scenes) {
    page_scenes.resize(p_page_scenes.size());
    for (int i = 0; i < p_page_scenes.size(); i++) {
        Ref<PackedScene> packed = p_page_scenes[i];
        page_scenes.write[i] = packed;
    }
}

Array REDIssue::get_pages_pos() const {
	Array y_temp;
    y_temp.resize(pages_pos.size());
	for (int i = 0; i < y_temp.size(); i++) {
        y_temp[i] = pages_pos[i];
	}
	return y_temp;
}

void REDIssue::set_pages_pos(const Array &new_y) {
    pages_pos.resize(new_y.size());
	for (int i = 0; i < pages_pos.size(); i++) {
        pages_pos.write[i] = new_y[i];
	}
}

void REDIssue::set_id(const int &p_id) {
    id = p_id;

	//if (num < 0) {
	//	id = 0;
	//} else if (num >= pages.size()) {
	//	id = pages.size() - 1;
	//} else {
	//	id = num;
	//}
	//Camera2D *camera = red::get_red(this)->get_camera();
	//camera->set_position(get_page()->get_frame()->get_global_transform().xform(Vector2(0, 0)));
}

int REDIssue::get_id() const {
    return id;
}

int REDIssue::get_instance_count() const {
    return instance_count;
}

void REDIssue::set_instance_count(const int &p_instance_count){
    instance_count = p_instance_count;
}

Vector2 REDIssue::get_pages_margin() const {
    return pages_margin;
}

void REDIssue::set_pages_margin(const Vector2 &p_pages_margin){
    pages_margin = p_pages_margin;
}
REDIssue::REDIssue() {
    id = 0;
    instance_count = 1;

    invert_pages = false;
    autostart = true;
}