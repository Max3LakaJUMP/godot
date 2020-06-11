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

#ifndef REDDEFORM_H
#define REDDEFORM_H

#include "scene/main/node.h"
#include "core/rid.h"

class REDDeform : public Node {
	GDCLASS(REDDeform, Node);

	float wind_rotation;
	float wind_offset;

	float wind1_time;
	float wind1_strength;
	float wind2_time;
	float wind2_strength;

	float scale_time;
	float scale_strength;
	float waves_count;
	float elasticity;

	RID ci;
protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	float get_wind_rotation() const;
	void set_wind_rotation(float p_wind_rotation);
	float get_wind_offset() const;
	void set_wind_offset(float p_wind_offset);
	float get_waves_count() const;
	void set_waves_count(float p_waves_count);
	float get_elasticity() const;
	void set_elasticity(float p_elasticity);
	
	float get_wind1_time() const;
	void set_wind1_time(float p_wind1_time);
	float get_wind1_strength() const;
	void set_wind1_strength(float p_wind1_strength);

	float get_wind2_time() const;
	void set_wind2_time(float p_wind2_time);
	float get_wind2_strength() const;
	void set_wind2_strength(float p_wind2_strength);

	float get_scale_time() const;
	void set_scale_time(float p_scale_time);
	float get_scale_strength() const;
	void set_scale_strength(float p_scale_strengt);
	RID get_ci();
	REDDeform();
	~REDDeform();
};

#endif // REDDEFORM_H
