#include "red.h"

#include "core/engine.h"
#include "scene/2d/camera_2d.h"

#include "red_engine.h"
#include "red_frame.h"
#include "red_page.h"
#include "red_issue.h"
#include "modules/red/red_controller_base.h"

Camera2D* RED::get_camera() const{
	return (Camera2D*)get_node(camera_path);
}

REDControllerBase* RED::get_controller() const{
	return (REDControllerBase*)get_node(controller_path);
}

NodePath RED::get_controller_path() {
	return controller_path;
}

void RED::set_controller_path(const NodePath &p_controller_path) {
	controller_path = p_controller_path;
}

/*
void RED::next_frame() {
    REDIssue *issue = Object::cast_to<REDIssue>(get_node(get_current_issue_path()));
    REDPage *page = Object::cast_to<REDPage>(issue->get_node(get_current_page_path()));
    REDFrame *frame = Object::cast_to<REDFrame>(page->get_node(get_current_frame_path()));


    if (frame->get_id()<frame->get_states_count()){
        frame->set_id(frame->get_id()+1);
        state->
    } else if (page->get_id()<page->get_frames_count()){

    } else if (issue->get_id()<issue->get_pages_count()) {

    }
}

void RED::update_camera() const {
    if (get_camera_mode() && !get_camera().is_empty() && !get_current_frame_path().is_empty()) {
        Camera2D *cam = Object::cast_to<Camera2D>(get_node(camera));
		REDFrame *frame = get_current_frame();
		Vector2 offset = frame->_edit_get_rect().get_size();
		Vector2 new_pos = frame->get_global_transform().xform(Vector2(0, 0))+offset/2.0f;
        cam->set_position(new_pos);
    }
}*/

/*
void RED::next_frame() {
	REDFrame *f = get_current_frame();
	//REDFrame *n = Object::cast_to<REDFrame>(get_node(f->get_next()));
	NodePath next_frame_path = f->get_next();
	if (next_frame_path.is_empty()) {
		Ref<PackedScene> next_page_path = get_current_page()->get_next_page();
		if (next_page_path == NULL) {
			print_line("ended");
		} else {
			REDIssue *i = Object::cast_to<REDIssue>(get_node(get_current_issue_path()));
			i->next_page();
		}
	} else {
		set_current_frame(*(f->get_node(next_frame_path)));
	}

}
*/






/*
void next_frame() {
	switch (t) {
		case true: {
			RED *r = get_red();
			REDFrame *f = r->get_current_frame();
			NodePath *f = f->get_next();
			r->set_current_frame()
					get_red()

		} break;
		case false:
			// do something
			break;
	}
}
*/

void RED::attach_camera() {
	if (!camera_path.is_empty()) {
		Camera2D *cam = Object::cast_to<Camera2D>(get_node(camera_path));
		cam->make_current();
	}
}

void RED::_notification(int p_what) {
	switch (p_what) {
		case RED::NOTIFICATION_READY: {

		} break;
	}
}

