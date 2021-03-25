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
#include "scene/resources/bit_map.h"

class Camera2D;
class RED;
class REDPage;
class REDFrame;
class REDControllerBase;
class Polygon2D;

// struct Edge{
// 	Vector3 start;
// 	Vector3 end;

// 	bool operator==(const Edge &p_edge) const {
// 		return (start == p_edge.start && end == p_edge.end) || (start == p_edge.end && end == p_edge.start);
// 	}
// }

class TransformC : public Transform{
public:
	// enum ROTATION_ORDER{
	// 	DEFAULT_ROTATION,
	// 	ZYX
	// };
private:
	bool _xform_dirty;
	Vector3 rotation;
	Vector3 scale;
	// ROTATION_ORDER rotation_order;
public:
	// void set_rotation_order(const ROTATION_ORDER &p_rotation_order) {
	// 	rotation_order = p_rotation_order;
	// 	_xform_dirty = true;
	// }

	// void get_rotation_order() const{
	// 	return rotation_order;
	// }

	void _update_xform_values() {

		// switch (ROTATION_ORDER)
		// {
		// case ROTATION_ORDER::ZYX:{
		// 	Basis m = basis.orthonormalized();
		// 	real_t det = m.determinant();
		// 	if (det < 0)
		// 		m.scale(Vector3(-1, -1, -1));
		// 	rotation = m.get_euler_zyx();
		// } break;
		// default:
			// rotation = get_basis().get_rotation();
		// 	break;
		// }	
		rotation = get_basis().get_rotation();
		scale = get_basis().get_scale();
		_xform_dirty = false;
	}

	void set_transform(const Transform &p_transform) {
		set_basis(p_transform.basis);
		set_origin(p_transform.origin);
		_xform_dirty = true;
	}

	void set_rotation(const Vector3 &p_radians) {
		if (_xform_dirty)
			((TransformC *)this)->_update_xform_values();
		rotation = p_radians;
		// switch (ROTATION_ORDER)
		// {
		// case ROTATION_ORDER::ZYX:{
		// 	basis.set_euler_zyx(rotation);
		// } break;
		// default:
		// 	basis.set_euler(rotation);
		// 	break;
		// }	
		basis.set_euler_scale(rotation, scale);

	}

	Vector3 get_rotation() const {
		if (_xform_dirty)
			((TransformC *)this)->_update_xform_values();

		return rotation;
	}

	void set_scale(const Vector3 &p_scale) {

		if (_xform_dirty)
			((TransformC *)this)->_update_xform_values();
		Vector3 temp(p_scale);
		if (temp.x == 0)
			temp.x = CMP_EPSILON;
		if (temp.y == 0)
			temp.y = CMP_EPSILON;
		if (temp.z == 0)
			temp.z = CMP_EPSILON;
		// basis.orthonormalize();
		// basis.scale(temp);
		basis.set_euler_scale(rotation, temp);
		// basis.scale(temp / scale);
		scale = p_scale;
	}

	Vector3 get_scale() const {
		if (_xform_dirty)
			((TransformC *)this)->_update_xform_values();

		return scale;
	}

	void set_rotation_degrees(const Vector3 &p_degrees) {
		set_rotation(Vector3(Math::deg2rad(p_degrees.x), Math::deg2rad(p_degrees.y), Math::deg2rad(p_degrees.z)));
	}

	Vector3 get_rotation_degrees() const {
		Vector3 r = get_rotation();
		return Vector3(Math::rad2deg(r.x), Math::rad2deg(r.y), Math::rad2deg(r.z));
	}

	TransformC(const Transform &p_transform) : Transform(p_transform)  {_xform_dirty = true;}
	TransformC() {
		rotation = Vector3(0, 0, 0);
		scale = Vector3(1, 1, 1);

		// rotation_order = DEFAULT_ROTATION;
	}
};

