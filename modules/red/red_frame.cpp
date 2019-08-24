#include "red_frame.h"
#include "scene/animation/animation_player.h"
#include "scene/animation/animation_tree.h"
#include "scene/animation/animation_node_state_machine.h"
#include "core/engine.h"

#include "redpage.h"
#include "red_engine.h"
#include "red.h"
#include "redissue.h"

#include "core/math/geometry.h"
#include "red_line.h"

#include "red_line_builder.h"
#include "core/math/math_funcs.h"
#include "core/core_string_names.h"
#include <string>

//Line

void REDFrame::set_use_outline(const bool b) {
    use_outline = b;
	update_outline = b;
	update();
}

bool REDFrame::get_use_outline() const{
    return use_outline;

}

void REDFrame::set_width(float p_width) {
	if (p_width < 0.0)
		p_width = 0.0;
	_width = p_width;
	update_outline = true;
	update();

}

float REDFrame::get_width() const {
	return _width;
}

void REDFrame::set_curve(const Ref<Curve> &p_curve) {
	// Cleanup previous connection if any
	if (_curve.is_valid()) {
		_curve->disconnect(CoreStringNames::get_singleton()->changed, this, "_curve_changed");
	}

	_curve = p_curve;

	// Connect to the curve so the line will update when it is changed
	if (_curve.is_valid()) {
		_curve->connect(CoreStringNames::get_singleton()->changed, this, "_curve_changed");
	}
	update_outline = true;
	update();
}

Ref<Curve> REDFrame::get_curve() const {
	return _curve;
}

void REDFrame::set_default_color(Color p_color) {
	_default_color = p_color;
	update_outline = true;
	update();
}

Color REDFrame::get_default_color() const {
	return _default_color;
}

void REDFrame::set_gradient(const Ref<Gradient> &p_gradient) {

	// Cleanup previous connection if any
	if (_gradient.is_valid()) {
		_gradient->disconnect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}

	_gradient = p_gradient;

	// Connect to the gradient so the line will update when the ColorRamp is changed
	if (_gradient.is_valid()) {
		_gradient->connect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}
	update_outline = true;
	update();
}

Ref<Gradient> REDFrame::get_gradient() const {
	return _gradient;
}

void REDFrame::set_line_texture(const Ref<Texture> &p_texture) {
	_texture = p_texture;
	update_outline = true;
	update();
}

Ref<Texture> REDFrame::get_line_texture() const {
	return _texture;
}

void REDFrame::set_texture_mode(const LineTextureMode p_mode) {
	_texture_mode = p_mode;
	update_outline = true;
	update();
}

REDFrame::LineTextureMode REDFrame::get_texture_mode() const {
	return _texture_mode;
}

void REDFrame::set_joint_mode(REDFrame::LineJointMode p_mode) {
	_joint_mode = p_mode;
	update_outline = true;
	update();
}

REDFrame::LineJointMode REDFrame::get_joint_mode() const {
	return _joint_mode;
}

void REDFrame::set_sharp_limit(float p_limit) {
	if (p_limit < 0.f)
		p_limit = 0.f;
	_sharp_limit = p_limit;
	update_outline = true;
	update();
}

float REDFrame::get_sharp_limit() const {
	return _sharp_limit;
}

void REDFrame::set_round_precision(int p_precision) {
	if (p_precision < 1)
		p_precision = 1;
	_round_precision = p_precision;
	update_outline = true;
	update();
}

int REDFrame::get_round_precision() const {
	return _round_precision;
}

