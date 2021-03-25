#include "red_engine.h"
#include "red.h"
#include "red_controller_base.h"
#include "red_page.h"
#include "red_frame.h"
#include "core/node_path.h"
#include "core/image.h"

#include "core/bind/core_bind.h"
#include "core/math/vector3.h"
#include "core/pool_vector.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/polygon_2d.h"
#include "scene/resources/bit_map.h"
#include <string>

namespace red {
PoolByteArray to_ascii(const String &input_string) {
	if (input_string.empty()) 
		return PoolByteArray();
	PoolByteArray out;
	CharString charstr = input_string.ascii();
	size_t len = charstr.length();
	out.resize(len);
	PoolByteArray::Write w = out.write();
	copymem(w.ptr(), charstr.ptr(), len);
	w.release();
	return out;
}

Vector2 cubic_bezier(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, const Vector2 &p3, float t){
    Vector2 q0 = p0.linear_interpolate(p1, t);
    Vector2 q1 = p1.linear_interpolate(p2, t);
    Vector2 q2 = p2.linear_interpolate(p3, t);

    Vector2 r0 = q0.linear_interpolate(q1, t);
    Vector2 r1 = q1.linear_interpolate(q2, t);

    Vector2 r = r0.linear_interpolate(r1, t);
    return r;
};

Vector2 quadratic_bezier(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, float t){
    Vector2 q0 = p0.linear_interpolate(p1, t);
    Vector2 q1 = p1.linear_interpolate(p2, t);
    Vector2 r = q0.linear_interpolate(q1, t);
    return r;
};

Vector<Vector2> tesselate(const Vector<Vector2> &low_res, int smooth_factor, const TesselateMode &p_interpolation) {
	Vector2 pre_a;
	Vector2 a;
	Vector2 b;
	Vector2 post_b;

	int size = low_res.size();
	int last_id = size - 1;
	int pre_last_id = last_id - 1;
	
	Vector2 first = low_res[0];
	Vector2 second = low_res[1];
	Vector2 third = low_res[2];
	Vector2 last = low_res[last_id];
	
	smooth_factor += 1;
	Vector<Vector2> high_res;
	int h_i = 0;
	switch(p_interpolation){
		case(CUBIC): {
			high_res.resize(size * smooth_factor);
			float k = 1.f / smooth_factor;
			for(int i = 0; i < size; ++i){
				if(i == 0){
					pre_a = last;
					a = first;
					b = second;
					post_b = third;
				}else{
					pre_a = a;
					a = b;
					b = post_b;
					post_b = i == pre_last_id ? first : i == last_id ? second : low_res[i + 2];
				}
				high_res.write[h_i] = a;
				h_i++;
				for(int j = 1; j < smooth_factor; ++j){
					high_res.write[h_i] = a.cubic_interpolate(b, pre_a, post_b, j * k);
					h_i++;
				}
			}
		} break;
		case(QUADRATIC_BEZIER): {
			high_res.resize(ceil(size / 2.0) * smooth_factor);
			float k = 1.f / smooth_factor;
			for(int i = 0; i < size; ++i){
				if(i == 0){
					pre_a = last;
					a = first;
					b = second;
					post_b = third;
				}else{
					pre_a = a;
					a = b;
					b = post_b;
					post_b = i == pre_last_id ? first : i == last_id ? second : low_res[i + 2];
				}
				if(i % 2)
					continue;
				high_res.write[h_i] = a;
				h_i++;
				for(int j = 1; j < smooth_factor; ++j){
					high_res.write[h_i] = quadratic_bezier(a, b, post_b, j * k);
					h_i++;
				}
			}
		} break;
		case(CUBIC_BEZIER): {
			high_res.resize(ceil(size / 3.0) * smooth_factor);
			float k = 1.f / smooth_factor;
			for(int i = 0; i < size; ++i){
				if(i == 0){
					pre_a = last;
					a = first;
					b = second;
					post_b = third;
				}else{
					pre_a = a;
					a = b;
					b = post_b;
					post_b = i == pre_last_id ? first : i == last_id ? second : low_res[i + 2];
				}
				if((i-1) % 3)
					continue;
				high_res.write[h_i] = pre_a;
				h_i++;
				for(int j = 1; j < smooth_factor; ++j){
					high_res.write[h_i] = cubic_bezier(pre_a, a, b, post_b, j * k);
					h_i++;
				}
			}
		} break;
	}
	return high_res;
}

Ref<BitMap> read_bitmap(const Ref<Image> p_img, float p_threshold, Size2 max_size){
	Vector2 image_size = p_img->get_size();
	Size2 resized_size = Vector2(CLAMP(max_size.x, 16, image_size.x), CLAMP(max_size.y, 16, image_size.y));
	Ref<BitMap> bitmap;
	bitmap.instance();
	if(resized_size != image_size){
		Ref<Image> img = p_img->duplicate();
		img->resize(resized_size.x, resized_size.y);
		bitmap->create_from_image_alpha(img, p_threshold);
	}else{
		bitmap->create_from_image_alpha(p_img, p_threshold);
	}
	return bitmap;
}

PoolColorArray get_normals(PoolVector2Array vtxs){
	PoolColorArray arr;
	for (int i = 0; i < vtxs.size(); i++){	
		int prev = i == 0 ? vtxs.size() - 1 : i - 1;
		int next = i == vtxs.size() - 1 ? 0 : i + 1;
		Vector2 baca = (vtxs[prev] - vtxs[i] - (vtxs[next] - vtxs[i])) / 2.0;
		Vector3 normal = Vector3(baca.x, baca.y, 0.0).cross(Vector3(0.0, 0.0, -1.0)).normalized();
		arr.push_back(Color(-normal.x * 0.5 + 0.5, -normal.y * 0.5 + 0.5, 0.0));
	}
	return arr;
}

Vector<Polygon2D*> bitmap_to_polygon2d(Ref<BitMap> mask_bitmap, Size2 &polygon_size, float polygon_grow, float epsilon, bool single, bool normals_to_colors, const Rect2 &crop_rect){
	Vector<Polygon2D*> meshes;
	Vector<PoolVector<Vector2> > low_res_list = red::bitmap_to_polygon(mask_bitmap, polygon_size, polygon_grow, epsilon, single, crop_rect);
	int poly_count = low_res_list.size();
	for (int poly_i = 0; poly_i < poly_count; poly_i++)
	{	
		Polygon2D *mesh = memnew(Polygon2D);
		mesh->set_name(String("bitmap_mesh") + String(Variant(poly_i)));
		PoolVector<Vector2> polygon = low_res_list[poly_i];
		// PoolVector<Vector2> polygon;
		// int end = low_res.size()-1;
		// int smooth_factor = 0;
		// for(int i=0; i<low_res.size(); ++i){
		// 	int b = i == end ? 0 : i + 1;
		// 	int pre_a = i == 0 ? end : i - 1;
		// 	int post_b = i == end - 1 ? 0 : b + 1;
		// 	polygon.push_back(low_res[i]);
		// 	for(int j=0; j<smooth_factor; ++j){
		// 		polygon.push_back(low_res[i].cubic_interpolate(low_res[b], low_res[pre_a], low_res[post_b], (j+1)*1.0/(smooth_factor+1)));
		// 	}
		// }
		if (normals_to_colors){
			PoolColorArray color = get_normals(polygon);
			mesh->set_vertex_colors(color);
		}
		mesh->set_polygon(polygon);
		{
			PoolVector<Vector2> uv;
			for (int i = 0; i < polygon.size(); i++)
				uv.append(polygon[i] / polygon_size);
			mesh->set_uv(uv);
		}
		{
			Vector<Vector2> result;
			int count = polygon.size();
			result.resize(count);
			for (int i = 0; i < count; i++) {
				result.write[i] = polygon[i];
			}
			Vector<int> delanay = Geometry::triangulate_delaunay_2d(result);
			Array polygons;
			for (int i = 0; i < delanay.size(); i += 3){
				PoolIntArray p;
				for (int j = 0; j < 3; j++){
					p.push_back(delanay[i + j]);
				}
				polygons.push_back(p);
			}
			mesh->set_polygons(polygons);
		}
		meshes.push_back(mesh);
	}
	return meshes;
}

Ref<Image> merge_images(Ref<Image> main_image, Vector<Ref<Image> > &additional_images, Vector<Vector2> &offsets){
	Ref<Image> img;
	int images_count = additional_images.size();
	ERR_FAIL_COND_V_MSG(images_count != offsets.size(), img, "Images and offsets error")
	int target_width = main_image->get_width();
	int target_height = main_image->get_height();
	PoolVector<uint8_t> data;
	int data_size = target_width * target_height * 4;
	data.resize(data_size);
	PoolVector<uint8_t>::Write w = data.write();
	// Point2 offset = Point2(layer->left - atlas_img.img_rect.position.x, layer->top - atlas_img.img_rect.position.y); //invert
	// Point2 size = Point2(layer->right - atlas_img.img_rect.position.x, layer->bottom - atlas_img.img_rect.position.y);
	int j = 0;
	int x = -1;
	int y = 0;

	Vector<Rect2> rects;
	Vector<int> ps;
	Vector<float> p_adds;
	for (int i = 0; i < images_count; i++)
	{
		Vector2 offset = offsets[i];

		Rect2 rect;
		rect.set_position(offset);
		rect.set_size(Size2(additional_images[i]->get_width(), additional_images[i]->get_height()) + offset);

		int p = MAX(offset.x, 0) + MAX(additional_images[i]->get_width() * offset.y, 0);
		float p_add = MAX(offset.x, 0) + MAX(additional_images[i]->get_width() - target_width, 0);

		rects.push_back(rect);
		ps.push_back(p);
		p_adds.push_back(p_add);

	}
	int p = 0;
	// int p = 0;
	// p += MAX(layer->width * (-offset.y), 0);
	// p += MAX(-offset.x, 0);
	// float p_add = MAX(-offset.x, 0) + MAX(layer->right - (atlas_img.img_rect.position.x + atlas_img.img_rect.size.width), 0);
	PoolVector<uint8_t> pixels = main_image->get_data();

	while(true){
		x++;
		if (x == target_width){
			for (int i = 0; i < images_count; i++){
				if (y >= rects[i].position.y && y < rects[i].size.y)
					ps.write[i] += p_adds[i];
			}
			x = 0;
			y++;
		}
		if (y == target_height || j >= data_size - 3){
			break;
		}
		float k = (float)((uint8_t) 255 / (pixels[p+3]));
		uint8_t r = 255;
		uint8_t g = 255;
		uint8_t b = 255;
		uint8_t a = 255;
		if (k < 1){
			for (int i = 0; i < images_count; i++){
				if (x >= rects[i].position.x && y >= rects[i].position.y && x < rects[i].size.x && y < rects[i].size.y){
					PoolVector<uint8_t> additional_pixels = additional_images[i]->get_data();
					int ps_i = ps[i];
					r = additional_pixels[ps_i];
					g = additional_pixels[ps_i+1];
					b = additional_pixels[ps_i+2];
					a = additional_pixels[ps_i+3];
					ps.write[i] = ps[i] + 4;
				}
			}
		}
		w[j] = (uint8_t) (r + k * (pixels[p] - r));
		w[j+1] = (uint8_t) (g + k * (pixels[p+1] - g));
		w[j+2] = (uint8_t) (b + k * (pixels[p+2] - b));
		w[j+3] = (uint8_t) (a + k * (pixels[p+3] - a));
		p += 4;

		j += 4;
	}
	img.instance();
	img->create(target_width, target_height, false, Image::FORMAT_RGBA8, data);
	return img;
}

Vector<Vector2> ramer_douglas_peucker(const Vector<Vector2> &p_point_list, double epsilon, bool is_closed) {
	Vector<Vector2> point_list;
	point_list.append_array(p_point_list);
	if(is_closed){
		point_list.push_back(point_list[0]);
		point_list.push_back(point_list[1]);
	}
	Vector<Vector2> resultList;

	float dmax = 0;
	int index = 0;
	for (int i = 1; i < point_list.size() - 1; ++i)
	{
		Point2 vec1 = Point2(point_list[i].x -point_list[0].x, point_list[i].y - point_list[0].y);
		Point2 vec2 = Point2(point_list[point_list.size() - 1].x - point_list[0].x, point_list[point_list.size() - 1].y - point_list[0].y);
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
		for (int i = 0; i <= index; ++i) pre_part.push_back(point_list[i]);
		for (int i = index; i < point_list.size(); ++i) next_part.push_back(point_list[i]);
		Vector<Vector2> resultList1 = ramer_douglas_peucker(pre_part, epsilon);
		Vector<Vector2> resultList2 = ramer_douglas_peucker(next_part, epsilon);
		resultList.append_array(resultList1);
		for (int i = 1; i < resultList2.size(); ++i) resultList.push_back(resultList2[i]);
	}
	else{
		resultList.push_back(point_list[0]);
		resultList.push_back(point_list[point_list.size() - 1]);
	}
	if(is_closed){
		Vector2 in1 = point_list[0];
		Vector2 in2 = point_list[1];
		Vector2 out_last1 = resultList[resultList.size()-1];
		Vector2 out_last2 = resultList[resultList.size()-2];
		Vector2 out1 = resultList[0];
		Vector2 out2 = resultList[1];
		if (out_last2 == in1){
			resultList.resize(resultList.size() - 2);
		}else if(out2 == in2){
			resultList.resize(resultList.size() - 1);
			resultList.remove(0);
		}else{
			resultList.resize(resultList.size() - 1);
			resultList.push_back(out1);
			resultList.remove(0);
			resultList = ramer_douglas_peucker(resultList, epsilon);
		}
	}
	return resultList;
}

Vector<PoolVector<Vector2> > bitmap_to_polygon(Ref<BitMap> mask_bitmap, Size2 &polygon_size, float polygon_grow, float epsilon, bool single, const Rect2 &crop_rect){
	Vector<PoolVector<Vector2> > result;
	auto check_polygon = [&](){
		if (result.size() == 0){
			PoolVector<Vector2> polygon;
			polygon.append(Vector2(0,0));
			polygon.append(Vector2(polygon_size.x, 0));
			polygon.append(polygon_size);
			polygon.append(Vector2(0, polygon_size.y));
			result.push_back(polygon);
		}
	};
	if (mask_bitmap.is_null()){
		check_polygon();
		return result;
	}
	Rect2 bitmap_rect = Rect2(Point2(0, 0), mask_bitmap->get_size());
	Rect2 real_crop_rect = crop_rect.size.x == 0 || crop_rect.size.y == 0 ? bitmap_rect : crop_rect;
	Rect2 poly_bitmap_rect = Rect2(Point2(0, 0), real_crop_rect.size);
	int grow = Math::round(bitmap_rect.size.width * polygon_grow / 128.0);
	int over_grow = 0;
	Ref<BitMap> poly_bitmap;
	if (grow > 0 || (crop_rect.size.x != 0 && crop_rect.size.y != 0)){
		if (grow > 0){
			over_grow = grow;
			poly_bitmap_rect.size += 2 * Size2(over_grow, over_grow);
		}
		poly_bitmap.instance();
		poly_bitmap->create(poly_bitmap_rect.size);
		int x_offset = Math::round(real_crop_rect.position.x);
		int y_offset = Math::round(real_crop_rect.position.y);
		int x_max = Math::round(real_crop_rect.size.x) + x_offset;
		int y_max = Math::round(real_crop_rect.size.y) + y_offset;
		for (int x = x_offset; x < x_max; x++) {
			for (int y = y_offset; y < y_max; y++) {
				poly_bitmap->set_bit(Point2(x + over_grow - x_offset, y + over_grow - y_offset), mask_bitmap->get_bit(Point2(x, y)));
			}
		}
	}else{
		poly_bitmap = mask_bitmap;
	}
	if (grow > 0)
		poly_bitmap->grow_mask(grow, poly_bitmap_rect);
	else if (grow < 0)
		poly_bitmap->shrink_mask(grow * -1, poly_bitmap_rect);	
	Vector2 k = polygon_size / real_crop_rect.size;
	Vector<Vector<Vector2> > result_pre;
	{
		Vector<Vector<Vector2> > polygon_array = poly_bitmap->clip_opaque_to_polygons(poly_bitmap_rect); //real_crop_rect);
		int count = polygon_array.size();
		if ((count == 1 && single) || !single){
			Vector<Vector2> frame_polygon;
			frame_polygon.resize(4);
			frame_polygon.write[0] = Vector2(over_grow, over_grow);
			frame_polygon.write[1] = Vector2(real_crop_rect.size.width + over_grow, over_grow);
			frame_polygon.write[2] = real_crop_rect.size + Vector2(over_grow, over_grow);
			frame_polygon.write[3] = Vector2(over_grow, real_crop_rect.size.height + over_grow);
			for (int i = 0; i < count; i++){
				if (polygon_array[i].size() > 3 && epsilon > 0.0){
					Vector<Vector2> out_array = red::ramer_douglas_peucker(polygon_array[i], epsilon, true);
					if(out_array.size() > 2){
						Vector<Vector<Vector2> > clipped = Geometry::intersect_polygons_2d(frame_polygon, out_array);
						if(clipped.size() == 0)
							result_pre.push_back(out_array);
						else
							result_pre.push_back(clipped[0]);
					}else{
						Vector<Vector<Vector2> > clipped = Geometry::intersect_polygons_2d(frame_polygon, polygon_array[i]);
						if(clipped.size() == 0)
							result_pre.push_back(polygon_array[i]);
						else
							result_pre.push_back(clipped[0]);
					}
				}
				else if (polygon_array[i].size() > 2){
					Vector<Vector<Vector2> > clipped = Geometry::intersect_polygons_2d(frame_polygon, polygon_array[i]);
					if(clipped.size() == 0)
						result_pre.push_back(polygon_array[i]);
					else
						result_pre.push_back(clipped[0]);
				}
			}
		}
	}
	for (size_t i = 0; i < result_pre.size(); i++){
		int poly_count = result_pre[i].size();
		PoolVector<Vector2> polygon;
		polygon.resize(poly_count);
		PoolVector<Vector2>::Write polygon_w = polygon.write();
		for (int j = 0; j < poly_count; j++)
			polygon_w[j] = (result_pre[i][j] - Size2(over_grow, over_grow)) * k; // - real_crop_rect.position result_pre[i][j];
		result.push_back(polygon);
	}
	check_polygon();
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

Size2 get_full_size(PoolVector<Vector2> &polygon, PoolVector<Vector2> &uv) {
	int l = polygon.size();
	PoolVector<Vector2>::Read polygon_r = polygon.read();
	PoolVector<Vector2>::Read uv_r = uv.read();
	Rect2 uv_rect = Rect2();
	Rect2 item_rect = Rect2();
	for (int i = 0; i < l; i++) {
		Vector2 uv_pos = uv_r[i];
		Vector2 pos = polygon_r[i];
		if (i == 0){
			uv_rect.position = uv_pos;
			item_rect.position = pos;
		}
		else{
			uv_rect.expand_to(uv_pos);
			item_rect.expand_to(pos);
		}
	}
	item_rect.size = item_rect.size / uv_rect.size;
	return item_rect.size;
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
PoolVector<int> pool_int_array(Array &p_arr){
	Array array;
	int count = p_arr.size();
	PoolVector<int> pool;
	pool.resize(count);
	PoolVector<int>::Write w = pool.write();
	for (int i = 0; i < count; i++){
		int el = p_arr[i];
		w[i] = el;
	}
	return pool;
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
String globalize(const String &p_path){
	ProjectSettings *settings = ProjectSettings::get_singleton();
	if (settings)
		return settings->globalize_path(p_path);
	else
		ERR_PRINTS("Error can't globalize: " + p_path);
	return String();
}
String localize(const String &p_path){
	ProjectSettings *settings = ProjectSettings::get_singleton();
	if (settings)
		return settings->localize_path(p_path);
	else
		ERR_PRINTS("Error can't localize: " + p_path);
	return String();
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