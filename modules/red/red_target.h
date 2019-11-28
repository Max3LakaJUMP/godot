#ifndef RED_TARGET_H
#define RED_TARGET_H

#include "scene/2d/position_2d.h"

class REDTarget : public Position2D {
	GDCLASS(REDTarget, Position2D);

protected:
	void _notification(int p_what);
    static void _bind_methods();
public:

	REDTarget();
};

#endif // RED_TARGET_H