void REDFrame::_draw_outline() {
	int len = polygon.size();

	Vector<Vector2> p_points;
	p_points.resize(len);
	{
		PoolVector<Vector2>::Read polyr = polygon.read();
		for (int i = 0; i < len; i++)
			p_points.write[i] = polyr[i] + offset;
	}
	if (p_points.size() <= 1 || _width == 0.f)
		return;

	Vector<float> new_thickness;
	if (thickness_list.size()>1){
		int pre_a;
		int b;
		int post_b;
		int end=thickness_list.size()-1;
		float smooth_thickness_iter = len*1.0f/thickness_list.size()-1;

		PoolVector<float>::Read thickness_list_read = thickness_list.read();
		for(int i=0; i<thickness_list.size(); ++i){
			if (i==end)
				b = 0;
			else
				b = i + 1;

			if (i==0)
				pre_a = end;
			else
				pre_a = i - 1;

			if (i==end - 1)
				post_b = 0;
			else
				post_b = b + 1;
			new_thickness.push_back(thickness_list_read[i]);
			for(int j=0; j<smooth_thickness_iter; ++j){
				new_thickness.push_back(thickness_list_read[i]+(j+1)*1.0/(smooth_thickness_iter+1)*(thickness_list_read[b]-thickness_list_read[i]));
			}
		}
	} else {
        int oldSize = MIN(thickness_list.size(), len);
        new_thickness.resize(len);
        PoolVector<float>::Read thickness_list_read = thickness_list.read();
        for (int i = 0; i < oldSize; i++) {
            new_thickness.write[i] = thickness_list_read[i];
        }
        for (int i = oldSize; i < len; i++) {
            new_thickness.write[i] = 1.f;
        }
	}
	// TODO Maybe have it as member rather than copying parameters and allocating memory?
	REDLineBuilder lb;
	lb.points = p_points;
	lb.default_color = _default_color;
	lb.gradient = *_gradient;
	lb.texture_mode = static_cast<REDLine::LineTextureMode>(_texture_mode);
	lb.joint_mode = static_cast<REDLine::LineJointMode>(_joint_mode);
	lb.begin_cap_mode = static_cast<REDLine::LineCapMode>(_begin_cap_mode);
	lb.end_cap_mode = static_cast<REDLine::LineCapMode>(_end_cap_mode);
	lb.round_precision = _round_precision;
	lb.sharp_limit = _sharp_limit;
	lb.width = _width;
	lb.curve = *_curve;
    lb.is_closed = true;
    lb.thickness_list = new_thickness;

	RID texture_rid;
	if (_texture.is_valid()) {
		texture_rid = _texture->get_rid();

		lb.tile_aspect = _texture->get_size().aspect();
	}

	lb.build();

	VS::get_singleton()->canvas_item_add_triangle_array(
			get_canvas_item(),
			lb.indices,
			lb.vertices,
			lb.colors,
			lb.uvs, Vector<int>(), Vector<float>(),

			texture_rid);
	update_outline = false;
	/*Draw wireframe
		if(lb.indices.size() % 3 == 0) {
			Color col(0,0,0);
			for(int i = 0; i < lb.indices.size(); i += 3) {
				int vi = lb.indices[i];
				int lbvsize = lb.vertices.size();
				Vector2 a = lb.vertices[lb.indices[i]];
				Vector2 b = lb.vertices[lb.indices[i+1]];
				Vector2 c = lb.vertices[lb.indices[i+2]];
				draw_line(a, b, col);
				draw_line(b, c, col);
				draw_line(c, a, col);
			}
			for(int i = 0; i < lb.vertices.size(); ++i) {
				Vector2 p = lb.vertices[i];
				draw_rect(Rect2(p.x-1, p.y-1, 2, 2), Color(0,0,0,0.5));
			}
		}*/
}

void REDFrame::_gradient_changed() {
	update_outline = true;
	update();

}

void REDFrame::_curve_changed() {
	update_outline = true;
	update();
}

void REDFrame::set_thickness_list(const PoolVector<float> &p_thickness_list) {
    thickness_list = p_thickness_list;
	update_outline = true;
    update();
}

PoolVector<float> REDFrame::get_thickness_list() const {
    return thickness_list;
}

Dictionary REDFrame::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["offset"] = offset;
	return state;
}

void REDFrame::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_offset(p_state["offset"]);
}

void REDFrame::_edit_set_pivot(const Point2 &p_pivot) {
	set_position(get_transform().xform(p_pivot));
	set_offset(get_offset() - p_pivot);
}

Point2 REDFrame::_edit_get_pivot() const {
	return Vector2();
}

bool REDFrame::_edit_use_pivot() const {
	return true;
}

Rect2 REDFrame::_edit_get_rect() const {
	if (rect_cache_dirty) {
		int l = polygon.size();
		PoolVector<Vector2>::Read r = polygon.read();
		item_rect = Rect2();
		for (int i = 0; i < l; i++) {
			Vector2 pos = r[i] + offset;
			if (i == 0)
				item_rect.position = pos;
			else
				item_rect.expand_to(pos);
		}
		rect_cache_dirty = false;
	}

	return item_rect;
}

bool REDFrame::_edit_use_rect() const {
	return polygon.size() > 0;
}

bool REDFrame::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {

	Vector<Vector2> REDFrame = Variant(polygon);
	return Geometry::is_point_in_polygon(p_point - get_offset(), REDFrame);
}

