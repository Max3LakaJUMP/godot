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
#include "scene/resources/bit_map.h"
#include <string>

namespace red {

Vector<Vector2> ramer_douglas_peucker(Vector<Vector2> &pointList, double epsilon, bool is_closed) {
	Vector<Vector2> resultList;
	if(is_closed){
		pointList.push_back(pointList[0]);
		pointList.push_back(pointList[1]);
	}
	float dmax = 0;
	int index = 0;
	for (int i = 1; i < pointList.size() - 1; ++i)
	{
		Point2 vec1 = Point2(pointList[i].x -pointList[0].x, pointList[i].y - pointList[0].y);
		Point2 vec2 = Point2(pointList[pointList.size() - 1].x - pointList[0].x, pointList[pointList.size() - 1].y - pointList[0].y);
		float d_vec2 = sqrt(vec2.x*vec2.x + vec2.y*vec2.y);
		float cross_product = vec1.x*vec2.y - vec2.x*vec1.y;
		float d = abs(cross_product / d_vec2);
		if (d > dmax) {
			index = i;
			dmax = d;
		}
	}
	// If max distance is greater than epsilon, recursively simplify
	if (dmax > epsilon)
	{
		Vector<Vector2> pre_part, next_part;
		for (int i = 0; i <= index; ++i) pre_part.push_back(pointList[i]);
		for (int i = index; i < pointList.size(); ++i) next_part.push_back(pointList[i]);
		Vector<Vector2> resultList1 = ramer_douglas_peucker(pre_part, epsilon);
		Vector<Vector2> resultList2 = ramer_douglas_peucker(next_part, epsilon);
		resultList.append_array(resultList1);
		for (int i = 1; i < resultList2.size(); ++i) resultList.push_back(resultList2[i]);
	}
	else
	{
		resultList.push_back(pointList[0]);
		resultList.push_back(pointList[pointList.size() - 1]);
	}
	if(is_closed){
		pointList.resize(pointList.size()-2);
		resultList.resize(resultList.size()-2);
	}
	return resultList;
}

Vector<PoolVector<Vector2> > bitmap_to_polygon(Ref<BitMap> bitmap_mask, Size2 &polygon_size, float polygon_grow, float epsilon, bool single){
	Vector<PoolVector<Vector2> > result;
	Vector<Vector<Vector2> > result_pre;
	if (bitmap_mask.is_valid()){
		float grow = bitmap_mask->get_size().width * polygon_grow / 128.0;
		if (grow != 0){
			Rect2 r = Rect2(Point2(0, 0), bitmap_mask->get_size());
			if (polygon_grow < 0){
				bitmap_mask->shrink_mask(Math::round(ABS(grow)), r);
			}else{
				bitmap_mask->grow_mask(Math::round(grow), r);
			}
		}
		if (bitmap_mask.is_valid()){
			Vector<Vector<Vector2> > polygon_array = bitmap_mask->clip_opaque_to_polygons(Rect2(Point2(), bitmap_mask->get_size()));
			int count = polygon_array.size();
			if ((count == 1 && single) || !single){
				for (int i = 0; i < count; i++)
				{
					if (polygon_array[i].size() > 3 && epsilon > 0.0){
						Vector<Vector2> in_array = polygon_array[i];
						Vector<Vector2> out_array = ramer_douglas_peucker(in_array, epsilon, true);
						result_pre.push_back(out_array);
					}
					else if (polygon_array[i].size() > 2){
						result_pre.push_back(polygon_array[i]);
					}
				}
			}
		}
	}
	{
		int count = result_pre.size();
		if (count > 0){
			for (size_t i = 0; i < count; i++){
				if (result_pre[i].size() > 3){
					PoolVector<Vector2> polygon;
					Vector2 k = polygon_size / bitmap_mask->get_size();
					for (int j = 0; j < result_pre[i].size(); j++){
						polygon.append(result_pre[i][j] * k);
					}
					result.push_back(polygon);
				}
			}
		} 
	}
	{
		int count = result.size();
		if (count == 0){
			PoolVector<Vector2> polygon;
			polygon.append(Vector2(0,0));
			polygon.append(Vector2(polygon_size.x, 0));
			polygon.append(polygon_size);
			polygon.append(Vector2(0, polygon_size.y));
			result.push_back(polygon);
		}
	}
	return result;
}


PoolVector<Vector2> new_uv(PoolVector<Vector2> old_polygon, PoolVector<Vector2> new_polygon, PoolVector<Vector2> old_uv, Size2 poly_size, Size2 uv_size) {
	int len = new_polygon.size();
	PoolVector<Vector2> uvs;
	uvs.resize(len);
	PoolVector<Vector2>::Write uvs_w = uvs.write();
	PoolVector<Vector2>::Read uvs_r = old_uv.read();
	PoolVector<Vector2>::Read polygon_r = old_polygon.read();
	PoolVector<Vector2>::Read new_polygon_r = new_polygon.read();
	for (int i = 0; i < len; i++){
		Vector2 target_uv = Vector2(0,0);
		if (old_uv.size() > i){
			target_uv += uvs_r[i];
		}	
		if (old_polygon.size() > i){
			target_uv += (new_polygon_r[i] - polygon_r[i]) * uv_size / poly_size;
		}else{
			target_uv += new_polygon_r[i] * uv_size / poly_size;
		}
		uvs_w[i] = target_uv;
	}
	return uvs;
}


Rect2 get_rect(PoolVector<Vector2> polygon, Vector2 offset) {
	int l = polygon.size();
	PoolVector<Vector2>::Read r = polygon.read();
	Rect2 item_rect = Rect2();
	for (int i = 0; i < l; i++) {
		Vector2 pos = r[i] + offset;
		if (i == 0)
			item_rect.position = pos;
		else
			item_rect.expand_to(pos);
	}
	return item_rect;
}

void print(const float number) {
	print_line(red::str(number));
}

void print(const String number) {
	print_line(number);
}

String str(const float number) {
	return String(std::to_string(number).c_str());
}

String str(const int number) {
	return String(std::to_string(number).c_str());
}

RED *red(const Node *n) {
	Node *r = n->get_tree()->get_root()->get_node(NodePath("red"));
	if (r == NULL)
		return nullptr;
	else
		return (RED*)r;
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
			ERR_PRINTS("Failed to create target folder " + dir);
			ERR_FAIL_V(FAILED);
		}
	}
	memdelete(da);
	return OK;
}

void scene_loader(const String &scene_path){
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
