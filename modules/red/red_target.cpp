#include "red_target.h"

void REDTarget::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			set_notify_local_transform(true);
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			emit_signal("_target_moved", get_transform());
			//emit_signal("_target_moved", get_transform());
		} break;
	}
}
void REDTarget::_bind_methods() {
	ADD_SIGNAL(MethodInfo("_target_moved"));
	//ADD_SIGNAL(MethodInfo("_target_moved", PropertyInfo(Variant::TRANSFORM2D, "target_transform")));
}
	
REDTarget::REDTarget() {
}
