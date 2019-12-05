/*************************************************************************/
/*  parallax_layer.h                                                     */
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

#ifndef RED_PARALLAX_FOLDER_H
#define RED_PARALLAX_FOLDER_H

#include "scene/2d/node_2d.h"

class REDParallaxFolder : public Node2D {

	GDCLASS(REDParallaxFolder, Node2D);

	bool shrink;
	bool expand;

	Point2 orig_offset;
	Point2 orig_scale;
	Size2 parallax_factor;
	Vector2 motion_offset;
	Point2 screen_offset;
	float zoom_scaling;

	float zoom_in_scale;
	float zoom_out_scale;
	
	Point2 camera_offset;
	Vector2 camera_zoom;

	Point2 parallax_offset;
	Point2 old_parallax_offset;
	bool parallax_offset_dirty;
	Vector2 parallax_scale;
	Vector2 old_parallax_scale;
	bool parallax_scale_dirty;



protected:
	static void _bind_methods();
	void _notification(int p_what);
	
public:
	void apply_parallax();

	void set_camera_offset(const Point2 &p_offset);
	void set_camera_zoom(const Vector2 &p_zoom);

	Point2 get_parallax_offset();
	Vector2 get_parallax_scale();

	void set_zoom_in_scale(float p_scale);
	float get_zoom_in_scale() const;
	void set_zoom_out_scale(float p_scale);
	float get_zoom_out_scale() const;

	void set_motion_offset(const Size2 &p_offset);
	Size2 get_motion_offset() const;

	void set_parallax_factor(const Size2 &p_scale);
	Size2 get_parallax_factor() const;


	virtual String get_configuration_warning() const;
	REDParallaxFolder();
};

#endif // RED_PARALLAX_FOLDER_H