namespace red {
enum t {
	STORY,
	VOLUME,
	ISSUE,
	PAGE,
	FRAME
};

enum TesselateMode {
	CUBIC = 0,
	QUADRATIC_BEZIER,
	CUBIC_BEZIER
};

template <class T>
T *find_parent_type(Node *node, int max_iter=10) {
	Node *parent = node->get_parent();
	for (int i = 0; i < max_iter; i++){
		T *n = Object::cast_to<T>(parent);
		if (n){
			return n;
		}
		parent = parent->get_parent();
	}
	return nullptr;
}

PoolByteArray to_ascii(const String &input_string);
Vector2 cubic_bezier(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, const Vector2 &p3, float t);
Vector2 quadratic_bezier(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, float t);
Vector<Vector2> tesselate(const Vector<Vector2> &low_res, const int smooth_factor=1, const TesselateMode &p_interpolation=TesselateMode::CUBIC);

Ref<BitMap> read_bitmap(Ref<Image> p_img, float p_threshold=0.1f, Size2 max_size=Size2(128.0f, 128.0f));
PoolColorArray get_normals(PoolVector2Array vtxs);
Vector<Polygon2D*> bitmap_to_polygon2d(Ref<BitMap> bitmap_mask, Size2 &polygon_size, float polygon_grow=1.0, float epsilon=4.0, bool single=false, bool normals_to_colors=false, Rect2 &crop_rect=Rect2());
Ref<Image> merge_images(Ref<Image> main_image, Vector<Ref<Image> > &additional_images, Vector<Vector2> &offsets);
Vector<Vector2> ramer_douglas_peucker(const Vector<Vector2> &p_point_list, double epsilon=2.0, bool is_closed=false);
Vector<PoolVector<Vector2> > bitmap_to_polygon(Ref<BitMap> bitmap_mask, Size2 &polygon_size, float polygon_grow=0, float epsilon=4.0, bool single=true, Rect2 &crop_rect=Rect2());

PoolVector<Vector2> new_uv(PoolVector<Vector2> old_polygon, PoolVector<Vector2> new_polygon, PoolVector<Vector2> old_uv, Size2 poly_size, Size2 uv_size);
Rect2 get_rect(PoolVector<Vector2> polygon, Vector2 offset = Vector2());
Size2 get_full_size(PoolVector<Vector2> &polygon, PoolVector<Vector2> &uv);

Node *get_singleton(const Node *n);
RED *red(const Node *n);
Vector2 get_zoom(const Node *n);

void get_all_children(const Node *node, Vector<Node*> &output);

REDPage *get_page_from_scene(const Node &n);
NodePath get_page_path_from_scene(const Node &n);
void next(t t, const Node &node);
Error create_dir(String dir);

void scene_loader(const String &scene_path);
Vector2 vector2(const Dictionary &p_dict);
Color color(const Dictionary &p_dict);
float get_z(const Dictionary &p_dict);

Dictionary dict(Vector2 &p_vector2);
Dictionary dict(Size2 &p_size);
Dictionary dict(Vector3 &p_vector3);
Dictionary dict(Vector2 &p_vector2, float &p_z);
Dictionary dict(Color &p_color);

Array arr(PoolVector<Vector2> &p_value);	
PoolVector<int> pool_int_array(Array &p_arr);
PoolVector<Vector2> pool_vector2_array(Array &p_arr);
PoolVector<Color> pool_color_array(Array &p_arr);
PoolVector<float> pool_real_array(Array &p_arr);

String globalize(const String &p_path);
String localize(const String &p_path);
template <class T>
T *create_node(Node *parent, const String &name="", Node *owner=nullptr, bool force=false){
	if (parent){
		NodePath p(name);
		if(!force){
			if (parent->has_node(p)){
				return Object::cast_to<T>(parent->get_node(p));
			}
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
	ERR_FAIL_V(nullptr);
}

template <class T>
Vector<T> vector(const PoolVector<T> p_array) {
	int count = p_array.size();
	PoolVector<T>::Read r = p_array.read();
	Vector<T> result;
	result.resize(count);
	for (int i = 0; i < count; i++) {
		result.write[i] = r[i];
	}
	return result;
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
