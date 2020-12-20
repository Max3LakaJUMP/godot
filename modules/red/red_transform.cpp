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
#include "scene/2d/skeleton_2d.h"
#include "core/message_queue.h"
#include "scene/gui/control.h"
#include "scene/main/viewport.h"
#include "servers/visual_server.h"

void REDTransform::_make_root_dirty(bool update_child) {
	if (!is_inside_tree() || !root_node)
		return;
	get_global_transform();
	Transform root_node_transform3d = Variant(root_node->get_global_transform());
	custom_transform_set_global(root_node_transform3d);
	if (update_child){
		for (int i = 0; i < get_child_count(); i++){
			REDTransform *pi = Object::cast_to<REDTransform>(get_child(i));
			if (pi)
				pi->_make_root_dirty(true);
			else
				break;
		}
	}
}

void REDTransform::_make_transform_dirty(bool update_child) {
	if (!is_inside_tree()){
		global_transform_dirty = true;
		return;
	}
	if (!global_transform_dirty){
		global_transform_dirty = true;
		//_update_custom_transform();
		call_deferred("_update_custom_transform", update_child);
	}
}

void REDTransform::_update_custom_transform(bool update_child){
	if (!global_transform_dirty)
		return;
	global_transform_dirty = false;
	Node *par = get_parent();
	REDTransform *parent_tr = Object::cast_to<REDTransform>(par);
	if (parent_tr)
		global_custom = parent_tr->get_global_custom_transform() * get_transform3d();
	else{
		Bone2D *parent_bone = Object::cast_to<Bone2D>(par);
		if (parent_bone){
			Skeleton2D *parent_skeleton = parent_bone->get_skeleton();
			if (parent_skeleton){
				Skeleton2D::Bone b = parent_skeleton->get_bone_struct(parent_bone->get_index_in_skeleton());
				Transform bone_transform = Variant(b.accum_transform);
				global_custom = bone_transform * get_transform3d();
			}
		}else{
			global_custom = get_transform3d();
		}
	}
	if(elasticity == 0.0f){
		Transform tr = global_custom * get_global_rest().affine_inverse();
		VisualServer::get_singleton()->custom_transform_set_old(ci, tr);
		VisualServer::get_singleton()->custom_transform_set(ci, tr);
	}
	else
		set_process_internal(true);
	
	if (update_child){
		for (int i = 0; i < get_child_count(); i++){
			REDTransform *pi = Object::cast_to<REDTransform>(get_child(i));
			if (pi)
				pi->_make_transform_dirty(true);
			else
				break;
		}
	}
}

void REDTransform::_make_rest_dirty(bool update_child) {
	global_rest_dirty = true;
	if (update_child){
		for (int i = 0; i < get_child_count(); i++){
			REDTransform *pi = Object::cast_to<REDTransform>(get_child(i));
			if (pi)
				pi->_make_rest_dirty();
			else
				break;
		}
	}
}

//custom_transform
Transform REDTransform::get_global_custom_transform() const{
	return global_custom;
}

void REDTransform::set_custom_transform(const Transform &p_transform) {
	custom.set_transform(p_transform);
	_make_transform_dirty();
}

Transform REDTransform::get_custom_transform() const {
	return (Transform) custom;
}

void REDTransform::set_custom_position(const Vector3 &p_pos) {
	custom.origin = p_pos;
	_make_transform_dirty();
	set_process_internal(true);
}

Vector3 REDTransform::get_custom_position() const {
	return custom.origin;
}

void REDTransform::set_custom_rotation(const Vector3 &p_radians) {
	custom.set_rotation(p_radians);
	_make_transform_dirty();
	set_process_internal(true);
}

Vector3 REDTransform::get_custom_rotation() const {
	return custom.get_rotation();
}

void REDTransform::set_custom_rotation_degrees(const Vector3 &p_degrees) {
	set_custom_rotation(Vector3(Math::deg2rad(p_degrees.x), Math::deg2rad(p_degrees.y), Math::deg2rad(p_degrees.z)));
}

Vector3 REDTransform::get_custom_rotation_degrees() const {
	Vector3 r = get_custom_rotation();
	return Vector3(Math::rad2deg(r.x), Math::rad2deg(r.y), Math::rad2deg(r.z));
}

