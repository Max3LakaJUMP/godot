#ifndef RED_ISSUE_H
#define RED_ISSUE_H

#include "red_folder.h"
#include "scene/resources/packed_scene.h"
#include "core/object.h"

class RED;
class REDPage;

class REDIssue : public REDFolder {
	GDCLASS(REDIssue, REDFolder);
	Vector<Ref<PackedScene> > page_scenes;
    Vector<REDPage*> pages;
    Vector<bool> instanced_list;

    Vector<Vector2> pages_pos;

	int id;
    int instance_count;
    Vector2 pages_margin;
    void resize_instanced_list(int p_size);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
    void run();
    Vector2 get_pages_margin() const;
    void set_pages_margin(const Vector2 &p_pages_margin);
    int get_instance_count() const;
    void set_instance_count(const int &p_instance_count);

    bool get_instanced_list (int p_id);
    REDPage *get_page (int p_id);
    void set_page (int p_id, bool is_prev=false, const Vector2 &p_zoom=Vector2(1.0, 1.0));
    void set_page_scenes(const Array &pages_new);
    Array get_page_scenes() const;

    Ref<PackedScene> get_page_scene (int p_id);

    int get_pages_count();
    int get_page_scenes_count();
    void update_camera_zoom(const Vector2 &p_zoom=Vector2(1.0, 1.0));
    void set_pages(const Array &pages_new);
    Array get_pages() const;
    void set_pages_pos(const Array &y);
    Array get_pages_pos() const;
    void set_id(const int &p_id);
    int get_id() const;
    void load_page(int p_id, bool is_prev=false, const Vector2 &p_zoom=Vector2(1.0, 1.0));
	void unload_page(int p_id);
	void load_pages();
	void unload_pages();

	void update_camera_pos() const;
	void to_prev();
	void to_next();


	REDIssue();
};

#endif // RED_ISSUE_H
