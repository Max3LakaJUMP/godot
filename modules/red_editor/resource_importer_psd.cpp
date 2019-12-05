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

#include "modules/red/red_clipper.h"
#include "modules/red/red_polygon.h"
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
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/layer_size"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "update/folder_pos", PROPERTY_HINT_ENUM, "Ignore, Reset, Move layers"), 2));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "update/layer_pos", PROPERTY_HINT_ENUM, "Ignore, Move, New UV, Keep UV"), 1));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/second_level", PROPERTY_HINT_ENUM, "Top left, Center"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/folder", PROPERTY_HINT_ENUM, "Top left, Center"), 0));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/root", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame"), 2));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/second_level", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 4));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/folder", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/layer", PROPERTY_HINT_ENUM, "Polygon2D, REDPolygon"), 1));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/max_size"), 4096));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/scale", PROPERTY_HINT_ENUM, "Original, Downscale, Upscale, Closest"), 1));
	
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/outline_shader", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/outline_shader.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/shader", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/shader.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/shader_mul", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/shader_mul.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/shader_add", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/shader_add.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/shader_sub", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/shader_sub.shader")));
	
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/masked_shader", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/masked_shader.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/masked_shader_mul", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/masked_shader_mul.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/masked_shader_add", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/masked_shader_add.shader")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "shaders/masked_shader_sub", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://redot/shaders/masked_shader_sub.shader")));
	
}

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
	Vector2 max_size(512, 512);
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
}

void ResourceImporterPSD::simplify_polygon(Vector<Vector2> &p_polygon, float min_dot=-0.75f){
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
	p_polygon = polygon_dist;
}

