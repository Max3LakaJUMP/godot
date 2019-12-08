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

#include "scene/2d/node_2d.h"

class REDTransform : public Node {

	GDCLASS(REDTransform, Node);

	Vector3 _pos;
	Vector3 _rotation;
	Vector3 _scale;
	Transform _mat;
	mutable Transform mat_global;
	bool _xform_dirty;
	mutable bool global_dirty;
	RID ci;
	
	void _update_custom_transform();
	void _update_xform_values();

protected:
	static void _bind_methods();

public:
	void set_custom_position(const Vector3 &p_pos);
	void set_custom_rotation(const Vector3 &p_radians);
	void set_custom_rotation_degrees(const Vector3 &p_degrees);
	void set_custom_scale(const Vector3 &p_scale);

	Vector3 get_custom_position() const;
	Vector3 get_custom_rotation() const;
	Vector3 get_custom_rotation_degrees() const;
	Vector3 get_custom_scale() const;

	void set_custom_transform(const Transform &p_transform);
	Transform get_custom_transform() const;
	Transform get_custom_global_transform() const;

	RID get_ci();
	REDTransform();
	~REDTransform();
};

#endif // REDTRANSFORM_H
