/*************************************************************************/
/*  resource_importer_texture_atlas.cpp                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "resource_importer_psd.h"

#include "core/io/image_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/file_access.h"
#include "editor/editor_atlas_packer.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"
#include "core/bind/core_bind.h"
#include "core/image.h"
#include "core/project_settings.h"

#include "libpsd/include/libpsd.h"
#include "scene/resources/packed_scene.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/polygon_2d.h"
#include "scene/resources/material.h"
#include "scene/main/node.h"
#include "editor/editor_plugin.h" 
#include "modules/red/red_frame.h" 
#include "modules/red/red_page.h" 
#include "scene/resources/bit_map.h"
#include "core/math/math_funcs.h"
#include <string>
#include "core/message_queue.h"
#include "editor/editor_node.h" 
#include "editor/import/resource_importer_texture_atlas.h"

//#include "modules/red/red_polygon.h"
#include "scene/main/viewport.h"
#include "scene/gui/viewport_container.h"
#include "modules/red/red_engine.h"
#include "modules/red/red_parallax_folder.h"
#include "modules/red/red_target.h"
#include "scene/gui/color_rect.h"
#include "scene/2d/back_buffer_copy.h"

String ResourceImporterPSD::get_importer_name() const {

	return "psd_frame";
}

String ResourceImporterPSD::get_visible_name() const {

	return "PSD file";
}
void ResourceImporterPSD::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("psd");
}

String ResourceImporterPSD::get_save_extension() const {
	return "";
}


String ResourceImporterPSD::get_resource_type() const {

	return "PSD";
}

bool ResourceImporterPSD::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {

	return true;
}

int ResourceImporterPSD::get_preset_count() const {
	return 0;
}
String ResourceImporterPSD::get_preset_name(int p_idx) const {

	return String();
}

void ResourceImporterPSD::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "main/scene_width"), 2000.0f));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "main/canvas_width"), 512));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/update"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/update_only_editor"), true));

	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/texture"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/page_height"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/frame_targets"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/folder_mask"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/order"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "update/folder_pos", PROPERTY_HINT_ENUM, "Ignore, Move, Move layers"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "update/layer_pos", PROPERTY_HINT_ENUM, "Ignore, Move and scale, Reset UV, Move UV, Move and reset UV, Move and move UV"), 3));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/first_level", PROPERTY_HINT_ENUM, "Top left, Center"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/second_level", PROPERTY_HINT_ENUM, "Top left, Center"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/folder", PROPERTY_HINT_ENUM, "Top left, Center"), 1));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/root", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame"), 2));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/first_level", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 4));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/second_level", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/folder", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 0));
	//r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/layer", PROPERTY_HINT_ENUM, "Polygon2D, REDPolygon"), 1));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/max_size"), 512));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/scale", PROPERTY_HINT_ENUM, "Original, Downscale, Upscale, Closest"), 2));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/bitmap_size"), 128));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "texture/alpha_grow"), 0.005));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "texture/polygon_grow"), 0.005));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "texture/alpha_to_polygon"), true));

	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/default", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/default.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/default_mul", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/default_mul.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/default_add", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/default_add.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/default_sub", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/default_sub.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/rim", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/rim.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/frame_outline", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/frame_outline.material")));
}

Vector<Vector2> ResourceImporterPSD::ramer_douglas_peucker(Vector<Vector2> &pointList, double epsilon, bool is_closed) {
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
/*
//https://gist.github.com/TimSC/0813573d77734bcb6f2cd2cf6cc7aa51
Vector<Vector2> ResourceImporterPSD::ramer_douglas_peucker(Vector<Point2> &pointList, double epsilon, bool is_closed) {
	if(pointList.size() < 2)
		return pointList;
	if(is_closed){
		pointList.push_back(pointList[0]);
	}
	Vector<Point2> out;
	// Find the point with the maximum distance from line between start and end
	double dmax = 0.0;
	size_t index = 0;
	size_t end = pointList.size()-1;
	for(size_t i = 1; i < end; i++)
	{
		//PerpendicularDistance(pointList[i], pointList[0], pointList[end]);
		//PerpendicularDistance(const Point &pt, const Point &lineStart, const Point &lineEnd)
		double dx = pointList[end].x - pointList[0].x;
		double dy = pointList[end].y - pointList[0].y;
		//Normalise
		double mag = pow(pow(dx,2.0)+pow(dy,2.0),0.5);
		if(mag > 0.0)
			dx /= mag; dy /= mag;
		double pvx = pointList[i].x - pointList[0].x;
		double pvy = pointList[i].y - pointList[0].y;
		//Get dot product (project pv onto normalized direction)
		double pvdot = dx * pvx + dy * pvy;
		//Scale line direction vector
		double dsx = pvdot * dx;
		double dsy = pvdot * dy;
		//Subtract this from pv
		double ax = pvx - dsx;
		double ay = pvy - dsy;
		double d = pow(pow(ax,2.0)+pow(ay,2.0),0.5);
		if (d > dmax)
		{
			index = i;
			dmax = d;
		}
	}

	// If max distance is greater than epsilon, recursively simplify
	if(dmax > epsilon)
	{
		// Recursive call
		Vector<Point2> recResults1;
		Vector<Point2> recResults2;
		Vector<Point2> firstLine;
		Vector<Point2> lastLine;
		for (int i = 0; i < index + 2; i++)
		{
			firstLine.push_back(pointList[i]);
		}
		for (int i = index; i < pointList.size(); i++)
		{
			lastLine.push_back(pointList[i]);
		}
		
		//firstLine.push_back(pointList[0]);
		//firstLine.push_back(pointList[index+1]);
		//lastLine.push_back(pointList[index]);
		//lastLine.push_back(pointList[pointList.size()-1]);
		recResults1 = ramer_douglas_peucker(firstLine, epsilon);
		recResults2 = ramer_douglas_peucker(lastLine, epsilon);
		//Vector<Point2> firstLine(pointList.begin(), pointList.begin()+index+1);
		//Vector<Point2> lastLine(pointList.begin()+index, pointList.end());
		// Build the result list
		//out.assign(recResults1.begin(), recResults1.end()-1);
		//out.insert(out.end(), recResults2.begin(), recResults2.end());
		out.append_array(recResults1);
		out.append_array(recResults2);
	} 
	else 
	{
		//Just return start and end points
		out.clear();
		out.push_back(pointList[0]);
		out.push_back(pointList[end]);
	}
	if(is_closed){
		pointList.resize(pointList.size()-1);
		out.resize(out.size()-1);
	}
	return out;
}
*/
Vector<Vector2> ResourceImporterPSD::simplify_polygon_corner(Vector<Vector2> &p_polygon, float min_distance){
	int prev = 0;
	int next = 0;
	Vector<Vector2> output;
	bool prev_added = true;
	int points_count = p_polygon.size();
	int l = points_count - 1;
	Point2 point = p_polygon[l];
	for (int i = 0; i < points_count; i++){
		if (prev_added){
			if (i == 0)
				prev = l;
			else
				prev = i - 1;
		}else{
			if (i <= 1)
				prev = points_count - 2;
			else
				prev = i - 2;
		}
		if (i == l)
			next = 0;
		else
			next = i + 1;
		point = p_polygon[i];
		float p_dist = p_polygon[prev].distance_to(point);
		float n_dist = p_polygon[next].distance_to(point);
		Vector2 p = (p_polygon[i] - p_polygon[prev]).normalized();
		Vector2 n = (p_polygon[next] - p_polygon[i]).normalized();
		if (n_dist > min_distance && p.dot(n) == 0.0){
			prev_added = true;
			output.push_back(point);
		}else{
			prev_added = false;
		}
	}
	return output;
}

