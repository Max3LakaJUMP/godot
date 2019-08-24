#ifndef RED_BASE_H
#define RED_BASE_H

#include "scene/main/node.h"
#include "scene/2d/camera_2d.h"
#include "core/node_path.h"

class REDPage;
class REDFrame;

class REDBase : public Node2D {
	GDCLASS(REDBase, Node2D);


protected:

public:
	REDBase();
};

#endif // RED_BASE_H
