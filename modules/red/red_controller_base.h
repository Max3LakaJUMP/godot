#ifndef RED_CONTROLLER_BASE_H
#define RED_CONTROLLER_BASE_H

#include "scene/main/node.h"
#include "scene/2d/camera_2d.h"
#include "core/node_path.h"

#include "red_engine.h"
class REDPage;
class REDFrame;
class REDIssue;
class Tween;

class REDControllerBase : public Node {
	GDCLASS(REDControllerBase, Node);
public:
	enum ControllerDirrection{
		DIRRECTION_NONE,
		DIRRECTION_FORWARD,
		DIRRECTION_BACKWARD,
	};
	enum CameraState{
		CAMERA_STATIC,
		CAMERA_LOCKED,
		CAMERA_MOVING,
	};
	enum Orientation{
		ORIENTATION_MAXIMUM,
		ORIENTATION_MINIMUM,
		ORIENTATION_HORIZONTAL,
		ORIENTATION_VERTICAL,
		ORIENTATION_HYBRID,
	};
private:
	REDIssue *issue;
    REDPage *page;
	REDFrame *frame;
	NodePath camera_path;
	Camera2D *camera;
	Timer *frame_timer;
	Timer *camera_timer;
	Tween *tween;
	String group_name;

	bool reset_camera_on_frame_change;
	bool camera_mode;
	bool camera_smooth;
	bool b_can_control;
	Vector2 camera_zoom_min;
	Vector2 camera_zoom_max;

	Vector2 mouse_pos;
	Vector2 mouse_offset_k;

	Bouncer2D mouse_offset;
	Bouncer zoom_k;
	Bouncer orientation_k;
	mutable Bouncer2D camera_zoom;
	mutable Bouncer2D frame_pos_local;
	mutable Bouncer2D frame_parallax;

	int target_id;
	Vector2 frame_expand;

	Vector2 frame_pos_global;

	bool frame_changing;
	bool init_parallax_tween;
	bool frame_timer_connected;
	bool frame_next_timer_connected;
	bool frame_prev_timer_connected;
	float camera_speed;
	
	mutable float screen_multiplayer;
	mutable float screen_multiplayer_start;

	Vector2 old_camera_zoom;
	CameraState camera_state;
	ControllerDirrection dirrection;
	ControllerDirrection last_dirrection;
	Orientation orientation;

	void _input(const Ref<InputEvent> &p_event);

protected:
	void _camera_moved();
	static void _bind_methods();
	void _notification(int p_what);

public:
	Size2 get_viewport_size();
	void set_camera_speed(float p_camera_speed);
	float get_camera_speed() const;

	void set_reset_camera_on_frame_change(bool p_reset_camera);
	bool is_reset_camera_on_frame_change() const;

	void set_frame_expand(const Vector2 &p_frame_expand);
	Vector2 get_frame_expand() const;

	void set_mouse_offset_k(const Vector2 &p_max_mouse_offset);
	Vector2 get_mouse_offset_k() const;
	bool to_prev_page();
    bool to_next_page();
	bool to_prev_frame();
    bool to_next_frame();
	bool to_prev_frame_state();
	bool to_next_frame_state();

	void to_next();
	void to_prev();

	void _mouse_moved(const Vector2 &p_mouse_pos);
	void _update_frame_parallax();
	void _update_camera_zoom();
	void _update_frame_local_pos();
	void _update_mouse_pos();
	void zoom_in(float p_val=0.25f);
	void zoom_out(float p_val=0.25f);
	void set_issue(REDIssue *p_issue);
	void set_issue_by_path(const NodePath &p_issue_path);
	REDIssue *get_issue() const;

	void set_page(REDPage *p_page);
	void set_page(int p_id, bool is_prev=false);
	REDPage *get_page() const;
	void set_frame(int p_id, bool to_next=true);
	REDFrame *get_frame() const;

	void set_camera_path(const NodePath &p_camera_path);
	NodePath get_camera_path() const;
	void set_camera(Camera2D *p_camera);
	Camera2D *get_camera() const;

	void set_tween(Tween *p_tween);
	Tween *get_tween() const;
	
	void set_camera_mode(bool b);
	bool get_camera_mode() const;

	void set_camera_zoom_min(const Vector2 &p_zoom);
	Vector2 get_camera_zoom_min() const;
	void set_camera_zoom_max(const Vector2 &p_zoom);
	Vector2 get_camera_zoom_max() const;

	void set_camera_smooth(const bool p_smooth);
	bool get_camera_smooth() const;

	Vector2 clamp_camera_zoom(const Vector2 &p_zoom) const;
	Vector2 get_global_camera_zoom() const;

	bool can_control() const;

	void _frame_change();
	void _frame_changed();

	void _frame_start();
	void _frame_end();

	void update_camera_to_frame(const bool first_frame=false);

	void zoom_reset();

	Vector2 get_camera_global_pos() const;
	Vector2 get_camera_zoom() const;
	Vector2 get_mouse_offset() const;
	Vector2 get_parallax_offset() const;
	Vector2 get_parallax_zoom() const;

	void set_orientation(const Orientation p_orientation);
	Orientation get_orientation() const;
	float get_screen_multiplayer() const;
	virtual void _window_resized();
	void _update_orientation();
	void rotate_camera();
	REDControllerBase();
};

VARIANT_ENUM_CAST(REDControllerBase::Orientation);

#endif // RED_CONTROLLER_BASE_H