Vector<Vector2> ResourceImporterPSD::simplify_polygon_distance(Vector<Vector2> &p_polygon, float min_distance, Size2 &size){
	int prev = 0;
	int next = 0;
	Vector<Vector2> output;
	bool prev_added = true;
	int points_count = p_polygon.size();
	int l = points_count - 1;
	Point2 point = p_polygon[l];
	bool last_on_side = point.x == 0 || point.y == 0 || point.x == size.x || point.y == size.y;
	for (int i = 0; i < points_count; i++){
		if (prev_added){
			if (i==0)
				prev = l;
			else
				prev = i - 1;
		}else{
			if (i<=1)
				prev = points_count - 2;
			else
				prev = i - 2;
		}
		if (i == l)
			next = 0;
		else
			next = i + 1;
		point = p_polygon[i];
		float p_dist = p_polygon[prev].distance_to(point);
		float n_dist = p_polygon[next].distance_to(point);
		bool on_side = point.x == 0 || point.y == 0 || point.x == size.x || point.y == size.y;
		if ((n_dist >= min_distance && p_dist >= min_distance) || on_side || (i == 0 && !last_on_side)){
			prev_added = true;
			output.push_back(point);
		}else{
			prev_added = false;
		}
	}
	return output;
}

Vector<Vector2> ResourceImporterPSD::simplify_polygon_direction(Vector<Vector2> &p_polygon, float max_dot){
	int prev = 0;
	int next = 0;
	Vector<Vector2> polygon_dist;
	bool prev_added = true;
	for (int i = 0; i < p_polygon.size(); i++){
		if (!prev_added){
			prev_added = true;
			polygon_dist.push_back(p_polygon[i]);
			continue;
		}
		if (i==0)
			prev = p_polygon.size()-1;
		else
			prev = i - 1;
		if (i == p_polygon.size()-1)
			next = 0;
		else
			next = i + 1;
		Vector2 p = (p_polygon[i] - p_polygon[prev]).normalized();
		Vector2 n = (p_polygon[next] - p_polygon[i]).normalized();
		if (p.dot(n) <= max_dot){
			prev_added = true;
			polygon_dist.push_back(p_polygon[i]);
		}else{
			prev_added = false;
		}
	}
	return polygon_dist;
}

Ref<Image> ResourceImporterPSD::read_mask(_psd_layer_record *layer){
	PoolVector<uint8_t> data;
	int len = layer->layer_mask_info.width*layer->layer_mask_info.height;
	if (layer->layer_mask_info.width < 2 || layer->layer_mask_info.height < 2){
		Ref<BitMap> bitmap;
		return bitmap;
	}
	data.resize(len * 2);
	PoolVector<uint8_t>::Write w = data.write();
	int j = 0;
	for (int i = 0; i < len; i++) {
		uint8_t alpha = (uint8_t)((char)(layer->layer_mask_info.mask_data[i]));
		w[j] = alpha;
		w[j+1] = alpha;
		j+=2;
	}
	Ref<Image> img;
	img.instance();
	img->create(layer->layer_mask_info.width, layer->layer_mask_info.height, false, Image::FORMAT_LA8, data);
	return img;
}

Ref<BitMap> ResourceImporterPSD::read_bitmap(Ref<Image> p_img, float p_threshold, Size2 max_size){
	Ref<Image> img = p_img->duplicate();
	Vector2 image_size = img->get_size();
	Size2 resized_size = Vector2(CLAMP(max_size.x, 16, image_size.x), CLAMP(max_size.y, 16, image_size.y));
	img->resize(resized_size.x, resized_size.y);
	Ref<BitMap> bitmap;
	bitmap.instance();
	bitmap->create_from_image_alpha(img, p_threshold);
	return bitmap;
}

Ref<BitMap> ResourceImporterPSD::read_bitmap(_psd_layer_record *layer, float p_threshold, Size2 max_size){
	Ref<Image> img = read_image(layer);
	Ref<BitMap> bitmap = read_bitmap(img, p_threshold, max_size);
	return bitmap;
}

Ref<Image> ResourceImporterPSD::read_atlas(_psd_layer_record *layer, img_data &atlas_img){
	Ref<Image> img;
	{
		PoolVector<uint8_t> data = atlas_img.img->get_data();
		int j = data.size();
		data.resize(data.size() + data.size() / atlas_img.count.width / atlas_img.count.height);
		PoolVector<uint8_t>::Write w = data.write();
		atlas_img.count.height += 1;

		Point2 offset = Point2(layer->left - atlas_img.img_rect.position.x, layer->top - atlas_img.img_rect.position.y);
		Point2 size = Point2(atlas_img.img_rect.position.x - layer->right, atlas_img.img_rect.position.y - layer->bottom);
		int x = -1;
		int y = 0;
		int len = data.size();
		int p = 0;
		p += MAX(layer->width * (-offset.y), 0);
		p += MAX(-offset.x, 0);

		while(true){
			x++;
			if (x == atlas_img.img->get_width()){
				if (y >= offset.y && y < size.y){
					p += MAX(-offset.x, 0);
					p += MAX(layer->right - (atlas_img.img_rect.position.x + atlas_img.img_rect.size.width), 0);
				}
				x = 0;
				y++;
			}
			if (y == atlas_img.img_rect.size.height){
				break;
			}
			if (x >= offset.x && y >= offset.y && x < size.x && y < size.y){
				unsigned int pixel = layer->image_data[p];
				w[j] = (uint8_t) ((pixel & 0x00FF0000) >> 16);
				w[j+1] = (uint8_t) ((pixel & 0x0000FF00) >> 8);
				w[j+2] = (uint8_t) ((pixel & 0x000000FF));
				w[j+3] = (uint8_t) ((pixel & 0xFF000000) >> 24);
				j += 4;
				p++;
			}
			else{
				w[j] = (uint8_t) 0;
				w[j+1] = (uint8_t) 0;
				w[j+2] = (uint8_t) 0;
				w[j+3] = (uint8_t) 0;
				j+=4;
			}
		}
		img.instance();
		img->create(atlas_img.img_rect.size.width*atlas_img.count.width, atlas_img.img_rect.size.height*atlas_img.count.height, false, Image::FORMAT_RGBA8, data);
	}
	return img;
}

Vector3 ResourceImporterPSD::find_normal_border(const Ref<BitMap> bitmap, const Point2 &point, int max_radius){
	float min_distance = 10000000;
	Size2 bitmap_size = bitmap->get_size();
	Point2 min_coord(point.x, point.y);
	int searching = 0;
	for(int r = 1; r <= max_radius; r++){
		Point2 current(-r,-r);
		for(current.x = -r + 1; current.x < r; current.x += 1){
			Vector2 current_abs((point.x + current.x), 0);
			for(current.y = -r; current.y <= r; current.y += r){
				if (current_abs.x < 0 || current_abs.y < 0 || current_abs.x > bitmap_size.x - 2 || current_abs.y > bitmap_size.y - 2 || !bitmap->get_bit(current_abs)){
					float distance = current_abs.distance_squared_to(point);
					if (distance < min_distance){
						min_distance = distance;
						min_coord.x = CLAMP(current_abs.x, 0, bitmap_size.x - 1);
						min_coord.y = CLAMP(current_abs.y, 0, bitmap_size.y - 1);
						if (searching == 0){
							searching = 1;
						}
					}
				}
			}
		}
		for(current.y = -r; current.y <= r; current.y += 1){
			Vector2 current_abs(0, (point.y + current.y));
			for(current.x = -r; current.x <= r; current.x += r){
				current_abs.x = point.x + current.x;
				if (current_abs.x < 0 || current_abs.y < 0 || current_abs.x > bitmap_size.x - 2 || current_abs.y > bitmap_size.y - 2 || !bitmap->get_bit(current_abs)){
					float distance = current_abs.distance_squared_to(point);
					if (distance < min_distance){
						min_distance = distance;
						min_coord.x = CLAMP(current_abs.x, 0, bitmap_size.x - 1);
						min_coord.y = CLAMP(current_abs.y, 0, bitmap_size.y - 1);
						if (searching == 0){
							searching = 1;
						}
					}
				}
			}
		}
		if (searching == 1){
			max_radius = r * 2;
			searching = 2;
		}
	}
	Vector2 normal2d = min_coord - point;
	return Vector3(normal2d.x, normal2d.y, 0.0).normalized();
}
RenderData::RenderData(){
	//init();
}