void REDFrame::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_INTERNAL_PROCESS: {
			update_screen_coords();
		} break;
		case NOTIFICATION_DRAW: {
			if (use_outline)
				_draw_outline();
		} break;
	}
}

void REDFrame::set_polygon(const PoolVector<Vector2> &p_polygon) {
	polygon = p_polygon;
	rect_cache_dirty = true;
	update_outline = true;
	update();
}

PoolVector<Vector2> REDFrame::get_polygon() const {

	return polygon;
}

void REDFrame::set_antialiased(bool p_antialiased) {

	antialiased = p_antialiased;
	update_outline = true;
	update();
}
bool REDFrame::get_antialiased() const {

	return antialiased;
}

void REDFrame::set_offset(const Vector2 &p_offset) {

	offset = p_offset;
	rect_cache_dirty = true;
	update_outline = true;
	update();
	_change_notify("offset");
}

Vector2 REDFrame::get_offset() const {

	return offset;
}





///////////////////////////////FRAME menegment///////////////////////////////////////
void REDFrame::run(bool is_prev=false){
	if (!anim_tree.is_empty()){
		set_id(0);
		AnimationTree *at = Object::cast_to<AnimationTree>(get_node(anim_tree));
		at->set_active(true);
		Ref<AnimationNodeStateMachine> machine = at->get_tree_root();
		if (machine->has_node(states[id])) {
			Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
			playback->travel(states[id]);
			print_line(states[id]);
		}
		if (get_script_instance())
			get_script_instance()->call("_run");
	}
}


void REDFrame::to_prev(){

	if (anim_tree.is_empty()){
		RED *r = red::get_red(*this);
		REDPage *page = r->get_current_page();
		REDIssue *issue = r->get_current_issue();
		if (issue->get_id() == 0 && page->get_id() == 0){
		} else {
			r->get_current_page()->to_prev();
			is_active = false;
		}
	}
	else{
		int old_id = get_id();
		set_id(old_id - 1);
		int new_id = get_id();

		if (old_id != new_id) {
			AnimationTree *at = Object::cast_to<AnimationTree>(get_node(anim_tree));
			Ref<AnimationNodeStateMachine> machine = at->get_tree_root();
			if (machine->has_node(states[id])) {
				Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
				playback->travel(states[id]);
			}
		}
		else{
			RED *r = red::get_red(*this);
			if(r){
				REDPage *page = r->get_current_page();
				REDIssue *issue = r->get_current_issue();
				if (issue->get_id() == 0 && page->get_id() == 0){
				} else {
					r->get_current_page()->to_prev();
					Object::cast_to<AnimationTree>(get_node(get_anim_tree()))->set_active(false);
					is_active = false;
				}
			}
		}
	}
}


void REDFrame::to_next() {
	if (anim_tree.is_empty()){
		ended();
	}
	else{
		int old_id = get_id();
    	set_id(old_id + 1);
    	int new_id = get_id();

		if (old_id != new_id) {
			AnimationTree *at = Object::cast_to<AnimationTree>(get_node(anim_tree));
			Ref<AnimationNodeStateMachine> machine = at->get_tree_root();
			if (machine->has_node(states[id])) {
				Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
				playback->travel(states[id]);
			}
		}
		else{
			AnimationTree *at = Object::cast_to<AnimationTree>(get_node(anim_tree));
			Ref<AnimationNodeStateMachine> machine = at->get_tree_root();
			Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
			playback->travel(machine->get_end_node());
		}
	}
}

int REDFrame::get_states_count() const{
    return states.size();
}

void REDFrame::recalc_state(){
    AnimationTree *at = Object::cast_to<AnimationTree>(get_node(anim_tree));
    Ref<AnimationNodeStateMachine> machine = at->get_tree_root();
    print_line(states[id]);
    if (is_starting){
        if (is_started){
            RED *r = red::get_red(*this);
            if(r){
                NodePath page_path = r->get_current_page_path();
                if(!page_path.is_empty()){
                    REDPage *page = Object::cast_to<REDPage>(r->get_node(page_path));
                    page->to_prev();
                    is_started = false;
                    is_active = false;
                }
            }
        }
        else{
            is_starting = false;
            is_started = true;
            is_ending = false;
            is_ended = false;
            if (get_script_instance())
                get_script_instance()->call("_started");
            Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
            playback->travel(machine->get_start_node());
            if (get_script_instance())
                get_script_instance()->call("_starting");
        }
    }
    else if (is_ending){
        is_starting = false;
        is_started = true;
        is_ending = false;
        is_ended = false;
        Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
        playback->travel(machine->get_end_node());
        if (get_script_instance())
            get_script_instance()->call("_ending");
    }
    else{
        if (get_states_count()>0){
            if (machine->has_node(states[id])) {
                Ref<AnimationNodeStateMachinePlayback> playback = at->get("parameters/playback");
                playback->travel(states[id]);
                if (get_script_instance())
                    get_script_instance()->call("_state_changed");
            }
        }
    }
}

