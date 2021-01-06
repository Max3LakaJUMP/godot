#ifndef RED_SHAPE_H
#define RED_SHAPE_H

#include "scene/2d/node_2d.h"
#include "scene/2d/line_2d.h"
#include "red_engine.h"

class REDShape;
class REDShapeRenderer;
class REDClipper;

class REDShape : public Node2D {
	GDCLASS(REDShape, Node2D);

	friend class REDShape;
	friend class REDShapeRenderer;
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
	enum Boolean{
		BOOLEAN_MAIN,
		BOOLEAN_MERGE,
		BOOLEAN_CLIP,
		BOOLEAN_INTERSECT,
		BOOLEAN_OVERRIDE
	};
private:
	//main
	REDShape* root_shape;
	Vector<REDShape*> boolean_nodes;
	Vector<REDShapeRenderer*> render_nodes;

	Boolean boolean;
	PoolVector<Vector2> polygon;
	Vector<Vector2> real_polygon;
	Vector<Vector2> boolean_polygon;
	Vector<float> real_width_list;
	Vector2 offset;
	int reorient;
	int smooth;
    red::TesselateMode interpolation;
	float simplify;
	float spikes = 0;
	Vector2 camera_zoom;
	bool polygon_dirty;
	bool boolean_dirty;
	bool width_dirty;
	mutable bool rect_cache_dirty;
	mutable Rect2 item_rect;
	// animation
	DeformationState deformation_state;
	DeformationState width_state;
	bool deformation_enable;
	float deformation_offset;
	float deformation_speed;
	float deformation_width_min;
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
	void update_renderers();
	void update_line_renderers();
	void update_polygon_renderers();
	void update_root();
	void set_polygon(const PoolVector<Vector2> &p_polygon);
	PoolVector<Vector2> get_polygon() const;
	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset() const;
	void set_boolean(Boolean p_interpolation);
	Boolean get_boolean() const;
    void set_smooth(const int p_smooth);
    int get_smooth() const;
	void set_interpolation(red::TesselateMode p_interpolation);
	red::TesselateMode get_interpolation() const;
	void set_simplify(float p_simplify);
	float get_simplify() const;
	void set_spikes(float p_spikes);
	float get_spikes() const;
	void add_spikes(Vector<Vector2> &p_points);
	Vector2 get_camera_zoom() const;
	void update_camera_zoom(const Vector2 p_camera_zoom=Vector2(1.0, 1.0));
	void set_reorient(int p_reorient);
	int get_reorient() const;
	void calc_polygon();
	void calc_boolean();
	Vector<float> resize_vector(const Vector<float> &in, int size) const;
	void calc_width_list();
	// animation
	void _animate_offset(const float deltatime);
	void _animate_width(const float deltatime);
	void set_deformation_enable(bool p_deformate);
	bool get_deformation_enable() const;
	void set_deformation_offset(float p_deformation_offset);
	float get_deformation_offset() const;
	void set_deformation_speed(float p_deformation_speed);
	float get_deformation_speed() const;

	void set_deformation_width_min(const float p_deformation_width_min);
	float get_deformation_width_min() const;
	void set_deformation_width_max(const float p_deformation_width_max);
	float get_deformation_width_max() const;
	REDShape();
};
VARIANT_ENUM_CAST(red::TesselateMode)
VARIANT_ENUM_CAST(REDShape::Boolean)
#endif // RED_SHAPE_H
