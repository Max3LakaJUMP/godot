#ifndef RED_PAGE_H
#define RED_PAGE_H

#include "scene/2d/node_2d.h"
#include "redelement.h"
#include "scene/resources/packed_scene.h"
#include "core/reference.h "

class REDPage : public REDElement {
	GDCLASS(REDPage, REDElement);
    Vector<NodePath> frames;

    NodePath prev;
    NodePath current;
    NodePath next;
    int id;
    float height;


protected:
	static void _bind_methods();

public:
    int get_frames_count();
    void run(bool is_prev);
    void update_camera() const;
    void to_prev();
    void to_next();

    void pause_frames();
    void set_prev(const NodePath &frame);
    void set_current(const NodePath &frame);
    void set_next(const NodePath &frame);
    NodePath get_prev() const;
    NodePath get_current() const;
    NodePath get_next() const;

    void set_frames(const Array &f);
    void set_id(const int &f);
    float get_height() const;
    Array get_frames() const;
    int get_id() const;
    void set_height(const float f);


	REDPage();

};

#endif // RED_PAGE_H
