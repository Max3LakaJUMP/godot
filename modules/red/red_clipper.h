#ifndef RED_CLIPPER_H
#define RED_CLIPPER_H

#include "red_shape.h"
#include "core/node_path.h"
#include "scene/animation/animation_node_state_machine.h"

class REDClipper : public REDShape {
	GDCLASS(REDClipper, REDShape);
	Vector<Vector2> targets;
	Vector<Vector2> offsets;
	Vector<Vector2> targets_old;
	Vector<float> times;
	Vector<float> timers;
	Vector<float> lerps;

	Vector2 rotation1;
	Vector2 rotation2;

	Vector<float> multiple;
	Vector<float> constant;
	Vector<Vector2> screen_coords;

	Vector<NodePath> material_objects;
	Vector<NodePath> material_objects2;
	Vector<Ref<ShaderMaterial> > cached_materials;
	int second_split_start_material_id;
	bool clip_enable;
	bool rotation_enable;
	bool clip_rect_enable;
	bool split;
	float split_angle;
	Vector2 split_offset;

	Vector<Vector3> output;
	bool materials_dirty;
	bool send_rotation_dirty;
	bool send_stencil_dirty;

	bool deformation_shape;
	float deformation_offset;
	float deformation_speed;

protected:
	void _notification(int p_what);
    static void _bind_methods();
public:
	enum Space{
		WORLD,
		LOCAL,
		SCREEN
	} space;
	enum DeformationState{
		DEFORMATION_NORMAL,
		DEFORMATION_END,
		DEFORMATION_ENDING,
		DEFORMATION_ENDED,
	} deformation_state;
	
	void set_deformation_shape(bool p_deformate);
	bool get_deformation_shape() const;
	void set_deformation_offset(float p_deformation_offset);
	float get_deformation_offset() const;
	void set_deformation_speed(float p_deformation_speed);
	float get_deformation_speed() const;
	
	void get_points(Vector<Vector2> &p_points) const;
	void _update_materials();
	void _update_stencil(const Vector<Vector2> &p_points) ;
	void _send_stencil();
	void _send_rotation();
	void _move_points(const float deltatime);
	Vector<Vector2> get_offsets();

	void set_rotation1(const Vector2 &p_rotation);
	Vector2 get_rotation1() const;
	void set_rotation2(const Vector2 &p_rotation);
	Vector2 get_rotation2() const;

	void set_split(bool p_split);
	bool get_split() const;
	void set_split_angle(float p_split_angle);
	float get_split_angle() const;
	void set_split_offset(const Vector2 &p_split_offset);
	Vector2 get_split_offset() const;

	void set_rotation_enable(bool p_rotation);
	bool get_rotation_enable() const;
	void set_clip_enable(bool p_clip);
	bool get_clip_enable() const;
	void set_clip_rect_enable(bool p_clip_rect_enable);
	bool get_clip_rect_enable() const;

	void set_space(REDClipper::Space p_space);
	REDClipper::Space get_space() const;
	void set_position_names(const Array &p_position_names);
	Array get_position_names() const;

	void set_material_objects(const Array &p_material_objects);
	Array get_material_objects() const;
	void set_material_objects2(const Array &p_material_objects2);
	Array get_material_objects2() const;
	
	Vector<Ref<ShaderMaterial> > get_cached_materials() const;

	REDClipper();
};

VARIANT_ENUM_CAST(REDClipper::Space);
#endif // RED_CLIPPER_H
