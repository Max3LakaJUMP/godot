#ifndef RED_CONTROLLER_H
#define RED_CONTROLLER_H

#include "scene/main/node.h"
#include "scene/2d/camera_2d.h"
#include "core/node_path.h"
#include "red_controller_base.h"

class REDPage;
class REDFrame;
class REDIssue;

class REDController : public REDControllerBase {
	GDCLASS(REDController, REDControllerBase);

protected:
	static void _bind_methods();

public:
	REDController();
};

#endif // RED_CONTROLLER_H
