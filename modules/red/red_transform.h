/*************************************************************************/
/*  node_2d.h                                                            */
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

#ifndef REDTRANSFORM_H
#define REDTRANSFORM_H

#include "scene/2d/skeleton_2d.h"
#include "core/math/transform.h"
#include "core/rid.h"
#include "modules/red/red_engine.h"

class REDTransform : public Bone2D {
	GDCLASS(REDTransform, Bone2D);
	float depth_position;
	TransformC rest;
	TransformC custom;
	
	// global
	mutable bool global_rest_dirty;
	mutable bool global_transform_dirty;
	mutable Transform global_custom;
	mutable Transform global_rest;

	// softbody
	float elasticity;
	Transform global_custom_old;
	Transform global_custom_velocity;
	float global_custom_euler_accum;

	Node2D *root_node;
	RID ci;
protected:
	void _notification(int p_what);
	static void _bind_methods();
public:
	void set_depth_position(float p_depth);
	float get_depth_position() const;
	void set_transform3d(const Transform &p_rest);
	Transform get_transform3d() const;
	Vector3 get_position3d() const;
	void set_elasticity(float p_elasticity);
	float get_elasticity() const;
	void custom_transform_set_global(const Transform &p_transform);
	void set_old_custom_transform(const Transform &p_transform);
	float get_global_depth_position() const;
	void _make_root_dirty(bool update_child=true);
	void _make_rest_dirty(bool update_child=true);
	void _make_transform_dirty(bool update_child=true);
	void _update_custom_transform(bool update_child=true);
	Node2D *get_root_node() const;
	void _update_old_custom_transform();

	//custom
	Transform get_global_rest() const;
	void set_custom_transform(const Transform &p_transform);
	Transform get_custom_transform() const;
	void set_custom_position(const Vector3 &p_pos);
	Vector3 get_custom_position() const;
	void set_custom_rotation(const Vector3 &p_radians);
	Vector3 get_custom_rotation() const;
	void set_custom_rotation_degrees(const Vector3 &p_degrees);
	Vector3 get_custom_rotation_degrees() const;
	void set_custom_scale(const Vector3 &p_scale);
	Vector3 get_custom_scale() const;
	
	// rest
	Transform get_global_custom_transform() const;
	void set_rest3d(const Transform &p_rest);
	Transform get_rest3d() const;
	void set_rest_position(const Vector3 &p_pos);
	Vector3 get_rest_position() const;
	void set_rest_rotation(const Vector3 &p_radians);
	Vector3 get_rest_rotation() const;
	void set_rest_rotation_degrees(const Vector3 &p_degrees);
	Vector3 get_rest_rotation_degrees() const;

	RID get_ci();
	REDTransform();
	~REDTransform();
};

#endif // REDTRANSFORM_H