void RenderData::init(){
	EditorData &editor_data = EditorNode::get_editor_data();
	Node *scene_root = editor_data.get_edited_scene_root();
	ERR_FAIL_NULL(scene_root);
	//generator_container = red::create_node<ViewportContainer>(scene_root, String("generator_container") + String(Variant(container_i)));
	//generator_container = memnew(ViewportContainer);
	//generator_container->set_name(String("generator_container") + String(Variant(container_i)));
	//normal_generator = memnew(Viewport);
	//container_i += 1;
	set_name(String("generator_container"));
	scene_root->add_child(this);
	set_owner(scene_root);

	viewport = red::create_node<Viewport>(this, "generator");
	viewport->set_update_mode(Viewport::UpdateMode::UPDATE_ALWAYS);
	viewport->set_usage(Viewport::USAGE_2D);
	viewport->set_use_own_world(true);
	viewport->set_vflip(true);
	viewport->set_size(resolution);
	viewport->set_transparent_background(false);

	generate_polygons();
	generate_pp();

	camera = red::create_node<Camera2D>(viewport, "camera");
	camera->make_current();
	camera->set_anchor_mode(Camera2D::ANCHOR_MODE_FIXED_TOP_LEFT);
	camera->set_visible(false);
}

PoolColorArray RenderData::get_normals(PoolVector2Array vtxs){
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

void RenderData::generate_polygons(){
	Vector<PoolVector<Vector2> > low_res_list = red::bitmap_to_polygon(bitmap, resolution, 1.0, 4.0, false);
	int poly_count = low_res_list.size();
	for (int poly_i = 0; poly_i < poly_count; poly_i++)
	{
		Polygon2D *mesh = red::create_node<Polygon2D>(viewport, String("mesh") + String(Variant(poly_i)));
		PoolVector<Vector2> low_res = low_res_list[poly_i];
		PoolVector<Vector2> polygon;
		int end = low_res.size()-1;
		int smooth_factor = 0;
		for(int i=0; i<low_res.size(); ++i){
			int b = i == end ? 0 : i + 1;
			int pre_a = i == 0 ? end : i - 1;
			int post_b = i == end - 1 ? 0 : b + 1;
			polygon.push_back(low_res[i]);
			for(int j=0; j<smooth_factor; ++j){
				polygon.push_back(low_res[i].cubic_interpolate(low_res[b], low_res[pre_a], low_res[post_b], (j+1)*1.0/(smooth_factor+1)));
			}
		}
		PoolColorArray color = get_normals(polygon);

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
				p.push_back(delanay[i+j]);
			}
			polygons.push_back(p);
		}
		PoolVector<Vector2> uv;
		for (int i = 0; i < polygon.size(); i++)
			uv.append(Vector2(polygon[i].x/resolution.x, polygon[i].y/resolution.y));
		mesh->set_polygons(polygons);
		mesh->set_polygon(polygon);
		mesh->set_uv(uv);
		mesh->set_vertex_colors(color);
		Ref<ShaderMaterial> material;
		material.instance();
		material->set_shader(ResourceLoader::load("res://redot/shaders/editor/normal_baker/mesh.shader", "Shader"));
		mesh->set_material(material);
		Ref<Texture> texture = ResourceLoader::load(png_path, "Texture");
		if (texture.is_valid())
			mesh->set_texture(texture);
	}
}

void RenderData::generate_pp(){
	back_mesh = red::create_node<Polygon2D>(viewport, "back_mesh");
	pass1_mesh = red::create_node<Polygon2D>(viewport, "pass1_mesh");
	bb = red::create_node<BackBufferCopy>(viewport, "bb");
	pass2_mesh = red::create_node<Polygon2D>(viewport, "pass2_mesh");
	
	viewport->move_child(back_mesh, 0);
	bb->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);
	
	PoolVector<Vector2> polygon;
	PoolVector<Vector2> uv;
	polygon.append(Vector2(0,0));
	polygon.append(Vector2(resolution.x, 0));
	polygon.append(resolution);
	polygon.append(Vector2(0, resolution.y));

	back_mesh->set_polygon(polygon);
	pass1_mesh->set_polygon(polygon);
	pass2_mesh->set_polygon(polygon);

	uv.append(Vector2(0, 0));
	uv.append(Vector2(1.0, 0));
	uv.append(Vector2(1.0, 1.0));
	uv.append(Vector2(0.0, 1.0));

	back_mesh->set_uv(uv);
	pass1_mesh->set_uv(uv);
	pass2_mesh->set_uv(uv);

	Ref<ShaderMaterial> back_mat;
	Ref<ShaderMaterial> pass1_mat;
	Ref<ShaderMaterial> pass2_mat;
	back_mat.instance();
	pass1_mat.instance();
	pass2_mat.instance();
	back_mat->set_shader(ResourceLoader::load("res://redot/shaders/editor/normal_baker/back.shader", "Shader"));
	pass1_mat->set_shader(ResourceLoader::load("res://redot/shaders/editor/normal_baker/pass1.shader", "Shader"));
	pass2_mat->set_shader(ResourceLoader::load("res://redot/shaders/editor/normal_baker/pass2.shader", "Shader"));

	back_mesh->set_material(back_mat);
	pass1_mesh->set_material(pass1_mat);
	pass2_mesh->set_material(pass2_mat);

	// todo blanc texture
	Ref<Texture> texture = ResourceLoader::load(png_path, "Texture");
	if (texture.is_valid())
		back_mesh->set_texture(texture);
	
	if (rim_image.is_valid()){
		Ref<ImageTexture> rim_texture;
		rim_texture.instance();
		rim_texture->create_from_image(rim_image);
		pass2_mesh->set_texture(rim_texture);
		pass2_mat->set_shader_param("use_texture", true);
	}
}

void RenderData::render(){
	VisualServer *server = VisualServer::get_singleton();
	server->draw();
	server->connect("frame_post_draw", this, "_render_png");
}

void RenderData::_render_png(){
	if(!is_queued_for_deletion()){
		VisualServer::get_singleton()->disconnect("frame_post_draw", this, "_render_png");
		Ref<Image> normal_texture = viewport->get_texture()->get_data();
		normal_texture->save_png(render_path);
		if (ResourceLoader::import){
			ResourceLoader::import(render_path);
		}
		//queue_delete();
		set_visible(false);
	}
}

