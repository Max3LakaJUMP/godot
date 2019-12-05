#ifndef RED_SHAPE_H
#define RED_SHAPE_H

#include "red_element.h"
#include "core/node_path.h"
#include "scene/animation/animation_node_state_machine.h"

class REDShape : public REDElement {
	GDCLASS(REDShape, REDElement);
public:
	enum LineJointMode {
		LINE_JOINT_SHARP = 0,
		LINE_JOINT_BEVEL,
		LINE_JOINT_ROUND
	};

	enum LineTextureMode {
		LINE_TEXTURE_NONE = 0,
		LINE_TEXTURE_TILE,
		LINE_TEXTURE_STRETCH
	};
private:
	PoolVector<Vector2> polygon;
	bool outline_width_constant;

	bool antialiased;

	Vector2 camera_zoom;
	Vector2 offset;

	bool use_outline;
	mutable bool rect_cache_dirty;
	mutable Rect2 item_rect;
	LineJointMode joint_mode;
	float width;

    PoolVector<float> width_list;
	bool closed;
	Ref<Curve> width_curve;
	Color default_color;
	Ref<Gradient> gradient;
	Ref<Texture> texture;
	LineTextureMode texture_mode;
	float sharp_limit;
	int round_precision;


	void _gradient_changed();
	void _width_curve_changed();

protected:

    static void _bind_methods();
	void _notification(int p_what);
	bool stencil_dirty;
	bool outline_dirty;

public:
	virtual Dictionary _edit_get_state() const;
	virtual void _edit_set_state(const Dictionary &p_state);

	virtual void _edit_set_pivot(const Point2 &p_pivot);
	virtual Point2 _edit_get_pivot() const;
	virtual bool _edit_use_pivot() const;
	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;

	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	void _draw_outline(Vector<Vector2> &p_points);

	void set_polygon(const PoolVector<Vector2> &p_polygon);
	PoolVector<Vector2> get_polygon() const;

	void set_use_outline(const bool b);
	bool get_use_outline() const;

	void set_outline_width_constant(const bool p_outline_width_constant);
	bool is_outline_width_constant() const;

	void set_closed(const bool p_closed);
	bool is_closed() const;

	void set_antialiased(bool p_antialiased);
	bool get_antialiased() const;

	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset() const;

	void set_camera_zoom(const Vector2 &p_camera_zoom);
	Vector2 get_camera_zoom() const;

	void update_camera_pos(const Vector2 p_camera_pos=Vector2(0.0, 0.0));
	void update_camera_zoom(const Vector2 p_camera_zoom=Vector2(1.0, 1.0));

	void set_width(float width);
	float get_width() const;

	void set_width_curve(const Ref<Curve> &p_width_curve);
	Ref<Curve> get_width_curve() const;

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

    void set_width_list(const PoolVector<float> &p_width_list);
    PoolVector<float> get_width_list() const;


	REDShape();
};
VARIANT_ENUM_CAST(REDShape::LineJointMode)
VARIANT_ENUM_CAST(REDShape::LineTextureMode)
#endif // RED_SHAPE_H
