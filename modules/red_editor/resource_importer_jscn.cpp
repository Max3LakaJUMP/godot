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

#include "resource_importer_jscn.h"

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
#include "modules/red/red_parallax_folder.h" 
#include "modules/red/red_target.h" 
#include "modules/red/red_transform.h" 
#include "modules/red/red_shape_renderer.h" 
#include "modules/red/red_clipper.h" 
#include "scene/resources/bit_map.h"
#include "core/math/math_funcs.h"
#include <string>
#include "core/message_queue.h"
#include "editor/editor_node.h" 
#include "editor/import/resource_importer_texture_atlas.h"

//#include "modules/red/red_polygon.h"
#include "core/io/json.h"
#include "scene/2d/skeleton_2d.h"
#include "modules/red/red_engine.h"
#include "modules/red/red_json.h"

String ResourceImporterJSCN::get_importer_name() const {

	return "jscn_scene";
}

String ResourceImporterJSCN::get_visible_name() const {

	return "JSON Scene file";
}
void ResourceImporterJSCN::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("jscn");
}

String ResourceImporterJSCN::get_save_extension() const {
	return "";
}


String ResourceImporterJSCN::get_resource_type() const {

	return "JSCN";
}

bool ResourceImporterJSCN::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {

	return true;
}

int ResourceImporterJSCN::get_preset_count() const {
	return 0;
}
String ResourceImporterJSCN::get_preset_name(int p_idx) const {

	return String();
}

void ResourceImporterJSCN::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/update"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/update_only_editor"), true));
	// r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/polygon"), true));
	// r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/position"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/texture"), false));
}

Node *ResourceImporterJSCN::get_edited_scene_root(const String &p_path) const{
	EditorData &editor_data = EditorNode::get_editor_data();
	for (int i = 0; i < editor_data.get_edited_scene_count(); i++) {
		if (editor_data.get_scene_path(i) == p_path)
			return editor_data.get_edited_scene_root(i);
	}
	return nullptr;
}

