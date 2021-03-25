#include "red_shape_renderer.h"
#include "red_shape.h"
#include "red.h"
#include "scene/2d/line_builder.h"
#include "core/core_string_names.h"

VARIANT_ENUM_CAST(Line2D::LineJointMode)
VARIANT_ENUM_CAST(Line2D::LineCapMode)
VARIANT_ENUM_CAST(Line2D::LineTextureMode)

// draw
void REDShapeRenderer::_draw_polygon() {
	if(shape->boolean_polygon.size() < 3 && shape->real_polygon.size() < 3)
		return;
	Vector<Color> colors;
	colors.push_back(color);
	VS::get_singleton()->canvas_item_add_polygon(get_canvas_item(), shape->boolean_polygon.size() > 0 ? shape->boolean_polygon : shape->real_polygon, colors);
}

void REDShapeRenderer::_draw_outline() {
	LineBuilder lb;
	lb.points = shape->boolean_polygon.size() > 0 ? shape->boolean_polygon : shape->real_polygon;
	lb.default_color = color;
	lb.gradient = *gradient;
	lb.texture_mode = line_texture_mode;
	lb.joint_mode = _joint_mode;
	lb.begin_cap_mode = _begin_cap_mode;
	lb.end_cap_mode = _end_cap_mode;
	lb.round_precision = _round_precision;
	lb.sharp_limit = _sharp_limit;
	lb.width = _width + width_zoom_const * ((_width * shape->get_camera_zoom().x / get_scale().x) - _width);
	lb.curve = *curve;
    lb.closed = true;
    lb.width_list = shape->real_width_list;
	RID texture_rid;
	if (texture.is_valid()) {
		texture_rid = texture->get_rid();
		lb.tile_aspect = texture->get_size().aspect();
	}
	lb.build();
	VS::get_singleton()->canvas_item_add_triangle_array(
			get_canvas_item(),
			lb.indices,
			lb.vertices,
			lb.colors,
			lb.uvs, Vector<int>(), Vector<float>(),
			texture_rid, -1, RID(),
			antialiased, true);
}

// main
void REDShapeRenderer::set_render_mode(REDShapeRenderer::RenderMode p_render_mode){
	if(render_mode == p_render_mode)
		return;
	render_mode = p_render_mode;
	update();
}

REDShapeRenderer::RenderMode REDShapeRenderer::get_render_mode() const{
	return render_mode;
}

void REDShapeRenderer::set_color(Color p_color) {
	color = p_color;
	update();
}

Color REDShapeRenderer::get_color() const {
	return color;
}

void REDShapeRenderer::set_gradient(const Ref<Gradient> &p_gradient) {

	// Cleanup previous connection if any
	if (gradient.is_valid()) {
		gradient->disconnect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}

	gradient = p_gradient;

	// Connect to the gradient so the line will update when the ColorRamp is changed
	if (gradient.is_valid()) {
		gradient->connect(CoreStringNames::get_singleton()->changed, this, "_gradient_changed");
	}

	update();
}

Ref<Gradient> REDShapeRenderer::get_gradient() const {
	return gradient;
}

void REDShapeRenderer::set_texture(const Ref<Texture> &p_texture) {
	texture = p_texture;
	update();
}

Ref<Texture> REDShapeRenderer::get_texture() const {
	return texture;
}

void REDShapeRenderer::set_antialiased(bool p_antialiased) {
	antialiased = p_antialiased;
	update();
}

bool REDShapeRenderer::get_antialiased() const {
	return antialiased;
}

// line
void REDShapeRenderer::set_width(float p_width) {
	if (p_width < 0.0)
		p_width = 0.0;
	_width = p_width;
	update();
}

float REDShapeRenderer::get_width() const {
	return _width;
}

void REDShapeRenderer::set_width_zoom_const(float p_width_constant){
	width_zoom_const = p_width_constant;
	update();
}

float REDShapeRenderer::get_width_zoom_const() const {
	return width_zoom_const;
}

void REDShapeRenderer::set_curve(const Ref<Curve> &p_curve) {
	// Cleanup previous connection if any
	if (curve.is_valid()) {
		curve->disconnect(CoreStringNames::get_singleton()->changed, this, "_curve_changed");
	}

	curve = p_curve;

	// Connect to the curve so the line will update when it is changed
	if (curve.is_valid()) {
		curve->connect(CoreStringNames::get_singleton()->changed, this, "_curve_changed");
	}

	update();
}