Ref<Image> ResourceImporterPSD::read_rim(_psd_layer_record *layer, img_data &atlas_img){
	Ref<Image> img;
	int target_width = atlas_img.img_rect.size.width;
	int target_height = atlas_img.img_rect.size.height;
	PoolVector<uint8_t> data;
	int data_size = target_width * target_height * 4;
	data.resize(data_size);
	PoolVector<uint8_t>::Write w = data.write();
	Point2 offset = Point2(layer->left - atlas_img.img_rect.position.x, layer->top - atlas_img.img_rect.position.y);
	Point2 size = Point2(layer->right - atlas_img.img_rect.position.x, layer->bottom - atlas_img.img_rect.position.y);
	
	int j = 0;
	int x = -1;
	int y = 0;
	
	int p = 0;

	p += MAX(layer->width * (-offset.y), 0);
	p += MAX(-offset.x, 0);
	float p_add = MAX(-offset.x, 0) + MAX(layer->right - (atlas_img.img_rect.position.x + atlas_img.img_rect.size.width), 0);
	psd_argb_color *pixels = layer->image_data;

	print_line("loading normal");

	while(true){
		x++;
		if (x == target_width){
			if (y >= offset.y && y < size.y){
				p += p_add;
				//p += MAX(-offset.x, 0);
				//p += MAX(layer->right - (atlas_img.img_rect.position.x + atlas_img.img_rect.size.width), 0);
			}
			x = 0;
			y++;
		}
		if (y == target_height || j >= data_size - 3){
			break;
		}
		if (x >= offset.x && y >= offset.y && x < size.x && y < size.y){
			//Vector3 normal = find_normal_border(atlas_bitmap, Point2((x * 127.0) / target_width, (y * 127.0) / target_height), 16);
			//normal.cross(normal);
			//w[j] = (uint8_t) (normal.x * 128 + 127.5);
			//w[j+1] = (uint8_t) (normal.y * 128 + 127.5);
			//w[j+2] = (uint8_t) 0;
			//w[j+3] = (uint8_t) 255;
			//j += 4;
			//continue;
			unsigned int pixel = pixels[p];
			uint8_t pixel_tr = (uint8_t) ((pixel & 0xFF000000) >> 24);

			w[j] = (uint8_t) 128;
			w[j+1] = (uint8_t) 128;
			w[j+2] = (uint8_t) (255 - pixel_tr);
			w[j+3] = (uint8_t) (pixel_tr);
			j += 4;
			p++;
		}
		else{
			w[j] = (uint8_t) 128;
			w[j+1] = (uint8_t) 128;
			w[j+2] = (uint8_t) 255;
			w[j+3] = (uint8_t) 0.0;
			j += 4;
		}
	}
	img.instance();
	img->create(atlas_img.img_rect.size.width, atlas_img.img_rect.size.height, false, Image::FORMAT_RGBA8, data);
	return img;
}

Ref<Image> ResourceImporterPSD::read_image(_psd_layer_record *layer){
	Ref<Image> img;
	int len = layer->width * layer->height;
	{
		PoolVector<uint8_t> data;
		data.resize(len*4);
		PoolVector<uint8_t>::Write w = data.write();
		int j = 0;
		for (int i = 0; i < len; i++) {
			
			unsigned int pixel = layer->image_data[i];
			w[j] = (uint8_t) ((pixel & 0x00FF0000) >> 16);
			w[j+1] = (uint8_t) ((pixel & 0x0000FF00) >> 8);
			w[j+2] = (uint8_t) ((pixel & 0x000000FF));
			uint8_t alpha = (uint8_t) ((pixel & 0xFF000000) >> 24);
			if (alpha > ((uint8_t) 32)){
				w[j+3] = alpha;
			}else{
				w[j+3] = ((uint8_t) 0);
			}
			j+=4;
		}
		img.instance();
		img->create(layer->width, layer->height, false, Image::FORMAT_RGBA8, data);
	}
	return img;
}

Rect2 ResourceImporterPSD::get_crop(Ref<BitMap> bitmap, const Vector2 &grow){
	Size2 size = bitmap->get_size();
	int x_min = size.width;
	int x_max = 0;
	int y_min = size.height;
	int y_max = 0;
	Point2 xt = Point2(0.f, 0.f);
	for (int x = 0; x < size.width; x++) {
		for (int y = 0; y < size.height; y++) {
			xt.x = x;
			xt.y = y;
			if (bitmap->get_bit(xt)){
				x_min = MIN(x_min, x - grow.x);
				x_max = MAX(x_max, x + grow.x);
				y_min = MIN(y_min, y - grow.y);
				y_max = MAX(y_max, y + grow.y);
			}
		}
	}
	x_min = MAX(x_min, 0);
	x_max = MIN(x_max, bitmap->get_size().x - 1);
	y_min = MAX(y_min, 0);
	y_max = MIN(y_max, bitmap->get_size().y - 1);
	return Rect2(x_min, y_min, x_max - x_min + 1, y_max - y_min + 1);
}

void ResourceImporterPSD::apply_scale(img_data &atlas_img, double scale, int texture_scale_mode, int texture_min_size, const Size2 &texture_max_size){
	Size2 n(atlas_img.img->get_width()/atlas_img.count.width, atlas_img.img->get_height()/atlas_img.count.height);
	if (scale != 0)
		n = n * scale;
	switch (texture_scale_mode)
	{
	case DOWNSCALE:
		n.width = previous_power_of_2(n.width);
		n.height = previous_power_of_2(n.height);
		break;
	case UPSCALE:
		n.width = next_power_of_2(n.width);
		n.height = next_power_of_2(n.height);
		break;
	case CLOSEST:
		n.width = closest_power_of_2(n.width);
		n.height = closest_power_of_2(n.height);
		break;
	default:
		break;
	}
	n.width = CLAMP(n.width * atlas_img.count.width, texture_min_size, texture_max_size.width);
	n.height = CLAMP(n.height * atlas_img.count.height, texture_min_size, texture_max_size.height);
	if (n.width != atlas_img.img->get_width() || n.height != atlas_img.img->get_height())
		atlas_img.img->resize(n.width, n.height, Image::INTERPOLATE_LANCZOS);
}

void ResourceImporterPSD::apply_scale(Ref<Image> img, double scale, int texture_scale_mode, int texture_min_size, const Size2 &texture_max_size){
	Size2 n(img->get_width(), img->get_height());
	if (scale != 0)
		n = n * scale;
	switch (texture_scale_mode)
	{
	case DOWNSCALE:
		n.width = previous_power_of_2(n.width);
		n.height = previous_power_of_2(n.height);
		break;
	case UPSCALE:
		n.width = next_power_of_2(n.width);
		n.height = next_power_of_2(n.height);
		break;
	case CLOSEST:
		n.width = closest_power_of_2(n.width);
		n.height = closest_power_of_2(n.height);
		break;
	default:
		break;
	}
	n.width = CLAMP(n.width, texture_min_size, texture_max_size.width);
	n.height = CLAMP(n.height, texture_min_size, texture_max_size.height);
	if (n.width != img->get_width() || n.height != img->get_height())
		img->resize(n.width, n.height, Image::INTERPOLATE_LANCZOS);
}

void ResourceImporterPSD::apply_border(Ref<Image> img, float p_threshold){
	int new_width = img->get_width();
	int new_height = img->get_height();
	int last = new_height - 1;
	img->lock();
	for (int x = 0; x < new_width; x++) {
		Color pixel = img->get_pixel(x, 0);
		if (pixel.a < p_threshold){
			pixel.a = 0.0;
			img->set_pixel(x, 0, pixel);
		}
		pixel = img->get_pixel(x, last);
		if (pixel.a < p_threshold){
			pixel.a = 0.0;
			img->set_pixel(x, last, pixel);
		}
	}
	last = new_width - 1;
	for (int y = 0; y < new_height; y++) {
		Color pixel = img->get_pixel(0, y);
		if (pixel.a < p_threshold){
			pixel.a = 0.0;
			img->set_pixel(0, y, pixel);
		}
		pixel = img->get_pixel(last, y);
		if (pixel.a < p_threshold){
			pixel.a = 0.0;
			img->set_pixel(last, y, pixel);
		}
	}
	img->unlock();
}
void RenderData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_render_png"), &RenderData::_render_png);
}

void ResourceImporterPSD::_bind_methods() {
    //ClassDB::bind_method(D_METHOD("_render_viewport"), &ResourceImporterPSD::_render_viewport);
}

Node *ResourceImporterPSD::get_edited_scene_root(const String &p_path) const{
	EditorData &editor_data = EditorNode::get_editor_data();
	//EditorData editor_data = EditorNode::get_editor_data();
	for (int i = 0; i < editor_data.get_edited_scene_count(); i++) {
		if (editor_data.get_scene_path(i) == p_path)
			return editor_data.get_edited_scene_root(i);
	}
	return nullptr;
}

