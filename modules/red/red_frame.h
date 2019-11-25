#ifndef RED_FRAME_H
#define RED_FRAME_H

#include "red_clipper.h"
#include "core/node_path.h"
#include "scene/animation/animation_node_state_machine.h"

class AnimationPlayer;

class REDFrame : public REDClipper {
	GDCLASS(REDFrame, REDClipper);
	Vector2 camera_pos;
	Vector2 camera_pos_zoom_in;
	Vector2 camera_pos_zoom_out;


	Vector2 parallax;

	Vector2 parallax_pos_current;
	Vector2 parallax_pos;
	Vector2 parallax_pos_zoom_in;
	Vector2 parallax_pos_zoom_out;

	Vector2 camera_zoom;

	Vector2 parallax_multiplayer;
	Vector2 current_parallax;
public:
	void set_camera_pos(const Vector2 &p_camera_pos);
	Vector2 get_camera_pos() const;
	void set_camera_pos_zoom_in(const Vector2 &p_camera_pos_zoom_in);
	Vector2 get_camera_pos_zoom_in() const;
	void  set_camera_pos_zoom_out(const Vector2 &p_camera_pos_zoom_out);
	Vector2  get_camera_pos_zoom_out() const;
	
	void set_parallax(const Vector2 &p_parallax);
	Vector2 get_parallax() const;
	float  get_scroll_scale() const;

	void set_parallax_pos_current(const Vector2 &p_parallax_pos);
	Vector2 get_parallax_pos_current() const;
	void set_parallax_pos(const Vector2 &p_parallax_pos);
	Vector2 get_parallax_pos() const;
	void set_parallax_pos_zoom_in(const Vector2 &p_parallax_pos_zoom_in);
	Vector2 get_parallax_pos_zoom_in() const;
	void  set_parallax_pos_zoom_out(const Vector2 &p_parallax_pos_zoom_out);
	Vector2  get_parallax_pos_zoom_out() const;

	void set_camera_zoom(const Vector2 &p_camera_zoom);
	Vector2 get_camera_zoom() const;


//line
public:
	void update_camera_zoom_and_child(const Vector2 &p_camera_zoom);
//Frame managment
private:
    bool reinit_tree;

	bool b_pre_starting;
    bool b_starting;
    bool b_started;
    bool b_ending;
    bool b_ended;
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

    void recalc_state();
    //void started();
    //void b_ended();
	void travel();
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
	void set_state (int p_id);
	String get_state (int p_id);
    void set_states(const Array &new_states);
    Array get_states() const;
	
	bool is_pre_starting() const;
	bool is_starting() const;
	bool is_started() const;
	bool is_ending() const;
	bool is_ended() const;

	void _pre_starting();
	void _starting();
	void _started();
	void _ending();
	void _ended();

	void set_active(bool p_active);
    bool is_active() const;
	REDFrame();
};
#endif // RED_FRAME_H
