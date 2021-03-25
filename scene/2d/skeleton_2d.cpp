/*************************************************************************/
/*  skeleton_2d.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "skeleton_2d.h"
#include "modules/red/root_bone_2d.h"

#ifdef SKELETON_3D
void Bone2D::spatial_to_transform(){
	spatial_lock = true;
	Vector3 new_rotation = spatial.get_rotation();
	if(get_rotation() != new_rotation.z)
		set_rotation(new_rotation.z);
	Vector2 new_pos(Vector2(spatial.origin.x, spatial.origin.y));
	if(get_position() != new_pos){
		set_position(new_pos);
	}
	Vector2 new_scale(spatial.get_scale().x, spatial.get_scale().y);
	if(get_scale() != new_scale){
		set_scale(new_scale);
	}
	spatial_lock = false;
}

void Bone2D::transform_to_spatial(){
	Vector3 old_rotation = spatial.get_rotation();
	if(old_rotation.z != get_rotation()){
		spatial.set_rotation(Vector3(old_rotation.x, old_rotation.y, get_rotation()));
		_change_notify("spatial_rotation");
		_change_notify("spatial_rotation_degrees");
	}
	Vector2 new_pos(get_position());
	if(Vector2(spatial.origin.x, spatial.origin.y) != new_pos){
		spatial.origin.x = new_pos.x;
		spatial.origin.y = new_pos.y;
		_change_notify("spatial_position");
	}
	Vector3 old_scale(spatial.get_scale());
	Vector2 new_scale(get_scale());
	if(Vector2(old_scale.x, old_scale.y) != new_scale){
		spatial.set_scale(Vector3(new_scale.x, new_scale.y, old_scale.z));
		_change_notify("spatial_scale");
	}
}

void Bone2D::set_spatial_transform(const Transform &p_transform) {
	spatial.set_transform(p_transform);
	spatial_to_transform();
	_change_notify("spatial_position");
	_change_notify("spatial_rotation");
	_change_notify("spatial_rotation_degrees");
	_change_notify("spatial_scale");
	if (skeleton) {
		skeleton->_make_transform_dirty();
	}else{
		_make_transform_dirty();
	}
}

Transform Bone2D::get_spatial_transform() const {
	return (Transform) spatial;
}

void Bone2D::set_preferred_rotation(const Vector3 &p_radians) {
	preferred_rotation = p_radians;
	_change_notify("preferred_rotation_degrees");
}

Vector3 Bone2D::get_preferred_rotation() const{
	return preferred_rotation;
}

void Bone2D::set_preferred_rotation_degrees(const Vector3 &p_degrees) {
	set_preferred_rotation(Vector3(Math::deg2rad(p_degrees.x), Math::deg2rad(p_degrees.y), Math::deg2rad(p_degrees.z)));
}

Vector3 Bone2D::get_preferred_rotation_degrees() const{
	Vector3 r = get_preferred_rotation();
	return Vector3(Math::rad2deg(r.x), Math::rad2deg(r.y), Math::rad2deg(r.z));
}

// spatial
void Bone2D::set_spatial_position(const Vector3 &p_pos) {
	spatial.origin = p_pos;
	set_position(Point2(p_pos.x, p_pos.y));
	if (skeleton) {
		skeleton->_make_transform_dirty();
	}else{
		_make_transform_dirty();
	}
}

Vector3 Bone2D::get_spatial_position() const {
	return spatial.origin;
}

void Bone2D::set_spatial_rotation(const Vector3 &p_radians) {
	spatial.set_rotation(p_radians);
	set_rotation(p_radians.z);
	_change_notify("spatial_rotation_degrees");
	if (skeleton) {
		skeleton->_make_transform_dirty();
	}else{
		_make_transform_dirty();
	}
}

Vector3 Bone2D::get_spatial_rotation() const {
	return spatial.get_rotation();
}

void Bone2D::set_spatial_rotation_degrees(const Vector3 &p_degrees) {
	set_spatial_rotation(Vector3(Math::deg2rad(p_degrees.x), Math::deg2rad(p_degrees.y), Math::deg2rad(p_degrees.z)));
}

Vector3 Bone2D::get_spatial_rotation_degrees() const {
	Vector3 r = get_spatial_rotation();
	return Vector3(Math::rad2deg(r.x), Math::rad2deg(r.y), Math::rad2deg(r.z));
}

void Bone2D::set_spatial_scale(const Vector3 &p_scale) {
	spatial.set_scale(p_scale);
	set_scale(Vector2(p_scale.x, p_scale.y));
	if (skeleton) {
		skeleton->_make_transform_dirty();
	}else{
		_make_transform_dirty();
	}
}

Vector3 Bone2D::get_spatial_scale() const {
	return spatial.get_scale();
}

// rest
void Bone2D::set_rest_position(const Vector3 &p_pos) {
	rest.origin = p_pos;
	if (skeleton)
		skeleton->_make_bone_setup_dirty();
	else{
		_make_rest_dirty();
		_make_transform_dirty();
	}
	update_configuration_warning();
}

Vector3 Bone2D::get_rest_position() const {
	return rest.origin;
}

void Bone2D::set_rest_rotation(const Vector3 &p_radians) {
	rest.set_rotation(p_radians);
	_change_notify("rest_rotation_degrees");
	if (skeleton)
		skeleton->_make_bone_setup_dirty();
	else{
		_make_rest_dirty();
		_make_transform_dirty();
	}
	update_configuration_warning();
}

Vector3 Bone2D::get_rest_rotation() const {
	return rest.get_rotation();
}

void Bone2D::set_rest_rotation_degrees(const Vector3 &p_degrees) {
	set_rest_rotation(Vector3(Math::deg2rad(p_degrees.x), Math::deg2rad(p_degrees.y), Math::deg2rad(p_degrees.z)));
}

Vector3 Bone2D::get_rest_rotation_degrees() const {
	return rest.get_rotation_degrees();
}

void Bone2D::set_rest_scale(const Vector3 &p_scale) {
	rest.set_scale(p_scale);
	if (skeleton)
		skeleton->_make_bone_setup_dirty();
	else{
		_make_rest_dirty();
		_make_transform_dirty();
	}
	update_configuration_warning();
}

Vector3 Bone2D::get_rest_scale() const {
	return rest.get_scale();
}
#endif

Skeleton2D *Bone2D::get_skeleton() const {
	return skeleton;
}

Skeleton2D::Bone Skeleton2D::get_bone_struct(int p_idx) {

	ERR_FAIL_COND_V(!is_inside_tree(), Skeleton2D::Bone());
	ERR_FAIL_INDEX_V(p_idx, bones.size(), Skeleton2D::Bone());

	return bones[p_idx];
}

void Bone2D::_notification(int p_what) {

	if (p_what == NOTIFICATION_ENTER_TREE) {
		Node *parent = get_parent();
		parent_bone = Object::cast_to<Bone2D>(parent);
		skeleton = NULL;
		while (parent) {
			skeleton = Object::cast_to<Skeleton2D>(parent);
			if (skeleton)
				break;
			if (!Object::cast_to<Bone2D>(parent))
				break; //skeletons must be chained to Bone2Ds.

			parent = parent->get_parent();
		}

		if (skeleton) {
			Skeleton2D::Bone bone;
			bone.bone = this;
			skeleton->bones.push_back(bone);
			skeleton->_make_bone_setup_dirty();
		}
	}
	if (p_what == NOTIFICATION_READY) {
		transform_to_spatial();
	}
	if (p_what == NOTIFICATION_LOCAL_TRANSFORM_CHANGED) {
		if(!spatial_lock)
			transform_to_spatial();
		if (skeleton) {
			skeleton->_make_transform_dirty();
		}
	}
	if (p_what == NOTIFICATION_MOVED_IN_PARENT) {
		if (skeleton) {
			skeleton->_make_bone_setup_dirty();
		}
	}

	if (p_what == NOTIFICATION_EXIT_TREE) {
		if (skeleton) {
			for (int i = 0; i < skeleton->bones.size(); i++) {
				if (skeleton->bones[i].bone == this) {
					skeleton->bones.remove(i);
					break;
				}
			}
			skeleton->_make_bone_setup_dirty();
			skeleton = NULL;
		}
		parent_bone = NULL;
	}
}

void Bone2D::_bind_methods() {
#ifdef SKELETON_3D
	ClassDB::bind_method(D_METHOD("set_preferred_rotation", "preferred_rotation"), &Bone2D::set_preferred_rotation);
	ClassDB::bind_method(D_METHOD("get_preferred_rotation"), &Bone2D::get_preferred_rotation);
	ClassDB::bind_method(D_METHOD("set_preferred_rotation_degrees", "preferred_rotation_degrees"), &Bone2D::set_preferred_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_preferred_rotation_degrees"), &Bone2D::get_preferred_rotation_degrees);
	ClassDB::bind_method(D_METHOD("set_spatial_transform", "transform"), &Bone2D::set_spatial_transform);
	ClassDB::bind_method(D_METHOD("get_spatial_transform"), &Bone2D::get_spatial_transform);

	ClassDB::bind_method(D_METHOD("set_rest_position", "position"), &Bone2D::set_rest_position);
	ClassDB::bind_method(D_METHOD("get_rest_position"), &Bone2D::get_rest_position);
	ClassDB::bind_method(D_METHOD("set_rest_rotation", "radians"), &Bone2D::set_rest_rotation);
	ClassDB::bind_method(D_METHOD("get_rest_rotation"), &Bone2D::get_rest_rotation);
	ClassDB::bind_method(D_METHOD("set_rest_rotation_degrees", "degrees"), &Bone2D::set_rest_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_rest_rotation_degrees"), &Bone2D::get_rest_rotation_degrees);
	ClassDB::bind_method(D_METHOD("set_rest_scale", "scale"), &Bone2D::set_rest_scale);
	ClassDB::bind_method(D_METHOD("get_rest_scale"), &Bone2D::get_rest_scale);

	ClassDB::bind_method(D_METHOD("set_spatial_position", "position"), &Bone2D::set_spatial_position);
	ClassDB::bind_method(D_METHOD("get_spatial_position"), &Bone2D::get_spatial_position);
	ClassDB::bind_method(D_METHOD("set_spatial_rotation", "radians"), &Bone2D::set_spatial_rotation);
	ClassDB::bind_method(D_METHOD("get_spatial_rotation"), &Bone2D::get_spatial_rotation);
	ClassDB::bind_method(D_METHOD("set_spatial_rotation_degrees", "degrees"), &Bone2D::set_spatial_rotation_degrees);
	ClassDB::bind_method(D_METHOD("get_spatial_rotation_degrees"), &Bone2D::get_spatial_rotation_degrees);
	ClassDB::bind_method(D_METHOD("set_spatial_scale", "scale"), &Bone2D::set_spatial_scale);
	ClassDB::bind_method(D_METHOD("get_spatial_scale"), &Bone2D::get_spatial_scale);
#endif

	ClassDB::bind_method(D_METHOD("set_rest", "rest"), &Bone2D::set_rest);
	ClassDB::bind_method(D_METHOD("get_rest"), &Bone2D::get_rest);
	ClassDB::bind_method(D_METHOD("apply_rest"), &Bone2D::apply_rest);
	ClassDB::bind_method(D_METHOD("get_skeleton_rest"), &Bone2D::get_skeleton_rest);
	ClassDB::bind_method(D_METHOD("get_index_in_skeleton"), &Bone2D::get_index_in_skeleton);

	ClassDB::bind_method(D_METHOD("set_default_length", "default_length"), &Bone2D::set_default_length);
	ClassDB::bind_method(D_METHOD("get_default_length"), &Bone2D::get_default_length);
	
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "default_length", PROPERTY_HINT_RANGE, "1,1024,1"), "set_default_length", "get_default_length");

#ifdef SKELETON_3D
	// ADD_GROUP("Rest Matrix", "");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "rest", PROPERTY_HINT_NONE, "", 0), "set_rest", "get_rest");
	ADD_GROUP("Spatial", "spatial_");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "spatial_transform", PROPERTY_HINT_NONE, "", 0), "set_spatial_transform", "get_spatial_transform");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "spatial_position"), "set_spatial_position", "get_spatial_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "spatial_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_spatial_rotation", "get_spatial_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "spatial_rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.001,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_spatial_rotation_degrees", "get_spatial_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "spatial_scale"), "set_spatial_scale", "get_spatial_scale");
	ADD_GROUP("Rest", "rest_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rest_position"), "set_rest_position", "get_rest_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rest_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_rest_rotation", "get_rest_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rest_rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.001,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_rest_rotation_degrees", "get_rest_rotation_degrees");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "rest_scale"), "set_rest_scale", "get_rest_scale");
	ADD_GROUP("Misc", "");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "preferred_rotation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_preferred_rotation", "get_preferred_rotation");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "preferred_rotation_degrees", PROPERTY_HINT_RANGE, "-360,360,0.001,or_lesser,or_greater", PROPERTY_USAGE_EDITOR), "set_preferred_rotation_degrees", "get_preferred_rotation_degrees");
#else
	ADD_GROUP("Rest Matrix", "");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM2D, "rest"), "set_rest", "get_rest");
#endif
}

#ifdef SKELETON_3D
void Bone2D::set_rest(const Transform &p_rest) {
	rest.set_transform(p_rest);
	_change_notify("rest_position");
	_change_notify("rest_rotation");
	_change_notify("rest_rotation_degrees");
	_change_notify("rest_scale");
	if (skeleton)
		skeleton->_make_bone_setup_dirty();
	else{
		_make_rest_dirty();
		_make_transform_dirty();
	}
	update_configuration_warning();
}

Transform Bone2D::get_rest() const {
	return rest;
}

Transform Bone2D::get_skeleton_rest() const {

	if (parent_bone) {
		if(parent_bone->object_bone && !object_bone){
			Transform rest_global = Object::cast_to<RootBone2D>(parent_bone)->get_global_rest();
			return rest_global * rest * rest_global.affine_inverse();
		}
		return parent_bone->get_skeleton_rest() * rest;
	} else {
		return rest;
	}
}
void Bone2D::apply_rest() {
	set_position(Vector2(rest.origin.x, rest.origin.y));
	set_spatial_transform(rest.basis);
}
#else
void Bone2D::set_rest(const Transform2D &p_rest) {
	rest = p_rest;
	if (skeleton)
		skeleton->_make_bone_setup_dirty();

	update_configuration_warning();
}

Transform2D Bone2D::get_rest() const {
	return rest;
}

Transform2D Bone2D::get_skeleton_rest() const {

	if (parent_bone) {
		if(parent_bone->object_bone && !object_bone){
			Transform2D rest_global = Variant(Object::cast_to<RootBone2D>(parent_bone)->get_global_rest());
			return rest_global * rest * rest_global.affine_inverse();
		}
		return parent_bone->get_skeleton_rest() * rest;
	} else {
		return rest;
	}
}

void Bone2D::apply_rest() {
	set_transform(rest);
}
#endif

void Bone2D::set_default_length(float p_length) {

	default_length = p_length;
}

float Bone2D::get_default_length() const {
	return default_length;
}

int Bone2D::get_index_in_skeleton() const {
	ERR_FAIL_COND_V(!skeleton, -1);
	skeleton->_update_bone_setup();
	return skeleton_index;
}
String Bone2D::get_configuration_warning() const {

	String warning = Node2D::get_configuration_warning();
	if (!skeleton && !object_bone) {
		if (warning != String()) {
			warning += "\n\n";
		}
		if (parent_bone) {
			warning += TTR("This Bone2D chain should end at a Skeleton2D node.");
		} else {
			warning += TTR("A Bone2D only works with a Skeleton2D or another Bone2D as parent node.");
		}
	}
#ifndef SKELETON_3D
	if (rest == Transform2D(0, 0, 0, 0, 0, 0) && !object_bone) {
		if (warning != String()) {
			warning += "\n\n";
		}
		warning += TTR("This bone lacks a proper REST pose. Go to the Skeleton2D node and set one.");
	}
#endif
	return warning;
}

Bone2D::Bone2D() {
	skeleton = NULL;
	parent_bone = NULL;
	skeleton_index = -1;
	default_length = 16;
	set_notify_local_transform(true);
	//this is a clever hack so the bone knows no rest has been set yet, allowing to show an error.
#ifdef SKELETON_3D
	preferred_rotation_degrees = Vector3(0, 0, 0);
	global_rest_dirty = true;
	global_transform_dirty = true;
	spatial_lock = false;
#else
	for (int i = 0; i < 3; i++) {
		rest[i] = Vector2(0, 0);
	}
#endif
	object_bone = false;
}

//////////////////////////////////////

void Skeleton2D::_make_bone_setup_dirty() {

	if (bone_setup_dirty)
		return;
	bone_setup_dirty = true;
	if (is_inside_tree()) {
		call_deferred("_update_bone_setup");
	}
}

void Skeleton2D::_update_bone_setup() {

	if (!bone_setup_dirty)
		return;

	bone_setup_dirty = false;
#ifdef SKELETON_3D
	VS::get_singleton()->skeleton_allocate(skeleton, bones.size(), mode_2d);
#else
	VS::get_singleton()->skeleton_allocate(skeleton, bones.size(), false);
#endif
	bones.sort(); //sorty so they are always in the same order/index

	for (int i = 0; i < bones.size(); i++) {
		if(bones[i].bone->object_bone){
			bones.write[i].bone->skeleton_index = i;
			Bone2D *parent_bone = Object::cast_to<Bone2D>(bones[i].bone->get_parent());
			if (parent_bone) {
				bones.write[i].parent_index = parent_bone->skeleton_index;
			} else {
				bones.write[i].parent_index = -1;
			}
			Object::cast_to<RootBone2D>(bones.write[i].bone)->_make_rest_dirty();
			continue;
		}
		bones.write[i].rest_inverse = bones[i].bone->get_skeleton_rest().affine_inverse(); //bind pose
		bones.write[i].bone->skeleton_index = i;
		Bone2D *parent_bone = Object::cast_to<Bone2D>(bones[i].bone->get_parent());
		if (parent_bone) {
			bones.write[i].parent_index = parent_bone->skeleton_index;
		} else {
			bones.write[i].parent_index = -1;
		}
	}

	transform_dirty = true;
	_update_transform();
	emit_signal("bone_setup_changed");
}

void Skeleton2D::_make_transform_dirty() {

	if (transform_dirty)
		return;
	transform_dirty = true;
	if (is_inside_tree()) {
		call_deferred("_update_transform");
	}
}

#ifdef SKELETON_3D
void Skeleton2D::_update_transform() {
	if(mode_2d){
		_update_transform_2d();
	} else{
		_update_transform_3d();
	}
}

void Skeleton2D::_update_transform_3d() {

	if (bone_setup_dirty) {
		_update_bone_setup();
		return; //above will update transform anyway
	}
	if (!transform_dirty)
		return;

	transform_dirty = false;
	for (int i = 0; i < bones.size(); i++) {
		if(bones[i].bone->object_bone){
			Object::cast_to<RootBone2D>(bones.write[i].bone)->_make_transform_dirty();
			continue;
		}
		ERR_CONTINUE(bones[i].parent_index >= i);
		if (bones[i].parent_index >= 0) {
			if(bones[bones[i].parent_index].bone->object_bone && !bones[i].bone->object_bone){
				Transform rest_global = Object::cast_to<RootBone2D>(bones[bones[i].parent_index].bone)->get_global_rest();
				bones.write[i].accum_transform = rest_global * bones[i].bone->get_spatial_transform() * rest_global.affine_inverse();
			}else{
				bones.write[i].accum_transform = bones[bones[i].parent_index].accum_transform * bones[i].bone->get_spatial_transform();
			}
		} else {
			bones.write[i].accum_transform = bones[i].bone->get_spatial_transform();
		}
	}
	for (int i = 0; i < bones.size(); i++) {
		if(bones[i].bone->object_bone){
			VS::get_singleton()->skeleton_bone_set_transform(skeleton, i, Transform());
			continue;
		}
		Transform final_xform = bones[i].accum_transform * bones[i].rest_inverse;
		VS::get_singleton()->skeleton_bone_set_transform(skeleton, i, final_xform);
	}
}

void Skeleton2D::_update_transform_2d() {
	if (bone_setup_dirty) {
		_update_bone_setup();
		return; //above will update transform anyway
	}
	if (!transform_dirty)
		return;

	transform_dirty = false;

	for (int i = 0; i < bones.size(); i++) {

		if(bones[i].bone->object_bone){
			Object::cast_to<RootBone2D>(bones.write[i].bone)->_make_transform_dirty();
			continue;
		}
		ERR_CONTINUE(bones[i].parent_index >= i);
		if (bones[i].parent_index >= 0) {
			if(bones[bones[i].parent_index].bone->object_bone && !bones[i].bone->object_bone){
				Transform2D rest_global = Variant(Object::cast_to<RootBone2D>(bones[bones[i].parent_index].bone)->get_global_rest());
				bones.write[i].accum_transform = Variant(rest_global * bones[i].bone->get_transform() * rest_global.affine_inverse());
			}else{
				bones.write[i].accum_transform = bones[bones[i].parent_index].accum_transform * Variant(bones[i].bone->get_transform());
			}
		} else {
			bones.write[i].accum_transform = Variant(bones[i].bone->get_transform());
		}
	}
	for (int i = 0; i < bones.size(); i++) {
		if(bones[i].bone->object_bone){
			VS::get_singleton()->skeleton_bone_set_transform_2d(skeleton, i, Transform2D());
			continue;
		}
		Transform2D final_xform = Transform2D(Variant(bones[i].accum_transform)) * Transform2D(Variant(bones[i].rest_inverse));
		VS::get_singleton()->skeleton_bone_set_transform_2d(skeleton, i, final_xform);
	}
}
#else
void Skeleton2D::_update_transform() {
	if (bone_setup_dirty) {
		_update_bone_setup();
		return; //above will update transform anyway
	}
	if (!transform_dirty)
		return;

	transform_dirty = false;

	for (int i = 0; i < bones.size(); i++) {

		if(bones[i].bone->object_bone){
			Object::cast_to<RootBone2D>(bones.write[i].bone)->_make_transform_dirty();
			continue;
		}
		ERR_CONTINUE(bones[i].parent_index >= i);
		if (bones[i].parent_index >= 0) {
			if(bones[bones[i].parent_index].bone->object_bone && !bones[i].bone->object_bone){
				Transform2D rest_global = Variant(Object::cast_to<RootBone2D>(bones[bones[i].parent_index].bone)->get_global_rest());
				bones.write[i].accum_transform = rest_global * bones[i].bone->get_transform() * rest_global.affine_inverse();
			}else{
				bones.write[i].accum_transform = bones[bones[i].parent_index].accum_transform * bones[i].bone->get_transform();
			}
		} else {
			bones.write[i].accum_transform = bones[i].bone->get_transform();
		}
	}
	for (int i = 0; i < bones.size(); i++) {
		if(bones[i].bone->object_bone){
			VS::get_singleton()->skeleton_bone_set_transform_2d(skeleton, i, Transform2D());
			continue;
		}
		Transform2D final_xform = bones[i].accum_transform * bones[i].rest_inverse;
		VS::get_singleton()->skeleton_bone_set_transform_2d(skeleton, i, final_xform);
	}
}
#endif

int Skeleton2D::get_bone_count() const {

	ERR_FAIL_COND_V(!is_inside_tree(), 0);

	if (bone_setup_dirty) {
		const_cast<Skeleton2D *>(this)->_update_bone_setup();
	}

	return bones.size();
}

Bone2D *Skeleton2D::get_bone(int p_idx) {

	ERR_FAIL_COND_V(!is_inside_tree(), NULL);
	ERR_FAIL_INDEX_V(p_idx, bones.size(), NULL);

	return bones[p_idx].bone;
}

#ifdef SKELETON_3D
void Skeleton2D::set_mode_2d(bool p_mode_2d) {

	if(mode_2d != p_mode_2d){
		mode_2d = p_mode_2d;
		_make_bone_setup_dirty();
	}

}

bool Skeleton2D::get_mode_2d() const {
	return mode_2d;
}
#endif

void Skeleton2D::_notification(int p_what) {

	if (p_what == NOTIFICATION_READY) {

		if (bone_setup_dirty)
			_update_bone_setup();
		if (transform_dirty)
			_update_transform();

		request_ready();
	}

	if (p_what == NOTIFICATION_TRANSFORM_CHANGED) {
		for (int i = 0; i < bones.size(); i++) {
			if(bones[i].bone->object_bone)
				Object::cast_to<RootBone2D>(bones.write[i].bone)->_make_root_dirty();
		}
		VS::get_singleton()->skeleton_set_base_transform_2d(skeleton, get_global_transform());
	}
}

RID Skeleton2D::get_skeleton() const {
	return skeleton;
}

void Skeleton2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("_update_bone_setup"), &Skeleton2D::_update_bone_setup);
	ClassDB::bind_method(D_METHOD("_update_transform"), &Skeleton2D::_update_transform);

	ClassDB::bind_method(D_METHOD("get_bone_count"), &Skeleton2D::get_bone_count);
	ClassDB::bind_method(D_METHOD("get_bone", "idx"), &Skeleton2D::get_bone);

	ClassDB::bind_method(D_METHOD("get_skeleton"), &Skeleton2D::get_skeleton);

	ClassDB::bind_method(D_METHOD("set_mode_2d", "mode_2d"), &Skeleton2D::set_mode_2d);
	ClassDB::bind_method(D_METHOD("get_mode_2d"), &Skeleton2D::get_mode_2d);
	
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mode_2d"), "set_mode_2d", "get_mode_2d");
	
	ADD_SIGNAL(MethodInfo("bone_setup_changed"));
}

Skeleton2D::Skeleton2D() {
	bone_setup_dirty = true;
	transform_dirty = true;

	skeleton = VS::get_singleton()->skeleton_create();
	set_notify_transform(true);
#ifdef SKELETON_3D
	mode_2d = false;
#endif
}

Skeleton2D::~Skeleton2D() {

	VS::get_singleton()->free(skeleton);
}