void REDTransform::set_custom_scale(const Vector3 &p_scale) {
	custom.set_scale(p_scale);
	_make_transform_dirty();
}

Vector3 REDTransform::get_custom_scale() const {
	return custom.get_scale();
}

// rest
Transform REDTransform::get_global_rest() const {
	if (global_rest_dirty){
		global_rest_dirty = false;
		Node *par = get_parent();
		REDTransform *parent_tr = Object::cast_to<REDTransform>(par);
		if (parent_tr) {
			global_rest = parent_tr->get_global_rest() * rest;
		} else {
			Bone2D *parent_bone = Object::cast_to<Bone2D>(par);
			if (parent_bone){
				Transform bone_rest = Variant(parent_bone->get_skeleton_rest());
				global_rest = bone_rest * rest;
			}
			else{
				global_rest = rest;
			}
		}
	}
	return global_rest;
}

void REDTransform::set_rest(const Transform &p_transform) {
	rest.set_transform(p_transform);
	_make_rest_dirty();
	_make_transform_dirty();
}

Transform REDTransform::get_rest() const{
	return (Transform) rest;
}

void REDTransform::set_rest_position(const Vector3 &p_pos) {
	rest.origin = p_pos;
	_make_rest_dirty();
	_make_transform_dirty();
}

Vector3 REDTransform::get_rest_position() const {
	return rest.origin;
}

void REDTransform::set_rest_rotation(const Vector3 &p_radians) {
	rest.set_rotation(p_radians);
	_make_rest_dirty();
	_make_transform_dirty();
}

Vector3 REDTransform::get_rest_rotation() const {
	return rest.get_rotation();
}

void REDTransform::set_rest_rotation_degrees(const Vector3 &p_degrees) {
	rest.set_rotation_degrees(p_degrees);
	_make_rest_dirty();
	_make_transform_dirty();
}

Vector3 REDTransform::get_rest_rotation_degrees() const {
	return rest.get_rotation_degrees();
}

void REDTransform::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			Node *parent = get_parent();
			Bone2D *is_bone;
			Skeleton2D *is_skeleton;
			REDTransform *is_transform;
			root_node = NULL;
			while (parent) {
				is_transform = Object::cast_to<REDTransform>(parent);
				if (is_transform){
					root_node = is_transform->get_root_node();
					if (root_node){
						break;
					}
				}else{
					is_bone = Object::cast_to<Bone2D>(parent);
					if (is_bone){
						root_node = (Node2D*)is_bone->get_skeleton();
						if (root_node)
							break;
					}else{
						is_skeleton = Object::cast_to<Skeleton2D>(parent);
						if (is_skeleton){
							root_node = is_skeleton;
							if (root_node)
								break;
						}
					}
					// no skeleton so update transform
					set_notify_transform(true);
				}
				root_node = Object::cast_to<Node2D>(parent);
				if(root_node)
					break;
				parent = parent->get_parent();
			}
			_update_custom_transform();
			Transform global_transform = Variant(root_node->get_global_transform());
			global_custom_old = global_transform * get_global_custom_transform();
			global_custom_velocity = Transform();
			global_custom_euler_accum = 0.0;
			Skeleton2D *skeleton = Object::cast_to<Skeleton2D>(root_node);
			if (skeleton) {
				skeleton->red_transforms.push_back(this);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			Skeleton2D *skeleton = Object::cast_to<Skeleton2D>(root_node);
			if (skeleton) {
				for (int i = 0; i < skeleton->bones.size(); i++) {
					if (skeleton->red_transforms[i] == this) {
						skeleton->red_transforms.remove(i);
						break;
					}
				}
			}
			set_notify_transform(false);
			if (root_node) {
				root_node = NULL;
			}
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			VisualServer::get_singleton()->custom_transform_set(ci, global_custom * get_global_rest().affine_inverse());
			_update_old_custom_transform();
		} break;
		case NOTIFICATION_TRANSFORM_CHANGED:{
			_make_root_dirty();
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			_make_transform_dirty();
		} break;
	}
}

