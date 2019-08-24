#ifndef RED_FRAME_H
#define RED_FRAME_H

#include "redelement.h"
#include "core/node_path.h"

class AnimationPlayer;

class REDFrame : public REDElement {
	GDCLASS(REDFrame, REDElement);


	PoolVector<Vector2> polygon;
	bool update_outline;
	bool antialiased;

	Vector2 offset;
	mutable bool rect_cache_dirty;
	mutable Rect2 item_rect;
	Vector<Vector2> screen_coords;
public:
	virtual Dictionary _edit_get_state() const;
	virtual void _edit_set_state(const Dictionary &p_state);

	virtual void _edit_set_pivot(const Point2 &p_pivot);
	virtual Point2 _edit_get_pivot() const;
	virtual bool _edit_use_pivot() const;
	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;

	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	void update_screen_coords();
	void send_mask_shader_param();
	void set_polygon(const PoolVector<Vector2> &p_polygon);
	PoolVector<Vector2> get_polygon() const;

	void set_antialiased(bool p_antialiased);
	bool get_antialiased() const;

	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset() const;

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
protected:
	void _draw_outline();
private:
	void _gradient_changed();
	void _curve_changed();
	bool use_outline;
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

//Frame managment
private:
    bool is_starting;
    bool is_started;
    bool is_ending;
    bool is_ended;
    bool is_active;
    int id;

    NodePath anim_tree;
    Vector<String> states;

protected:
	void _notification(int p_what);
    static void _bind_methods();

public:
	enum {
		NOTIFICATION_FRAME_MASK = 100,
	};
    void run(bool is_prev);
    void to_prev();
    void to_next();

    int get_states_count() const;

    void recalc_state();
    void started();
    void ended();

    int get_id() const;
    void set_id(int new_id);
    void set_anim_tree(const NodePath &new_anim_tree);
    NodePath get_anim_tree() const;

    void set_states(const Array &new_states);
    Array get_states() const;
	REDFrame();
};
VARIANT_ENUM_CAST(REDFrame::LineJointMode)
VARIANT_ENUM_CAST(REDFrame::LineCapMode)
VARIANT_ENUM_CAST(REDFrame::LineTextureMode)
#endif // RED_FRAME_H
