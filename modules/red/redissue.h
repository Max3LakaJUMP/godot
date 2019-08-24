#ifndef RED_ISSUE_H
#define RED_ISSUE_H

#include "redfolder.h"
#include "scene/resources/packed_scene.h"
#include "core/object.h"

class RED;
class REDPage;

class REDIssue : public REDFolder {
	GDCLASS(REDIssue, REDFolder);
	Vector<Ref<PackedScene> > pages;
    Vector<float> y_offset;




    NodePath prev;
    NodePath current;
    NodePath next;
	int id;


protected:
	static void _bind_methods();

public:
    int get_pages_count();

    void set_prev(const NodePath &page);
    void set_current(const NodePath &page);
    void set_next(const NodePath &page);
    NodePath get_prev() const;
    NodePath get_current() const;
    NodePath get_next() const;
    void set_pages(const Array &pages_new);
    Array get_pages() const;
    void set_y_offset(const Array &y);
    Array get_y_offset() const;
    void set_id(const int &num);
    int get_id() const;

    NodePath load_page(const int &i, bool is_prev=false);
	void unload_page(Node *page)  const;
	void load_pages();
	void unload_pages();
	void run();
	void update_camera() const;
	void to_prev();
	void to_next();


	REDIssue();
};

#endif // RED_ISSUE_H