void ResourceImporterPSD::save_png(_psd_layer_record *layer, String png_path, int texture_max_size, int texture_scale){
	Ref<Image> img;
	{
		int len = layer->width * layer->height;
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

	if (texture_scale != ORIGINAL)
	{
		int new_width = img->get_width();
		int new_height = img->get_height();
		if (texture_scale == DOWNSCALE)
		{
			new_width = previous_power_of_2(layer->width);
			new_height = previous_power_of_2(layer->height);
		}else if (texture_scale == UPSCALE){
			new_width = next_power_of_2(layer->width);
			new_height = next_power_of_2(layer->height);
		}else if (texture_scale == CLOSEST){
			new_width = closest_power_of_2(layer->width);
			new_height = closest_power_of_2(layer->height);
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
	img->save_png(png_path);
	if (ResourceLoader::import){
		ResourceLoader::import(png_path);
	}
}

void ResourceImporterPSD::_mask_to_node(_psd_layer_record *layer, float target_width, Node2D *node2d, _psd_context *context, int anchor_second_level){
	Vector<Vector2> polygon = load_polygon_from_mask(layer, target_width, context->width);
	if (polygon.size()>3)
		simplify_polygon(polygon);
	if (polygon.size()<3){
		float width = target_width;
		float height = (target_width*(float)context->height)/context->width;
		polygon.clear();
		polygon.push_back(Vector2(0, 0));
		polygon.push_back(Vector2(width, 0));
		polygon.push_back(Vector2(width, height));
		polygon.push_back(Vector2(0, height));
	}
	PoolVector<Vector2> polygon_pool;
	Rect2 item_rect = Rect2();
	for (int i = 0; i < polygon.size(); i++){
		polygon_pool.append(polygon[i]);
		Vector2 pos = polygon[i];
		if (i == 0)
			item_rect.position = pos;
		else
			item_rect.expand_to(pos);
	}

	REDClipper *clipper = Object::cast_to<REDClipper>(node2d);	
	if (clipper){
		clipper->set_polygon(polygon_pool);
	}
	/*
	Point2 center_pos = item_rect.size/2.0f;
	Point2 output_offset = Point2(0, 0);
	Vector2 move_val = -node2d->get_position();
	switch (anchor_second_level){
		case ANCHOR_CENTER:{
			output_offset = center_pos;

			//node2d->set_position(center_pos);

		} break;
		case ANCHOR_TOP_LEFT:
			output_offset = Point2(0, 0);
		default:{
		}break;
	}
	if (clipper){
		clipper->set_offset(-center_pos);
	}
	//move_val += node2d->get_position();
	//if (move_val.length() > 0){
	//	for (int i = 0; i < node2d->get_child_count(); i++)
	//	{
	//		Node2D *child = Object::cast_to<Node2D>(node2d->get_child(i));
	//		if (child)
	//			child->set_position(child->get_position()-move_val);
	//	}
	//}
	*/
}


Node *ResourceImporterPSD::get_edited_scene_root(String p_path) const{
	EditorData editor_data = EditorNode::get_singleton()->get_editor_data();
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
		if (b_update_only_editor){
			if (EditorNode::get_singleton()->is_scene_open(scene_path)){
				root = get_edited_scene_root(scene_path);
			}
		}
	}
	if (root == nullptr){
		red::scene_loader(scene_path);
		Ref<PackedScene> scene = ResourceLoader::load(scene_path, "PackedScene");
		root = scene->instance();
	}
	return root;
}

void MeshData::calc(REDPolygon *poly, bool vtx, bool uv, bool faces, bool obj){
	if (vtx && poly->get_polygon().size() > 0){
		PoolVector<Vector2>::Read vtxr = poly->get_polygon().read();
		vtx_min = vtxr[0];
		vtx_max = vtxr[0];
		int count = poly->get_polygon().size();
		for (int i = 0; i < count; i++)
		{
			Vector2 vtx = vtxr[i];
			vtx_min.x = MIN(vtx.x, vtx_min.x);
			vtx_max.x = MAX(vtx.x, vtx_max.x);
			vtx_min.y = MIN(vtx.y, vtx_min.y);
			vtx_max.y = MAX(vtx.y, vtx_max.y);
		}
		vtx_size = vtx_max - vtx_min;
	}
	if (uv && poly->get_uv().size() > 0){
		PoolVector<Vector2>::Read uvr = poly->get_uv().read();
		uv_min = uvr[0];
		uv_max = uvr[0];
		int count = poly->get_uv().size();
		for (int i = 0; i < count; i++)
		{
			Vector2 uv = uvr[i];
			uv_min.x = MIN(uv.x, uv_min.x);
			uv_min.y = MIN(uv.y, uv_min.y);
			
			uv_max.x = MAX(uv.x, uv_max.x);
			uv_max.y = MAX(uv.y, uv_max.y);
		}
		uv_size = uv_max - uv_min;
       	uv_offset = Vector2(uv_min.x, uv_min.y);
	}
	if (vtx && uv){
		vtx_offset = Vector2(uv_min.x * vtx_size.x/uv_size.x, (1.0-uv_max.y) * uv_size.y/uv_size.y);
	}
	if (obj){
		obj_pos = poly->get_global_position();
	}
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
		print_line(nodes[i]->get_class_name());
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


int ResourceImporterPSD::load_folder(_psd_context *context, String target_dir, int start, Materials &materials, 
									 Node *parent, Vector2 parent_pos, Vector2 parent_offset, const Map<StringName, Variant> &p_options, bool force_save, int counter, int folder_level, REDClipper *parent_clipper){
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
	
	_File file_сheck;
	for (int iter = start; iter + offset < end && loop; iter++){	
		final_iter = iter + offset; count++;
		_psd_layer_record *layer = &layers[final_iter];
		String name = reinterpret_cast<char const*>(layers[final_iter].layer_name);

		switch (layers[final_iter].layer_type){
			case psd_layer_type::psd_layer_type_folder:{
				if (counter > 0){
					loop = false;
					REDClipper* clipper = (REDClipper*)parent_clipper;
					
					int mode = 0;
					if(folder_level==0)
						mode = p_options["types/root"];
					else if (folder_level==1)
						mode = p_options["types/second_level"];
					else if (folder_level>1)
						mode = p_options["types/folder"];
					if (clipper != nullptr && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL)){
						Vector<NodePath> mat_objs = red::vector<NodePath>(clipper->get_material_objects());

						Vector<Ref<ShaderMaterial> > mats = clipper->get_cached_materials();
						int mat_objs_old_size = mat_objs.size();
						Vector<Node*> all_children;
						red::get_all_children(clipper, all_children);
						for (int i = 0; i < all_children.size(); i++)
						{
							if (!all_children[i]->is_class("CanvasItem"))
								continue;
							CanvasItem *c = (CanvasItem*)all_children[i];
							if (c!=nullptr){
								Ref<ShaderMaterial> mat = Ref<ShaderMaterial>(c->get_material());
								if (mat.is_valid()){
									bool found = false;
									int mats_size = mats.size();
									for (int j = 0; j < mats_size; j++){
										if (mat == mats[j]){
											found = true;
											break;
										}
									}
									if (!found){
										mats.push_back(mat);
										mat_objs.push_back(clipper->get_path_to(c));
									}
								}
							}
						}
						clipper->set_material_objects(red::array(mat_objs));
					}

					if((mode == FOLDER_PAGE)){
						if (parent->is_class("REDPage")){
							REDPage *page = (REDPage *)parent;
							Array frames;
							int l = parent->get_child_count();
							frames.resize(l);

							for (int i = 0; i < l; i++){
								Node *child = parent->get_child(i);
								frames[l-1-i] = NodePath(child->get_name());
							}
							page->set_frames(frames);
						}
					}
				}
			} break;
			case psd_layer_type::psd_layer_type_normal:{
				Node2D *node;
				int mode = p_options["types/layer"];
				bool need_create = parent->has_node(name) ? false : true;
				
				int len = layer->width*layer->height;
				if (len==0) 

					continue;
				red::create_dir( target_dir + "/textures");
				String png_path = target_dir + "/textures/"+ name +".png";
				
				Vector2 texture_resize(1, 1);
				Vector2 old_texture_size(0, 0);
				if (file_сheck.file_exists(png_path)){
					Ref<Texture> texture = ResourceLoader::load(png_path, "Texture");
					if (texture.is_valid())
						old_texture_size = texture->get_size();
				}

				if (!file_сheck.file_exists(png_path))
					save_png(layer, png_path, p_options["texture/max_size"], p_options["texture/scale"]);
				else if (p_options["update/texture"] && updateble){
					save_png(layer, png_path, p_options["texture/max_size"], p_options["texture/scale"]);
				}
				
				if (old_texture_size.x != 0){
					Ref<Texture> texture = ResourceLoader::load(png_path, "Texture");
					if (texture.is_valid())
						old_texture_size = texture->get_size()/old_texture_size;
				}

				
				Vector2 polygon_size(layer->width * resolution_width / context->width, layer->height * resolution_width / context->width);
				Point2 global_pos = Point2(layer->left * resolution_width / context->width, layer->top * resolution_width / context->width);

				float k = layer->height/layer->width;
				if (need_create){
					Ref<Texture> texture = ResourceLoader::load(png_path, "Texture");
					PoolVector<Vector2> uv;
					PoolVector<Vector2> polygon;
					polygon.append(Vector2(0,0));
					polygon.append(Vector2(polygon_size.x, 0));
					polygon.append(polygon_size);
					polygon.append(Vector2(0, polygon_size.y));
					if (mode==LAYER_POLYGON2D){
						Polygon2D *poly = memnew(Polygon2D);
						node = (Node2D*)poly;
						Size2 texture_size;
						if (texture.is_valid()){
							poly->set_texture(texture);
							texture_size = texture->get_size();
						}
						else
							texture_size = Size2(1024, 1024);
						uv.append(Vector2(0,0));
						uv.append(Vector2(texture_size.x, 0));
						uv.append(texture_size);
						uv.append(Vector2(0, texture_size.y));
						poly->set_uv(uv);
					}
					else{
						REDPolygon *poly = memnew(REDPolygon);
						node = (Node2D*)poly;
						poly->set_polygon(polygon);
						if (texture.is_valid())
							poly->set_texture(texture);
						uv.append(Vector2(0, 0));
						uv.append(Vector2(1, 0));
						uv.append(Vector2(1, 1));
						uv.append(Vector2(0, 1));
						poly->set_uv(uv);
					}
					switch(layer->blend_mode){
						case psd_blend_mode_multiply:{
							if (parent_clipper == nullptr){
								if (materials.material_mul.is_valid())
									node->set_material(materials.material_mul);
							}else{
								if (materials.masked_material_mul.is_valid())
									node->set_material(materials.masked_material_mul);
							}
							break;
						}
						case psd_blend_mode_linear_dodge:{
							if (parent_clipper == nullptr){
								if (materials.material_add.is_valid())
									node->set_material(materials.material_add);
							}else{
								if (materials.masked_material_add.is_valid())
									node->set_material(materials.masked_material_add);
							}
							break;
						}

						case psd_blend_mode_difference:{
							if (parent_clipper == nullptr){
								if (materials.material_sub.is_valid())
									node->set_material(materials.material_sub);
							}else{
								if (materials.masked_material_sub.is_valid())
									node->set_material(materials.masked_material_sub);
							}
							break;	
						}

						default:{
							if (parent_clipper==nullptr){
								if (materials.material.is_valid())
									node->set_material(materials.material);
							}else{
								if (materials.masked_material.is_valid())
									node->set_material(materials.masked_material);
							}
							break;
						}
					}
					parent->add_child(node);
					Node *owner = parent->get_owner();
					if (!owner){
						owner = parent;
					}
					node->set_owner(owner);
					node->set_name(name);
					node->set_draw_behind_parent(true);
				}
				else {
					node = (Node2D*)(parent->get_node(NodePath(name)));
				}
				if (!need_create && updateble && p_options["update/layer_size"]){
					node = (Node2D*)(parent->get_node(NodePath(name)));
					{
						Polygon2D *poly = (Polygon2D*)node;
						if (poly){
							if (poly != nullptr){
								PoolVector<Vector2> uvs = poly->get_uv();
								if (uvs.size() > 0){
									Vector2 real_size = poly->_edit_get_rect().get_size();
									PoolVector<Vector2>::Read uvr = uvs.read();
									Vector2 uv_min = uvr[0];
									Vector2 uv_max = uv_min;
									for (int i = 0; i < poly->get_uv().size(); i++)
									{
										Vector2 uv = uvr[i];
										uv_min.x = MIN(uv.x, uv_min.x);
										uv_max.x = MAX(uv.x, uv_max.x);
										uv_min.y = MIN(uv.y, uv_min.y);
										uv_max.y = MAX(uv.y, uv_max.y);
									}
									Vector2 resizer = (uv_max - uv_min)/poly->get_texture()->get_size()*polygon_size/real_size;
									PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
									PoolVector<Vector2> new_pool;
									for (int i = 0; i < poly->get_polygon().size(); i++)
										new_pool.append(polyr[i]*resizer);
									poly->set_polygon(new_pool);
								}
							}
						}
					}
					{
						REDPolygon *poly = (REDPolygon*)node;
						if (poly){
							PoolVector<Vector2> uvs = poly->get_uv();
							if (uvs.size() > 0){
								Vector2 real_size = poly->_edit_get_rect().get_size();
								PoolVector<Vector2>::Read uvr = uvs.read();
								Vector2 uv_min = uvr[0];
								Vector2 uv_max = uv_min;
								for (int i = 0; i < poly->get_uv().size(); i++)
								{
									Vector2 uv = uvr[i];
									uv_min.x = MIN(uv.x, uv_min.x);
									uv_max.x = MAX(uv.x, uv_max.x);
									uv_min.y = MIN(uv.y, uv_min.y);
									uv_max.y = MAX(uv.y, uv_max.y);
								}
								Vector2 resizer = (uv_max - uv_min)*polygon_size/real_size;
								PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
								PoolVector<Vector2> new_pool;
								for (int i = 0; i < poly->get_polygon().size(); i++)
									new_pool.append(polyr[i] * resizer);
								poly->set_polygon(new_pool);
							}
						}
					}
				}
				int update_pos_mode = need_create ? LAYER_POS_MOVE : updateble ? p_options["update/layer_pos"] : LAYER_POS_IGNORE;

				if (node != nullptr && update_pos_mode != LAYER_POS_IGNORE){
					if (update_pos_mode==LAYER_POS_MOVE){
						{
							Polygon2D *poly = (Polygon2D*)node;
							if (poly){
								//node->set_global_position(global_pos - poly->get_offset());
								node->set_position(global_pos - parent_pos - poly->get_offset() - parent_offset);
							}
						}
						{
							REDPolygon *poly = (REDPolygon*)node;
							if (poly){
								//node->set_global_position(global_pos - poly->get_offset() + poly->get_psd_offset());
								node->set_position(global_pos - parent_pos - poly->get_offset() + poly->get_psd_offset() - parent_offset);
							}
						}
					}
					else if (update_pos_mode == LAYER_POS_NEW_UV || update_pos_mode == LAYER_POS_KEEP_UV){
						{
							Polygon2D *poly = (Polygon2D*)node;
							if (poly && update_pos_mode == LAYER_POS_NEW_UV){
								Vector2 real_size = poly->_edit_get_rect().get_size();
								Vector2 psd_offset = global_pos - poly->get_position() - parent_pos - poly->get_offset() - parent_offset;
								{
									Vector<Vector2> uv_temp;
									if (poly->get_uv().size() != poly->get_polygon().size() || update_pos_mode==LAYER_POS_NEW_UV){
										PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
										for (int i = 0; i < poly->get_polygon().size(); i++)
											uv_temp.push_back(polyr[i]/real_size);
									}
									else{

									}
									PoolVector<Vector2> new_uv;
									for (int i = 0; i < uv_temp.size(); i++)
										new_uv.append(uv_temp[i] *  real_size/polygon_size - psd_offset/polygon_size);
									poly->set_uv(new_uv);
								}
								poly->set_position(global_pos - parent_pos - poly->get_offset() - psd_offset);
							}
						}
						{
							REDPolygon *poly = (REDPolygon*)node;
							if (poly){
								Vector2 real_size = poly->_edit_get_rect().get_size();
								Vector2 old_psd_offset = poly->get_psd_uv_offset();

								poly->set_position(poly->get_position() - poly->get_psd_applied_offset());
								Vector2 psd_offset = global_pos - poly->get_position() - parent_pos - poly->get_offset();
								poly->set_psd_uv_offset(psd_offset);
								poly->_change_notify("psd_uv_offset");

								Vector2 old_psd_scale = poly->get_psd_uv_scale();
								poly->set_psd_uv_scale(real_size/polygon_size);
								poly->_change_notify("psd_uv_scale");
								Vector2 psd_offset_uv = psd_offset/polygon_size;
								Vector2 psd_scale_uv = real_size/polygon_size;
								{
									Vector<Vector2> uv_temp;
									if (poly->get_uv().size() != poly->get_polygon().size() || update_pos_mode==LAYER_POS_KEEP_UV){
										PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
										for (int i = 0; i < poly->get_polygon().size(); i++)
											uv_temp.push_back(polyr[i]/real_size);
									}
									else{
										PoolVector<Vector2>::Read uvr = poly->get_uv().read();
										for (int i = 0; i < poly->get_uv().size(); i++)
											uv_temp.push_back(uvr[i]/old_psd_scale+old_psd_offset/polygon_size);
									}
									PoolVector<Vector2> new_uv;
									for (int i = 0; i < uv_temp.size(); i++)
										new_uv.append(uv_temp[i] * psd_scale_uv - psd_offset_uv);
									poly->set_uv(new_uv);
								}
								poly->set_position(global_pos - parent_pos - poly->get_offset() - psd_offset + poly->get_psd_offset());
								poly->set_psd_applied_offset(poly->get_psd_offset());
								poly->_change_notify("psd_applied_offset");
							}
						}
					}
				}
			} break;
			case psd_layer_type::psd_layer_type_hidden:{
				Node *node_external = nullptr;
				Node2D *node_2d = nullptr;
				REDPage *page = nullptr;
				REDFrame *frame = nullptr;
				REDClipper *clipper = parent_clipper;

				Node2D *anchor_object = nullptr;


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
				if(new_folder_level==0)
					mode = p_options["types/root"];
				else if (new_folder_level==1){
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

				switch (mode){
					case FOLDER_NODE2D:{
						node_2d = need_create ? red::create_node<Node2D>(parent, folder_name) : (Node2D*)parent->get_node(folder_name);
						anchor_object = node_2d;
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
						clipper = (REDClipper*)frame;
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

						if (b_update_only_editor){
							root_node = get_edited_scene_root(scene_path);
						}
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
						clipper = (REDClipper*)frame;
						node_2d = (Node2D*)parent->get_node(folder_name);
						anchor_object = (Node2D*)frame;
					} break;
					default:
					case FOLDER_PARALLAX:{
						node_2d = need_create ? (Node2D*)red::create_node<REDParallaxFolder>(parent, folder_name) : (Node2D*)parent->get_node(folder_name);
						anchor_object = node_2d;
					} break;
				}
				Vector2 new_parent_pos = parent_pos;
				Vector2 new_parent_offset = Vector2(0, 0);

				
				if (anchor_object){
					if (((updateble && p_options["update/folder_mask"]) || need_create) && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL)){
						_mask_to_node(layer_folder, resolution_width, anchor_object, context, anchor);
					}
				}
				float offset_x = layer_folder->layer_mask_info.left * resolution_width / context->width;
				float offset_y = layer_folder->layer_mask_info.top * resolution_width / context->width;
				Vector2 local_pos = Vector2(offset_x, offset_y);
				Vector2 half_frame = (clipper) ? clipper->_edit_get_rect().size/2 : Vector2(0,0);
				Vector2 frame_anchor_offset = (anchor==ANCHOR_CENTER) ? -half_frame : Vector2(0,0);
				new_parent_pos += local_pos;
				new_parent_offset = -frame_anchor_offset;

				int folder_update_mode = p_options["update/folder_pos"];


				if ((updateble && folder_update_mode == FOLDER_POS_RESET) || need_create){
					if (clipper && mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL){
						clipper->set_offset(frame_anchor_offset);
						clipper->set_position(-frame_anchor_offset);
					}
					if (node_2d){
						node_2d->set_position(local_pos - parent_pos - parent_offset - frame_anchor_offset);
					}
				}
				else if (folder_update_mode == FOLDER_MOVE_LAYERS && node_2d){
					new_parent_pos -= (local_pos - parent_pos - parent_offset - frame_anchor_offset)-node_2d->get_position();
				}

				if (((updateble &&p_options["update/page_height"]) || need_create) && mode == FOLDER_PAGE){
					page->set_size(Size2(resolution_width, ((float)context->height) / context->width*resolution_width));
				}

				if (((updateble &&p_options["update/frame_targets"]) || need_create) && frame != nullptr && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL)){
					Node2D *targets2d = red::create_node<Node2D>(anchor_object, "targets");
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
						camera_pos->set_position(half_frame-frame_anchor_offset);
						camera_pos_in->set_position(half_frame-frame_anchor_offset);
						camera_pos_out->set_position(half_frame-frame_anchor_offset);
						parallax_pos->set_position(half_frame-frame_anchor_offset);
						parallax_pos_in->set_position(half_frame-frame_anchor_offset);
						parallax_pos_out->set_position(half_frame-frame_anchor_offset);
					}
				}

				if ((updateble || need_create) && frame != nullptr && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL)){
					new_materials.init(p_options, frame);
					frame->set_material(new_materials.outline_material);
					frame->set_line_texture(ResourceLoader::load("res://redot/textures/frame_outline.png", "Texture"));
					parent->move_child(node_2d, parent->get_child_count()-1);
				}
				else{
					new_materials = materials;
				}
				Node *node = nullptr;
				if (mode == FOLDER_FRAME_EXTERNAL){
					node = node_external;
				}
				else {
					node = (Node*) node_2d;
				}
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