void REDFrame::started(){
    is_starting = false;
    is_started = true;
    is_ending = false;
    is_ended = false;
    recalc_state();
    if (get_script_instance())
        get_script_instance()->call("_started");
}

void REDFrame::ended(){
    is_starting = false;
    is_started = false;
    is_ending = false;
    is_ended = true;
    RED *r = red::get_red(*this);

    if(r){
        REDPage *page = r->get_current_page();
        REDIssue *issue = r->get_current_issue();
        if (issue->get_id() >= issue->get_pages_count()-1 && page->get_id() >= page->get_frames_count()-1){

        } else {
            page->to_next();
			if (!anim_tree.is_empty()){
            	Object::cast_to<AnimationTree>(get_node(get_anim_tree()))->set_active(false);
			}
            is_active = false;
        };

    }
    if (get_script_instance())
        get_script_instance()->call("_ended");
}

void REDFrame::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update_screen_coords"), &REDFrame::update_screen_coords);
	//Line
	ClassDB::bind_method(D_METHOD("set_thickness_list", "thickness_list"), &REDFrame::set_thickness_list);
    ClassDB::bind_method(D_METHOD("get_thickness_list"), &REDFrame::get_thickness_list);

	ClassDB::bind_method(D_METHOD("set_width", "width"), &REDFrame::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &REDFrame::get_width);

	ClassDB::bind_method(D_METHOD("set_curve", "curve"), &REDFrame::set_curve);
	ClassDB::bind_method(D_METHOD("get_curve"), &REDFrame::get_curve);

	ClassDB::bind_method(D_METHOD("set_default_color", "color"), &REDFrame::set_default_color);
	ClassDB::bind_method(D_METHOD("get_default_color"), &REDFrame::get_default_color);

	ClassDB::bind_method(D_METHOD("set_gradient", "color"), &REDFrame::set_gradient);
	ClassDB::bind_method(D_METHOD("get_gradient"), &REDFrame::get_gradient);

	ClassDB::bind_method(D_METHOD("set_line_texture", "line_texture"), &REDFrame::set_line_texture);
	ClassDB::bind_method(D_METHOD("get_line_texture"), &REDFrame::get_line_texture);

	ClassDB::bind_method(D_METHOD("set_texture_mode", "mode"), &REDFrame::set_texture_mode);
	ClassDB::bind_method(D_METHOD("get_texture_mode"), &REDFrame::get_texture_mode);

	ClassDB::bind_method(D_METHOD("set_joint_mode", "mode"), &REDFrame::set_joint_mode);
	ClassDB::bind_method(D_METHOD("get_joint_mode"), &REDFrame::get_joint_mode);

	ClassDB::bind_method(D_METHOD("set_sharp_limit", "limit"), &REDFrame::set_sharp_limit);
	ClassDB::bind_method(D_METHOD("get_sharp_limit"), &REDFrame::get_sharp_limit);

	ClassDB::bind_method(D_METHOD("set_round_precision", "precision"), &REDFrame::set_round_precision);
	ClassDB::bind_method(D_METHOD("get_round_precision"), &REDFrame::get_round_precision);

	ClassDB::bind_method(D_METHOD("set_use_outline", "use_outline"), &REDFrame::set_use_outline);
    ClassDB::bind_method(D_METHOD("get_use_outline"), &REDFrame::get_use_outline);

	ClassDB::bind_method(D_METHOD("set_polygon", "polygon"), &REDFrame::set_polygon);
	ClassDB::bind_method(D_METHOD("get_polygon"), &REDFrame::get_polygon);

	ClassDB::bind_method(D_METHOD("set_antialiased", "antialiased"), &REDFrame::set_antialiased);
	ClassDB::bind_method(D_METHOD("get_antialiased"), &REDFrame::get_antialiased);

	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &REDFrame::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &REDFrame::get_offset);
	
	ADD_GROUP("Polygon", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "antialiased"), "set_antialiased", "get_antialiased");

	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "polygon"), "set_polygon", "get_polygon");

	//Line
	ADD_GROUP("Line", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_outline"), "set_use_outline", "get_use_outline");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_color"), "set_default_color", "get_default_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "thickness_list"), "set_thickness_list", "get_thickness_list");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "width_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_curve", "get_curve");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "joint_mode", PROPERTY_HINT_ENUM, "Sharp,Bevel,Round"), "set_joint_mode", "get_joint_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharp_limit"), "set_sharp_limit", "get_sharp_limit");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round_precision"), "set_round_precision", "get_round_precision");

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "line_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_line_texture", "get_line_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_mode", PROPERTY_HINT_ENUM, "None,Tile,Stretch"), "set_texture_mode", "get_texture_mode");

	BIND_ENUM_CONSTANT(LINE_JOINT_SHARP);
	BIND_ENUM_CONSTANT(LINE_JOINT_BEVEL);
	BIND_ENUM_CONSTANT(LINE_JOINT_ROUND);

	BIND_ENUM_CONSTANT(LINE_CAP_NONE);
	BIND_ENUM_CONSTANT(LINE_CAP_BOX);
	BIND_ENUM_CONSTANT(LINE_CAP_ROUND);

	BIND_ENUM_CONSTANT(LINE_TEXTURE_NONE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_TILE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_STRETCH);

	
	ClassDB::bind_method(D_METHOD("_gradient_changed"), &REDFrame::_gradient_changed);
	ClassDB::bind_method(D_METHOD("_curve_changed"), &REDFrame::_curve_changed);


    //Frame managment
    ClassDB::bind_method(D_METHOD("run"), &REDFrame::run);

    ClassDB::bind_method(D_METHOD("recalc_state"), &REDFrame::recalc_state);

    ClassDB::bind_method(D_METHOD("started"), &REDFrame::started);
    ClassDB::bind_method(D_METHOD("ended"), &REDFrame::ended);


    ClassDB::bind_method(D_METHOD("to_prev"), &REDFrame::to_prev);
    ClassDB::bind_method(D_METHOD("to_next"), &REDFrame::to_next);
    ClassDB::bind_method(D_METHOD("get_states_count"), &REDFrame::get_states_count);

    ClassDB::bind_method(D_METHOD("set_anim_tree", "anim_tree"), &REDFrame::set_anim_tree);
    ClassDB::bind_method(D_METHOD("get_anim_tree"), &REDFrame::get_anim_tree);
    ClassDB::bind_method(D_METHOD("set_id", "id"), &REDFrame::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &REDFrame::get_id);
    ClassDB::bind_method(D_METHOD("set_states", "states"), &REDFrame::set_states);
    ClassDB::bind_method(D_METHOD("get_states"), &REDFrame::get_states);

    BIND_VMETHOD(MethodInfo("_run"));
    BIND_VMETHOD(MethodInfo("_starting"));
    BIND_VMETHOD(MethodInfo("_started"));
    BIND_VMETHOD(MethodInfo("_ending"));
    BIND_VMETHOD(MethodInfo("_ended"));
    BIND_VMETHOD(MethodInfo("_state_changed"));

    ADD_GROUP("Main", "");
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "anim_tree", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "AnimationTree"), "set_anim_tree", "get_anim_tree");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "states"), "set_states", "get_states");

    ADD_GROUP("Shape", "");

}

