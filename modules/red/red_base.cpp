#include "red_base.h"
#include "scene/2d/camera_2d.h"
/*
void REDShape::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
            applied_pivot = pivot_pos;
			if (use_outline){
				Vector<Vector2> points;
				int count = get_polygon().size();
				points.resize(count);
				PoolVector<Vector2>::Read polyr = polygon.read();
				for (int i = 0; i < count; i++)
					points.write[i] = polyr[i] + get_offset();
				_draw_outline(points);
			}
		} break;
	}
}

void REDBase::_edit_set_pivot(const Point2 &p_pivot) {
    if (!pivot_changed){
        applied_pivot = pivot_pos;
    }
    Point2 move = p_pivot-applied_pivot;
    get_transform().xform(move);
	for (int i = 0; i < get_child_count(); i++)
	{
		Node2D *node = (Node2D*)get_child(i);
		if (!node)
			continue;
		node.xform(-move);
	}

    applied_pivot = pivot_pos;
    pivot_pos = p_pivot;
    pivot_changed = true;
}

Point2 REDBase::_edit_get_pivot() const {
	return pivot_pos;
}

bool REDBase::_edit_use_pivot() const {
	return true;
}*/

REDBase::REDBase() {
    //pivot_changed = false;
    //pivot_pos = Point2(0,0);
    //applied_pivot = Point2(0,0);
}
