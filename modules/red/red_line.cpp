#include "red_line.h"
#include "red_line_builder.h"
#include "core/math/math_funcs.h"
#include "core/core_string_names.h"

// Needed so we can bind functions
VARIANT_ENUM_CAST(REDLine::LineJointMode)
VARIANT_ENUM_CAST(REDLine::LineCapMode)
VARIANT_ENUM_CAST(REDLine::LineTextureMode)

REDLine::REDLine() {
	_joint_mode = LINE_JOINT_SHARP;
	_begin_cap_mode = LINE_CAP_NONE;
	_end_cap_mode = LINE_CAP_NONE;
	_width = 10;
	_default_color = Color(0.4, 0.5, 1);
	_texture_mode = LINE_TEXTURE_NONE;
	_sharp_limit = 2.f;
	_round_precision = 8;
}

Rect2 REDLine::_edit_get_rect() const {

	if (_points.size() == 0)
		return Rect2(0, 0, 0, 0);
	Vector2 d = Vector2(_width, _width);
	Rect2 aabb = Rect2(_points[0] - d, 2 * d);
	for (int i = 1; i < _points.size(); i++) {
		aabb.expand_to(_points[i] - d);
		aabb.expand_to(_points[i] + d);
	}
	return aabb;
}

bool REDLine::_edit_use_rect() const {
	return true;
}

bool REDLine::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {

	const real_t d = _width / 2 + p_tolerance;
	PoolVector<Vector2>::Read points = _points.read();
	for (int i = 0; i < _points.size() - 1; i++) {
		Vector2 p = Geometry::get_closest_point_to_segment_2d(p_point, &points[i]);
		if (p.distance_to(p_point) <= d)
			return true;
	}

	return false;
}

void REDLine::set_points(const PoolVector<Vector2> &p_points) {
	_points = p_points;
	update();
}

void REDLine::set_width(float p_width) {
	if (p_width < 0.0)
		p_width = 0.0;
	_width = p_width;
	update();
}

float REDLine::get_width() const {
	return _width;
}

void REDLine::set_curve(const Ref<Curve> &p_curve) {
	// Cleanup previous connection if any
	if (_curve.is_valid()) {
		_curve->disconnect(CoreStringNames::get_singleton()->changed, this, "_curve_changed");
	}

	_curve = p_curve;

	// Connect to the curve so the line will update when it is changed
	if (_curve.is_valid()) {
		_curve->connect(CoreStringNames::get_singleton()->changed, this, "_curve_changed");
	}

	update();
}

Ref<Curve> REDLine::get_curve() const {
	return _curve;
}

PoolVector<Vector2> REDLine::get_points() const {
	return _points;
}

void REDLine::set_point_position(int i, Vector2 p_pos) {
	_points.set(i, p_pos);
	update();
}

Vector2 REDLine::get_point_position(int i) const {
	ERR_FAIL_INDEX_V(i, _points.size(), Vector2());
	return _points.get(i);
}

int REDLine::get_point_count() const {
	return _points.size();
}

void REDLine::clear_points() {
	int count = _points.size();
	if (count > 0) {
		_points.resize(0);
		update();
	}
}

void REDLine::add_point(Vector2 p_pos, int p_atpos) {
    thickness_list.resize(_points.size());
	if (p_atpos < 0 || _points.size() < p_atpos) {
		_points.append(p_pos);
        thickness_list.append(1.0f);
	} else {
		_points.insert(p_atpos, p_pos);
        thickness_list.insert(p_atpos, 1.0f);
	}
	update();
}

void REDLine::remove_point(int i) {
	_points.remove(i);
    thickness_list.remove(i);
	update();
}

void REDLine::set_default_color(Color p_color) {
	_default_color = p_color;
	update();
}

Color REDLine::get_default_color() const {
	return _default_color;
}

void REDLine::set_gradient(const Ref<Gradient> &p_gradient) {

	// Cleanup previous connection if any
	if (_gradient.is_valid()) {
		_gradient->disconnect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}

	_gradient = p_gradient;

	// Connect to the gradient so the line will update when the ColorRamp is changed
	if (_gradient.is_valid()) {
		_gradient->connect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}

	update();
}

Ref<Gradient> REDLine::get_gradient() const {
	return _gradient;
}

void REDLine::set_texture(const Ref<Texture> &p_texture) {
	_texture = p_texture;
	update();
}

Ref<Texture> REDLine::get_texture() const {
	return _texture;
}

void REDLine::set_texture_mode(const LineTextureMode p_mode) {
	_texture_mode = p_mode;
	update();
}

REDLine::LineTextureMode REDLine::get_texture_mode() const {
	return _texture_mode;
}

