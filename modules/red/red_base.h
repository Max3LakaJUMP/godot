#ifndef RED_BASE_H
#define RED_BASE_H

#include "scene/main/node.h"
#include "scene/2d/camera_2d.h"
#include "core/node_path.h"

class REDPage;
class REDFrame;

class REDBase : public Node2D {
	GDCLASS(REDBase, Node2D);
	//Point2 pivot_pos;
	//Point2 applied_pivot;
	////bool pivot_changed;

protected:
	//void _notification(int p_what);

public:
	//virtual void _edit_set_pivot(const Point2 &p_pivot);
	//virtual Point2 _edit_get_pivot() const;
	//virtual bool _edit_use_pivot() const;

	REDBase();
};

#endif // RED_BASE_H
