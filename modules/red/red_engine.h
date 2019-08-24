#ifndef RED_ENGINE_H
#define RED_ENGINE_H

#include "core/node_path.h"
#include "core/ustring.h"
#include "core/color.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/pool_vector.h"
#include "core/dictionary.h"


class RED;
class REDPage;
class REDFrame;
class Node;

namespace red {
enum t {
	STORY,
	VOLUME,
	ISSUE,
	PAGE,
	FRAME
};
RED *get_red(const Node &n);
REDPage *get_page_from_scene(const Node &n);
NodePath get_page_path_from_scene(const Node &n);
void next(t t, const Node &node);
Error create_dir(String dir);

void scene_loader(String &scene_path);
Vector2 vector2(const Dictionary &p_dict);
Color color(const Dictionary &p_dict);
float get_z(const Dictionary &p_dict);

Dictionary dict(Vector2 &p_vector2);
Dictionary dict(Size2 &p_size);
Dictionary dict(Vector3 &p_vector3);
Dictionary dict(Vector2 &p_vector2, float &p_z);
Dictionary dict(Color &p_color);

Array arr(PoolVector<Vector2> &p_value);	

PoolVector<Vector2> pool_vector2_array(Array &p_arr);
PoolVector<Color> pool_color_array(Array &p_arr);

String globalize(String &p_path);
String localize(String &p_path);
template <class T>
T *create_node(Node *parent=nullptr, const String &name="", Node *owner=nullptr){
	T *node = memnew(T);
	if (name != ""){
		node->set_name(name);
	}
	if (parent!=nullptr){
		if (owner==nullptr){
			owner = parent->get_owner();
			if (!owner){
				owner = parent;
			}
		}
		parent->add_child(node);
		node->set_owner(owner);
	}
    return node;
}
} // namespace red
#endif // RED_ENGINE_H