void REDTransform::_update_old_custom_transform(){
	Transform global_transform = Variant(root_node->get_global_transform());
	Transform target = global_transform * get_global_custom_transform();
	if(target.is_equal_approx(global_custom_old)){
		global_custom_old = target;
		global_custom_velocity = Transform();
		global_custom_euler_accum = 0.0;
		set_process_internal(false);
	}
	else{
		// Bounce
		float delta = CLAMP(get_process_delta_time() * 4.0, 0.001, 0.2);
		Transform target_velocity = target * global_custom_old.affine_inverse();
		target_velocity.origin = 2.0 * target_velocity.origin;
		Vector3 euler = target_velocity.basis.get_euler();
		global_custom_euler_accum += delta * euler.length();
		euler.x = MIN(ABS(2.0f * euler.x), 0.7853) * SGN(euler.x);
		euler.y = MIN(ABS(2.0f * euler.y), 0.7853) * SGN(euler.y);
		euler.z = MIN(ABS(2.0f * euler.z), 0.7853) * SGN(euler.z);
		target_velocity.basis.set_euler(2.0f * euler);
		global_custom_velocity = global_custom_velocity.interpolate_with(target_velocity, delta);
		global_custom_old = global_custom_old.interpolate_with(global_custom_velocity * target, delta);
		// max offset
		Vector3 origin_diff = (global_custom_old.origin - target.origin);
		global_custom_old.origin = origin_diff * MIN(elasticity / origin_diff.length(), 1.0) + target.origin;
		//float elasticity_angle = 15.0;
		//Vector3 euler_diff = (global_custom_old * target.affine_inverse()).basis.get_euler();
		//global_custom_old.basis.set_euler(euler_diff * MIN(elasticity_angle * 3.14 / 180.0 / euler_diff.length(), 1.0) + target.basis.get_euler());
		// update origin faster when rotating
		global_custom_old.origin = global_custom_old.origin.linear_interpolate(target.origin, MIN(global_custom_euler_accum * 180 / 3.14 * delta * 0.1, 1.0f));
		// print_line(Variant(global_custom_euler_accum * 180 / 3.14 * delta * 0.1));
		global_custom_euler_accum = global_custom_euler_accum - delta * global_custom_euler_accum;
	}
	VisualServer::get_singleton()->custom_transform_set_old(ci, global_transform.affine_inverse() * global_custom_old * get_global_rest().affine_inverse());
}

void REDTransform::set_depth_position(float p_depth){
	if (depth_position == p_depth){
		return;
	}
	depth_position = p_depth;
	_make_transform_dirty();
}

float REDTransform::get_depth_position() const{
	return depth_position;
}

void REDTransform::set_transform3d(const Transform &p_transform){
	Transform temp(p_transform);
	set_position(Vector2(temp.origin.x, temp.origin.y));
	set_depth_position(temp.origin.z);
	temp.origin = Vector3();
	set_custom_transform(temp);
}

Transform REDTransform::get_transform3d() const{
	Transform c = Variant(get_transform());
	c.origin.z = depth_position;
	return c * get_custom_transform();
}

void REDTransform::set_elasticity(float p_elasticity){
	elasticity = p_elasticity;
}

float REDTransform::get_elasticity() const{
	return elasticity;
}

Vector3 REDTransform::get_position3d() const{
	Vector2 pos2d = get_position();
	return Vector3(pos2d.x, pos2d.y, get_depth_position());
}

void REDTransform::custom_transform_set_global(const Transform &p_transform){
	VisualServer::get_singleton()->custom_transform_set_global(ci, p_transform);
	if(elasticity != 0.0f)
		set_process_internal(true);
}

void REDTransform::set_old_custom_transform(const Transform &p_transform){
	global_custom_old = p_transform;
	if(elasticity == 0.0f)
		VisualServer::get_singleton()->custom_transform_set_old(ci, global_custom * get_global_rest().affine_inverse());
	else
		set_process_internal(true);
}

float REDTransform::get_global_depth_position() const{
	if (!is_inside_tree())
		return depth_position;
	REDTransform *pi = Object::cast_to<REDTransform>(get_parent());
	if (pi)
		return pi->get_global_depth_position() + depth_position;
	return depth_position;
}

Node2D *REDTransform::get_root_node() const{
	return root_node;
}

RID REDTransform::get_ci(){
	return ci;
}

