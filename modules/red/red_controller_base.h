#ifndef RED_CONTROLLER_BASE_H
#define RED_CONTROLLER_BASE_H

#include "scene/main/node.h"
#include "scene/2d/camera_2d.h"
#include "core/node_path.h"

class REDPage;
class REDFrame;
class REDIssue;
class Tween;

class REDControllerBase : public Node {
	GDCLASS(REDControllerBase, Node);
	REDIssue *issue;
    REDPage *page;
	REDFrame *frame;

    REDPage *prev_page;
    REDPage *next_page;

	NodePath camera_path;
	NodePath tween_path;
	Camera2D *camera;
	Tween *tween;

	bool camera_mode;
	bool camera_smooth;
	bool b_can_control;
	
	float camera_zoom;

	Vector2 camera_zoom_min;
	Vector2 camera_zoom_max;
	Vector2 camera_pos;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void frame_start(StringName old_name, StringName new_name);
	void frame_end(StringName old_name, StringName new_name);
	void update_camera();
	void update_camera_pos() const;
	void update_camera_zoom();
	//void unload_page(REDPage *page) const;
	//REDPage *load_page(const int &i, bool is_prev=false, int state=0);
	//void unload_pages();
	//void load_pages();

	bool to_prev_page();
    bool to_next_page();
	bool to_prev_frame();
    bool to_next_frame();
	bool to_prev_frame_state();
	bool to_next_frame_state();

	void to_next();
	void to_prev();
	void zoom_in();
	void zoom_out();
	void set_issue(REDIssue *p_issue);
	void set_issue_by_path(const NodePath &p_issue_path);
	REDIssue *get_issue() const;
	void set_page(int p_id, bool is_prev=false);
	REDPage *get_page() const;
	void set_frame(int p_id, bool ended=true);
	REDFrame *get_frame() const;
	void set_state(int p_id);

	void set_camera_path(const NodePath &p_camera_path);
	NodePath get_camera_path() const;
	void set_camera(Camera2D *p_camera);
	Camera2D *get_camera() const;

	void set_tween_path(const NodePath &p_tween_path);
	NodePath get_tween_path() const;
	void set_tween(Tween *p_tween);
	Tween *get_tween() const;
	
	void set_camera_mode(bool b);
	bool get_camera_mode() const;
	void set_camera_pos(const Vector2 &p_pos);
	Vector2 get_camera_pos() const;
	void set_camera_zoom(const float p_zoom);
	float get_camera_zoom() const;

	void set_camera_zoom_min(const Vector2 &p_zoom);
	Vector2 get_camera_zoom_min() const;
	void set_camera_zoom_max(const Vector2 &p_zoom);
	Vector2 get_camera_zoom_max() const;

	void set_camera_smooth(const bool p_smooth);
	bool get_camera_smooth() const;

	Vector2 clamp_camera_zoom(const Vector2 &p_zoom) const;
	Vector2 get_global_camera_zoom() const;
	bool can_control() const;
	REDControllerBase();
};

#endif // RED_CONTROLLER_BASE_H
