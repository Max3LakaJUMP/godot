#include "root_bone_2d.h"
#include "scene/2d/skeleton_2d.h"
#include "core/message_queue.h"
#include "scene/gui/control.h"
#include "scene/main/viewport.h"
#include "servers/visual_server.h"

void RootBone2D::_make_transform_dirty(bool update_child) {
	if (!is_inside_tree()){
		global_transform_dirty = true;
		return;
	}
	if (!global_transform_dirty){
		global_transform_dirty = true;
		call_deferred("_update_global_spatial", update_child);
	}
}

void RootBone2D::_make_rest_dirty(bool update_child) const{
	global_rest_dirty = true;
	if (update_child){
		for (int i = 0; i < get_child_count(); i++){
			Bone2D *pi = Object::cast_to<Bone2D>(get_child(i));
			if (pi)
				pi->_make_rest_dirty();
			else
				break;
		}
	}
}

void RootBone2D::_make_root_dirty(bool update_child) {
	if (!is_inside_tree() || !root_node)
		return;
	get_global_transform();
	Transform root_node_transform3d = Variant(root_node->get_global_transform());
	custom_transform_set_global(root_node_transform3d);
	if (update_child){
		for (int i = 0; i < get_child_count(); i++){
			RootBone2D *pi = Object::cast_to<RootBone2D>(get_child(i));
			if (pi)
				pi->_make_root_dirty(true);
		}
	}
}

void RootBone2D::_update_global_spatial(bool update_child){
	if (!global_transform_dirty)
		return;
	global_transform_dirty = false;
	Node *par = get_parent();
	RootBone2D *parent_tr = Object::cast_to<RootBone2D>(par);
	if (parent_tr){
		global_spatial = parent_tr->get_global_spatial_transform() * spatial;
	}else{
		Bone2D *parent_bone = Object::cast_to<Bone2D>(par);
		if (parent_bone){
			Skeleton2D *parent_skeleton = parent_bone->get_skeleton();
			if (parent_skeleton){
				Skeleton2D::Bone b = parent_skeleton->get_bone_struct(parent_bone->get_index_in_skeleton());
				Transform bone_transform = b.accum_transform;
				global_spatial = bone_transform * spatial;
			}
		}else{
			global_spatial = spatial;
		}
	}
	if(max_offset == 0.0f){
		Transform tr = global_spatial * get_global_rest().affine_inverse();
		VisualServer::get_singleton()->custom_transform_set_old(ci, tr);
		VisualServer::get_singleton()->custom_transform_set(ci, tr);
	}
	else{
		set_process_internal(true);
	}
	if (update_child){
		for (int i = 0; i < get_child_count(); i++){
			RootBone2D *pi = Object::cast_to<RootBone2D>(get_child(i));
			if (pi)
				pi->_make_transform_dirty(true);
		}
	}
}

void RootBone2D::_update_old_custom_transform(){
	Transform global_transform = Variant(root_node->get_global_transform());
	Transform target = global_transform * get_global_spatial_transform();
	Transform r = get_global_rest();
	Transform r_inv = r.affine_inverse();
	Transform gl_inv = global_transform.affine_inverse();
	if(target.is_equal_approx(global_spatial_old)){
		global_spatial_old = target;
		spatial_velocity = Transform();
		set_process_internal(false);
		VisualServer::get_singleton()->custom_transform_set_old(ci, gl_inv * global_spatial_old * r_inv);
	}
	else{
		float bounces = bounce;
		Transform local_custom_old = gl_inv * global_spatial_old * r_inv;
		Transform local_target = get_global_spatial_transform() * r_inv;
		{
			float delta_time = get_process_delta_time();
			float delta_velocity = 0.5f / (1.f + 4.0 * MAX(bounces, 0.f));
			float delta_speed = MIN(delta_time * speed * 4.f, 0.5f);
			// get smooth velocity for origin
			spatial_velocity.origin = spatial_velocity.origin.linear_interpolate(	
				(1.0 + 4.0 * bounces) * (target.origin - global_spatial_old.origin), delta_velocity);
			// apply
			global_spatial_old.origin = global_spatial_old.origin.linear_interpolate(
				spatial_velocity.origin + global_spatial_old.origin, delta_speed);
			// max offset
			Vector3 diff = global_spatial_old.origin - target.origin;
			float k = MIN(max_offset / diff.length(), 1.f);
			global_spatial_old.origin = target.origin + k * diff;
			// get smooth velocity for basis
			spatial_velocity.basis = spatial_velocity.interpolate_with(
				local_target * local_custom_old.affine_inverse(), MIN(delta_velocity * 1.0f, 0.5f)).basis;
			// apply
			local_custom_old.basis = local_custom_old.interpolate_with(
				spatial_velocity * local_custom_old, MIN(delta_speed * 3.0f, 0.5f)).basis;
		}
		// set
		global_spatial_old.basis = global_transform.basis * local_custom_old.basis * r.basis;
		VisualServer::get_singleton()->custom_transform_set_old(ci, gl_inv * global_spatial_old * r_inv);
	}
}

void RootBone2D::custom_transform_set_global(const Transform &p_transform){
	VisualServer::get_singleton()->custom_transform_set_global(ci, p_transform);
	if(max_offset != 0.0f)
		set_process_internal(true);
}

