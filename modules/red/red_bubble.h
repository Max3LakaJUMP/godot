/*************************************************************************/
/*  polygon_2d.h                                                         */
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

#ifndef RED_BUBBLE_H
#define RED_BUBBLE_H

#include "scene/2d/node_2d.h"
#include "red_line.h"

class REDBubble : public Node2D {

	GDCLASS(REDBubble, Node2D);

	PoolVector<Vector2> polygon;
	PoolVector<Vector2> polygon_tail;
	PoolVector<Vector2> uv;
	PoolVector<Color> vertex_colors;

	struct Bone {
		NodePath path;
		PoolVector<float> weights;
	};

	Vector<Bone> bone_weights;

	Color color;
	Ref<Texture> texture;
	Size2 tex_scale;
	Vector2 tex_ofs;
	bool tex_tile;
	float tex_rot;

	bool antialiased;

	Vector2 offset;
	mutable bool rect_cache_dirty;
	mutable Rect2 item_rect;

	NodePath skeleton;
	ObjectID current_skeleton_id;

	Array _get_bones() const;
	void _set_bones(const Array &p_bones);

	void _skeleton_bone_setup_changed();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	virtual Dictionary _edit_get_state() const;
	virtual void _edit_set_state(const Dictionary &p_state);

	virtual void _edit_set_pivot(const Point2 &p_pivot);
	virtual Point2 _edit_get_pivot() const;
	virtual bool _edit_use_pivot() const;
	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;

	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	void set_polygon(const PoolVector<Vector2> &p_polygon);
	PoolVector<Vector2> get_polygon() const;

	void set_uv(const PoolVector<Vector2> &p_uv);
	PoolVector<Vector2> get_uv() const;

	void set_color(const Color &p_color);
	Color get_color() const;

	void set_vertex_colors(const PoolVector<Color> &p_colors);
	PoolVector<Color> get_vertex_colors() const;

	void set_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_texture() const;

	void set_texture_offset(const Vector2 &p_offset);
	Vector2 get_texture_offset() const;

	void set_texture_rotation(float p_rot);
	float get_texture_rotation() const;

	void set_texture_rotation_degrees(float p_rot);
	float get_texture_rotation_degrees() const;

	void set_texture_scale(const Size2 &p_scale);
	Size2 get_texture_scale() const;

	void set_antialiased(bool p_antialiased);
	bool get_antialiased() const;

	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset() const;

public:
    enum Interpolation {
        CUBIC = 0,
        QUADRATIC_BEZIER,
        CUBIC_BEZIER
    };

private:
    REDLine *outline_node;
    bool edit_tail;
	int smooth;
	float spikes;
	bool use_outline;
	bool reorient;
    Interpolation interpolation;
	Interpolation tale_interpolation;
	Vector<Vector2> _tesselate(Vector<Vector2> &low_res, const Interpolation &p_interpolation) const;
	void _add_spikes(Vector<Vector2> &target, const Vector<Vector2> &source);
	void _add_spikes(Vector<Vector2> &points);
public:
    void set_interpolation(Interpolation p_interpolation);
    Interpolation get_interpolation() const;
	void set_tail_interpolation(Interpolation p_tale_interpolation);
	Interpolation get_tail_interpolation() const;

	void set_spikes(const float b);
    float get_spikes() const;
    void set_smooth(const int b);
    int get_smooth() const;
	void set_edit_tail(const bool b);
	bool get_edit_tail() const;
	void set_polygon_tail(const PoolVector<Vector2> &p_polygon);
	PoolVector<Vector2> get_polygon_tail() const;
	void set_reorient(const bool b);
	bool get_reorient() const;
	//void _update_outline(Vector<Vector2> outline_points);
	REDBubble();

//line
public:
	enum LineJointMode {
		LINE_JOINT_SHARP = 0,
		LINE_JOINT_BEVEL,
		LINE_JOINT_ROUND
	};

	enum LineCapMode {
		LINE_CAP_NONE = 0,
		LINE_CAP_BOX,
		LINE_CAP_ROUND
	};

	enum LineTextureMode {
		LINE_TEXTURE_NONE = 0,
		LINE_TEXTURE_TILE,
		LINE_TEXTURE_STRETCH
	};
	void set_use_outline(const bool b);
	bool get_use_outline() const;

	void set_width(float width);
	float get_width() const;

	void set_curve(const Ref<Curve> &curve);
	Ref<Curve> get_curve() const;

	void set_default_color(Color color);
	Color get_default_color() const;

	void set_gradient(const Ref<Gradient> &gradient);
	Ref<Gradient> get_gradient() const;

	void set_line_texture(const Ref<Texture> &texture);
	Ref<Texture> get_line_texture() const;

	void set_texture_mode(const LineTextureMode mode);
	LineTextureMode get_texture_mode() const;

	void set_joint_mode(LineJointMode mode);
	LineJointMode get_joint_mode() const;

	void set_sharp_limit(float limit);
	float get_sharp_limit() const;

	void set_round_precision(int precision);
	int get_round_precision() const;

    PoolVector<float> thickness_list;
    void set_thickness_list(const PoolVector<float> &p_thickness_list);
    PoolVector<float> get_thickness_list() const;

private:
	void _gradient_changed();
	void _curve_changed();
protected:
	void _draw_outline(Vector<Vector2> &p_points);
private:
	LineJointMode _joint_mode;
	LineCapMode _begin_cap_mode;
	LineCapMode _end_cap_mode;
	float _width;
	Ref<Curve> _curve;
	Color _default_color;
	Ref<Gradient> _gradient;
	Ref<Texture> _texture;
	LineTextureMode _texture_mode;
	float _sharp_limit;
	int _round_precision;
};
VARIANT_ENUM_CAST(REDBubble::Interpolation);
VARIANT_ENUM_CAST(REDBubble::LineJointMode)
VARIANT_ENUM_CAST(REDBubble::LineCapMode)
VARIANT_ENUM_CAST(REDBubble::LineTextureMode)

#endif // POLYGON_2D_H
