#ifndef RED_ROOT_BONE_H
#define RED_ROOT_BONE_H

#include "scene/2d/skeleton_2d.h"
#include "core/math/transform.h"
#include "core/rid.h"
#include "modules/red/red_engine.h"

class RootBone2D : public Bone2D {
	GDCLASS(RootBone2D, Bone2D);
	mutable Transform global_rest;
	mutable Transform global_spatial;
	// softbody
	float max_offset;
	float bounce;
	float speed;
	Transform global_spatial_old;
	Transform spatial_velocity;

	Node2D *root_node;
	RID ci;
protected:
	void _notification(int p_what);
	static void _bind_methods();
public:
	virtual void _make_rest_dirty(bool update_child=true) const;
	virtual void _make_transform_dirty(bool update_child=true);
	void _make_root_dirty(bool update_child=true);
	void _update_global_spatial(bool update_child=true);
	void _update_old_custom_transform();
	Transform get_global_rest() const;

	void set_max_offset(float p_max_offset);
	float get_max_offset() const;
	void set_speed(float p_speed);
	float get_speed() const;
	void set_bounce(float p_bounce);
	float get_bounce() const;

	Transform get_global_spatial_transform() const;
	void custom_transform_set_global(const Transform &p_transform);
	void set_old_custom_transform(const Transform &p_transform);
	float get_global_depth_position() const;
	Node2D *get_root_node() const;

	RID get_ci();
	RootBone2D();
	~RootBone2D();
};

#endif // RED_ROOT_BONE_H
