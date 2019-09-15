#ifndef RED_LINE_H
#define RED_LINE_H

#include "scene/2d/node_2d.h"

class REDLine : public Node2D {

	GDCLASS(REDLine, Node2D);

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

	REDLine();

	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	void set_points(const PoolVector<Vector2> &p_points);
	PoolVector<Vector2> get_points() const;

	void set_point_position(int i, Vector2 pos);
	Vector2 get_point_position(int i) const;

	int get_point_count() const;

	void clear_points();

	void add_point(Vector2 pos, int atpos = -1);
	void remove_point(int i);

	void set_width(float width);
	float get_width() const;

	void set_width_curve(const Ref<Curve> &width_curve);
	Ref<Curve> get_width_curve() const;

	void set_default_color(Color color);
	Color get_default_color() const;

	void set_gradient(const Ref<Gradient> &gradient);
	Ref<Gradient> get_gradient() const;

	void set_texture(const Ref<Texture> &texture);
	Ref<Texture> get_texture() const;

	void set_texture_mode(const LineTextureMode mode);
	LineTextureMode get_texture_mode() const;

	void set_joint_mode(LineJointMode mode);
	LineJointMode get_joint_mode() const;

	void set_begin_cap_mode(LineCapMode mode);
	LineCapMode get_begin_cap_mode() const;

	void set_end_cap_mode(LineCapMode mode);
	LineCapMode get_end_cap_mode() const;

	void set_sharp_limit(float limit);
	float get_sharp_limit() const;

	void set_round_precision(int precision);
	int get_round_precision() const;

    PoolVector<float> width_list;
    void set_width_list(const PoolVector<float> &p_width_list);
    PoolVector<float> get_width_list() const;
    bool is_closed;
    void set_is_closed(const bool is_closed);
    bool get_is_closed() const;

protected:
	void _notification(int p_what);
	void _draw();

	static void _bind_methods();

private:
	void _gradient_changed();
	void _width_curve_changed();

private:
	PoolVector<Vector2> _points;
	LineJointMode _joint_mode;
	LineCapMode _begin_cap_mode;
	LineCapMode _end_cap_mode;
	float _width;
	Ref<Curve> _width_curve;
	Color _default_color;
	Ref<Gradient> _gradient;
	Ref<Texture> _texture;
	LineTextureMode _texture_mode;
	float _sharp_limit;
	int _round_precision;
};

#endif // RED_LINE_H