Node *ResourceImporterPSD::_get_root(_psd_context *context, const String &scene_path, bool &force_save, const Map<StringName, Variant> &p_options) const{
	_psd_layer_record *last_layer = &(context->layer_records[context->layer_count-1]);
	Node *root = nullptr;
	bool b_update = p_options["main/update"];
	bool b_update_only_editor = p_options["main/update_only_editor"];
	_File file_сheck;
	if (file_сheck.file_exists(scene_path)){
		force_save = false;
		if (!b_update){
			return nullptr;
		}
	}
	else{
		red::scene_loader(scene_path);
	}
	if (!EditorNode::get_singleton()->is_scene_open(scene_path)){
		EditorNode::get_singleton()->load_scene(scene_path);
	}
	root = get_edited_scene_root(scene_path);
	return root;
}

void Materials::init(const Map<StringName, Variant> &p_options, Node *node){
	Vector<Node*> nodes;
	red::get_all_children(node, nodes);
	int count = nodes.size();

	frame_outline = p_options["materials/frame_outline"];
	material = p_options["materials/default"];
	material_mul = p_options["materials/default_mul"];
	material_add = p_options["materials/default_add"];
	material_sub = p_options["materials/default_sub"];

	rim = p_options["materials/rim"];
}

void Materials::apply_material(_psd_layer_record *layer, Node2D *node, REDFrame *parent_clipper, img_data &atlas_img){
	switch(layer->blend_mode){
		case psd_blend_mode_multiply:{
			if (material_mul.is_valid())
				node->set_material(material_mul);
			break;
		}
		case psd_blend_mode_linear_dodge:{
			if (material_add.is_valid())
				node->set_material(material_add);
			break;
		}
		case psd_blend_mode_difference:{
			if (material_sub.is_valid())
				node->set_material(material_sub);
			break;	
		}
		default:{
			if (atlas_img.count.height == 2 && rim.is_valid()){
				node->set_material(rim);
			}
			else{
				if (material.is_valid())
					node->set_material(material);
			}
			break;
		}
	}
}

void ResourceImporterPSD::load_clipped_layers(_psd_context *context, int start, img_data &atlas, bool load_layer_data){
	int end = context->layer_count;
	psd_layer_record *layers = context->layer_records;
	for (int iter = start; iter < end; iter++){	
		_psd_layer_record *layer = &layers[iter];
		if (layer->layer_type != psd_layer_type::psd_layer_type_normal){
			continue;
		}
		if (!layer->clipping){
			break;
		}
		int len = layer->width * layer->height;
		if (len == 0) 
			continue;
		String name = reinterpret_cast<char const*>(layer->layer_name);

		if (load_layer_data){
			if(name == "rim"){
				atlas.normal = read_rim(layer, atlas);
			}else{
				atlas.img = read_atlas(layer, atlas);
			}
		}
		else{
			if(name == "rim"){
				
			}else{
				atlas.count.height += 1;
			}
		}
	}
}

