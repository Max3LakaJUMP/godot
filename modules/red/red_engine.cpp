#include "red_engine.h"
#include "red.h"
#include "red_controller_base.h"
#include "red_page.h"
#include "red_frame.h"
#include "core/node_path.h"
#include "core/node_path.h"

#include "core/bind/core_bind.h"
#include "core/math/vector3.h"
#include "core/pool_vector.h"
#include "scene/2d/camera_2d.h"

namespace red {

RED *get_red(const Node *n) {
	Node *root = n->get_tree()->get_root();
	Node *r = root->find_node(NodePath("RED"));
	if (r==NULL)
		return nullptr;
	else
		return (RED*)r;
}

REDControllerBase *get_controller(const Node *n) {
	Node *root = n->get_tree()->get_root();
	if (root->has_node(NodePath("/root/red/RED"))){
		RED *r = (RED*)(root->get_node(NodePath("/root/red/RED")));
		NodePath controller_path = r->get_controller_path();
		if (!controller_path.is_empty())
			return (REDControllerBase*)(r->get_node(controller_path));
	}
	return nullptr;
}

Camera2D *get_camera(const Node *n) {
	REDControllerBase* controller = get_controller(n);
	if (controller != nullptr){
		return controller->get_camera();
	}
	return nullptr;
}

void get_all_children(const Node *node, Vector<Node*> &output){
	ERR_FAIL_COND(!node);
	int count = node->get_child_count();
	for (int i = 0; i < count; i++)
	{
		Node *child = node->get_child(i);
		output.push_back(child);
		get_all_children(child, output);
	}
}

Error create_dir(String dir){
	DirAccess *da = DirAccess::open("res://");
	if (da->change_dir(dir) != OK) {
		Error err = da->make_dir(dir);
		if (err) {
			memdelete(da);
			ERR_EXPLAIN("Failed to create target folder " + dir);
			ERR_FAIL_V(FAILED);
		}
	}
	memdelete(da);
	return OK;
}

void scene_loader(String &scene_path){
	_File file2Check;
	if (!file2Check.file_exists(scene_path)){
		Ref<PackedScene> scene = memnew(PackedScene);
		Node2D *root_node = memnew(Node2D);
		Error err;
		root_node->set_name("Node2D");

		err = scene->pack(root_node);
		ERR_FAIL_COND(err != OK);

		err = ResourceSaver::save(scene_path, scene);
		ERR_FAIL_COND(err != OK);
	}
}

Vector2 vector2(const Dictionary &p_dict){
	return Vector2(p_dict["x"], p_dict["y"]);
}

Color color(const Dictionary &p_dict){
	return Color(p_dict["r"], p_dict["g"], p_dict["b"], p_dict["a"]);
}

float get_z(const Dictionary &p_dict){
	return p_dict["z"];
}

Dictionary dict(Vector2 &p_vector2){
	Dictionary d;
	Vector2 w;
	d["x"] = p_vector2.x;
	d["y"] = p_vector2.y;
	return d;
}

Dictionary dict(Vector3 &p_vector3){
	Dictionary d;
	Vector2 w;
	d["x"] = p_vector3.x;
	d["y"] = p_vector3.y;
	d["z"] = p_vector3.z;
	return d;
}

Dictionary dict(Vector2 &p_vector2, float &p_z){
	Dictionary d;
	Vector2 w;
	d["x"] = p_vector2.x;
	d["y"] = p_vector2.y;
	d["z"] = p_z;
	return d;
}

Dictionary dict(Color &p_color){
	Dictionary d;
	Vector2 w;
	d["r"] = p_color.r;
	d["g"] = p_color.g;
	d["b"] = p_color.b;
	d["a"] = p_color.a;
	return d;
}

Array arr(PoolVector<Vector2> &p_value){
	Array result;
	int value_count = p_value.size();
	result.resize(value_count);
	for (int i = 0; i < value_count; i++){
		Dictionary dict;
		Vector2 el = p_value[i];
		result.append(red::dict(el));
	}
	return result;
}		
PoolVector<Vector2> pool_vector2_array(Array &p_arr){
	Array array;
	int count = p_arr.size();
	PoolVector<Vector2> pool;
	pool.resize(count);
	PoolVector<Vector2>::Write w = pool.write();
	for (int i = 0; i < count; i++){
		Vector2 el = p_arr[i];
		w[i] = el;
	}
	return pool;
}
PoolVector<Color> pool_color_array(Array &p_arr){
	Array array;
	int count = p_arr.size();
	PoolVector<Color> pool;
	pool.resize(count);
	PoolVector<Color>::Write w = pool.write();
	for (int i = 0; i < count; i++){
		Color el = p_arr[i];
		w[i] = el;
	}
	return pool;
}

PoolVector<float> pool_real_array(Array &p_arr){
	Array array;
	int count = p_arr.size();
	PoolVector<float> pool;
	pool.resize(count);
	PoolVector<float>::Write w = pool.write();
	for (int i = 0; i < count; i++){
		float el = p_arr[i];
		w[i] = el;
	}
	return pool;
}
// RECT2, TRANSFORM2D, PLANE, QUAT, AABB, BASIS, TRANSFORM, NODE_PATH, _RID, OBJECT, DICTIONARY, ARRAY, VARIANT_MAX
/*
Variant dict(Variant &p_value){
	Variant output;
	switch (value->get_type()) {
		case Variant::BOOL: 
		case Variant::INT: 
		case Variant::REAL: 
		case Variant::STRING: 		
		{
			Vector2 o = p_value;
			output = dict(o);
			break;
		}
		case Variant::VECTOR2: {
			Vector2 o = p_value;
			output = dict(o);
			break;
		}
		case Variant::VECTOR3: {
			Vector3 o = p_value;
			output = dict(o);
			break;
		}
		case Variant::COLOR: {
			Color o = p_value;
			output = dict(o);
			break;
		}
		case Variant::POOL_VECTOR2_ARRAY: {
			PoolVector<Vector2> o = p_value;
			output = dict(o);
			break;
		}
		default: {
			break;
		}
	}
	return output;
}

void dict(Variant &p_value, Array &output){

}
*/
String globalize(String &p_path){
	ProjectSettings *settings = ProjectSettings::get_singleton();
	if (settings)
		return settings->globalize_path(p_path);
	else
		ERR_PRINTS("Error can't globalize: " + p_path);
		return p_path;
}
String localize(String &p_path){
	ProjectSettings *settings = ProjectSettings::get_singleton();
	if (settings)
		return settings->localize_path(p_path);
	else
		ERR_PRINTS("Error can't localize: " + p_path);
		return p_path;
}
/*
REDPage *get_page_from_scene(const Node &n) {
	return Object::cast_to<REDPage>(n.get_child(0));
}

NodePath get_page_path_from_scene(const Node &n) {
	return n.get_child(0)->get_path();
}

void next(t t, const Node &node) {
	switch (t) {
		case FRAME: {
			RED *r = get_red(node);
			REDFrame *f = Object::cast_to<REDFrame>(r->get_current_frame());
			REDFrame *n = Object::cast_to<REDFrame>(node.get_node(f->get_next()));
			r->set_current_frame(*n);

		} break;
		case PAGE:
			// do something
			break;
	}
}
*/
} // namespace red
