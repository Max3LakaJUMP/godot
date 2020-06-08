#ifndef RED_ENGINE_H
#define RED_ENGINE_H

#include "core/node_path.h"
#include "core/ustring.h"
#include "core/color.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/pool_vector.h"
#include "core/dictionary.h"
#include "scene/main/node.h"

class Camera2D;
class RED;
class REDPage;
class REDFrame;
class REDControllerBase;

namespace red {
enum t {
	STORY,
	VOLUME,
	ISSUE,
	PAGE,
	FRAME
};
PoolVector<Vector2> new_uv(PoolVector<Vector2> old_polygon, PoolVector<Vector2> new_polygon, PoolVector<Vector2> old_uv, Size2 poly_size, Size2 uv_size);
Rect2 get_rect(PoolVector<Vector2> polygon, Vector2 offset = Vector2());

void print(const int number);
void print(const String number);
String str(const int number);
Node *get_singleton(const Node *n);
RED *get_red(const Node *n);
Camera2D *get_camera(const Node *n);
REDControllerBase *get_controller(const Node *n);
Vector2 get_zoom(const Node *n);



void get_all_children(const Node *node, Vector<Node*> &output);

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
PoolVector<float> pool_real_array(Array &p_arr);

String globalize(String &p_path);
String localize(String &p_path);
template <class T>
T *create_node(Node *parent, const String &name="", Node *owner=nullptr){
	if (parent){
		NodePath p(name);
		if (parent->has_node(p)){
			return Object::cast_to<T>(parent->get_node(p));
		}
		T *node = memnew(T);
		if (name != ""){
			node->set_name(name);
		}
		if (owner==nullptr){
			owner = parent->get_owner();
			if (!owner){
				owner = parent;
			}
		}
		parent->add_child(node);
		node->set_owner(owner);
		return node;
	}
	return NULL;
}

template <class T>
Vector<T> vector(const Array p_array) {
	int count = p_array.size();
	Vector<T> result;
	result.resize(count);
	for (int i = 0; i < count; i++) {
		result.write[i] = p_array[i];
	}
	return result;
}

template <class T>
Array array(const Vector<T> p_vector) {
	int count = p_vector.size();
	Array result;
	result.resize(count);
	for (int i = 0; i < count; i++) {
		result[i] = p_vector[i];
	}
	return result;
}
} // namespace red
#endif // RED_ENGINE_H