void ResourceImporterJSCN::dict_to_node(Dictionary &serialized, Node **root_ptr, String &scene_path, const Map<StringName, Variant> &p_options){
	bool b_update = p_options["main/update"];
	bool b_update_only_editor = p_options["main/update_only_editor"];
	// bool update_polygon = p_options["update/polygon"];
	// bool update_position = p_options["update/position"];
	bool update_texture = p_options["update/texture"];
	bool force_save = !b_update_only_editor;

	Node *node;
	String name = serialized["name"];
	String parent_path = serialized["parent"];
	String type = serialized["type"];
	bool need_create = true;

	Node *parent;
	if((*root_ptr) == nullptr){
		parent = nullptr;
		print_line("no root");
	}else{
		if(parent_path == "")
			return;
		parent = (*root_ptr)->get_parent()->get_node(NodePath(parent_path));
		if(!parent){
			print_line("parent error");
			return;
		}
	}
	if (parent == nullptr){
		_File file_сheck;
		if (file_сheck.file_exists(scene_path)){
			if (!b_update){
				return;
			}
		}
		else{
			red::scene_loader(scene_path);
			force_save = true;
		}
		if (!EditorNode::get_singleton()->is_scene_open(scene_path)){
			EditorNode::get_singleton()->load_scene(scene_path);
		}
		node = get_edited_scene_root(scene_path);
		if (node == nullptr){
			ERR_PRINTS("Can't get root node in edited scene");
			return;
		}	
		need_create = false;
		*root_ptr = node;
	}
	// if (parent == nullptr){
	// 	Ref<PackedScene> scene;
	// 	_File file2Check;
	// 	if (file2Check.file_exists(scene_path)){
	// 		scene = ResourceLoader::load(scene_path, "PackedScene");
	// 		node = scene->instance();
	// 		//node = EditorNode::get_singleton()->get_edited_scene()->get_child(0);
	// 		need_create = false;
	// 		if (node == nullptr)
	// 			return;
	// 	}
	// }
	else if (parent->has_node(name)){
		node = parent->get_node(name);
		need_create = false;
	}
	if (need_create){
		if (type=="Node")
			node = red::create_node<Node>(parent, name);
		else if (type=="Node2D")
			node = red::create_node<Node2D>(parent, name);
		else if (type=="Polygon2D")
			node = red::create_node<Polygon2D>(parent, name);
		else if (type=="REDFrame")
			node = red::create_node<REDFrame>(parent, name);
		else if (type=="REDPage")
			node = red::create_node<REDPage>(parent, name);
		else if (type=="REDParallaxFolder")
			node = red::create_node<REDParallaxFolder>(parent, name);
		else if (type=="REDTarget")
			node = red::create_node<REDTarget>(parent, name);
		else if (type=="REDTransform")
			node = red::create_node<REDTransform>(parent, name);
		else if (type=="AnimationPlayer")
			node = red::create_node<AnimationPlayer>(parent, name);
		else if (type=="Skeleton2D")
			node = red::create_node<Skeleton2D>(parent, name);
		else if (type=="Bone2D")
			node = red::create_node<Bone2D>(parent, name);
		else if (type=="REDShapeRenderer")
			node = red::create_node<REDShapeRenderer>(parent, name);
		else if (type=="REDClipper")
			node = red::create_node<REDClipper>(parent, name);
		else{
			return;
		}
	}
	List<PropertyInfo> plist;
	ClassDB::get_property_list(node->get_class(), &plist, false, node);
	//print_line(node->get_name());
	for (List<PropertyInfo>::Element *E = plist.front(); E; E = E->next()) {
		String name = E->get().name;
		if (name == "name" || name == "owner" || name == "_import_path" || 
			name == "custom_multiplayer" || name == "multiplayer" || 
			name == "global_scale" || name == "global_position" || 
			name == "global_rotation_degrees" || name == "global_rotation" || 
			name == "transform" || name == "global_transform" ||
			name == "current_animation" || name == "assigned_animation") 
			continue;
		if (serialized.has(name)){
			if (name == "texture" && (need_create || update_texture)){
				String global_path = serialized[name];
				node->set(name, ResourceLoader::load(global_path, "Texture"));
			}
			else if (name == "material" && need_create){
				String global_path = serialized[name];
				Ref<ShaderMaterial> material;
				material.instance();
				Ref<Shader> shader = ResourceLoader::load(global_path, "Shader");
				material->set_shader(shader);
				node->set(name, material);
			}else if (name == "bones"){
				if (type=="Polygon2D"){
					Polygon2D *polygon = (Polygon2D*)node;
					Array bones = serialized[name];
					for (int i = 0; i < bones.size(); i++){
						bool found=false;
						Dictionary bone = bones[i];
						String path = bone["path"];
						PoolVector<float> weights = bone["weights"];
						NodePath bone_path(path.replace_first("/", ""));
						for (int j = 0; j < polygon->get_bone_count(); j++){
							if (polygon->get_bone_path(j) == bone_path){
								polygon->set_bone_weights(j, weights);
								found = true;
								break;
							}
						}
						if (!found){
							polygon->add_bone(bone_path, weights);
						}
					}
				}
			} else if (serialized[name].get_type() == node->get(name).get_type() || 
			(serialized[name].get_type() == Variant::INT && node->get(name).get_type() == Variant::REAL) || 
			(serialized[name].get_type() == Variant::REAL && node->get(name).get_type() == Variant::INT)){
				node->set(name, serialized[name]);
				//print_line("Appled " + name + " property");
				//print_line(std::to_string((int)serialized[name].get_type()).c_str());
				//print_line(std::to_string((int)node->get(name).get_type()).c_str());
			}else if (serialized[name].get_type() == Variant::STRING && node->get(name).get_type() == Variant::NODE_PATH){
				node->set(name, NodePath(serialized[name]));
			}else {
				print_line("Ignored " + name + " property");
				//print_line(std::to_string((int)serialized[name].get_type()).c_str());
				//print_line(std::to_string((int)node->get(name).get_type()).c_str());
			}
				
		}
	}
	if (node->is_class("AnimationPlayer")){
		AnimationPlayer *player = (AnimationPlayer*)node;
		Array animations = serialized["animations"]; //animations
		int animations_count = animations.size();
		for (int i = 0; i < animations_count; i++){
			Dictionary animation_dict = animations[i];
			String animation_name = animation_dict["name"];
			Ref<Animation> animation = player->get_animation(animation_name);
			if (animation.is_null()){
				animation = Ref<Animation>(memnew(Animation));
				player->add_animation(animation_name, animation);
			}
			if (animation_dict.has("length")){
				animation->set_length(animation_dict["length"]);
			}
			List<PropertyInfo> animation_plist;
			ClassDB::get_property_list("Animation", &animation_plist, false, animation.ptr());
			for (List<PropertyInfo>::Element *E = animation_plist.front(); E; E = E->next()) {
				String name = E->get().name;
				if (name == "resource_local_to_scene" || name == "resource_path" || name == "resource_name" || 
					name == "current_animation_length" || name == "current_animation_position") continue;
				if (serialized.has(name)){
					if (serialized[name].get_type() == animation->get(name).get_type())
						animation->set(name, serialized[name]);
					else
						print_line("Ignored " + name + " animation property");
				}
			}
			Array tracks = animation_dict["tracks"];
			int track_count = tracks.size();
			for (int track_i = 0; track_i < track_count; track_i++){
				Dictionary track = tracks[track_i];
				int track_type = track["type"];
				String track_path = track["path"];
				int track_i_new = -1;
				if (animation->get_track_count()>0)
					track_i_new = animation->find_track(track_path);
				// Remove track (optional)
				if (track_i_new != -1){
					animation->remove_track(track_i_new);
					track_i_new = -1;
				}
				if (track_i_new == -1){
					track_i_new = animation->add_track((Animation::TrackType)track_type, -1);
					animation->track_set_path(track_i_new, track["path"]);
				} else if (animation->track_get_type(track_i_new) != track_type){
					track_i_new = animation->add_track((Animation::TrackType)track_type, -1);
					animation->track_set_path(track_i_new, track["path"]);
				}
				int track_interpolation_type = track["interpolation_type"];
				animation->track_set_interpolation_type(track_i_new, (Animation::InterpolationType)track_interpolation_type);
				Array keys = track["keys"];
				int keys_count = keys.size();
				for (int key_i = 0; key_i < keys_count; key_i++){
					Dictionary key = keys[key_i];
					int key_i_new = animation->track_find_key (track_i_new, key["time"], true);
					if (key_i_new == -1){
						animation->track_insert_key(track_i_new, key["time"], key["value"], key["transition"]);
						key_i_new = animation->track_find_key(track_i_new, key["time"], true);
					}
					else{
						animation->track_set_key_value(track_i_new, key_i_new, key["value"]);
					}
					animation->track_set_key_transition(track_i_new, key_i_new, key["transition"]);
				}
			}
		}
	}
	if (serialized.has("children")){
		Array children = serialized["children"];
		int count = children.size();
		for (int i = 0; i < count; i++){
			Dictionary child = children[i];
			dict_to_node(child, root_ptr, scene_path, p_options);
		}
	}
}

