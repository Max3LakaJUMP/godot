#include "redelement.h"
#include "red.h"

void REDElement::_bind_methods() {
    /*
	ClassDB::bind_method(D_METHOD("set_id", "id"), &REDElement::set_id);
	ClassDB::bind_method(D_METHOD("get_id"), &REDElement::get_id);
	ClassDB::bind_method(D_METHOD("set_next", "next"), &REDElement::set_next);
	ClassDB::bind_method(D_METHOD("get_next"), &REDElement::get_next);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "id"), "set_id", "get_id");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "next", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "REDElement"), "set_next", "get_next");
     */
}
/*
void REDElement::set_id(int i) {
id = id;
}

int REDElement::get_id() const {
return id;
}

void REDElement::set_prev(const NodePath &n) {
if (next == n)
    return;
prev = n;
}

NodePath REDElement::get_prev() const {
return prev;
}
void REDElement::set_next(const NodePath &n) {
if (next == n)
    return;
next = n;
}

NodePath REDElement::get_next() const {
return next;
}
*/
REDElement::REDElement() {

}
