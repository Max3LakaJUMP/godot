#ifndef RED_PAGE_H
#define RED_PAGE_H

#include "scene/2d/node_2d.h"
#include "red_element.h"
#include "scene/resources/packed_scene.h"
#include "core/reference.h "

class REDFrame;

class REDPage : public REDElement {
	GDCLASS(REDPage, REDElement);
    Vector<NodePath> frames;

    int id;
    Size2 size;
    struct Date
    {
        int year;
        int month;
        int day;
    } creation_date;

    struct Meta
    {
        String name;
        String title;
        String character;
        String type;
        String additional;
        String additional2;
    } meta;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
    void pause_frames();
    void update_camera_pos(const Vector2 &p_camera_pos);
    void update_camera_zoom(const Vector2 &p_camera_zoom);
    void run();

    int get_frames_count();
    void set_frame(int p_id, bool force_inactive=true, bool ended=true);
    REDFrame *get_frame (int p_id);
    void set_frames(const Array &f);
    Array get_frames() const;
    void set_id(int num);
    int get_id() const;

    void set_size(const Size2 &p_size);
    Size2 get_size() const;

    REDPage();

};

#endif // RED_PAGE_H
