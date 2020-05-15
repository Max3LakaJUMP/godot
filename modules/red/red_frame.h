#ifndef RED_FRAME_H
#define RED_FRAME_H

#include "red_shape.h"
#include "core/node_path.h"
#include "scene/animation/animation_node_state_machine.h"

class REDFrame : public REDShape {
	GDCLASS(REDFrame, REDShape);
	// Clipper
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


private:
	bool start_immediate;
	int old_z_index;
	
	float start_delay;
	float end_delay;

	Vector2 scroll_scale;

	Vector2 frame_scale_factor;
	Vector2 scale_parallax_pos;
	Vector2 old_scale_offset;
	Vector2 old_offset;
	Vector2 parallax_factor;
	Vector2 position_motion_scale;

	Point2 origin_pos_gl;
	Point2 origin_scale;
	Point2 origin_pos;


	Vector2 parallax;

	Vector2 parallax_pos_current;
	Vector2 parallax_zoom;
	Vector2 parallax_offset;

	Vector2 camera_zoom;

	Vector2 parallax_multiplayer;
	Vector2 current_parallax;

	Transform2D camera_pos;
	Transform2D camera_pos_in;
	Transform2D camera_pos_out;
	Transform2D parallax_pos;
	Transform2D parallax_pos_in;
	Transform2D parallax_pos_out;

	NodePath camera_pos_path;
	NodePath camera_pos_in_path;
	NodePath camera_pos_out_path;
	NodePath parallax_pos_path;
	NodePath parallax_pos_in_path;
	NodePath parallax_pos_out_path;
public:
	enum Anchor{
		FRAME_ANCHOR_CENTER,
		FRAME_ANCHOR_TOP_LEFT,
	};

private:
	Anchor anchor;

public:
	void set_start_immediate(bool p_start_immediate);
	bool is_start_immediate() const;

	void set_anchor(REDFrame::Anchor p_anchor);
	REDFrame::Anchor get_anchor() const;


	void unload_target(const NodePath &p_path, StringName function);
	Transform2D load_target(const NodePath &p_path, StringName function);
	void load_targets();
	void unload_targets();
	void _target_parallax_moved();
	void _target_pos_moved();

	void set_camera_pos_path(const NodePath &p_camera_pos_path);
	NodePath get_camera_pos_path() const;
	void set_camera_pos_in_path(const NodePath &p_camera_pos_path);
	NodePath get_camera_pos_in_path() const;
	void set_camera_pos_out_path(const NodePath &p_camera_pos_path);
	NodePath get_camera_pos_out_path() const;

	void set_parallax_pos_path(const NodePath &p_parallax_path);
	NodePath get_parallax_pos_path() const;
	void set_parallax_pos_in_path(const NodePath &p_parallax_path);
	NodePath get_parallax_pos_in_path() const;
	void set_parallax_pos_out_path(const NodePath &p_parallax_path);
	NodePath get_parallax_pos_out_path() const;

	void set_camera_pos(const Transform2D &p_camera_pos);
	Transform2D get_camera_pos() const;
	void set_camera_pos_in(const Transform2D &p_camera_pos);
	Transform2D get_camera_pos_in() const;
	void  set_camera_pos_out(const Transform2D &p_camera_pos);
	Transform2D  get_camera_pos_out() const;
	
	void set_parallax_pos(const Transform2D &p_parallax_pos);
	Transform2D get_parallax_pos() const;
	void set_parallax_pos_in(const Transform2D &p_parallax_pos);
	Transform2D get_parallax_pos_in() const;
	void  set_parallax_pos_out(const Transform2D &p_parallax_pos);
	Transform2D  get_parallax_pos_out() const;


	Vector2 get_scale_offset() const;

	void set_scale_pos(const Vector2 &p_scale_pos);
	void set_scene_scale(const Vector2 &p_scale_parallax_pos);


	void set_start_delay(float p_delay);
	float get_start_delay() const;
	void set_end_delay(float p_delay);
	float get_end_delay() const;

