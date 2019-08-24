#include "redissue.h"
#include "red.h"
#include "red_engine.h"
#include "red_frame.h"
#include "redpage.h"

#include "core/class_db.h"
#include "core/object.h"
#include "core/string_name.h"

int REDIssue::get_pages_count() {
    return pages.size();
}

NodePath REDIssue::load_page(const int &i, bool is_prev) {
	if ((i >= 0) && (i < pages.size())) {

        Node2D *node2d = Object::cast_to<Node2D>(pages[i]->instance());
        add_child(node2d);
        node2d->set_owner(this);
        if (i < y_offset.size()) {
            node2d->set_position(Vector2(0, y_offset[i]));
        }
        REDPage *page = Object::cast_to<REDPage>(node2d->get_child(0));
        if (is_prev){
            Array frames = page->get_frames();
            int id = frames.size()-1;
            page->set_id(id);
            page->pause_frames();
            return page->get_path();
        } else
            page->pause_frames();
		    return node2d->get_child(0)->get_path();
	}
	return NodePath();
}

void REDIssue::unload_page(Node *page) const {
	if (page != nullptr) {
		page->queue_delete();
        page = nullptr;
	}
}

void REDIssue::load_pages() {
	int i = get_id();
    set_current(load_page(i));
    set_prev(load_page(i - 1));
	set_next(load_page(i + 1));
}


void REDIssue::unload_pages() {
	unload_page(get_node(get_prev()));
	unload_page(get_node(get_next()));
	unload_page(get_node(get_current()));
}

void REDIssue::run() {
	unload_pages();
	load_pages();
    if (!current.is_empty()){
        RED *r = red::get_red(*this);
        REDPage *p = Object::cast_to<REDPage>(get_node(current));
        r->set_current_issue_path(r->get_path_to(this));
        p->run(false);
    }

}

void REDIssue::update_camera() const {
	RED *r = red::get_red(*this);
	if (r->get_camera_mode()) {
		//Object::cast_to<Node2D>(get_node(r->get_camera()))->
		//        set_position(get_node(get_current())->get_global_transform().xform(Vector2(0, 0)));
	}
}

void REDIssue::to_prev() {
	int old_id = get_id();
    set_id(old_id - 1);
	int new_id = get_id();
	if (old_id != new_id) {
		if (!next.is_empty()) {
			unload_page(get_node(next));
		}
        set_next(current);
        set_current(get_prev());

        RED *r = red::get_red(*this);
        r->set_current_page_path(get_current());
        set_prev(load_page(new_id-1, true));
        Object::cast_to<REDPage>(get_node(current))->run(true);
	}
}

void REDIssue::to_next() {
	int old_id = get_id();
    set_id(old_id + 1);
	int new_id = get_id();
	if (old_id != new_id) {
        print_line("bug in issue file");
        if (!prev.is_empty()) {
            unload_page(get_node(prev));
        }
        set_prev(current);
        set_current(get_next());

        RED *r = red::get_red(*this);
        r->set_current_page_path(get_current());
        set_next(load_page(new_id+1));
        Object::cast_to<REDPage>(get_node(current))->run(false);
	}
}

void REDIssue::_bind_methods() {

    ClassDB::bind_method(D_METHOD("set_prev", "prev"), &REDIssue::set_prev);
    ClassDB::bind_method(D_METHOD("get_prev"), &REDIssue::get_prev);
    ClassDB::bind_method(D_METHOD("set_current", "current"), &REDIssue::set_current);
    ClassDB::bind_method(D_METHOD("get_current"), &REDIssue::get_current);
    ClassDB::bind_method(D_METHOD("set_next", "next_page"), &REDIssue::set_next);
    ClassDB::bind_method(D_METHOD("get_next"), &REDIssue::get_next);

    ClassDB::bind_method(D_METHOD("set_pages", "pages"), &REDIssue::set_pages);
    ClassDB::bind_method(D_METHOD("get_pages"), &REDIssue::get_pages);
    ClassDB::bind_method(D_METHOD("set_id", "id"), &REDIssue::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &REDIssue::get_id);
    ClassDB::bind_method(D_METHOD("set_y_offset", "y_offset"), &REDIssue::set_y_offset);
    ClassDB::bind_method(D_METHOD("get_y_offset"), &REDIssue::get_y_offset);

    ClassDB::bind_method(D_METHOD("to_prev"), &REDIssue::to_prev);
    ClassDB::bind_method(D_METHOD("to_next"), &REDIssue::to_next);
	ClassDB::bind_method(D_METHOD("run"), &REDIssue::run);
	ClassDB::bind_method(D_METHOD("load_page", "i"), &REDIssue::load_page);
	ClassDB::bind_method(D_METHOD("unload_page", "node"), &REDIssue::unload_page);
	ClassDB::bind_method(D_METHOD("load_pages"), &REDIssue::load_pages);
	ClassDB::bind_method(D_METHOD("unload_pages"), &REDIssue::unload_pages);
	ClassDB::bind_method(D_METHOD("update_camera"), &REDIssue::update_camera);


    ADD_GROUP("Main", "");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "pages", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_pages", "get_pages");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "y_offset"), "set_y_offset", "get_y_offset");

    ADD_GROUP("Data", "");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "prev", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDPage"), "set_prev", "get_prev");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "current", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDPage"), "set_current", "get_current");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "next", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDPage"), "set_next", "get_next");

}

Array REDIssue::get_pages() const {
    Array pages_temp;
    pages_temp.resize(pages.size());
    for (int i = 0; i < pages_temp.size(); i++) {
        pages_temp[i] = pages[i];
    }
    return pages_temp;
}

void REDIssue::set_pages(const Array &pages_new) {
    pages.resize(pages_new.size());
    for (int i = 0; i < pages.size(); i++) {
        pages.write[i] = pages_new[i];
    }
}

Array REDIssue::get_y_offset() const {
	Array y_temp;
    y_temp.resize(y_offset.size());
	for (int i = 0; i < y_temp.size(); i++) {
        y_temp[i] = y_offset[i];
	}
	return y_temp;
}

void REDIssue::set_y_offset(const Array &new_y) {
    y_offset.resize(new_y.size());
	for (int i = 0; i < y_offset.size(); i++) {
        y_offset.write[i] = new_y[i];
	}
}

void REDIssue::set_id(const int &num) {
	if (num < 0) {
		id = 0;
	} else if (num >= pages.size()) {
		id = pages.size() - 1;
	} else {
		id = num;
	}
	//Camera2D *camera = red::get_red(*this)->get_camera();
	//camera->set_position(get_page()->get_frame()->get_global_transform().xform(Vector2(0, 0)));
}

int REDIssue::get_id() const {
    return id;
}

void REDIssue::set_prev(const NodePath &page) {
    prev = page;
}

void REDIssue::set_current(const NodePath &page) {
    current = page;
    if (is_inside_tree()){
        RED *r = red::get_red(*this);
        r->set_current_page_path(page);
    }
}

void REDIssue::set_next(const NodePath &page) {
    next = page;
}

NodePath REDIssue::get_prev() const {
    return prev;
}

NodePath REDIssue::get_current() const {
    return current;
}

NodePath REDIssue::get_next() const {
    return next;
}

REDIssue::REDIssue() {
    id = 0;
}