Ref<Curve> REDShapeRenderer::get_curve() const {
	return curve;
}

void REDShapeRenderer::set_line_texture_mode(const Line2D::LineTextureMode p_mode) {
	line_texture_mode = p_mode;
	update();
}

Line2D::LineTextureMode REDShapeRenderer::get_line_texture_mode() const {
	return line_texture_mode;
}

void REDShapeRenderer::set_joint_mode(Line2D::LineJointMode p_mode) {
	_joint_mode = p_mode;
	update();
}

Line2D::LineJointMode REDShapeRenderer::get_joint_mode() const {
	return _joint_mode;
}

void REDShapeRenderer::set_begin_cap_mode(Line2D::LineCapMode p_mode) {
	_begin_cap_mode = p_mode;
	update();
}

Line2D::LineCapMode REDShapeRenderer::get_begin_cap_mode() const {
	return _begin_cap_mode;
}

void REDShapeRenderer::set_end_cap_mode(Line2D::LineCapMode p_mode) {
	_end_cap_mode = p_mode;
	update();
}

Line2D::LineCapMode REDShapeRenderer::get_end_cap_mode() const {
	return _end_cap_mode;
}

void REDShapeRenderer::set_sharp_limit(float p_limit) {
	if (p_limit < 0.f)
		p_limit = 0.f;
	_sharp_limit = p_limit;
	update();
}

float REDShapeRenderer::get_sharp_limit() const {
	return _sharp_limit;
}

void REDShapeRenderer::set_round_precision(int p_precision) {
	if (p_precision < 1)
		p_precision = 1;
	_round_precision = p_precision;
	update();
}

int REDShapeRenderer::get_round_precision() const {
	return _round_precision;
}

void REDShapeRenderer::_gradient_changed() {
	update();
}

void REDShapeRenderer::_curve_changed() {
	update();
}

void REDShapeRenderer::set_closed(const bool p_closed) {
    _closed = p_closed;
    update();
}

bool REDShapeRenderer::get_closed() const{
    return _closed;
}

