/*************************************************************************/
/*  skeleton_2d.h                                                        */
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

#ifndef SKELETON_2D_H
#define SKELETON_2D_H

#include "scene/2d/node_2d.h"
#include "modules/red/red_engine.h"

#define SKELETON_3D
class Skeleton2D;
class RootBone2D;

class Bone2D : public Node2D {
	GDCLASS(Bone2D, Node2D);

	friend class Skeleton2D;
	friend class RootBone2D;
#ifdef TOOLS_ENABLED
	friend class AnimatedValuesBackup;
#endif

	Bone2D *parent_bone;
	Skeleton2D *skeleton;
#ifdef SKELETON_3D
	TransformC rest;
	TransformC spatial;
	Vector3 preferred_rotation;
	Vector3 preferred_rotation_degrees;
	mutable bool global_rest_dirty;
	mutable bool global_transform_dirty;
	bool spatial_lock;
#else
	Transform2D rest;
#endif
	float default_length;

	int skeleton_index;
	bool object_bone;
	

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	Skeleton2D *get_skeleton() const;
#ifdef SKELETON_3D
	virtual void _make_rest_dirty(bool update_child=true) const{};
	virtual void _make_transform_dirty(bool update_child=true){};
	Transform get_global_spatial_transform() const;
	void spatial_to_transform();
	void transform_to_spatial();
	void set_spatial_transform(const Transform &p_transform);
	Transform get_spatial_transform() const;
	void set_rest(const Transform &p_rest);
	Transform get_rest() const;
	void apply_rest();
	Transform get_skeleton_rest() const;
	void set_preferred_rotation(const Vector3 &p_radians);
	Vector3 get_preferred_rotation() const;
	void set_preferred_rotation_degrees(const Vector3 &p_degrees);
	Vector3 get_preferred_rotation_degrees() const;
	// spatial
	void set_spatial_position(const Vector3 &p_pos);
	Vector3 get_spatial_position() const;
	void set_spatial_rotation(const Vector3 &p_radians);
	Vector3 get_spatial_rotation() const;
	void set_spatial_rotation_degrees(const Vector3 &p_degrees);
	Vector3 get_spatial_rotation_degrees() const;
	void set_spatial_scale(const Vector3 &p_scale);
	Vector3 get_spatial_scale() const;
	// rest
	void set_rest_position(const Vector3 &p_pos);
	Vector3 get_rest_position() const;
	void set_rest_rotation(const Vector3 &p_radians);
	Vector3 get_rest_rotation() const;
	void set_rest_rotation_degrees(const Vector3 &p_degrees);
	Vector3 get_rest_rotation_degrees() const;
	void set_rest_scale(const Vector3 &p_scale);
	Vector3 get_rest_scale() const;
#else
	void set_rest(const Transform2D &p_rest);
	Transform2D get_rest() const;
	void apply_rest();
	Transform2D get_skeleton_rest() const;
#endif
	String get_configuration_warning() const;

	void set_default_length(float p_length);
	float get_default_length() const;

	int get_index_in_skeleton() const;

	Bone2D();
};

class Skeleton2D : public Node2D {
	GDCLASS(Skeleton2D, Node2D);

	friend class Bone2D;
	friend class RootBone2D;
#ifdef TOOLS_ENABLED
	friend class AnimatedValuesBackup;
#endif
public:
	struct Bone {
		bool operator<(const Bone &p_bone) const {
			return p_bone.bone->is_greater_than(bone);
		}
		Bone2D *bone;
		int parent_index;
#ifdef SKELETON_3D
		Transform accum_transform;
		Transform rest_inverse;
#else
		Transform2D accum_transform;
		Transform2D rest_inverse;
#endif
	};
private:
	Vector<Bone> bones;
	Vector<RootBone2D*> red_transforms;

	bool bone_setup_dirty;
	void _make_bone_setup_dirty();
	void _update_bone_setup();

	bool transform_dirty;
	void _make_transform_dirty();
	void _update_transform();

	RID skeleton;
#ifdef SKELETON_3D
	void _update_transform_2d();
	void _update_transform_3d();
	bool mode_2d;
#endif

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	Skeleton2D::Bone get_bone_struct(int p_idx);

	int get_bone_count() const;
	Bone2D *get_bone(int p_idx);

#ifdef SKELETON_3D
	void set_mode_2d(bool p_mode_2d);
	bool get_mode_2d() const;
#endif

	RID get_skeleton() const;
	Skeleton2D();
	~Skeleton2D();
};

#endif // SKELETON_2D_H
