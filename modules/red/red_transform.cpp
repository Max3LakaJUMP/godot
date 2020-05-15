/*************************************************************************/
/*  node_2d.cpp                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "red_transform.h"

#include "core/message_queue.h"
#include "scene/gui/control.h"
#include "scene/main/viewport.h"
#include "servers/visual_server.h"

Transform REDTransform::get_custom_global_transform() const{
	ERR_FAIL_COND_V(!is_inside_tree(), get_custom_transform());
	if (global_dirty) {
		
		const REDTransform *pi = Object::cast_to<REDTransform>(get_parent());
		if (pi)
			mat_global = pi->get_custom_global_transform() * get_custom_transform();
		else
			mat_global = get_custom_transform();

		global_dirty = false;
	}

	return mat_global;
}

void REDTransform::_update_xform_values() {
	_pos = _mat.basis.elements[2];
	_rotation = _mat.get_basis().get_rotation();
	_scale = _mat.get_basis().get_scale();
	_xform_dirty = false;
}

void REDTransform::_update_custom_transform() {
	_mat.basis.set_euler_scale(_rotation, _scale);
	_mat.set_origin(_pos);
	global_dirty = true;
	VS::get_singleton()->custom_transform_set(ci, _mat);

	if (!is_inside_tree())
		return;

	////_notify_transform();
}

void REDTransform::set_custom_position(const Vector3 &p_pos) {
	if (_xform_dirty)
		((REDTransform *)this)->_update_xform_values();
	_pos = p_pos;
	_update_custom_transform();
}

void REDTransform::set_custom_rotation(const Vector3 &p_radians) {
	if (_xform_dirty)
		((REDTransform *)this)->_update_xform_values();
	_rotation = p_radians;
	_update_custom_transform();
}

void REDTransform::set_custom_rotation_degrees(const Vector3 &p_degrees) {

	set_custom_rotation(Vector3(Math::deg2rad(p_degrees.x), Math::deg2rad(p_degrees.y), Math::deg2rad(p_degrees.z)));
}

void REDTransform::set_custom_scale(const Vector3 &p_scale) {

	if (_xform_dirty)
		((REDTransform *)this)->_update_xform_values();
	_scale = p_scale;
	if (_scale.x == 0)
		_scale.x = CMP_EPSILON;
	if (_scale.y == 0)
		_scale.y = CMP_EPSILON;
	_update_custom_transform();
}

Vector3 REDTransform::get_custom_position() const {

	if (_xform_dirty)
		((REDTransform *)this)->_update_xform_values();
	return _pos;
}

Vector3 REDTransform::get_custom_rotation() const {
	if (_xform_dirty)
		((REDTransform *)this)->_update_xform_values();

	return _rotation;
}

Vector3 REDTransform::get_custom_rotation_degrees() const {
	Vector3 r = get_custom_rotation();
	return Vector3(Math::rad2deg(r.x), Math::rad2deg(r.y), Math::rad2deg(r.z));
}

Vector3 REDTransform::get_custom_scale() const {
	if (_xform_dirty)
		((REDTransform *)this)->_update_xform_values();

	return _scale;
}

Transform REDTransform::get_custom_transform() const {

	return _mat;
}

void REDTransform::set_custom_transform(const Transform &p_transform) {

	_mat = p_transform;
	_xform_dirty = true;

	VisualServer::get_singleton()->custom_transform_set(ci, _mat);

	if (!is_inside_tree())
		return;

	//_notify_transform();
}

RID REDTransform::get_ci(){
	return ci;
}

void REDTransform::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_custom_position", "position"), &REDTransform::set_custom_position);
	ClassDB::bind_method(D_METHOD("set_custom_rotation", "radians"), &REDTransform::set_custom_rotation);
	ClassDB::bind_method(D_METHOD("set_custom_rotation_degrees", "degrees"), &REDTransform::set_custom_rotation_degrees);
	ClassDB::bind_method(D_METHOD("set_custom_scale", "scale"), &REDTransform::set_custom_scale);

	ClassDB::bind_method(D_METHOD("get_custom_position"), &REDTransform::get_custom_position);
	ClassDB::bind_method(D_METHOD("get_custom_rotation"), &REDTransform::get_custom_rotation);
	ClassDB::bind_method(D_METHOD("get_custom_rotation_degrees"), &REDTransform::get_custom_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_custom_scale"), &REDTransform::get_custom_scale);
	
	ClassDB::bind_method(D_METHOD("get_custom_transform"), &REDTransform::get_custom_transform);
	ClassDB::bind_method(D_METHOD("set_custom_transform", "xform"), &REDTransform::set_custom_transform);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "custom_position"), "set_custom_position", "get_custom_position");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_custom_rotation", "get_custom_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_custom_rotation_degrees", "get_custom_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "scale"), "set_custom_scale", "get_custom_scale");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "transform", PROPERTY_HINT_NONE, "", 0), "set_custom_transform", "get_custom_transform");
}

REDTransform::REDTransform() {
	_pos = Vector3(0, 0, 0);
	_rotation = Vector3(0, 0, 0);
	_scale = Vector3(1, 1, 1);
	_xform_dirty = false;
	global_dirty = true;
	ci = VS::get_singleton()->custom_transform_create();
}
REDTransform::~REDTransform() {

	VS::get_singleton()->free(ci);
}