void REDShapeRenderer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if(!is_inside_tree())
				return;
			ERR_FAIL_COND_MSG(!shape, "No parent REDShape found!");
			switch (render_mode)
			{
			case RenderMode::RENDER_POLYGON:
				_draw_polygon();
				break;
			case RenderMode::RENDER_LINE:
				_draw_outline();
				break;	
			default:
				break;
			}
		}
		case NOTIFICATION_ENTER_TREE: {
			Node *parent = get_parent();
			shape = NULL;
			for (int i = 0; i < 10; i++){
				shape = Object::cast_to<REDShape>(parent);
				if (shape){
					shape->render_nodes.push_back(this);
					break;
				}
				parent = get_parent();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (shape){
				int remove_index = shape->render_nodes.find(this);
				if(remove_index > 0)
					shape->render_nodes.remove(shape->render_nodes.find(this));
			}
			shape = NULL;
		} break;
	}
}
void REDShapeRenderer::_bind_methods() {

	// main
    ClassDB::bind_method(D_METHOD("set_render_mode", "render_mode"), &REDShapeRenderer::set_render_mode);
    ClassDB::bind_method(D_METHOD("get_render_mode"), &REDShapeRenderer::get_render_mode);
	ClassDB::bind_method(D_METHOD("set_color", "color"), &REDShapeRenderer::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &REDShapeRenderer::get_color);
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &REDShapeRenderer::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &REDShapeRenderer::get_texture);
	ClassDB::bind_method(D_METHOD("set_gradient", "color"), &REDShapeRenderer::set_gradient);
	ClassDB::bind_method(D_METHOD("get_gradient"), &REDShapeRenderer::get_gradient);
	ClassDB::bind_method(D_METHOD("set_antialiased", "antialiased"), &REDShapeRenderer::set_antialiased);
	ClassDB::bind_method(D_METHOD("get_antialiased"), &REDShapeRenderer::get_antialiased);
	// line
	ClassDB::bind_method(D_METHOD("set_width", "width"), &REDShapeRenderer::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &REDShapeRenderer::get_width);
	ClassDB::bind_method(D_METHOD("set_width_zoom_const", "width_zoom_const"), &REDShapeRenderer::set_width_zoom_const);
    ClassDB::bind_method(D_METHOD("get_width_zoom_const"), &REDShapeRenderer::get_width_zoom_const);
	ClassDB::bind_method(D_METHOD("set_curve", "curve"), &REDShapeRenderer::set_curve);
	ClassDB::bind_method(D_METHOD("get_curve"), &REDShapeRenderer::get_curve);
	ClassDB::bind_method(D_METHOD("set_line_texture_mode", "mode"), &REDShapeRenderer::set_line_texture_mode);
	ClassDB::bind_method(D_METHOD("get_line_texture_mode"), &REDShapeRenderer::get_line_texture_mode);
	ClassDB::bind_method(D_METHOD("set_joint_mode", "mode"), &REDShapeRenderer::set_joint_mode);
	ClassDB::bind_method(D_METHOD("get_joint_mode"), &REDShapeRenderer::get_joint_mode);
	ClassDB::bind_method(D_METHOD("set_begin_cap_mode", "mode"), &REDShapeRenderer::set_begin_cap_mode);
	ClassDB::bind_method(D_METHOD("get_begin_cap_mode"), &REDShapeRenderer::get_begin_cap_mode);
	ClassDB::bind_method(D_METHOD("set_end_cap_mode", "mode"), &REDShapeRenderer::set_end_cap_mode);
	ClassDB::bind_method(D_METHOD("get_end_cap_mode"), &REDShapeRenderer::get_end_cap_mode);
	ClassDB::bind_method(D_METHOD("set_sharp_limit", "limit"), &REDShapeRenderer::set_sharp_limit);
	ClassDB::bind_method(D_METHOD("get_sharp_limit"), &REDShapeRenderer::get_sharp_limit);
	ClassDB::bind_method(D_METHOD("set_round_precision", "precision"), &REDShapeRenderer::set_round_precision);
	ClassDB::bind_method(D_METHOD("get_round_precision"), &REDShapeRenderer::get_round_precision);
    ClassDB::bind_method(D_METHOD("set_closed", "_closed"), &REDShapeRenderer::set_closed);
    ClassDB::bind_method(D_METHOD("get_closed"), &REDShapeRenderer::get_closed);
	// main
	ADD_GROUP("Renderer", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "render_mode", PROPERTY_HINT_ENUM, "Polygon, Line"), "set_render_mode", "get_render_mode");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "antialiased"), "set_antialiased", "get_antialiased");
	ADD_GROUP("Line", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
    ADD_PROPERTY(PropertyInfo(Variant::REAL, "width_zoom_const"), "set_width_zoom_const", "get_width_zoom_const");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "width_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_curve", "get_curve");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "line_texture_mode", PROPERTY_HINT_ENUM, "None,Tile,Stretch"), "set_line_texture_mode", "get_line_texture_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "joint_mode", PROPERTY_HINT_ENUM, "Sharp,Bevel,Round"), "set_joint_mode", "get_joint_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "begin_cap_mode", PROPERTY_HINT_ENUM, "None,Box,Round"), "set_begin_cap_mode", "get_begin_cap_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "end_cap_mode", PROPERTY_HINT_ENUM, "None,Box,Round"), "set_end_cap_mode", "get_end_cap_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharp_limit"), "set_sharp_limit", "get_sharp_limit");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "round_precision"), "set_round_precision", "get_round_precision");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "closed"), "set_closed", "get_closed");
	ClassDB::bind_method(D_METHOD("_gradient_changed"), &REDShapeRenderer::_gradient_changed);
	ClassDB::bind_method(D_METHOD("_curve_changed"), &REDShapeRenderer::_curve_changed);
}

REDShapeRenderer::REDShapeRenderer(){	
	// main
	shape = NULL;
	render_mode = RENDER_POLYGON;
	color = Color(1.f, 1.f, 1.f, 1.f);
	antialiased = false;
	// line
	_width = 5;
	width_zoom_const = 1.0f;
	_joint_mode = Line2D::LINE_JOINT_SHARP;
	_begin_cap_mode = Line2D::LINE_CAP_NONE;
	_end_cap_mode = Line2D::LINE_CAP_NONE;
	line_texture_mode = Line2D::LINE_TEXTURE_STRETCH;
	_sharp_limit = 1000.f;
	_round_precision = 8;
    _closed=true;
}