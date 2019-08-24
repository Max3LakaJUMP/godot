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
#include "modules/red/redpage.h" 
#include "scene/resources/bit_map.h"
#include "core/math/math_funcs.h"
#include <string>
#include "core/message_queue.h"
#include "editor/editor_node.h" 
#include "editor/import/resource_importer_texture_atlas.h"

#include "modules/red/red_polygon.h"
#include "modules/red/red_engine.h"
#include "modules/red/red_engine.h"

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
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "Layer shader", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), ResourceLoader::load("res://addons/red.tomaya/form_content.shader")));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/root", PROPERTY_HINT_ENUM, "Node2D, Page"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/folder", PROPERTY_HINT_ENUM, "Node2D, Page, Frame, Frame external"), 3));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "types/layer", PROPERTY_HINT_ENUM, "Node2D, Polygon2D, REDPolygon"), 2));
	
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/texture"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/folder_mask"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/folder_pos"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/layer_pos"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "resolution/width"), 2000.0f));
}

Vector<Vector2> ResourceImporterPSD::load_polygon_from_mask(_psd_layer_record *layer, float target_width, int psd_width){
	PoolVector<uint8_t> data;

	int len = layer->layer_mask_info.width*layer->layer_mask_info.height;
	if ((layer->layer_mask_info.width)<2 || (layer->layer_mask_info.height)<2){
		Vector<Vector2> output;
		return output;
	}

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
	if (polygon_array.size()>0){
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

void ResourceImporterPSD::save_png(_psd_layer_record *layer, String png_path){
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
	Ref<Image> img;
	img.instance();
	img->create(layer->width, layer->height, false, Image::FORMAT_RGBA8, data);

	int new_width = previous_power_of_2(layer->width);
	int new_height = previous_power_of_2(layer->height);

	if (new_width != img->get_width() || new_height != img->get_height())
		img->resize(new_width, new_height,Image::INTERPOLATE_CUBIC);

	img->save_png(png_path);

	if (ResourceLoader::import){
		ResourceLoader::import(png_path);
	}
}

void ResourceImporterPSD::_mask_to_node(_psd_layer_record *layer, float target_width, int psd_width, REDFrame *frame){
	Vector<Vector2> polygon = load_polygon_from_mask(layer, target_width, psd_width);
	if (polygon.size()>3)
		simplify_polygon(polygon);
	if (polygon.size()>2){
		REDFrame *node = Object::cast_to<REDFrame>(frame);
		if (node!=nullptr){
			PoolVector<Vector2> polygon_pool;
			for (int i = 0; i < polygon.size(); i++)
				polygon_pool.append(polygon[i]);
			node->set_polygon(polygon_pool);
		}
	}
}

Node *ResourceImporterPSD::_get_root(_psd_context *context, const String &target_dir) const{
	red::create_dir(target_dir);
	_psd_layer_record *last_layer = &(context->layer_records[context->layer_count-1]);
	String scene_path;

	scene_path = target_dir + "/" + target_dir.get_file() + ".tscn";
	red::scene_loader(scene_path);
	Ref<PackedScene> scene = ResourceLoader::load(scene_path, "PackedScene");
	Node *root = scene->instance();
	return root;
}

int ResourceImporterPSD::load_folder(_psd_context *context, String target_dir, int start, Ref<ShaderMaterial> material, Node *parent, Vector2 parent_pos,
									const Map<StringName, Variant> &p_options, int counter){
	psd_layer_record *layers = context->layer_records;
	int count = 0;
	int offset = 0;
	int end = context->layer_count;
	bool loop = true;
	for (int iter = start; iter + offset < end && loop; iter++){	
		int i = iter + offset; count++;
		_psd_layer_record *layer = &layers[i];
		String name = reinterpret_cast<char const*>(layers[i].layer_name);
		_File file2check;
		switch (layers[i].layer_type){
			case psd_layer_type::psd_layer_type_folder:{
				if (counter>0){
					loop = false;
					break;
				}
			} break;
			case psd_layer_type::psd_layer_type_normal:{
				Node2D *node;
				int mode = p_options["types/layer"];
				float size = p_options["resolution/width"];
				bool need_create = parent->has_node(name) ? false : true;
				switch (mode){
					case LAYER_NODE2D:{
						node = need_create ? red::create_node<Node2D>(parent, name) : (Node2D*)parent->get_node(NodePath(name));
					} break;
					case LAYER_POLYGON2D:
					case LAYER_POLYGON:{
						int len = layer->width*layer->height;
						if (len==0) 
							continue;
						String png_path = target_dir + "/" + name +".png";
						if (p_options["update/texture"] || !file2check.file_exists(png_path)){
							save_png(layer, png_path);
						}
						Vector2 polygon_size(layer->width*size/context->width, layer->height*size/context->width);
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
							if (parent->get_child_count()>0){
								node->set_material(((CanvasItem*)(parent->get_child(0)))->get_material());
							}
							else if (material.is_valid()){
								node->set_material(material);
							}
							parent->add_child(node);
							Node *owner = parent->get_owner();
							if (!owner){
								owner=parent;
							}
							node->set_owner(owner);
							node->set_name(name);
							node->set_draw_behind_parent(true);
						}
						else 
							node = (Node2D*)parent->get_node(NodePath(name));
					} break;
					default:
						break;
				}
				if (p_options["update/layer_pos"]){
					float offset_ratio = (float)(context->height)/context->width;
					Vector2 local_pos = Vector2(layer->left * size / context->width, layer->top * (size * offset_ratio) / context->height);
					node->set_position(local_pos-parent_pos);
				}
			} break;
			case psd_layer_type::psd_layer_type_hidden:{
				Node *node;
				Node *node_external;
				Node2D *node_2d;
				REDFrame *frame;
				_psd_layer_record *layer_folder;
				{
					int child_folder_count = 0;
					int child_end_count = 0;
					for (int j = i + 1; j < end; j++){
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
				if (folder_name==""){
					folder_name = "root";
				}
				String new_target_dir = target_dir + "/" + folder_name;
				red::create_dir(new_target_dir);
				Ref<Shader> content_shader = p_options["Layer shader"];
				Ref<ShaderMaterial> new_material;
				bool need_create = parent->has_node(folder_name) ? false : true;
				bool external_frame_created = false;
				int mode = (counter==0) ? p_options["types/root"] : p_options["types/folder"];
				switch (mode){
					case LAYER_NODE2D:{
						node_2d = need_create ? red::create_node<Node2D>(parent, folder_name) : (Node2D*)parent->get_node(folder_name);
					} break;
					case FOLDER_PAGE:{
						node_2d = need_create ? (Node2D*)red::create_node<REDPage>(parent, folder_name) : (Node2D*)parent->get_node(folder_name);
					} break;
					case FOLDER_FRAME:{
						frame = need_create ? red::create_node<REDFrame>(parent, folder_name) : (REDFrame*)parent->get_node(NodePath(folder_name));
						node_2d = (Node2D*)frame;
					} break;
					case FOLDER_FRAME_EXTERNAL:{
						String scene_path = (new_target_dir + "/" + new_target_dir.get_file() + ".tscn");
						red::scene_loader(scene_path);
						if (need_create){
							RES res = ResourceLoader::load(scene_path);
							Ref<PackedScene> scene = Ref<PackedScene>(Object::cast_to<PackedScene>(*res));
							Node *instanced_scene = scene->instance(PackedScene::GEN_EDIT_STATE_INSTANCE);
							instanced_scene->set_name(folder_name);
							parent->add_child(instanced_scene);
							instanced_scene->set_owner(parent->get_owner());
						}
						Ref<PackedScene> scene = ResourceLoader::load(scene_path, "PackedScene");
						Node *root_node = scene->instance();
						if (!root_node->has_node(folder_name)){
							frame = red::create_node<REDFrame>(root_node, folder_name);
							external_frame_created = true;
						}
						else
							frame = (REDFrame*)root_node->get_node(folder_name);
						node_external = (Node*)frame;
						node_2d = (Node2D*)parent->get_node(folder_name);
					} break;
					default:
						break;
				}
				Vector2 new_parent_pos = parent_pos;
				if (node_2d != nullptr){
					if (p_options["update/folder_pos"] || need_create){
						float size = p_options["resolution/width"];
						float offset_x = layer_folder->layer_mask_info.left*size/context->width;
						float offset_y = layer_folder->layer_mask_info.top*size/context->width;
						Vector2 local_pos = Vector2(offset_x, offset_y);
						node_2d->set_position(local_pos - parent_pos);
						new_parent_pos+=local_pos;
					}
				}
				if (frame != nullptr && (p_options["update/folder_mask"] || external_frame_created)){
					float size = p_options["resolution/width"];
					_mask_to_node(layer_folder, size, context->width, frame);
					new_material.instance();
					new_material->set_shader(content_shader);
				}
				if (mode == FOLDER_FRAME_EXTERNAL){
					if (node_external != nullptr)
						node = node_external;
				}
				else {
					if (node_2d!=nullptr)
						node = (Node*) node_2d;
				}
				if (node != nullptr)
					offset += load_folder(context, new_target_dir, i+1, new_material, node, new_parent_pos, p_options, counter+1);
				else
					ERR_PRINTS("Error can't create node " + folder_name);
			} break;
			default:
				break;
		}
	}

	int mode = (counter==0) ? p_options["types/root"] : p_options["types/folder"];
	if(counter==0 || (counter>1 && mode == FOLDER_FRAME_EXTERNAL)){
		String scene_path = target_dir + "/" + target_dir.get_file() + ".tscn";
		Ref<PackedScene> scene = memnew(PackedScene);
		Node *owner = parent->get_owner();
		if (!owner){
			owner = parent;
		}
		Error err = scene->pack(owner);
		if (err==OK){
			print_line(std::to_string(counter).c_str());
			print_line(scene_path);
			err = ResourceSaver::save(scene_path, scene);
			if (err==OK)
				EditorNode::get_singleton()->reload_scene(scene_path);
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

	Node *parent = _get_root(context, target_dir);
	load_folder(context, target_dir, 0, nullptr, parent, Vector2(0,0), p_options, 0);
	psd_image_free(context);



	//add_control_to_container(CustomControlContainer p_location, Control *p_control)

	//HBoxContainer *menu_hb = EditorNode::get_menu_hb();
	//MenuButton *file = (MenuButton *)(menu_hb->get_child(1)->get_child(0));
	//print_line(file->get_text());
	//PopupMenu *file_popup = file->get_popup();
	//file_popup->add_item("ddddddddd");
	return OK;
}

ResourceImporterPSD::ResourceImporterPSD() {
}
