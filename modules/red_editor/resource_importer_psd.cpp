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
#include "modules/red/red_engine.h"
#include "modules/red/red_parallax_folder.h"
#include "modules/red/red_target.h"

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
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "main/resolution_width"), 2000.0f));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/update"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/update_only_editor"), true));

	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/texture"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/page_height"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/frame_targets"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/folder_mask"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/order"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "update/folder_pos", PROPERTY_HINT_ENUM, "Ignore, Move, Move layers"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "update/layer_pos", PROPERTY_HINT_ENUM, "Ignore, Move and scale, Reset UV, Move UV, Move and reset UV, Move and move UV"), 2));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/second_level", PROPERTY_HINT_ENUM, "Top left, Center"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/folder", PROPERTY_HINT_ENUM, "Top left, Center"), 1));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/root", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame"), 2));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/second_level", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 4));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/folder", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 1));
	//r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/layer", PROPERTY_HINT_ENUM, "Polygon2D, REDPolygon"), 1));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/max_size"), 4096));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/scale", PROPERTY_HINT_ENUM, "Original, Downscale, Upscale, Closest"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/bitmap_size"), 128));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "texture/alpha_to_polygon"), false));
	
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/outline_shader", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/outline_shader.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/shader", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/default.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/shader_mul", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/default_mul.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/shader_add", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/default_add.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/shader_sub", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/default_sub.shader")));
	
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/masked_shader", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/masked_shader.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/masked_shader_mul", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/masked_shader_mul.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/masked_shader_add", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/masked_shader_add.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/masked_shader_sub", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/masked_shader_sub.shader")));
}

Vector<Vector2> ResourceImporterPSD::simplify_polygon_distance(Vector<Vector2> &p_polygon, float min_distance, Size2 size){
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
		if ((n_dist >= min_distance && p_dist >= min_distance) || (i == 0 && !last_on_side) || on_side){
			prev_added = true;
			output.push_back(point);
		}else{
			prev_added = false;
		}
	}
	return output;
}

Vector<Vector2> ResourceImporterPSD::simplify_polygon_direction(Vector<Vector2> &p_polygon, float min_dot){
	int prev = 0;
	int next = 0;
	Vector<Vector2> polygon_dist;
	for (int i = 0; i < p_polygon.size(); i++){
		if (i==0)
			prev = p_polygon.size()-1;
		else
			prev = i - 1;
		if (i == p_polygon.size()-1)
			next = 0;
		else
			next = i + 1;
		Vector2 p = (p_polygon[prev] - p_polygon[i]).normalized();
		Vector2 n = (p_polygon[next] - p_polygon[i]).normalized();
		if (p.dot(n)>min_dot)
			polygon_dist.push_back(p_polygon[i]);
	}
	return polygon_dist;
}
/*
Vector<Vector2> ResourceImporterPSD::load_polygon_from_mask(_psd_layer_record *layer, float target_width, int psd_width){
	PoolVector<uint8_t> data;
	int len = layer->layer_mask_info.width*layer->layer_mask_info.height;
	if ((layer->layer_mask_info.width)<2 || (layer->layer_mask_info.height)<2){
		Vector<Vector2> output;
		return output;
	}
	data.resize(len*2);
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
	Vector2 image_size = img->get_size();
	Vector2 max_size(128, 128);
	Vector2 resized_size = Vector2(MIN(max_size.x, image_size.x), MIN(max_size.y, image_size.y));
	img->resize(resized_size.x, resized_size.y);

	Ref<BitMap> bitmap;
	bitmap.instance();
	bitmap->create_from_image_alpha(img, 0.95f);
	Rect2 rect;
	rect.size = resized_size;
	Vector<Vector<Vector2> > polygon_array = bitmap->clip_opaque_to_polygons(rect);
	if (polygon_array.size() > 0){
		int points_count = polygon_array[0].size();
		float aspect_ratio = (float)(image_size.height)/image_size.width;
		float width = image_size.width*target_width/psd_width;
		float height = width*aspect_ratio;
		Vector2 mask_size = Vector2(width, height);
		Vector2 k = mask_size/resized_size;

		int prev = 0;
		int next = 0;
		Vector<Vector2> output;
		for (int i = 0; i <points_count; i++){
			if (i==0)
				prev = points_count-1;
			else
				prev = i - 1;
			if (i == points_count-1)
				next = 0;
			else
				next = i + 1;
			float p_dist = polygon_array[0][prev].distance_to(polygon_array[0][i]);
			float n_dist = polygon_array[0][next].distance_to(polygon_array[0][i]);
			if (n_dist>=2.0f){
				output.push_back(polygon_array[0][i] * k);
			}
		}
		return output;
	}else{
		Vector<Vector2> output;
		return output;
	}
}*/

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
	Size2 resized_size = Vector2(MIN(max_size.x, image_size.x), MIN(max_size.y, image_size.y));
	img->resize(resized_size.x, resized_size.y);
	Ref<BitMap> bitmap;
	bitmap.instance();
	bitmap->create_from_image_alpha(img);
	return bitmap;
}