void REDFrame::set_id(int num){
    if (!anim_tree.is_empty()){
        int new_id;
        if (num < 0) {
            is_starting = true;
            new_id = 0;
        } else if (num >= get_states_count()) {
            is_ending = true;
            new_id = get_states_count() - 1;
            if (new_id<0)
                new_id=0;
        } else {
            new_id = num;
        }
        if (id != new_id){
            id = new_id;
        }
    }
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

void REDFrame::set_anim_tree(const NodePath &new_anim_tree) {
    anim_tree = new_anim_tree;
}

NodePath REDFrame::get_anim_tree() const {
    return anim_tree;
}
void REDFrame::update_screen_coords() {
	if(!is_inside_tree()){
		return;
	}
	int cc = get_child_count();
	if (cc<1)
		return;
	Node *ch = get_child(0);
	CanvasItem *c = (CanvasItem*)(ch);
	if (c==nullptr)
		return;
	int size = polygon.size();
	if (size<3)
		return;
	Ref<ShaderMaterial> mat = c->get_material();
	if (mat.is_null())
		return;
	int j = size-1;

	Vector<float> constant;
	Vector<float> multiple;

	Transform2D tr = get_viewport_transform() * get_global_transform();
	Size2 res = get_viewport()->get_size();
	screen_coords.clear();
	for (int i = 0; i < size; i++) {
		Vector2 screen_coord = tr.xform(polygon[i]) / res;
		screen_coord.y = 1.0f - screen_coord.y;
		screen_coords.push_back(screen_coord);
	}
	for(int i=0; i < size; i++) {
		if(screen_coords[j].y==screen_coords[i].y) {
			constant.push_back(screen_coords[i].x);
			multiple.push_back(0);
		}
		else {
			constant.push_back(screen_coords[i].x-(screen_coords[i].y*screen_coords[j].x)/(screen_coords[j].y-screen_coords[i].y)+(screen_coords[i].y*screen_coords[i].x)/(screen_coords[j].y-screen_coords[i].y));
			multiple.push_back((screen_coords[j].x-screen_coords[i].x)/(screen_coords[j].y-screen_coords[i].y));
		}
		j=i; 
	}

	if (screen_coords.size() == 3) {
		mat->set_shader_param("mxy1", Vector3(multiple[0], screen_coords[0].x, screen_coords[0].y));
		mat->set_shader_param("mxy2", Vector3(multiple[1], screen_coords[1].x, screen_coords[1].y));
		mat->set_shader_param("mxy3", Vector3(multiple[2], screen_coords[2].x, screen_coords[2].y));
		mat->set_shader_param("c1", Vector3(constant[0], constant[1], constant[2]));
	} else if (screen_coords.size() == 4) {
		mat->set_shader_param("mxy1", Vector3(multiple[0], screen_coords[0].x, screen_coords[0].y));
		mat->set_shader_param("mxy2", Vector3(multiple[1], screen_coords[1].x, screen_coords[1].y));
		mat->set_shader_param("mxy3", Vector3(multiple[2], screen_coords[2].x, screen_coords[2].y));
		mat->set_shader_param("mxy4", Vector3(multiple[3], screen_coords[3].x, screen_coords[3].y));
		mat->set_shader_param("c1", Vector3(constant[0], constant[1], constant[2]));
		mat->set_shader_param("c2", constant[3]);
	}
}
/* 
void REDFrame::update_screen_coords() {
    int size = polygon.size();
	screen_coords.clear();
    if (size>2){
        Transform2D tr = get_viewport_transform() * get_global_transform();
        Size2 res = get_viewport()->get_size();
        for (int i = 0; i < polygon.size(); i++) {
            Vector2 screen_coord = tr.xform(polygon[i]) / res;
            screen_coord.y = 1.0f - screen_coord.y;
            screen_coords.push_back(screen_coord);
        }
    }
}
*/
void REDFrame::send_mask_shader_param() {
	int cc = get_child_count();
	if (cc<1)
		return;
	Node *ch = get_child(0);
	CanvasItem *c = Object::cast_to<CanvasItem>(ch);
	Ref<ShaderMaterial> mat = c->get_material();
	if (screen_coords.size() == 3) {
		mat->set_shader_param("maskX", Vector3(screen_coords[0].x, screen_coords[1].x, screen_coords[2].x));
		mat->set_shader_param("maskY", Vector3(screen_coords[0].y, screen_coords[1].y, screen_coords[2].y));
	} else if (screen_coords.size() == 4) {
		mat->set_shader_param("maskX", Vector3(screen_coords[0].x, screen_coords[1].x, screen_coords[2].x));
		mat->set_shader_param("maskY", Vector3(screen_coords[0].y, screen_coords[1].y, screen_coords[2].y));
		mat->set_shader_param("maskXY", Vector2(screen_coords[3].x, screen_coords[3].y));
	}
}


REDFrame::REDFrame() {
	_joint_mode = LINE_JOINT_SHARP;
	_begin_cap_mode = LINE_CAP_NONE;
	_end_cap_mode = LINE_CAP_NONE;
	_width = 4;
	_default_color = Color(0.0, 0.0, 0.0);
	_texture_mode = LINE_TEXTURE_NONE;
	_sharp_limit = 2.f;
	_round_precision = 8;
	
	use_outline = true;
	
	antialiased = false;
	rect_cache_dirty = true;
	bool update_outline = true;

    is_starting = true;
    is_started = false;
    is_ending = false;
    is_ended = false;
    is_active = false;
    id = 0;
	set_process_internal(true);
}
