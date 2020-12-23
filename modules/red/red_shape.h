#ifndef RED_SHAPE_H
#define RED_SHAPE_H

#include "red_element.h"
#include "core/node_path.h"
#include "scene/2d/line_2d.h"
#include "scene/animation/animation_node_state_machine.h"

class REDOutline;
class REDClipper;

class REDShape : public Node2D {
	GDCLASS(REDShape, Node2D);
	friend class REDOutline;
	friend class REDClipper;
public:
#ifdef TOOLS_ENABLED
	virtual Dictionary _edit_get_state() const;
	virtual void _edit_set_state(const Dictionary &p_state);

	virtual void _edit_set_pivot(const Point2 &p_pivot);
	virtual Point2 _edit_get_pivot() const;
	virtual bool _edit_use_pivot() const;
	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
#endif
	enum DeformationState{
		DEFORMATION_NORMAL,
		DEFORMATION_END,
		DEFORMATION_ENDING,
		DEFORMATION_ENDED,
	};
	enum DeformationWidthState{
		DEFORMATION_WIDTH_EXPANDING0,
		DEFORMATION_WIDTH_EXPANDING1,
		DEFORMATION_WIDTH_TAPERING0,
		DEFORMATION_WIDTH_TAPERING1,
	};

private:
	//main
	PoolVector<Vector2> polygon;
	Vector2 offset;
	Vector2 camera_zoom;
	bool width_dirty;
	bool polygon_dirty;
	mutable bool rect_cache_dirty;
	mutable Rect2 item_rect;
	// content
	REDClipper *content_node;
	bool use_content;
	// line
	REDOutline *outline_node;
	bool use_outline;
	float outline_width_zoom_const;
    PoolVector<float> width_list;
	float width;
	Vector<Vector2> real_polygon;
	Vector<float> real_width_list;
	Line2D::LineJointMode joint_mode;
	Color line_color;
	Ref<Texture> texture;
	Line2D::LineTextureMode texture_mode;
	bool antialiased;
	// Deformation
	DeformationState deformation_state;
	bool deformation_enable;
	float deformation_offset;
	float deformation_speed;
	float deformation_width_factor;
	float deformation_width_max;
	Vector<Vector2> targets;
	Vector<Vector2> offsets;
	Vector<Vector2> targets_old;
	Vector<float> times;
	Vector<float> timers;

	Vector<float> width_offsets_old;
	Vector<float> width_offsets;
	Vector<float> width_offsets_state;
	float width_timer;
protected:
    static void _bind_methods();
	void _notification(int p_what);

public:
	// main
	void set_polygon(const PoolVector<Vector2> &p_polygon);
	PoolVector<Vector2> get_polygon() const;
	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset() const;
	Vector2 get_camera_zoom() const;
	void update_camera_zoom(const Vector2 p_camera_zoom=Vector2(1.0, 1.0));
	void calc_polygon();
	// content
	void update_content();
	void set_use_content(bool b);
	bool get_use_content() const;
	void set_content(REDClipper *p_content);
	REDClipper *get_content();
	// line 
	void update_outline();
	void set_use_outline(bool b);
	bool get_use_outline() const;
	void set_outline(REDOutline *p_outline);
	REDOutline *get_outline();
	void set_width(float width);
	float get_width() const;
    void set_width_list(const PoolVector<float> &p_width_list);
    PoolVector<float> get_width_list() const;
	void set_line_color(Color color);
	Color get_line_color() const;
	void set_line_texture(const Ref<Texture> &texture);
	Ref<Texture> get_line_texture() const;
	void set_texture_mode(const Line2D::LineTextureMode mode);
	Line2D::LineTextureMode get_texture_mode() const;
	void set_joint_mode(Line2D::LineJointMode mode);
	Line2D::LineJointMode get_joint_mode() const;
	void set_antialiased(bool p_antialiased);
	bool get_antialiased() const;
	void calc_width_list();
	// Deformation
	void _move_points(const float deltatime);
	void set_deformation_enable(bool p_deformate);
	bool get_deformation_enable() const;
	void set_deformation_offset(float p_deformation_offset);
	float get_deformation_offset() const;
	void set_deformation_speed(float p_deformation_speed);
	float get_deformation_speed() const;
	void set_deformation_width_factor(const float p_deformation_width_factor);
	float get_deformation_width_factor() const;
	void set_deformation_width_max(const float p_deformation_width_max);
	float get_deformation_width_max() const;
	void set_outline_width_zoom_const(float p_outline_width_zoom_const);
	float get_outline_width_zoom_const() const;
	REDShape();
};
VARIANT_ENUM_CAST(Line2D::LineJointMode)
//VARIANT_ENUM_CAST(Line2D::LineCapMode)
VARIANT_ENUM_CAST(Line2D::LineTextureMode)
#endif // RED_SHAPE_H
