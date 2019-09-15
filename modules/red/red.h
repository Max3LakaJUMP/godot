#ifndef RED_H
#define RED_H

#include "scene/main/node.h"
#include "scene/2d/camera_2d.h"
#include "core/node_path.h"


class REDPage;
class REDFrame;
class REDIssue;

class RED : public Node {
	GDCLASS(RED, Node);

	bool vertical_mode;
	bool html_mode;

	NodePath camera;
	NodePath controller_path;

	NodePath issue;
	NodePath page;
	NodePath frame;

	bool camera_mode;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	NodePath get_controller_path();
	void set_controller_path(const NodePath &p_controller_path);
	//void run_frame();
	void attach_camera();

    void update_camera() const;
	void update_camera_to(const REDFrame *node) const;
	void set_camera_mode(bool b);
	bool get_camera_mode() const;
	void set_vertical_mode(bool b);
	bool get_vertical_mode() const;
	void set_html_mode(bool b);
	bool get_html_mode() const;
	void set_camera(const NodePath &c);
	NodePath get_camera() const;
	RED();
};

#endif // RED_H