void RED::_bind_methods() {
    ClassDB::bind_method(D_METHOD("attach_camera"), &RED::attach_camera);
    //ClassDB::bind_method(D_METHOD("update_camera"), &RED::update_camera);

	ClassDB::bind_method(D_METHOD("set_vertical_mode", "vertical_mode"), &RED::set_vertical_mode);
	ClassDB::bind_method(D_METHOD("get_vertical_mode"), &RED::get_vertical_mode);
	ClassDB::bind_method(D_METHOD("set_html_mode", "html_mode"), &RED::set_html_mode);
	ClassDB::bind_method(D_METHOD("get_html_mode"), &RED::get_html_mode);

	
	ClassDB::bind_method(D_METHOD("set_camera_mode", "camera_mode"), &RED::set_camera_mode);
	ClassDB::bind_method(D_METHOD("get_camera_mode"), &RED::get_camera_mode);
	ClassDB::bind_method(D_METHOD("set_camera_path", "camera"), &RED::set_camera_path);
	ClassDB::bind_method(D_METHOD("get_camera_path"), &RED::get_camera_path);

	ClassDB::bind_method(D_METHOD("set_controller_path", "controller_path"), &RED::set_controller_path);
	ClassDB::bind_method(D_METHOD("get_controller_path"), &RED::get_controller_path);
	
	ClassDB::bind_method(D_METHOD("get_controller"), &RED::get_controller);
	ClassDB::bind_method(D_METHOD("get_camera"), &RED::get_camera);
	/*
    ClassDB::bind_method(D_METHOD("set_current_issue_path", "issue"), &RED::set_current_issue_path);
    ClassDB::bind_method(D_METHOD("get_current_issue_path"), &RED::get_current_issue_path);
	ClassDB::bind_method(D_METHOD("set_current_page_path", "page"), &RED::set_current_page_path);
	ClassDB::bind_method(D_METHOD("get_current_page_path"), &RED::get_current_page_path);
	ClassDB::bind_method(D_METHOD("set_current_frame_path", "frame"), &RED::set_current_frame_path);
	ClassDB::bind_method(D_METHOD("get_current_frame_path"), &RED::get_current_frame_path);*/
	//ClassDB::bind_method(D_METHOD("next_frame"), &RED::next_frame);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller", PROPERTY_HINT_RESOURCE_TYPE, "REDControllerBase", 0), "", "get_controller");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "camera", PROPERTY_HINT_RESOURCE_TYPE, "Camera2D", 0), "", "get_camera");
	ADD_GROUP("Properties", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Camera2D"), "set_camera_path", "get_camera_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "controller_path", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDControllerBase"), "set_controller_path", "get_controller_path");

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_mode"), "set_camera_mode", "get_camera_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vertical_mode"), "set_vertical_mode", "get_vertical_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "html_mode"), "set_html_mode", "get_html_mode");
}

void RED::set_camera_mode(bool b) {
	camera_mode = b;
}

bool RED::get_camera_mode() const {
	return camera_mode;
}
void RED::set_vertical_mode(bool b) {
	vertical_mode = b;
}

bool RED::get_vertical_mode() const {
	return vertical_mode;
}

void RED::set_html_mode(bool b) {
	html_mode = b;
}

bool RED::get_html_mode() const {
	return html_mode;
}

void RED::set_camera_path(const NodePath &c) {
	if (camera_path == c)
		return;
	camera_path = c;
}

NodePath RED::get_camera_path() const {
	return camera_path;
}
/*
void RED::set_current_page_path(const NodePath &p) {
	if (page == p)
		return;
	page = p;
}
NodePath RED::get_current_page_path() const {
	return page;
}
void RED::set_current_page(const REDPage &p) {
	NodePath path = p.get_path();
	if (page == path)
		return;
	page = path;
    set_current_frame_path(p.get_current());
}

REDIssue *RED::get_current_issue() const {
    return Object::cast_to<REDIssue>(get_node(issue));
}

REDPage *RED::get_current_page() const {
	return Object::cast_to<REDPage>(get_current_issue()->get_node(page));
}

void RED::set_current_frame_path(const NodePath &f) {
	if (frame == f)
		return;
	frame = f;
	if (!frame.is_empty())
    	update_camera();
}
NodePath RED::get_current_frame_path() const {
	return frame;
}

void RED::set_current_frame(const REDFrame &p) {
	NodePath path = p.get_path();
	if (page == path)
		return;
	page = path;
}*/
/*
void RED::set_current_frame(const Node &p) {
	NodePath path = get_node(page)->get_path_to(&p);
	if (frame == path)
		return;
	set_current_frame_path(path);
}
REDFrame *RED::get_current_frame() const {
	REDPage *p = get_current_page();
	return Object::cast_to<REDFrame>(p->get_node(p->get_current()));
}


void RED::set_current_frame(const NodePath &p) {
	REDFrame *f = Object::cast_to<REDFrame>(get_node(p));
	if (frame == f)
		return;
	frame = f;
	Camera2D *camera = Object::cast_to<Camera2D>(get_node(cam));
	camera->set_pos(f.get_global_transform()->xform(Vector2(0, 0)));
}
*/

RED::RED() {
	vertical_mode = false;
	html_mode = false;
	camera_mode = true;
}