void REDLine::set_joint_mode(LineJointMode p_mode) {
	_joint_mode = p_mode;
	update();
}

REDLine::LineJointMode REDLine::get_joint_mode() const {
	return _joint_mode;
}

void REDLine::set_begin_cap_mode(LineCapMode p_mode) {
	_begin_cap_mode = p_mode;
	update();
}

REDLine::LineCapMode REDLine::get_begin_cap_mode() const {
	return _begin_cap_mode;
}

void REDLine::set_end_cap_mode(LineCapMode p_mode) {
	_end_cap_mode = p_mode;
	update();
}

REDLine::LineCapMode REDLine::get_end_cap_mode() const {
	return _end_cap_mode;
}

void REDLine::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW:
			_draw();
			break;
	}
}

void REDLine::set_sharp_limit(float p_limit) {
	if (p_limit < 0.f)
		p_limit = 0.f;
	_sharp_limit = p_limit;
	update();
}

float REDLine::get_sharp_limit() const {
	return _sharp_limit;
}

void REDLine::set_round_precision(int p_precision) {
	if (p_precision < 1)
		p_precision = 1;
	_round_precision = p_precision;
	update();
}

int REDLine::get_round_precision() const {
	return _round_precision;
}

void REDLine::_draw() {
	if (_points.size() <= 1 || _width == 0.f)
		return;

	// TODO Is this really needed?
	// Copy points for faster access
	Vector<Vector2> points;
	points.resize(_points.size());
    Vector<float> t_l;
	int len = points.size();
	{
		PoolVector<Vector2>::Read points_read = _points.read();
		for (int i = 0; i < len; ++i) {
			points.write[i] = points_read[i];
		}


        int oldSize = MIN(thickness_list.size(), len);
        t_l.resize(len);
        PoolVector<float>::Read thickness_list_read = thickness_list.read();
        for (int i = 0; i < oldSize; i++) {
            t_l.write[i] = thickness_list_read[i];
        }
        for (int i = oldSize; i < len; i++) {
            t_l.write[i] = 1.f;
        }
	}

	// TODO Maybe have it as member rather than copying parameters and allocating memory?
	REDLineBuilder lb;
	lb.points = points;
	lb.default_color = _default_color;
	lb.gradient = *_gradient;
	lb.texture_mode = _texture_mode;
	lb.joint_mode = _joint_mode;
	lb.begin_cap_mode = _begin_cap_mode;
	lb.end_cap_mode = _end_cap_mode;
	lb.round_precision = _round_precision;
	lb.sharp_limit = _sharp_limit;
	lb.width = _width;
	lb.curve = *_curve;
    lb.is_closed = is_closed;
    lb.thickness_list = t_l;

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

	// DEBUG
	// Draw wireframe
	//	if(lb.indices.size() % 3 == 0) {
	//		Color col(0,0,0);
	//		for(int i = 0; i < lb.indices.size(); i += 3) {
	//			int vi = lb.indices[i];
	//			int lbvsize = lb.vertices.size();
	//			Vector2 a = lb.vertices[lb.indices[i]];
	//			Vector2 b = lb.vertices[lb.indices[i+1]];
	//			Vector2 c = lb.vertices[lb.indices[i+2]];
	//			draw_line(a, b, col);
	//			draw_line(b, c, col);
	//			draw_line(c, a, col);
	//		}
	//		for(int i = 0; i < lb.vertices.size(); ++i) {
	//			Vector2 p = lb.vertices[i];
	//			draw_rect(Rect2(p.x-1, p.y-1, 2, 2), Color(0,0,0,0.5));
	//		}
	//	}
}

void REDLine::_gradient_changed() {
	update();
}

void REDLine::_curve_changed() {
	update();
}

void REDLine::set_is_closed(const bool b) {
    is_closed = b;
    update();
}

bool REDLine::get_is_closed() const{
    return is_closed;
}

void REDLine::set_thickness_list(const PoolVector<float> &p_thickness_list) {
    thickness_list = p_thickness_list;
    update();
}

PoolVector<float> REDLine::get_thickness_list() const {
    return thickness_list;
}