Vector<PoolVector<Vector2> > ResourceImporterPSD::bitmap_to_polygon(Ref<BitMap> bitmap_mask, Size2 &polygon_size, float polygon_grow, float epsilon, bool single){
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

int ResourceImporterPSD::load_folder(_psd_context *context, const String &scene_path, int start, Materials &materials, 
									 Node *parent, Vector2 parent_pos, Vector2 parent_offset, const Map<StringName, Variant> &p_options, 
									 bool force_save, int counter, int folder_level, REDFrame *parent_clipper){
	psd_layer_record *layers = context->layer_records;
	int count = 0;
	int offset = 0;
	int final_iter = 0;
	int end = context->layer_count;
	bool loop = true;

	Size2 context_size = Size2(context->width, context->height);
	double context_aspect_ratio = context->height / context->width;
	Size2 scene_size = Size2(p_options["main/scene_width"], 0.0f);
	double scene_k = (scene_size.width * 1.0) / context_size.width;
	scene_size.height = context->height * scene_k;
	Size2 canvas_size = Size2(p_options["main/canvas_width"], 0.0f);
	canvas_size.y = canvas_size.x * context_aspect_ratio;
	double image_resize = (canvas_size.width * 1.0) / context->width;
	double image_size_to_scene_size = (scene_size.width * 1.0) / context->width;

	bool b_update = p_options["main/update"];
	bool b_update_only_editor = p_options["main/update_only_editor"];
	bool updateble = true;
	if (parent->get_owner()){
		updateble = b_update && (!b_update_only_editor || EditorNode::get_singleton()->is_scene_open(parent->get_owner()->get_filename()));
	}
	else{
		updateble = b_update && (!b_update_only_editor || EditorNode::get_singleton()->is_scene_open(parent->get_filename()));
	}
	bool saveble = force_save || (!b_update_only_editor && b_update);
	Vector<String> names;
	for (int iter = start; iter + offset < end && loop; iter++){
		names.push_back(reinterpret_cast<char const*>(layers[iter + offset].layer_name));
	}
	String target_dir = scene_path.get_basename().get_base_dir();
	red::create_dir(target_dir);
	_File file_сheck;
	bool first = true;
	for (int iter = start; iter + offset < end && loop; iter++){	
		final_iter = iter + offset; count++;
		_psd_layer_record *layer = &layers[final_iter];
		if (layer->clipping && !first){
			continue;
		}
		first = false;
		_psd_layer_record *next_layer = (final_iter + 1 < end) ? &layers[final_iter + 1] : nullptr;
		String name = reinterpret_cast<char const*>(layers[final_iter].layer_name);
		if (!layer->visible && psd_layer_type::psd_layer_type_normal){
			continue;
		}
		switch (layer->layer_type){
			case psd_layer_type::psd_layer_type_folder:{
				if (counter > 0){
					loop = false;
				}
			} break;
			case psd_layer_type::psd_layer_type_normal:{
				int len = layer->width * layer->height;
				if (len == 0) 
					continue;
				print_line("Loading " + name);
				int folder_pos = p_options["update/folder_pos"];
				bool update_pos_move_layers = updateble && folder_pos == FOLDER_MOVE_LAYERS;
				
				String png_path = target_dir + "/" + name + ".png";
				String normal_path = target_dir + "/" + name + "_normal.png";
				String rim_path = target_dir + "/" + name + "_normal_rim.png";
				bool save_texture = !file_сheck.file_exists(png_path) || (p_options["update/texture"] && updateble);
				int update_pos = p_options["update/layer_pos"];

				bool update_material = false;
				bool update_polygon = false;
				bool update_polygon_pos = (updateble && (update_pos == LAYER_MOVE_AND_RESET_UV || update_pos == LAYER_MOVE_AND_MOVE_UV)) || update_pos_move_layers;
				bool update_polygon_transform = updateble && update_pos == LAYER_MOVE_AND_SCALE;
				bool update_uv = updateble && (update_pos == LAYER_RESET_UV ||
											   update_pos == LAYER_MOVE_UV ||
											   update_pos == LAYER_MOVE_AND_RESET_UV || 
											   update_pos == LAYER_MOVE_AND_MOVE_UV);
				bool reset_uv = update_pos == LAYER_RESET_UV || update_pos == LAYER_MOVE_AND_RESET_UV;
				bool reorder =  updateble && p_options["update/order"];
				bool need_create = parent->has_node(name) ? false : true;

				Polygon2D *poly;
				if (need_create){
					Node *owner = parent->get_owner();
					poly = red::create_node<Polygon2D>(parent, name, owner ? owner : parent);
					poly->set_draw_behind_parent(true);
					if (parent_clipper != nullptr)
						poly->set_clipper(poly->get_path_to(parent_clipper));
					update_material = true;
					update_polygon = true;
					update_polygon_pos = true;
					update_polygon_transform = false;
					update_uv = true;
					reset_uv = true;
					reorder = true;
				} 
				else {
					poly = (Polygon2D*)parent->get_node(name);
				}
				if (poly == nullptr){
					continue;
				}
				bool old_move_polygon_with_uv = poly->get_move_polygon_with_uv();
				poly->set_move_polygon_with_uv(true);
				poly->set_move_polygon_with_uv(false);
				
				float vertex_count_per_scene = 32; 						// vertexes per scene width
				float polygon_grow = p_options["texture/polygon_grow"];	// grow per scene width
				float alpha_grow = p_options["texture/alpha_grow"];	// grow per scene width
				float bitmap_threshold = 0.2f;
				Size2 bitmap_size(p_options["texture/bitmap_size"], p_options["texture/bitmap_size"]);
				Size2 texture_max_size(p_options["texture/max_size"], p_options["texture/max_size"]);
				
				img_data clipper_image_data;
				Ref<Image> layer_image = read_image(layer);
				

				Vector2 polygon_size = layer_image->get_size() * image_size_to_scene_size;
				Ref<BitMap> bitmap = read_bitmap(layer_image, bitmap_threshold, bitmap_size);
				// Rect2 crop_rect = get_crop(bitmap, alpha_grow * bitmap->get_size() * scene_size / polygon_size);
				Rect2 crop_rect = get_crop(bitmap, bitmap->get_size() * scene_size * alpha_grow / 128.0);
				{
					clipper_image_data.img = layer_image;
					clipper_image_data.bitmap = bitmap;
					Vector2 bitmap_k = layer_image->get_size() / bitmap->get_size();
					crop_rect.position = Point2(Math::round(crop_rect.position.x * bitmap_k.x), Math::round(crop_rect.position.y * bitmap_k.y));
					crop_rect.size = Size2(Math::round(crop_rect.size.width * bitmap_k.x), Math::round(crop_rect.size.height * bitmap_k.y));
					clipper_image_data.img_rect = Rect2(Point2(layer->left, layer->top) + crop_rect.position, crop_rect.size);
					clipper_image_data.polygon_name = name;
				}
				polygon_size = clipper_image_data.img_rect.size * image_size_to_scene_size;
				Point2 global_pos = clipper_image_data.img_rect.position * image_size_to_scene_size;
				Point2 local_pos = global_pos - parent_pos - parent_offset - poly->get_offset();

				if (save_texture || update_polygon){
					layer_image->crop_from_point(crop_rect.position.x, crop_rect.position.y, crop_rect.size.width, crop_rect.size.height);
					//Ref<Image> img_save = clipper_image_data.img->duplicate();
					load_clipped_layers(context, final_iter+1, clipper_image_data, true);
					apply_scale(clipper_image_data, image_resize, p_options["texture/scale"], 32, texture_max_size);
					apply_border(clipper_image_data.img, 0.75f);

					clipper_image_data.img->save_png(png_path);
					if (ResourceLoader::import){
						ResourceLoader::import(png_path);
					}
					if (clipper_image_data.normal.is_valid()){
						apply_scale(clipper_image_data.normal, image_resize, p_options["texture/scale"], 32, texture_max_size);
						//clipper_image_data.normal->save_png(rim_path);
						//if (ResourceLoader::import){
						//	ResourceLoader::import(rim_path);
						//}
					}else{
						rim_path = "";
					}

					RenderData *normal_data = memnew(RenderData);
					normal_data->resolution = clipper_image_data.img->get_size();
					normal_data->png_path = png_path;
					normal_data->render_path = normal_path;
					normal_data->bitmap = read_bitmap(layer_image, bitmap_threshold, bitmap_size);
					normal_data->rim_image = clipper_image_data.normal;

					normal_data->init();
					normal_data->render();
					//generate_normal();
				}else{
					load_clipped_layers(context, final_iter+1, clipper_image_data, false);
				}
				if (update_material){
					Ref<Texture> texture = ResourceLoader::load(png_path, "Texture");
					if (texture.is_valid())
						poly->set_texture(texture);
					if (clipper_image_data.normal.is_valid() && poly->get_normalmap().is_null()){
						Ref<Texture> normalmap = ResourceLoader::load(normal_path, "Texture");
						if (normalmap.is_valid()){
							poly->set_normalmap(normalmap);
							poly->set_light_mask(2);
						}
					}
					materials.apply_material(layer, poly, parent_clipper, clipper_image_data);
				}
				if (update_polygon){
					Ref<BitMap> bitmap_poly;
					if(p_options["texture/alpha_to_polygon"])
						bitmap_poly = read_bitmap(layer_image, bitmap_threshold, bitmap_size);
					poly->set_polygon(bitmap_to_polygon(bitmap_poly, polygon_size, scene_size.width * polygon_grow, 3.0, true)[0]);
				}
				if (update_polygon && false){
					PoolVector<Vector2> polygon;
					Vector<Vector2> polygon_vec;
					Ref<BitMap> bitmap_poly;
					if(p_options["texture/alpha_to_polygon"]){
						bitmap_poly = read_bitmap(layer_image, bitmap_threshold, bitmap_size);
						Rect2 r = Rect2(Point2(0, 0), bitmap_poly->get_size());
						// todo better grow poly value
						float grow = bitmap_poly->get_size().width * scene_size.width * polygon_grow / (MAX(polygon_size.width, polygon_size.height));
						bitmap_poly->grow_mask(grow, r);
						Vector<Vector<Vector2> > polygon_array = bitmap_poly->clip_opaque_to_polygons(r);
						if (polygon_array.size() == 1){
							polygon_vec = polygon_array[0];
							float min_distance = bitmap_poly->get_size().width * scene_size.width / (MAX(polygon_size.width, polygon_size.height) * vertex_count_per_scene);
							polygon_vec = simplify_polygon_distance(polygon_vec, min_distance, bitmap_poly->get_size());
							for (int i = 0; i < 3; i++){
								Vector<Vector2> polygon_vec2 = simplify_polygon_direction(polygon_vec, 0.95);
								if (polygon_vec.size() > 4){
									polygon_vec = polygon_vec2;
								}else{
									break;
								}
							}
						}
					}
					if (polygon_vec.size() > 4){
						Vector2 k = polygon_size / bitmap_poly->get_size();
						for (int i = 0; i < polygon_vec.size(); i++){
							polygon.append(polygon_vec[i]*k);
						}
					}else{
						polygon.append(Vector2(0,0));
						polygon.append(Vector2(polygon_size.x, 0));
						polygon.append(polygon_size);
						polygon.append(Vector2(0, polygon_size.y));
					}
					poly->set_polygon(polygon);
				}
				if (update_polygon_transform){
					Rect2 k = red::get_rect(poly->get_uv());
					k.size = k.size * clipper_image_data.count;
					Rect2 real = red::get_rect(poly->get_polygon());
					PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
					Rect2 target = Rect2(polygon_size * k.position, polygon_size * k.size);
					PoolVector<Vector2> new_poly;
					for (int i = 0; i < poly->get_polygon().size(); i++)
						new_poly.append(((polyr[i] - real.position) / real.size) * target.size + target.position);
					poly->set_polygon(new_poly);
					poly->set_position(local_pos);
				}
				else if (update_polygon_pos){
					poly->set_position(local_pos);
				}
				if (update_uv){
					Vector2 psd_offset = local_pos - poly->get_position();
					Rect2 poly_rect = red::get_rect(poly->get_polygon());
					Vector<Vector2> uv_temp;
					if (poly->get_uv().size() != poly->get_polygon().size() || reset_uv){
						PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
						for (int i = 0; i < poly->get_polygon().size(); i++)
							uv_temp.push_back(polyr[i] / poly_rect.size);
					}
					else{
						PoolVector<Vector2>::Read uvr = poly->get_uv().read();
						Rect2 real = red::get_rect(poly->get_uv());
						for (int i = 0; i < poly->get_uv().size(); i++)
							uv_temp.push_back((uvr[i] - real.position) / (real.size));
					}
					PoolVector<Vector2> new_uv;
					Vector2 uv_offset = psd_offset / polygon_size;
					Vector2 uv_size = poly_rect.size / polygon_size;
					for (int i = 0; i < uv_temp.size(); i++)
						new_uv.append((uv_temp[i] * uv_size - uv_offset)/clipper_image_data.count);
					poly->set_uv(new_uv);
				}
				if (reorder){
					Node *p = poly->get_parent();
					int extra_nodes = 0;
					for (int i = 0; i < MIN(p->get_child_count(), iter-start+extra_nodes+1); i++)
					{
						Node *child = p->get_child(i);
						bool found = false;
						for (int j = 0; j < names.size(); j++)
						{
							if (child->get_name() == names[j]){
								found = true;
								break;
							}
						}
						if (!found){
							extra_nodes++;
						}
					}
					int cliped_count_order = 0;
					for (int i = start+1; i < final_iter-1; i++)
					{
						if (layers[i].clipping){
							cliped_count_order += 1;
						}
					}
					p->move_child(poly, iter - start + extra_nodes - cliped_count_order);
				}
				poly->set_move_polygon_with_uv(old_move_polygon_with_uv);
			} break;
			case psd_layer_type::psd_layer_type_hidden:{
				_psd_layer_record *layer_folder;
				int new_folder_level = folder_level;
				{
					int child_folder_count = 0;
					int child_end_count = 0;
					for (int j = start; j < final_iter + 1; j++){
						if (layers[j].layer_type == psd_layer_type::psd_layer_type_hidden)
							new_folder_level++;
						if (layers[j].layer_type == psd_layer_type::psd_layer_type_folder)
							new_folder_level--;
					}
					for (int j = final_iter + 1; j < end; j++){
						if (layers[j].layer_type == psd_layer_type::psd_layer_type_hidden)
							child_folder_count++;
						if (layers[j].layer_type == psd_layer_type::psd_layer_type_folder)
							child_end_count++;
						if (child_end_count > child_folder_count){
							layer_folder = &layers[j];
							break;
						}
					}
				}
				String folder_name = reinterpret_cast<char const*>(layer_folder->layer_name);
				int mode = 0;
				int anchor = 0;
				if(new_folder_level == 0){
					mode = p_options["type/root"];
					anchor = ANCHOR_TOP_LEFT;
				}
				else if (new_folder_level == 1){
					mode = p_options["type/first_level"];
					anchor = p_options["anchor/first_level"];
				}
				else if (new_folder_level == 2){
					mode = p_options["type/second_level"];
					anchor = p_options["anchor/second_level"];
				}
				else{
					mode = p_options["type/folder"];
					anchor = p_options["anchor/folder"];
				}
				if (folder_name == ""){
					folder_name = "root";
				}
				else{
					if ((mode == FOLDER_FRAME_EXTERNAL || mode == FOLDER_FRAME) && folder_name.find("_internal") != -1)
					{
						mode = FOLDER_FRAME;
						folder_name = folder_name.replace_first("_internal", "");
					}
					if ((mode == FOLDER_FRAME_EXTERNAL || mode == FOLDER_FRAME) && folder_name.find("_external") != -1)
					{
						mode = FOLDER_FRAME_EXTERNAL;
						folder_name = folder_name.replace_first("_external", "");
					}
					if (folder_name.find("_center") != -1)
					{
						anchor = ANCHOR_CENTER;
						folder_name = folder_name.replace_first("_center", "");
					}
				}
				String new_target_dir = mode != FOLDER_PAGE ? target_dir + "/" + folder_name : target_dir;
				String new_scene_path = new_target_dir + "/" + folder_name + ".tscn";

				Materials new_materials;
				bool new_force_save = false;
				bool need_create = parent->has_node(folder_name) ? false : true;
				bool external_frame_created = false;
				Node *node_external = nullptr;
				Node2D *node_2d = nullptr;
				REDPage *page = nullptr;
				REDFrame *frame = nullptr;
				REDFrame *clipper = parent_clipper;
				Node2D *anchor_object = nullptr;
				switch (mode){
					case FOLDER_NODE2D:{
						node_2d = need_create ? red::create_node<Node2D>(parent, folder_name) : (Node2D*)parent->get_node(folder_name);
						anchor_object = node_2d;
						anchor_object->set_draw_behind_parent(true);
					} break;
					case FOLDER_PAGE:{
						page = need_create ? red::create_node<REDPage>(parent, folder_name) : (REDPage*)parent->get_node(folder_name);
						node_2d = (Node2D*)page;
						anchor_object = node_2d;
					} break;
					case FOLDER_FRAME:{
						frame = need_create ? red::create_node<REDFrame>(parent, folder_name) : (REDFrame*)parent->get_node(NodePath(folder_name));
						
						if (layer_folder->layer_mask_info.width!=0 && layer_folder->layer_mask_info.height!=0){
							bool need_create_clipper = frame->has_node(folder_name + "_mask") ? false : true;
						}
						clipper = (REDFrame*)frame;
						node_2d = (Node2D*)frame;
						anchor_object = node_2d;
					} break;
					case FOLDER_FRAME_EXTERNAL:{
						red::create_dir(new_target_dir);
						if (!file_сheck.file_exists(new_scene_path))
							new_force_save = true;
						red::scene_loader(new_scene_path);
						if (need_create){
							RES res = ResourceLoader::load(new_scene_path);
							Ref<PackedScene> scene = Ref<PackedScene>(Object::cast_to<PackedScene>(*res));
							Node *instanced_scene = scene->instance(PackedScene::GEN_EDIT_STATE_INSTANCE);
							instanced_scene->set_name(folder_name);
							parent->add_child(instanced_scene);
							instanced_scene->set_owner(parent->get_owner());
						}
						Node *root_node = nullptr;
						if (!EditorNode::get_singleton()->is_scene_open(new_scene_path)){
							EditorNode::get_singleton()->load_scene(new_scene_path);
						}
						root_node = get_edited_scene_root(new_scene_path);
						if (root_node == nullptr){
							Ref<PackedScene> scene = ResourceLoader::load(new_scene_path, "PackedScene");
							root_node = scene->instance();
						}
						if (!root_node->has_node(folder_name)){
							frame = red::create_node<REDFrame>(root_node, folder_name);
							external_frame_created = true;
						}
						else
							frame = (REDFrame*)root_node->get_node(folder_name);
						
						node_external = (Node*)frame;
						clipper = (REDFrame*)frame;
						node_2d = (Node2D*)parent->get_node(folder_name);
						anchor_object = (Node2D*)frame;
					} break;
					default:
					case FOLDER_PARALLAX:{
						node_2d = need_create ? (Node2D*)red::create_node<REDParallaxFolder>(parent, folder_name) : (Node2D*)parent->get_node(folder_name);
						anchor_object = node_2d;
						anchor_object->set_draw_behind_parent(true);
					} break;
				}
				if (!node_2d || (frame == nullptr && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL)) || (!page && (mode == FOLDER_PAGE)))
					continue;
				Vector2 new_parent_pos = parent_pos;
				Vector2 new_parent_offset = Vector2(0, 0);
				bool folder_mask = p_options["update/folder_mask"];
				bool frame_targets = p_options["update/frame_targets"];
				bool page_height = p_options["update/page_height"];
				int folder_pos = p_options["update/folder_pos"];
				bool apply_mask = (updateble && folder_mask) && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL) && clipper;
				bool update_targets = (updateble && frame_targets) && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
				bool update_page_height = updateble && page_height && mode == FOLDER_PAGE;
				bool update_frame_params = updateble && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
				bool update_pos_reset = updateble && folder_pos == FOLDER_POS_RESET;
				bool update_pos_move_layers = updateble && folder_pos == FOLDER_MOVE_LAYERS;
				
				float offset_x = layer_folder->layer_mask_info.left * scene_k;
				float offset_y = layer_folder->layer_mask_info.top * scene_k;
				Vector2 local_pos = Vector2(offset_x, offset_y);
				if (need_create){
					apply_mask = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL) && clipper;
					update_targets = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
					update_page_height = mode == FOLDER_PAGE;
					update_frame_params = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
					update_pos_reset = true;
					update_pos_move_layers = false;
				}

				if (apply_mask){
					Ref<BitMap> bitmap_poly;
					Ref<Image> bitmap_img = read_mask(layer_folder);
					Size2 image_size;
					Size2 polygon_size = image_size * scene_k;
					if (bitmap_img.is_valid())
						image_size = bitmap_img->get_size();

					if (polygon_size.width > 0 && polygon_size.height > 0)
						bitmap_poly = read_bitmap(bitmap_img, 0.2f, Vector2(MIN(128, image_size.x), MIN(128, image_size.y)));
					else
						polygon_size = scene_size;
					clipper->set_polygon(bitmap_to_polygon(bitmap_poly, polygon_size, 0.0, 4.0, true)[0]);
				}

				if (apply_mask && false){
					Ref<Image> bitmap_img = read_mask(layer_folder);
					PoolVector<Vector2> polygon;
					if (bitmap_img.is_valid()){
						Vector2 image_size = bitmap_img->get_size();
						Vector2 resized_size = Vector2(MIN(128, image_size.x), MIN(128, image_size.y));
						bitmap_img->resize(resized_size.x, resized_size.y);
						Ref<BitMap> bitmap_mask;
						bitmap_mask.instance();
						bitmap_mask->create_from_image_alpha(bitmap_img, 0.2f);
						if (bitmap_mask.is_valid()){
							Vector<Vector<Vector2> > polygon_array = bitmap_mask->clip_opaque_to_polygons(Rect2(Point2(), bitmap_mask->get_size()));
							Vector<Vector2> polygon_vec;
							if (polygon_array.size() > 0){
								polygon_vec = polygon_array[0];
								if (polygon_vec.size() > 4){
									polygon_vec = simplify_polygon_distance(polygon_vec, 4.0f, bitmap_mask->get_size());
								}
								if (polygon_vec.size() > 4){
									polygon_vec = simplify_polygon_direction(polygon_vec);
								}
								if (polygon_vec.size() > 2){
									Size2 k = Size2(image_size.width * scene_k, 
													image_size.height * scene_k) / bitmap_mask->get_size();
									for (int i = 0; i < polygon_vec.size(); i++){
										polygon.append(polygon_vec[i] * k);
									}
								}
							}
						}
					}
					if (polygon.size() < 2){
						polygon.append(Vector2(0, 0));
						polygon.append(Vector2(scene_size.width, 0));
						polygon.append(scene_size);
						polygon.append(Vector2(0, scene_size.height));
					}
					clipper->set_polygon(polygon);
				}
				Vector2 half_frame = (clipper) ? clipper->_edit_get_rect().size / 2.0 : Vector2(0,0);
				Vector2 frame_anchor_offset = (anchor == ANCHOR_CENTER) ? -half_frame : Vector2(0,0);
				new_parent_pos += local_pos;
				new_parent_offset = -frame_anchor_offset;
				if (update_pos_reset){
					if (clipper && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL)){
						clipper->set_offset(frame_anchor_offset);
						clipper->set_position(-frame_anchor_offset);
					}
					if (node_2d && mode != FOLDER_FRAME_EXTERNAL){
						node_2d->set_position(local_pos - parent_pos - parent_offset - frame_anchor_offset);
					}
				}
				else if (update_pos_move_layers){
					if (mode == FOLDER_FRAME_EXTERNAL){
						new_parent_offset += node_2d->get_position();// - frame_anchor_offset;
					}
					else{
						new_parent_offset -= (local_pos - parent_pos - parent_offset - frame_anchor_offset) - node_2d->get_position();
					}
				}
				if (update_page_height){
					page->set_size(scene_size);
				}
				if (update_targets){
					Node *targets = red::create_node<Node>(anchor_object, "targets");
					if(targets){
						REDTarget *camera_pos = red::create_node<REDTarget>(targets, "camera_pos");
						REDTarget *camera_pos_in = red::create_node<REDTarget>(targets, "camera_pos_in");
						REDTarget *camera_pos_out = red::create_node<REDTarget>(targets, "camera_pos_out");
						REDTarget *parallax_pos = red::create_node<REDTarget>(targets, "parallax_pos");
						REDTarget *parallax_pos_in = red::create_node<REDTarget>(targets, "parallax_pos_in");
						REDTarget *parallax_pos_out = red::create_node<REDTarget>(targets, "parallax_pos_out");
						frame->set_camera_pos_path(frame->get_path_to(camera_pos));
						frame->set_camera_pos_in_path(frame->get_path_to(camera_pos_in));
						frame->set_camera_pos_out_path(frame->get_path_to(camera_pos_out));
						frame->set_parallax_pos_path(frame->get_path_to(parallax_pos));
						frame->set_parallax_pos_in_path(frame->get_path_to(parallax_pos_in));
						frame->set_parallax_pos_out_path(frame->get_path_to(parallax_pos_out));
						camera_pos->set_position(half_frame + frame_anchor_offset);
						camera_pos_in->set_position(half_frame + frame_anchor_offset);
						camera_pos_out->set_position(half_frame + frame_anchor_offset);
						parallax_pos->set_position(half_frame + frame_anchor_offset);
						parallax_pos_in->set_position(half_frame + frame_anchor_offset);
						parallax_pos_out->set_position(half_frame + frame_anchor_offset);
					}
				}
				if (update_frame_params){
					new_materials.init(p_options, frame);
					frame->set_material(new_materials.frame_outline);
					frame->set_line_texture(ResourceLoader::load("res://redot/textures/outline/16.png", "Texture"));
					parent->move_child(node_2d, parent->get_child_count() - 1);
				}
				else{
					new_materials = materials;
				}
				Node *node = mode == FOLDER_FRAME_EXTERNAL ? node_external : node_2d;
				if (node != nullptr)
					offset += load_folder(context, new_scene_path, final_iter+1, new_materials, node, new_parent_pos, new_parent_offset, p_options, new_force_save, counter+1, new_folder_level, clipper);
				else
					ERR_PRINTS("Error can't create node " + folder_name);
			} break;
			default:
				break;
		}
		
	}
	if (saveble){
		int mode = 0;
		if (folder_level == 0){
			mode = p_options["type/root"];
		}
		else if (folder_level == 1){
			mode = p_options["type/first_level"];
		}
		else if (folder_level == 2){
			mode = p_options["type/second_level"];
		}
		else{
			mode = p_options["type/folder"];
		}
		if(folder_level == -1 || (mode == FOLDER_FRAME_EXTERNAL)){
			Ref<PackedScene> scene = memnew(PackedScene);
			Node *owner = parent->get_owner();
			if (!owner){
				owner = parent;
			}
			Error err = scene->pack(owner);
			if (err == OK){
				err = ResourceSaver::save(scene_path, scene);
			}
		}
	}
	return count + offset;
}
Error ResourceImporterPSD::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	ERR_FAIL_COND_V_MSG(!ProjectSettings::get_singleton(), OK, "Error can't read path: " + p_source_file);
	char *global = _strdup(ProjectSettings::get_singleton()->globalize_path(p_source_file).utf8().get_data());
	String scene_path = p_source_file.get_basename() + ".tscn";
	_psd_context * context = NULL;
	psd_status status = psd_image_load(&context, global);
	ERR_FAIL_COND_V_MSG(status != psd_status::psd_status_done, ERR_FILE_CANT_READ, 
						"Error can't read file: " + ProjectSettings::get_singleton()->globalize_path(p_source_file));
	bool force_save = true;
	Node *parent = _get_root(context, scene_path, force_save, p_options);
	ERR_FAIL_NULL_V(parent, OK);
	Materials mats;
	mats.init(p_options, parent);
	load_folder(context, scene_path, 0, mats, parent, Vector2(0,0), Vector2(0,0), p_options, force_save, 0);
	psd_image_free(context);
	return OK;
}

ResourceImporterPSD::ResourceImporterPSD() {
}
