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
	NodePath issue;
	NodePath page;
	NodePath frame;

	bool camera_mode;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
    void next_frame();
    void prev_frame();
	//void run_frame();
	void attach_camera();

    void update_camera() const;

	void set_camera_mode(bool b);
	bool get_camera_mode() const;
	void set_vertical_mode(bool b);
	bool get_vertical_mode() const;
	void set_html_mode(bool b);
	bool get_html_mode() const;
	void set_camera(const NodePath &c);
	NodePath get_camera() const;




	void set_current_issue_path(const NodePath &i);
	NodePath get_current_issue_path() const;
	void set_current_page_path(const NodePath &p);
	NodePath get_current_page_path() const;
    void set_current_frame_path(const NodePath &p);
    NodePath get_current_frame_path() const;

    REDIssue *get_current_issue() const;
    void set_current_frame(const Node &f);
    REDFrame *get_current_frame() const;
	void set_current_page(const REDPage &p);
	REDPage *get_current_page() const;
	//void set_current_frame(const REDFrame &f);
	RED();
};

#endif // RED_H
