#ifndef RED_CLIPPER_H
#define RED_CLIPPER_H

#include "red_shape.h"
#include "core/node_path.h"
#include "scene/animation/animation_node_state_machine.h"

class REDClipper : public REDShape {
	GDCLASS(REDClipper, REDShape);
	Vector<float> multiple;
	Vector<float> constant;
	Vector<Vector2> screen_coords;
	bool clip_enable;
	bool clip_rect_enable;
	RID ci;
	bool split;
	float split_angle;
	Vector2 split_offset;
	bool send_stencil_dirty;

	//Delete
	Vector<NodePath> material_objects;
	Vector<NodePath> material_objects2;
	Vector<Ref<ShaderMaterial> > cached_materials;
	int second_split_start_material_id;
	bool materials_dirty;

protected:
	void _notification(int p_what);
    static void _bind_methods();

public:
	enum Space{
		CLIPPER_SPACE_WORLD,
		CLIPPER_SPACE_LOCAL,
		CLIPPER_SPACE_SCREEN
	};
private:
	Space space;
public:
	RID get_ci() const;
	void _update_stencil(const Vector<Vector2> &p_points) ;
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

	void _update_materials();
	void _send_rotation();
	void set_material_objects(const Array &p_material_objects);
	Array get_material_objects() const;
	void set_material_objects2(const Array &p_material_objects2);
	Array get_material_objects2() const;
	
	Vector<Ref<ShaderMaterial> > get_cached_materials() const;

	REDClipper();
	~REDClipper();
};

VARIANT_ENUM_CAST(REDClipper::Space);
#endif // RED_CLIPPER_H