Error ResourceImporterJSCN::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	if (EditorNode::get_editor_data().get_edited_scene_count()==0)
		return OK;
	FileAccess *f = FileAccess::open(p_source_file, FileAccess::READ);
	ERR_FAIL_COND_V(!f, ERR_CANT_OPEN);
	String json_text = f->get_as_utf8_string();

	Variant parse_result;
	String err_string;
	int err_int;
	Error err = REDJSON::parse(json_text, parse_result, err_string, err_int);
	if(err != OK){
		memdelete(f);
		return ERR_CANT_OPEN;
	}
	Array data = parse_result;
	memdelete(f);
	
	String scene_path = p_source_file.get_basename() + ".tscn";
	Node *root = nullptr;
	Node **root_ptr = &root;
	for (int i = 1; i < data.size(); i++){
		Dictionary d = data[i];
		print_line(String(d["name"]));
		dict_to_node(d, root_ptr, scene_path, p_options);
	}
	if(root == nullptr)
		return OK;
	if (!p_options["main/update_only_editor"]){
		Ref<PackedScene> scene = memnew(PackedScene);
		Error err = scene->pack(root);
		if (err == OK){
			err = ResourceSaver::save(scene_path, scene);
		}
	}
	EditorData &editor_data = EditorNode::get_editor_data();
	int index = 0;
	for (int i = 0; i < editor_data.get_edited_scene_count(); i++) {
		if (editor_data.get_scene_path(i) == scene_path)
			index = i;
	}
	editor_data.set_edited_scene(index);
	//EditorNode::get_singleton()->reload_scene(scene_path);
	
	return OK;
}

ResourceImporterJSCN::ResourceImporterJSCN() {
}