// static
void REDLine::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_thickness_list", "thickness_list"), &REDLine::set_thickness_list);
    ClassDB::bind_method(D_METHOD("get_thickness_list"), &REDLine::get_thickness_list);

    ClassDB::bind_method(D_METHOD("set_is_closed", "is_closed"), &REDLine::set_is_closed);
    ClassDB::bind_method(D_METHOD("get_is_closed"), &REDLine::get_is_closed);

	ClassDB::bind_method(D_METHOD("set_points", "points"), &REDLine::set_points);
	ClassDB::bind_method(D_METHOD("get_points"), &REDLine::get_points);

	ClassDB::bind_method(D_METHOD("set_point_position", "i", "position"), &REDLine::set_point_position);
	ClassDB::bind_method(D_METHOD("get_point_position", "i"), &REDLine::get_point_position);

	ClassDB::bind_method(D_METHOD("get_point_count"), &REDLine::get_point_count);

	ClassDB::bind_method(D_METHOD("add_point", "position", "at_position"), &REDLine::add_point, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("remove_point", "i"), &REDLine::remove_point);

	ClassDB::bind_method(D_METHOD("clear_points"), &REDLine::clear_points);

	ClassDB::bind_method(D_METHOD("set_width", "width"), &REDLine::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &REDLine::get_width);

	ClassDB::bind_method(D_METHOD("set_curve", "curve"), &REDLine::set_curve);
	ClassDB::bind_method(D_METHOD("get_curve"), &REDLine::get_curve);

	ClassDB::bind_method(D_METHOD("set_default_color", "color"), &REDLine::set_default_color);
	ClassDB::bind_method(D_METHOD("get_default_color"), &REDLine::get_default_color);

	ClassDB::bind_method(D_METHOD("set_gradient", "color"), &REDLine::set_gradient);
	ClassDB::bind_method(D_METHOD("get_gradient"), &REDLine::get_gradient);

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &REDLine::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &REDLine::get_texture);

	ClassDB::bind_method(D_METHOD("set_texture_mode", "mode"), &REDLine::set_texture_mode);
	ClassDB::bind_method(D_METHOD("get_texture_mode"), &REDLine::get_texture_mode);

	ClassDB::bind_method(D_METHOD("set_joint_mode", "mode"), &REDLine::set_joint_mode);
	ClassDB::bind_method(D_METHOD("get_joint_mode"), &REDLine::get_joint_mode);

	ClassDB::bind_method(D_METHOD("set_begin_cap_mode", "mode"), &REDLine::set_begin_cap_mode);
	ClassDB::bind_method(D_METHOD("get_begin_cap_mode"), &REDLine::get_begin_cap_mode);

	ClassDB::bind_method(D_METHOD("set_end_cap_mode", "mode"), &REDLine::set_end_cap_mode);
	ClassDB::bind_method(D_METHOD("get_end_cap_mode"), &REDLine::get_end_cap_mode);

	ClassDB::bind_method(D_METHOD("set_sharp_limit", "limit"), &REDLine::set_sharp_limit);
	ClassDB::bind_method(D_METHOD("get_sharp_limit"), &REDLine::get_sharp_limit);

	ClassDB::bind_method(D_METHOD("set_round_precision", "precision"), &REDLine::set_round_precision);
	ClassDB::bind_method(D_METHOD("get_round_precision"), &REDLine::get_round_precision);

	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "points"), "set_points", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_closed"), "set_is_closed", "get_is_closed");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "thickness_list"), "set_thickness_list", "get_thickness_list");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "width_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_curve", "get_curve");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_color"), "set_default_color", "get_default_color");
	ADD_GROUP("Fill", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_mode", PROPERTY_HINT_ENUM, "None,Tile,Stretch"), "set_texture_mode", "get_texture_mode");
	ADD_GROUP("Capping", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "joint_mode", PROPERTY_HINT_ENUM, "Sharp,Bevel,Round"), "set_joint_mode", "get_joint_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "begin_cap_mode", PROPERTY_HINT_ENUM, "None,Box,Round"), "set_begin_cap_mode", "get_begin_cap_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "end_cap_mode", PROPERTY_HINT_ENUM, "None,Box,Round"), "set_end_cap_mode", "get_end_cap_mode");
	ADD_GROUP("Border", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharp_limit"), "set_sharp_limit", "get_sharp_limit");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round_precision"), "set_round_precision", "get_round_precision");

	BIND_ENUM_CONSTANT(LINE_JOINT_SHARP);
	BIND_ENUM_CONSTANT(LINE_JOINT_BEVEL);
	BIND_ENUM_CONSTANT(LINE_JOINT_ROUND);

	BIND_ENUM_CONSTANT(LINE_CAP_NONE);
	BIND_ENUM_CONSTANT(LINE_CAP_BOX);
	BIND_ENUM_CONSTANT(LINE_CAP_ROUND);

	BIND_ENUM_CONSTANT(LINE_TEXTURE_NONE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_TILE);
	BIND_ENUM_CONSTANT(LINE_TEXTURE_STRETCH);

	ClassDB::bind_method(D_METHOD("_gradient_changed"), &REDLine::_gradient_changed);
	ClassDB::bind_method(D_METHOD("_curve_changed"), &REDLine::_curve_changed);
}
