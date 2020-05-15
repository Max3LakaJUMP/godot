#ifndef RED_ELEMENT_H
#define RED_ELEMENT_H

#include "scene/2d/node_2d.h"
#include "core/node_path.h"
#include "red.h"
#include "red_base.h"

class REDElement : public REDBase {
	GDCLASS(REDElement, REDBase);

protected:
	static void _bind_methods();

public:
    /*
	void set_id(int i);
	int get_id() const;

	NodePath get_prev() const;
	void set_prev(const NodePath &n);

	void set_next(const NodePath &n);
	NodePath get_next() const;
    */
	REDElement();
};

#endif // RED_ELEMENT_H