	void set_old_z_index(int p_z_index);
	void revert_z_index();

	void set_frame_scale(const Vector2 &p_scale_factor);
	/*
	void set_position_motion_scale(const Vector2 &p_motion_scale);
	Vector2 get_position_motion_scale() const;
	*/
	void set_parallax_factor(const Vector2 &p_motion_scale);
	Vector2 get_parallax_factor() const;

	void update_origin_pos_gl();
	void set_origin_pos_gl(const Point2 &p_origin_pos_gl);
	Point2 get_origin_pos_gl() const;

// Parallax, const Vector2 &p_parallax_pos, const Vector2 &p_scale
	void set_parallax_offset(const Point2 &p_parallax);
	void set_parallax_zoom(const Vector2 &p_zoom);
	Vector2 get_parallax_zoom();
	void apply_parallax();

	Point2 get_parallax_offset() const;
	Vector2 get_scroll_scale() const;

	void set_parallax_pos_current(const Vector2 &p_parallax_pos);
	Vector2 get_parallax_pos_current() const;


	void set_camera_zoom(const Vector2 &p_camera_zoom);
	Vector2 get_camera_zoom() const;

public:
	enum FrameLoop{
		FRAME_LOOP_START,
		FRAME_LOOP_MAIN,
		FRAME_LOOP_END,
	};

	enum Status{
		FRAME_STATE_STARTING,
		FRAME_STATE_STARTED,
		FRAME_STATE_ENDING,
		FRAME_STATE_ENDED,
	};

	enum CameraState{
		CAMERA_STATE_MOOVING_TO_FRAME,
		CAMERA_STATE_ON_FRAME,
		CAMERA_STATE_MOOVING_FROM_FRAME,
	};
	void update_camera_zoom_and_child(const Vector2 &p_camera_zoom);
//Frame managment
private:
	FrameLoop target_loop;
	Status status;
	CameraState camera_state;

    bool reinit_tree;

	bool b_start_loop;
    bool b_starting;
    bool b_started;
    bool b_ending;
    bool b_ended;
	bool b_end_loop;
    
	bool focused;
	bool deformate_on_activation;
    bool b_active;
    int id;
    NodePath anim_tree;
    Vector<String> states;

    String start_transition;
    String end_transition;
    String end_state;
	
protected:
	void _notification(int p_what);
    static void _bind_methods();

public:
    void run(bool is_prev);
    void to_prev();
    void to_next();

	void animation_changed(StringName old_name, StringName new_name);
   	void set_reinit_tree(bool p_reinit_tree);
    int get_states_count() const;
	
	void set_deformate_on_activation(bool p_deformate);
	bool get_deformate_on_activation() const;

	void travel_end();
	void travel_state();
	void travel_start();
	void _travel(const StringName &p_state);
    int get_id() const;
    void set_id(int np_id);
    void set_anim_tree(const NodePath &new_anim_tree);
    NodePath get_anim_tree() const;
	Ref<AnimationNodeStateMachinePlayback> get_playback() const;
	void set_start_transition (const String &p_start_transition);
	String get_start_transition ();
	void set_end_transition (const String &p_end_transition);
	String get_end_transition ();
	void set_end_state (const String &p_end_state);
	String get_end_state ();
	//void set_state (int p_id);
	String get_state (int p_id);
	String get_state ();
    void set_states(const Array &new_states);
    Array get_states() const;
	/*
	bool is_start_loop() const;
	bool is_starting() const;
	bool is_started() const;
	bool is_ending() const;
	bool is_ended() const;
	bool is_end_loop() const;

	void _start_loop();
	void _starting();
	void _started();
	void _ending();
	void _ended();
	void _end_loop();
	*/
	void set_focused(bool p_focused);
    bool is_focused() const;
	void set_active(bool p_active);
    bool is_active() const;
	REDFrame();
	~REDFrame();
};

VARIANT_ENUM_CAST(REDFrame::Anchor);
VARIANT_ENUM_CAST(REDFrame::Space);
#endif // RED_FRAME_H