Ref<BitMap> ResourceImporterPSD::read_bitmap(_psd_layer_record *layer, float p_threshold, Size2 max_size){
	Ref<Image> img = read_image(layer);
	Vector2 image_size = img->get_size();
	Size2 resized_size = Vector2(MIN(max_size.x, image_size.x), MIN(max_size.y, image_size.y));
	img->resize(resized_size.x, resized_size.y);
	Ref<BitMap> bitmap;
	bitmap.instance();
	bitmap->create_from_image_alpha(img);
	return bitmap;
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
			w[j+3] = (uint8_t) ((pixel & 0xFF000000) >> 24);
			j+=4;
		}
		img.instance();
		img->create(layer->width, layer->height, false, Image::FORMAT_RGBA8, data);
	}
	return img;
}

Rect2 ResourceImporterPSD::crop_alpha(Ref<BitMap> bitmap, int grow){
	int x_min = 0;
	int x_max = 0;
	int y_min = 0;
	int y_max = 0;
	Size2 size = bitmap->get_size();
	bitmap->grow_mask(grow, Rect2(Point2(0.0f, 0.0f), size));
	Point2 xt = Point2(0.f, 0.f);
	for (int x = 0; x < size.width; x++) {
		for (int y = 0; y < size.height; y++) {
			xt.x = x;
			xt.y = y;
			if (bitmap->get_bit(xt)){
				x_min = MIN(x_min, x);
				x_max = MAX(x_min, x);
				y_min = MIN(y_min, y);
				y_max = MAX(y_max, y);
			}
		}
	}
	return Rect2(x_min, y_min, x_max - x_min, y_max - y_min);
}

