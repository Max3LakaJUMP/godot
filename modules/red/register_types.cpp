/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "red_story.h"
#include "red_volume.h"
#include "red_issue.h"
#include "red_page.h"
#include "red_frame.h"
#include "red_bubble.h"
#include "red.h"
#include "red_line.h"
#include "red_polygon.h"
#include "red_controller.h"
#include "red_controller_base.h"
#include "red_shape.h"
#include "red_clipper.h"
#include "red_parallax_folder.h"
#include "red_target.h"

void register_red_types() {
	ClassDB::register_virtual_class<REDShape>();
	ClassDB::register_class<REDClipper>();
	ClassDB::register_class<REDPolygon>();
	ClassDB::register_class<REDLine>();
	ClassDB::register_class<REDStory>();
	ClassDB::register_class<REDVolume>();
	ClassDB::register_class<REDIssue>();
	ClassDB::register_class<REDPage>();
	ClassDB::register_class<REDFrame>();
	ClassDB::register_class<REDBubble>();
	ClassDB::register_virtual_class<REDControllerBase>();
	ClassDB::register_class<REDController>();
	ClassDB::register_class<REDParallaxFolder>();
	ClassDB::register_class<REDTarget>();
	ClassDB::register_class<RED>();
}

void unregister_red_types() {
	// Nothing to do here in this example.
}
