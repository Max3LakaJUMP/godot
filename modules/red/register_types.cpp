/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "redstory.h"
#include "redvolume.h"
#include "redissue.h"
#include "redpage.h"
#include "red_frame.h"
#include "red_bubble.h"
#include "red.h"
#include "red_line.h"
#include "red_polygon.h"

void register_red_types() {
	ClassDB::register_class<REDPolygon>();
	ClassDB::register_class<REDLine>();
	ClassDB::register_class<REDStory>();
	ClassDB::register_class<REDVolume>();
	ClassDB::register_class<REDIssue>();
	ClassDB::register_class<REDPage>();
	ClassDB::register_class<REDFrame>();
	ClassDB::register_class<REDBubble>();
	ClassDB::register_class<RED>();
}

void unregister_red_types() {
	// Nothing to do here in this example.
}