void REDTransform::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_make_transform_dirty", "update_child"), &REDTransform::_make_transform_dirty);
	ClassDB::bind_method(D_METHOD("_update_custom_transform", "update_child"), &REDTransform::_update_custom_transform);
	ClassDB::bind_method(D_METHOD("get_global_depth_position"), &REDTransform::get_global_depth_position);
	
	ClassDB::bind_method(D_METHOD("set_depth_position", "depth"), &REDTransform::set_depth_position);
	ClassDB::bind_method(D_METHOD("get_depth_position"), &REDTransform::get_depth_position);
	ClassDB::bind_method(D_METHOD("set_transform3d", "p_transform"), &REDTransform::set_transform3d);
	ClassDB::bind_method(D_METHOD("get_transform3d"), &REDTransform::get_transform3d);
	ClassDB::bind_method(D_METHOD("set_elasticity", "elasticity"), &REDTransform::set_elasticity);
	ClassDB::bind_method(D_METHOD("get_elasticity"), &REDTransform::get_elasticity);
	ClassDB::bind_method(D_METHOD("set_old_custom_transform"), &REDTransform::set_old_custom_transform);
	ClassDB::bind_method(D_METHOD("get_global_custom_transform"), &REDTransform::get_global_custom_transform);

	ClassDB::bind_method(D_METHOD("get_global_rest"), &REDTransform::get_global_rest);
	ClassDB::bind_method(D_METHOD("set_rest", "p_rest"), &REDTransform::set_rest);
	ClassDB::bind_method(D_METHOD("get_rest"), &REDTransform::get_rest);
	ClassDB::bind_method(D_METHOD("set_rest_position", "position"), &REDTransform::set_rest_position);
	ClassDB::bind_method(D_METHOD("get_rest_position"), &REDTransform::get_rest_position);
	ClassDB::bind_method(D_METHOD("set_rest_rotation", "p_rotation"), &REDTransform::set_rest_rotation);
	ClassDB::bind_method(D_METHOD("get_rest_rotation"), &REDTransform::get_rest_rotation);
	ClassDB::bind_method(D_METHOD("set_rest_rotation_degrees", "degrees"), &REDTransform::set_rest_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_rest_rotation_degrees"), &REDTransform::get_rest_rotation_degrees);

	ClassDB::bind_method(D_METHOD("set_custom_transform", "transform"), &REDTransform::set_custom_transform);
	ClassDB::bind_method(D_METHOD("get_custom_transform"), &REDTransform::get_custom_transform);
	ClassDB::bind_method(D_METHOD("set_custom_position", "position"), &REDTransform::set_custom_position);
	ClassDB::bind_method(D_METHOD("get_custom_position"), &REDTransform::get_custom_position);
	ClassDB::bind_method(D_METHOD("set_custom_rotation", "radians"), &REDTransform::set_custom_rotation);
	ClassDB::bind_method(D_METHOD("get_custom_rotation"), &REDTransform::get_custom_rotation);
	ClassDB::bind_method(D_METHOD("set_custom_rotation_degrees", "degrees"), &REDTransform::set_custom_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_custom_rotation_degrees"), &REDTransform::get_custom_rotation_degrees);
	ClassDB::bind_method(D_METHOD("set_custom_scale", "scale"), &REDTransform::set_custom_scale);
	ClassDB::bind_method(D_METHOD("get_custom_scale"), &REDTransform::get_custom_scale);
	
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "custom_transform", PROPERTY_HINT_NONE, "", 0), "set_custom_transform", "get_custom_transform");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "depth_position"), "set_depth_position", "get_depth_position");
	// ADD_GROUP("3D", "custom_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "elasticity"), "set_elasticity", "get_elasticity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "custom_position"), "set_custom_position", "get_custom_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "custom_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_custom_rotation", "get_custom_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "custom_rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_custom_rotation_degrees", "get_custom_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "custom_scale"), "set_custom_scale", "get_custom_scale");
	ADD_GROUP("Rest", "rest_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rest_position"), "set_rest_position", "get_rest_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rest_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_rest_rotation", "get_rest_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rest_rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_rest_rotation_degrees", "get_rest_rotation_degrees");
}

REDTransform::REDTransform() {
	global_rest_dirty = true;
	global_transform_dirty = true;

	global_custom_euler_accum = 0.0f;
	depth_position = 0.0f;
	root_node = NULL;
	elasticity = 0.0f;
	ci = VS::get_singleton()->custom_transform_create();
	set_notify_local_transform(true);
}
REDTransform::~REDTransform() {

	VS::get_singleton()->free(ci);
}