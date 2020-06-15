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

void REDTransform::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_notify_transform(true);
			VisualServer::get_singleton()->custom_transform_set(ci, get_custom_global_transform());
		} break;
		case NOTIFICATION_TRANSFORM_CHANGED: {
			global_dirty = true;
			VisualServer::get_singleton()->custom_transform_set(ci, get_custom_global_transform());
		} break;
	}
}
// void REDTransform::_update_custom_global_transform() const{
// 	ERR_FAIL_COND_V(!is_inside_tree(), get_custom_transform());
// 	REDTransform *pi = Object::cast_to<REDTransform>(get_parent());
// 	Transform translator = Transform();
// 	translator.set_origin(Vector3(get_position().x, get_position().y, 0.0f));
// 	Transform translator_global = Transform();
// 	translator_global.set_origin(Vector3(get_global_position().x - get_position().x, get_global_position().y - get_position().y, 0.0f));
// 	if (pi){
// 		Transform pi_translator = Transform();
// 		pi_translator.set_origin(Vector3(-pi->get_global_position().x + get_position().x, -pi->get_global_position().y + get_position().y, 0.0f));
// 		mat_global = pi_translator.affine_inverse() * pi->get_custom_global_transform() * pi->get_custom_transform() * pi_translator;
// 	}
// 	else
// 	{
// 		mat_global = Transform();
// 	}
// 	VisualServer::get_singleton()->custom_transform_set_global(ci, mat_global, translator, translator_global);
// 	for (int i = 0; i < get_child_count(); i++)
// 	{
// 		Node *child = get_child(i);
// 		REDTransform *pi = Object::cast_to<REDTransform>(child);
// 		if (pi)
// 			pi->_update_custom_global_transform();
// 	}
// 	global_dirty = false;

// }
// void REDTransform::_update_custom_global_transform() {
// 	//ERR_FAIL_COND_V(!is_inside_tree(), get_custom_transform());
// 	REDTransform *pi = Object::cast_to<REDTransform>(get_parent());
// 	//Transform translator = Transform();
// 	//translator.set_origin(Vector3(get_position().x, get_position().y, 0.0f));
// 	//Transform translator_global = Transform();
// 	//translator_global.set_origin(Vector3(get_global_position().x, get_global_position().y, get_global_depth_position()));
// 	if (pi){
// 		//Transform pi_translator = Transform();
// 		//pi_translator.set_origin(Vector3(-pi->get_global_position().x, -pi->get_global_position().y, -pi->get_global_depth_position()));
// 		mat_global = pi->get_custom_global_transform() * get_custom_transform();
// 		//mat_global = pi_translator.affine_inverse() * pi->get_custom_global_transform() * pi->get_custom_transform() * pi_translator;
// 	}
// 	else
// 	{
// 		mat_global = get_custom_transform();
// 	}
// 	VisualServer::get_singleton()->custom_transform_set(ci, mat_global);
// 	for (int i = 0; i < get_child_count(); i++)
// 	{
// 		Node *child = get_child(i);
// 		REDTransform *pi = Object::cast_to<REDTransform>(child);
// 		if (pi)
// 			pi->_update_custom_transform();
// 	}
// 	global_dirty = false;
// }

float REDTransform::get_global_depth_position() const{
	REDTransform *pi = Object::cast_to<REDTransform>(get_parent());
	if (pi){
		return pi->get_global_depth_position() + depth_position;
	}
	return depth_position;
}


float REDTransform::get_depth_position() const{
	return depth_position;
}

void REDTransform::set_depth_position(float p_depth){
	if (depth_position == p_depth){
		return;
	}
	depth_position = p_depth;
	_update_custom_transform(true, false);
}

Transform REDTransform::get_custom_global_transform() const{
	ERR_FAIL_COND_V(!is_inside_tree(), get_custom_transform());
	if (global_dirty) {
		Transform translator_global = Transform();
		translator_global.set_origin(Vector3(get_global_position().x, get_global_position().y, get_global_depth_position()));
		REDTransform *pi = Object::cast_to<REDTransform>(get_parent());
		if (pi)
			mat_global = pi->get_custom_global_transform() * translator_global * _mat * translator_global.affine_inverse();
		else
			mat_global = translator_global * _mat * translator_global.affine_inverse();
	}
	return mat_global;
}

void REDTransform::_update_custom_xform_values() {
	_custom_pos = _mat.basis.elements[2];
	_custom_rotation = _mat.get_basis().get_rotation();
	_custom_scale = _mat.get_basis().get_scale();
	_custom_xform_dirty = false;
}

