/*************************************************************************/
/*  line_2d.h                                                            */
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

#ifndef RED_SHAPE_RENDERER
#define RED_SHAPE_RENDERER

#include "scene/2d/node_2d.h"
#include "scene/2d/line_2d.h"

class REDShape;
class REDClipper;

class REDShapeRenderer : public Node2D {

	GDCLASS(REDShapeRenderer, Node2D);
	friend class REDClipper;
public:
	enum RenderMode{
		RENDER_POLYGON,
		RENDER_LINE,
	};
private:
	// main
	REDShape *shape;
	RenderMode render_mode;
	Color color;
	Ref<Gradient> gradient;
	Ref<Texture> texture;
	bool antialiased;
	// line
	Line2D::LineJointMode _joint_mode;
	Line2D::LineCapMode _begin_cap_mode;
	Line2D::LineCapMode _end_cap_mode;
	float _width;
	float width_zoom_const;
    PoolVector<float> width_list;
	Ref<Curve> curve;
	Line2D::LineTextureMode line_texture_mode;
	float _sharp_limit;
	int _round_precision;
    bool _closed;
	void _gradient_changed();
	void _curve_changed();
protected:
	void _notification(int p_what);
	static void _bind_methods();
public:
	// draw
	void _draw_polygon();
	void _draw_outline();
	// main
	void set_render_mode(RenderMode p_interpolation);
	RenderMode get_render_mode() const;
	void REDShapeRenderer::set_color(Color p_color);
	Color REDShapeRenderer::get_color() const;
	void set_gradient(const Ref<Gradient> &gradient);
	Ref<Gradient> get_gradient() const;
	void set_texture(const Ref<Texture> &texture);
	Ref<Texture> get_texture() const;
	void set_antialiased(bool p_antialiased);
	bool get_antialiased() const;
	// line
	void set_width(float width);
	float get_width() const;
	void set_width_zoom_const(float p_width_constant);
	float get_width_zoom_const() const;
	void set_curve(const Ref<Curve> &curve);
	Ref<Curve> get_curve() const;
	void set_line_texture_mode(const Line2D::LineTextureMode mode);
	Line2D::LineTextureMode get_line_texture_mode() const;
	void set_joint_mode(Line2D::LineJointMode mode);
	Line2D::LineJointMode get_joint_mode() const;
	void set_begin_cap_mode(Line2D::LineCapMode mode);
	Line2D::LineCapMode get_begin_cap_mode() const;
	void set_end_cap_mode(Line2D::LineCapMode mode);
	Line2D::LineCapMode get_end_cap_mode() const;
	void set_sharp_limit(float limit);
	float get_sharp_limit() const;
	void set_round_precision(int precision);
	int get_round_precision() const;
	void set_closed(bool p_closed);
    bool get_closed() const;
	REDShapeRenderer();
};
VARIANT_ENUM_CAST(REDShapeRenderer::RenderMode)
#endif // RED_SHAPE_RENDERER
