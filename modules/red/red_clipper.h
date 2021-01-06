#ifndef RED_CLIPPER_H
#define RED_CLIPPER_H

#include "red_shape_renderer.h"
#include "core/node_path.h"
#include "scene/animation/animation_node_state_machine.h"

class REDShape;

class REDClipper : public REDShapeRenderer {
	GDCLASS(REDClipper, REDShapeRenderer);
public:
	enum Space{
		CLIPPER_SPACE_WORLD,
		CLIPPER_SPACE_LOCAL,
		CLIPPER_SPACE_SCREEN
	};
private:
	bool clip_enable;
	bool clip_rect_enable;
	bool split;

	Vector<float> multiple;
	Vector<float> constant;
	Vector<Vector2> screen_coords;
	float split_angle;
	Vector2 split_offset;
	RID ci;
	Space space;

	mutable bool send_stencil_dirty;
	mutable bool stencil_dirty;
public:
	void _update_stencil();
	void _send_stencil();
	
	void set_split(bool p_split);
	bool get_split() const;
	void set_split_angle(float p_split_angle);
	float get_split_angle() const;
	void set_split_offset(const Vector2 &p_split_offset);
	Vector2 get_split_offset() const;
	void set_clip_enable(bool p_clip);
	bool get_clip_enable() const;
	void set_clip_rect_enable(bool p_clip_rect_enable);
	bool get_clip_rect_enable() const;
	void set_space(Space p_space);
	Space get_space() const;
	RID get_ci() const;
	REDClipper();
	~REDClipper();
protected:
	void _notification(int p_what);
    static void _bind_methods();
};
VARIANT_ENUM_CAST(REDClipper::Space);
#endif // RED_CLIPPER_H