void RootBone2D::set_old_custom_transform(const Transform &p_transform){
	global_spatial_old = p_transform;
	if(max_offset == 0.0f)
		VisualServer::get_singleton()->custom_transform_set_old(ci, global_spatial * get_global_rest().affine_inverse());
	else
		set_process_internal(true);
}

Transform RootBone2D::get_global_spatial_transform() const{
	return global_spatial;
}

Transform RootBone2D::get_global_rest() const {
	if (global_rest_dirty){
		global_rest_dirty = false;
		Node *par = get_parent();
		RootBone2D *parent_tr = Object::cast_to<RootBone2D>(par);
		if (parent_tr) {
			global_rest = parent_tr->get_global_rest() * get_rest();
		} else {
			Bone2D *parent_bone = Object::cast_to<Bone2D>(par);
			if (parent_bone){
				Transform bone_rest = Variant(parent_bone->get_skeleton_rest());
				global_rest = bone_rest * get_rest();
			}
			else{
				global_rest = get_rest();
			}
		}
	}
	return global_rest;
}

float RootBone2D::get_global_depth_position() const{
	if (!is_inside_tree())
		return get_spatial_position().z;
	RootBone2D *pi = Object::cast_to<RootBone2D>(get_parent());
	if (pi)
		return pi->get_global_depth_position() + get_spatial_position().z;
	return get_spatial_position().z;
}

void RootBone2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			Node *parent = get_parent();
			Bone2D *is_bone;
			Skeleton2D *is_skeleton;
			RootBone2D *is_transform;
			root_node = NULL;
			if(skeleton){
				root_node = Object::cast_to<Node2D>(skeleton);
			}else{
				while (parent) {
					is_transform = Object::cast_to<RootBone2D>(parent);
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
			}
			_update_global_spatial();
			Transform global_transform = Variant(root_node->get_global_transform());
			global_spatial_old = global_transform * get_global_spatial_transform();
			spatial_velocity = Transform();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			set_notify_transform(false);
			if (root_node) {
				root_node = NULL;
			}
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			VisualServer::get_singleton()->custom_transform_set(ci, global_spatial * get_global_rest().affine_inverse());
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

void RootBone2D::set_max_offset(float p_max_offset){
	max_offset = p_max_offset;
}

float RootBone2D::get_max_offset() const{
	return max_offset;
}

void RootBone2D::set_speed(float p_speed){
	speed = p_speed;
}

float RootBone2D::get_speed() const{
	return speed;
}

void RootBone2D::set_bounce(float p_bounce){
	bounce = p_bounce;
}

float RootBone2D::get_bounce() const{
	return bounce;
}

Node2D *RootBone2D::get_root_node() const{
	return root_node;
}

RID RootBone2D::get_ci(){
	return ci;
}

void RootBone2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_make_transform_dirty", "update_child"), &RootBone2D::_make_transform_dirty);
	ClassDB::bind_method(D_METHOD("_make_rest_dirty", "update_child"), &RootBone2D::_make_rest_dirty);
	ClassDB::bind_method(D_METHOD("_make_root_dirty", "update_child"), &RootBone2D::_make_root_dirty);
	ClassDB::bind_method(D_METHOD("_update_global_spatial", "update_child"), &RootBone2D::_update_global_spatial);
	ClassDB::bind_method(D_METHOD("_update_old_custom_transform"), &RootBone2D::_update_old_custom_transform);

	ClassDB::bind_method(D_METHOD("custom_transform_set_global", "transform"), &RootBone2D::custom_transform_set_global);
	ClassDB::bind_method(D_METHOD("set_old_custom_transform", "transform"), &RootBone2D::set_old_custom_transform);
	ClassDB::bind_method(D_METHOD("get_global_spatial_transform"), &RootBone2D::get_global_spatial_transform);
	ClassDB::bind_method(D_METHOD("get_global_rest"), &RootBone2D::get_global_rest);
	ClassDB::bind_method(D_METHOD("get_global_depth_position"), &RootBone2D::get_global_depth_position);

	ClassDB::bind_method(D_METHOD("set_max_offset", "max_offset"), &RootBone2D::set_max_offset);
	ClassDB::bind_method(D_METHOD("get_max_offset"), &RootBone2D::get_max_offset);
	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &RootBone2D::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &RootBone2D::get_speed);
	ClassDB::bind_method(D_METHOD("set_bounce", "bounce"), &RootBone2D::set_bounce);
	ClassDB::bind_method(D_METHOD("get_bounce"), &RootBone2D::get_bounce);
	ADD_GROUP("Soft body", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_offset"), "set_max_offset", "get_max_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "speed"), "set_speed", "get_speed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "bounce"), "set_bounce", "get_bounce");
}

RootBone2D::RootBone2D() {
	max_offset = 0.f;
	speed = 1.f;
	bounce = 1.f;
	ci = VS::get_singleton()->custom_transform_create();
	set_notify_local_transform(true);
	object_bone = true;
	root_node = NULL;
}
RootBone2D::~RootBone2D() {

	VS::get_singleton()->free(ci);
}