#include "red_element.h"
#include "red.h"

void REDElement::_bind_methods() {
    /*
	ClassDB::bind_method(D_METHOD("set_id", "id"), &red_element::set_id);
	ClassDB::bind_method(D_METHOD("get_id"), &red_element::get_id);
	ClassDB::bind_method(D_METHOD("set_next", "next"), &red_element::set_next);
	ClassDB::bind_method(D_METHOD("get_next"), &red_element::get_next);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "next", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "red_element"), "set_next", "get_next");
     */
}
/*
void red_element::set_id(int i) {
id = id;
}

int red_element::get_id() const {
return id;
}

void red_element::set_prev(const NodePath &n) {
if (next == n)
    return;
prev = n;
}

NodePath red_element::get_prev() const {
return prev;
}
void red_element::set_next(const NodePath &n) {
if (next == n)
    return;
next = n;
}

NodePath red_element::get_next() const {
return next;
}
*/
REDElement::REDElement() {

}