void ResourceImporterPSD::apply_scale(Ref<Image> img, int texture_scale, int texture_max_size){
	if (texture_scale != ORIGINAL)
	{
		int new_width = img->get_width();
		int new_height = img->get_height();
		if (texture_scale == DOWNSCALE)
		{
			new_width = previous_power_of_2(new_width);
			new_height = previous_power_of_2(new_height);
		}else if (texture_scale == UPSCALE){
			new_width = next_power_of_2(new_width);
			new_height = next_power_of_2(new_height);
		}else if (texture_scale == CLOSEST){
			new_width = closest_power_of_2(new_width);
			new_height = closest_power_of_2(new_height);
		}
		if (new_width > texture_max_size){
			new_width = texture_max_size;
		}
		if (new_height > texture_max_size){
			new_height = texture_max_size;
		}
		if (new_width != img->get_width() || new_height != img->get_height())
			img->resize(new_width, new_height, Image::INTERPOLATE_CUBIC);
	}
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

Node *ResourceImporterPSD::get_edited_scene_root(String p_path) const{
	EditorData &editor_data = EditorNode::get_editor_data();
	//EditorData editor_data = EditorNode::get_editor_data();
	for (int i = 0; i < editor_data.get_edited_scene_count(); i++) {
		if (editor_data.get_scene_path(i) == p_path)
			return editor_data.get_edited_scene_root(i);
	}
	return nullptr;
}


Node *ResourceImporterPSD::_get_root(_psd_context *context, const String &target_dir, bool &force_save, const Map<StringName, Variant> &p_options) const{
	red::create_dir(target_dir);
	_psd_layer_record *last_layer = &(context->layer_records[context->layer_count-1]);
	String scene_path;
	Node *root = nullptr;

	scene_path = target_dir + ".tscn";

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
	//if (b_update_only_editor){
	if (!EditorNode::get_singleton()->is_scene_open(scene_path)){
		EditorNode::get_singleton()->load_scene(scene_path);
	}
	root = get_edited_scene_root(scene_path);
	//}
	if (root == nullptr){
		
		Ref<PackedScene> scene = ResourceLoader::load(scene_path, "PackedScene");
		root = scene->instance();
	}
	return root;
}

void Materials::init(const Map<StringName, Variant> &p_options, Node *node){
	Vector<Node*> nodes;
	red::get_all_children(node, nodes);
	int count = nodes.size();

	Ref<Shader> outline_shader = p_options["shaders/outline_shader"];

	Ref<Shader> shader = p_options["shaders/shader"];
	Ref<Shader> shader_mul = p_options["shaders/shader_mul"];
	Ref<Shader> shader_add = p_options["shaders/shader_add"];
	Ref<Shader> shader_sub = p_options["shaders/shader_sub"];

	Ref<Shader> masked_shader = p_options["shaders/masked_shader"];
	Ref<Shader> masked_shader_mul = p_options["shaders/masked_shader_mul"];
	Ref<Shader> masked_shader_add = p_options["shaders/masked_shader_add"];
	Ref<Shader> masked_shader_sub = p_options["shaders/masked_shader_sub"];
	for (int i = 0; i < count; i++)
	{
		if (!nodes[i]->is_class("CanvasItem"))
			continue;
		CanvasItem *c = (CanvasItem*)nodes[i];
		Ref<Material> m = c->get_material();
		if (m.is_null())
			continue;
		Ref<ShaderMaterial> mat = Ref<ShaderMaterial>(m);
		if (mat.is_null())
			continue;
		Ref<Shader> shader = mat->get_shader();
		if (shader.is_null())
			continue;
		String shader_path = shader->get_path();
		if (shader_path == masked_shader->get_path()){
			masked_material = mat;
		} else if (shader_path==masked_shader_mul->get_path()){
			masked_material_mul = mat;
		} else if (shader_path==masked_shader_add->get_path()){
			masked_material_add = mat;
		} else if (shader_path==masked_shader_sub->get_path()){
			masked_material_sub = mat;
		}
	}
	Vector<Node*> root_nodes;
	Node *owner = node->get_owner();
	if (!owner){
		owner = node->get_parent();
		if (!owner){
			owner = node;
		}
	}
	red::get_all_children(owner, root_nodes);
	count = root_nodes.size();
	for (int i = 0; i < count; i++){
		if (!root_nodes[i]->is_class("CanvasItem"))
			continue;
		CanvasItem *c = (CanvasItem*)root_nodes[i];
		Ref<Material> m = c->get_material();
		if (m.is_null())
			continue;
		Ref<ShaderMaterial> mat = Ref<ShaderMaterial>(m);
		if (mat.is_null())
			continue;
		Ref<Shader> shader = mat->get_shader();
		if (shader.is_null())
			continue;
		String shader_path = shader->get_path();
		if (shader_path == shader->get_path()){
			material = mat;
		} else if (shader_path == shader_mul->get_path()){
			material_mul = mat;
		} else if (shader_path == shader_add->get_path()){
			material_add = mat;
		} else if (shader_path == shader_sub->get_path()){
			material_sub = mat;
		}else if (shader_path == outline_shader->get_path()){
			outline_material = mat;
		}
	}
	if (outline_material.is_null()){
		outline_material.instance();
		outline_material->set_shader(outline_shader);
	}
	if (material.is_null()){
		material.instance();
		material->set_shader(shader);
	}
	if (material_mul.is_null()){
		material_mul.instance();
		material_mul->set_shader(shader_mul);
	}
	if (material_add.is_null()){
		material_add.instance();
		material_add->set_shader(shader_add);
	}
	if (material_sub.is_null()){
		material_sub.instance();
		material_sub->set_shader(shader_sub);
	}
	if (masked_material.is_null()){
		masked_material.instance();
		masked_material->set_shader(masked_shader);
	}
	if (masked_material_mul.is_null()){
		masked_material_mul.instance();
		masked_material_mul->set_shader(masked_shader_mul);
	}
	if (masked_material_add.is_null()){
		masked_material_add.instance();
		masked_material_add->set_shader(masked_shader_add);
	}
	if (masked_material_sub.is_null()){
		masked_material_sub.instance();
		masked_material_sub->set_shader(masked_shader_sub);
	}
}

void Materials::apply_material(_psd_layer_record *layer, Node2D *node, REDFrame *parent_clipper){
	switch(layer->blend_mode){
		case psd_blend_mode_multiply:{
			if (parent_clipper == nullptr){
				if (material_mul.is_valid())
					node->set_material(material_mul);
			}else{
				if (masked_material_mul.is_valid())
					node->set_material(material_mul);
			}
			break;
		}
		case psd_blend_mode_linear_dodge:{
			if (parent_clipper == nullptr){
				if (material_add.is_valid())
					node->set_material(material_add);
			}else{
				if (masked_material_add.is_valid())
					node->set_material(material_add);
			}
			break;
		}

		case psd_blend_mode_difference:{
			if (parent_clipper == nullptr){
				if (material_sub.is_valid())
					node->set_material(material_sub);
			}else{
				if (masked_material_sub.is_valid())
					node->set_material(material_sub);
			}
			break;	
		}

		default:{
			if (parent_clipper == nullptr){
				if (material.is_valid()){
					node->set_material(material);
				}
			}else{
				if (masked_material.is_valid()){
					node->set_material(material);
				}
			}
			break;
		}
	}
}


int ResourceImporterPSD::load_folder(_psd_context *context, String target_dir, int start, Materials &materials, 
									 Node *parent, Vector2 parent_pos, Vector2 parent_offset, const Map<StringName, Variant> &p_options, bool force_save, int counter, int folder_level, REDFrame *parent_clipper){
	psd_layer_record *layers = context->layer_records;
	int count = 0;
	int offset = 0;
	int final_iter = 0;
	int end = context->layer_count;
	bool loop = true;

	float resolution_width = p_options["main/resolution_width"];
	bool b_update = p_options["main/update"];
	bool b_update_only_editor = p_options["main/update_only_editor"];
	bool updateble = true;
	if (parent->get_owner()){
		updateble = p_options["main/update"] && (!p_options["main/update_only_editor"] || EditorNode::get_singleton()->is_scene_open(parent->get_owner()->get_filename()));
	}
	else{
		updateble = p_options["main/update"] && (!p_options["main/update_only_editor"] || EditorNode::get_singleton()->is_scene_open(parent->get_filename()));
	}
	bool saveble = force_save || (!p_options["main/update_only_editor"] && p_options["main/update"]);
	Vector<String> names;
	for (int iter = start; iter + offset < end && loop; iter++){
		names.push_back(reinterpret_cast<char const*>(layers[iter + offset].layer_name));
	}
	_File file_сheck;
	for (int iter = start; iter + offset < end && loop; iter++){	
		final_iter = iter + offset; count++;
		_psd_layer_record *layer = &layers[final_iter];
		String name = reinterpret_cast<char const*>(layers[final_iter].layer_name);

		switch (layers[final_iter].layer_type){
			case psd_layer_type::psd_layer_type_folder:{
				if (counter > 0){
					loop = false;
				}
			} break;
			case psd_layer_type::psd_layer_type_normal:{
				int len = layer->width*layer->height;
				if (len == 0) 
					continue;
				red::create_dir( target_dir + "/textures");
				String png_path = target_dir + "/textures/"+ name +".png";
				bool save_texture = !file_сheck.file_exists(png_path) || (p_options["update/texture"] && updateble);
				int update_pos = p_options["update/layer_pos"];

				bool update_material = false;
				bool update_polygon = false;
				bool update_polygon_pos = updateble && (update_pos == LAYER_MOVE_AND_RESET_UV || update_pos == LAYER_MOVE_AND_MOVE_UV);
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
				Size2 bitmap_size = Size2(p_options["texture/bitmap_size"], p_options["texture/bitmap_size"]);
				Ref<Image> img = read_image(layer);
				Ref<BitMap> bitmap = read_bitmap(img, 0.1f, bitmap_size);
				bitmap_size = bitmap->get_size();
				Rect2 crop = crop_alpha(bitmap, bitmap_size.width / 16);
				crop.position = crop.position * img->get_size() / bitmap_size;
				crop.size = ((crop.size) * img->get_size()) / (bitmap_size - Vector2(1.0f, 1.0f));
				Ref<Image> img_cropped = img->duplicate();
				Ref<Image> img_save;
				if (save_texture || update_polygon){
					img_cropped->crop_from_point(crop.position.x, crop.position.y, crop.size.width, crop.size.height);
					img_save = img_cropped->duplicate();
				}
				Vector2 polygon_size(crop.size.width * resolution_width / context->width, 
									 crop.size.height * resolution_width / context->width);
				Point2 global_pos = Point2((layer->left+crop.position.x) * resolution_width / context->width, 
										   (layer->top+crop.position.y) * resolution_width / context->width);
				if (save_texture){
					apply_scale(img_save, p_options["texture/scale"], p_options["texture/max_size"]);
					apply_border(img_save, 0.75f);
					img_save->save_png(png_path);
					if (ResourceLoader::import){
						ResourceLoader::import(png_path);
					}
				}
				if (update_material){
					Ref<Texture> texture = ResourceLoader::load(png_path, "Texture");
					if (texture.is_valid())
						poly->set_texture(texture);
					materials.apply_material(layer, poly, parent_clipper);
				}
				if (update_polygon){
					PoolVector<Vector2> polygon;
					Vector<Vector2> polygon_vec;
					Ref<BitMap> bitmap_poly;
					if(p_options["texture/alpha_to_polygon"]){
						bitmap_poly = read_bitmap(img_cropped, 0.1f, Size2(128, 128));
						Rect2 r = Rect2(Point2(0, 0), bitmap_poly->get_size());
						bitmap_poly->grow_mask(8, r);
						Vector<Vector<Vector2> > polygon_array = bitmap_poly->clip_opaque_to_polygons(r);
						if (polygon_array.size() == 1){
							polygon_vec = polygon_array[0];
							polygon_vec = simplify_polygon_distance(polygon_vec, 4.0f, bitmap_poly->get_size());
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
					Rect2 real = red::get_rect(poly->get_polygon());
					PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
					Rect2 target = Rect2(polygon_size*k.position, polygon_size * k.size);
					PoolVector<Vector2> new_poly;
					for (int i = 0; i < poly->get_polygon().size(); i++)
						new_poly.append(((polyr[i] - real.position) / real.size) * target.size + target.position);
					poly->set_polygon(new_poly);
					poly->set_position(global_pos - parent_pos - parent_offset - poly->get_offset());
				}
				else if (update_polygon_pos){
					poly->set_position(global_pos - parent_pos - parent_offset - poly->get_offset());
				}
				if (update_uv){
					Vector2 psd_offset = global_pos - parent_pos - parent_offset - poly->get_offset() - poly->get_position();
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
						new_uv.append(uv_temp[i] * uv_size - uv_offset);
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
					p->move_child(poly, iter-start+extra_nodes);
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
				int anchor = ANCHOR_TOP_LEFT;
				if(new_folder_level == 0)
					mode = p_options["types/root"];
				else if (new_folder_level == 1){
					mode = p_options["types/second_level"];
					anchor = p_options["anchor/second_level"];
				}
				else{
					mode = p_options["types/folder"];
					anchor = p_options["anchor/folder"];
				}
				if (folder_name == ""){
					folder_name = "root";
				}
				else{
					if ((mode == FOLDER_FRAME_EXTERNAL || mode == FOLDER_FRAME)&& folder_name.find("_internal") != -1)
					{
						mode = FOLDER_FRAME;
						folder_name = folder_name.replace_first("_internal", "");
					}
					if ((mode == FOLDER_FRAME_EXTERNAL || mode == FOLDER_FRAME)&& folder_name.find("_external") != -1)
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
				String new_target_dir = target_dir;
				if (mode == FOLDER_FRAME_EXTERNAL)
				{
					new_target_dir += "/" + folder_name;	
				}
				red::create_dir(new_target_dir);
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
						String scene_path = (new_target_dir + ".tscn");

						if (!file_сheck.file_exists(scene_path))
							new_force_save = true;

						red::scene_loader(scene_path);
						if (need_create){
							RES res = ResourceLoader::load(scene_path);
							Ref<PackedScene> scene = Ref<PackedScene>(Object::cast_to<PackedScene>(*res));
							Node *instanced_scene = scene->instance(PackedScene::GEN_EDIT_STATE_INSTANCE);
							instanced_scene->set_name(folder_name);
							parent->add_child(instanced_scene);
							instanced_scene->set_owner(parent->get_owner());
						}
						Node *root_node = nullptr;
						//if (b_update_only_editor){
						if (!EditorNode::get_singleton()->is_scene_open(scene_path)){
							EditorNode::get_singleton()->load_scene(scene_path);
						}
						root_node = get_edited_scene_root(scene_path);
						//}
						if (root_node == nullptr){
							Ref<PackedScene> scene = ResourceLoader::load(scene_path, "PackedScene");
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
				bool update_pos_reset = updateble && (folder_pos == FOLDER_POS_RESET);
				bool update_pos_move_layers = (updateble && folder_pos == FOLDER_MOVE_LAYERS);
				
				float offset_x = layer_folder->layer_mask_info.left * resolution_width / context->width;
				float offset_y = layer_folder->layer_mask_info.top * resolution_width / context->width;
				Vector2 local_pos = Vector2(offset_x, offset_y);
				Vector2 half_frame = (clipper) ? clipper->_edit_get_rect().size/2 : Vector2(0,0);
				Vector2 frame_anchor_offset = (anchor == ANCHOR_CENTER) ? -half_frame : Vector2(0,0);
				new_parent_pos += local_pos;
				new_parent_offset = -frame_anchor_offset;
				if (need_create){
					apply_mask = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL) && clipper;
					update_targets = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
					update_page_height = mode == FOLDER_PAGE;
					update_frame_params = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
					update_pos_reset = true;
					update_pos_move_layers = false;
				}
				if (apply_mask){
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
									float aspect_ratio = (float)(image_size.height)/image_size.width;
									float width = image_size.width*resolution_width/context->width;
									float height = width*aspect_ratio;
									Vector2 k = Vector2(width, height) / bitmap_mask->get_size();
									for (int i = 0; i < polygon_vec.size(); i++){
										polygon.append(polygon_vec[i] * k);
									}
								}
							}
						}
					}
					if (polygon.size() < 2){
						float width = resolution_width;
						float height = (resolution_width*(float)context->height)/context->width;
						polygon.append(Vector2(0, 0));
						polygon.append(Vector2(width, 0));
						polygon.append(Vector2(width, height));
						polygon.append(Vector2(0, height));
					}
					clipper->set_polygon(polygon);
				}
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
						new_parent_pos += node_2d->get_position();// - frame_anchor_offset;
					}
					else{
						new_parent_pos -= (local_pos - parent_pos - parent_offset - frame_anchor_offset)-node_2d->get_position();
					}
				}
				if (update_page_height){
					page->set_size(Size2(resolution_width, ((float)context->height) / context->width*resolution_width));
				}
				if (update_targets){
					Node2D *targets2d = red::create_node<Node2D>(anchor_object, "targets");
					//Node *targets = red::create_node<Node>(anchor_object, "targets");
					if(targets2d){
						REDTarget *camera_pos = red::create_node<REDTarget>(targets2d, "camera_pos");
						REDTarget *camera_pos_in = red::create_node<REDTarget>(targets2d, "camera_pos_in");
						REDTarget *camera_pos_out = red::create_node<REDTarget>(targets2d, "camera_pos_out");
						REDTarget *parallax_pos = red::create_node<REDTarget>(targets2d, "parallax_pos");
						REDTarget *parallax_pos_in = red::create_node<REDTarget>(targets2d, "parallax_pos_in");
						REDTarget *parallax_pos_out = red::create_node<REDTarget>(targets2d, "parallax_pos_out");
						frame->set_camera_pos_path(frame->get_path_to(camera_pos));
						frame->set_camera_pos_in_path(frame->get_path_to(camera_pos_in));
						frame->set_camera_pos_out_path(frame->get_path_to(camera_pos_out));
						frame->set_parallax_pos_path(frame->get_path_to(parallax_pos));
						frame->set_parallax_pos_in_path(frame->get_path_to(parallax_pos_in));
						frame->set_parallax_pos_out_path(frame->get_path_to(parallax_pos_out));
						camera_pos->set_position(half_frame+frame_anchor_offset);
						camera_pos_in->set_position(half_frame+frame_anchor_offset);
						camera_pos_out->set_position(half_frame+frame_anchor_offset);
						parallax_pos->set_position(half_frame+frame_anchor_offset);
						parallax_pos_in->set_position(half_frame+frame_anchor_offset);
						parallax_pos_out->set_position(half_frame+frame_anchor_offset);
					}
				}
				if (update_frame_params){
					new_materials.init(p_options, frame);
					frame->set_material(new_materials.outline_material);
					frame->set_line_texture(ResourceLoader::load("res://redot/textures/frame_outline.png", "Texture"));
					parent->move_child(node_2d, parent->get_child_count()-1);
				}
				else{
					new_materials = materials;
				}
				Node *node = mode == FOLDER_FRAME_EXTERNAL ? node_external : node_2d;
				if (node != nullptr)
					offset += load_folder(context, new_target_dir, final_iter+1, new_materials, node, new_parent_pos, new_parent_offset, p_options, new_force_save, counter+1, new_folder_level, clipper);
				else
					ERR_PRINTS("Error can't create node " + folder_name);
			} break;
			default:
				break;
		}
		
	}
	if (saveble){
		int mode = (folder_level==0) ? p_options["types/root"] : (folder_level==1) ? p_options["types/folder"] : FOLDER_NODE2D;
		if(folder_level == -1 || (mode == FOLDER_FRAME_EXTERNAL)){
			String scene_path = target_dir + ".tscn";
			Ref<PackedScene> scene = memnew(PackedScene);
			Node *owner = parent->get_owner();
			if (!owner){
				owner = parent;
			}
			Error err = scene->pack(owner);
			if (err==OK){
				err = ResourceSaver::save(scene_path, scene);
				if (err==OK)
					EditorNode::get_singleton()->reload_scene(scene_path);
			}
		}
	}
	return count + offset;
}
Error ResourceImporterPSD::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	char *global;
	if (ProjectSettings::get_singleton()){
		global = _strdup(ProjectSettings::get_singleton()->globalize_path(p_source_file).utf8().get_data());
	}
	else{
		ERR_PRINTS("Error can't read path: " + p_source_file);
		return ERR_FILE_BAD_PATH;
	}
	String target_dir = p_source_file.get_basename();
	red::create_dir(target_dir);

	_psd_context * context = NULL;
	psd_status status = psd_image_load(&context, global);
	if (status!=psd_status::psd_status_done){
		ERR_PRINTS("Error can't read file: " + ProjectSettings::get_singleton()->globalize_path(p_source_file));
		return ERR_FILE_CANT_READ;
	}

	bool force_save = true;
	Node *parent = _get_root(context, target_dir, force_save, p_options);
	if (parent == nullptr){
		return OK;
	}
	Materials mats;

	mats.init(p_options, parent);

	load_folder(context, target_dir, 0, mats, parent, Vector2(0,0), Vector2(0,0), p_options, force_save, 0);
	psd_image_free(context);
	return OK;
}

ResourceImporterPSD::ResourceImporterPSD() {
}