void REDTransform::_update_custom_transform(bool update_child, bool update_matrix) {
	if (update_matrix){
		_mat.basis.set_euler_scale(_custom_rotation, _custom_scale);
		_mat.set_origin(_custom_pos);
	}
	if (!is_inside_tree())
		return;
	global_dirty = true;
	VisualServer::get_singleton()->custom_transform_set(ci, get_custom_global_transform());
	if (update_child){
		for (int i = 0; i < get_child_count(); i++)
		{
			Node *child = get_child(i);
			REDTransform *pi = Object::cast_to<REDTransform>(child);
			if (pi)
				pi->_update_custom_transform(true, false);
		}
		global_dirty = false;
	}
}

void REDTransform::set_custom_position(const Vector3 &p_pos) {
	if (_custom_xform_dirty)
		((REDTransform *)this)->_update_custom_xform_values();
	_custom_pos = p_pos;
	_update_custom_transform(true, true);
}

void REDTransform::set_custom_rotation(const Vector3 &p_radians) {
	if (_custom_xform_dirty)
		((REDTransform *)this)->_update_custom_xform_values();
	_custom_rotation = p_radians;
	_update_custom_transform(true, true);
}

void REDTransform::set_custom_rotation_degrees(const Vector3 &p_degrees) {

	set_custom_rotation(Vector3(Math::deg2rad(p_degrees.x), Math::deg2rad(p_degrees.y), Math::deg2rad(p_degrees.z)));
}

void REDTransform::set_custom_scale(const Vector3 &p_scale) {

	if (_custom_xform_dirty)
		((REDTransform *)this)->_update_custom_xform_values();
	_custom_scale = p_scale;
	if (_custom_scale.x == 0)
		_custom_scale.x = CMP_EPSILON;
	if (_custom_scale.y == 0)
		_custom_scale.y = CMP_EPSILON;
	_update_custom_transform(true, true);
}

Vector3 REDTransform::get_custom_position() const {

	if (_custom_xform_dirty)
		((REDTransform *)this)->_update_custom_xform_values();
	return _custom_pos;
}

Vector3 REDTransform::get_custom_rotation() const {
	if (_custom_xform_dirty)
		((REDTransform *)this)->_update_custom_xform_values();

	return _custom_rotation;
}

Vector3 REDTransform::get_custom_rotation_degrees() const {
	Vector3 r = get_custom_rotation();
	return Vector3(Math::rad2deg(r.x), Math::rad2deg(r.y), Math::rad2deg(r.z));
}

Vector3 REDTransform::get_custom_scale() const {
	if (_custom_xform_dirty)
		((REDTransform *)this)->_update_custom_xform_values();

	return _custom_scale;
}

Transform REDTransform::get_custom_transform() const {

	return _mat;
}

void REDTransform::set_custom_transform(const Transform &p_transform) {

	_mat = p_transform;
	_update_custom_xform_values();
	_update_custom_transform(true, false);
	// if (!is_inside_tree())
	// 	return;

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
	
	ClassDB::bind_method(D_METHOD("get_depth_position"), &REDTransform::get_depth_position);
	ClassDB::bind_method(D_METHOD("set_depth_position", "xform"), &REDTransform::set_depth_position);
	ClassDB::bind_method(D_METHOD("get_custom_transform"), &REDTransform::get_custom_transform);
	ClassDB::bind_method(D_METHOD("set_custom_transform", "xform"), &REDTransform::set_custom_transform);
	
	ClassDB::bind_method(D_METHOD("get_custom_global_transform"), &REDTransform::get_custom_global_transform);


	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "custom_position"), "set_custom_position", "get_custom_position");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "custom_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_custom_rotation", "get_custom_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "custom_rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_custom_rotation_degrees", "get_custom_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "custom_scale"), "set_custom_scale", "get_custom_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "depth_position"), "set_depth_position", "get_depth_position");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "transform", PROPERTY_HINT_NONE, "", 0), "set_custom_transform", "get_custom_transform");
}

REDTransform::REDTransform() {
	depth_position = 0.0f;
	_custom_pos = Vector3(0, 0, 0);
	_custom_rotation = Vector3(0, 0, 0);
	_custom_scale = Vector3(1, 1, 1);
	_custom_xform_dirty = false;
	global_dirty = true;
	ci = VS::get_singleton()->custom_transform_create();
}
REDTransform::~REDTransform() {

	VS::get_singleton()->free(ci);
}