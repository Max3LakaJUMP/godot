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
#include "scene/resources/bit_map.h"
#include "core/math/math_funcs.h"
#include <string>
#include "core/message_queue.h"
#include "editor/editor_node.h" 
#include "editor/import/resource_importer_texture_atlas.h"

#include "modules/red/red_polygon.h"
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
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/polygon"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/position"), true));
}
void ResourceImporterJSCN::dict_to_node(Dictionary &serialized, Node *parent, const Map<StringName, Variant> &p_options, String &scene_path, bool parent_is_root){
	Node *node;
	String name = serialized["name"];
	String type = serialized["type"];
	bool need_create = true;
	if (parent==nullptr){
		Ref<PackedScene> scene;
		_File file2Check;
		if (file2Check.file_exists(scene_path)){
			scene = ResourceLoader::load(scene_path, "PackedScene");
			node = scene->instance();
			need_create = false;
		}
	}
	else if (parent->has_node(name)){
		node = parent->get_node(name);
		need_create = false;
	}
	if (need_create){
		if (type=="Node")
			node = red::create_node<Node>(parent, name);
		else if (type=="Node2D")
			node = red::create_node<Node2D>(parent, name);
		else if (type=="Polygon2D"){
			node = red::create_node<Polygon2D>(parent, name);
			print_line(type);}
		else if (type=="REDPolygon")
			node = red::create_node<REDPolygon>(parent, name);
		else if (type=="REDFrame")
			node = red::create_node<REDFrame>(parent, name);
		else if (type=="REDPage")
			node = red::create_node<REDPage>(parent, name);
		else if (type=="AnimationPlayer")
			node = red::create_node<AnimationPlayer>(parent, name);
		else if (type=="Skeleton2D")
			node = red::create_node<Skeleton2D>(parent, name);
		else if (type=="Bone2D")
			node = red::create_node<Bone2D>(parent, name);
		else{
			return;
		}
	}
	List<PropertyInfo> plist;
	ClassDB::get_property_list(node->get_class(), &plist, false, node);
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
			if (name == "texture" && need_create){
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
			}
			else if (serialized[name].get_type() == node->get(name).get_type() || 
			(serialized[name].get_type() == Variant::INT && node->get(name).get_type() == Variant::REAL) || 
			(serialized[name].get_type() == Variant::REAL && node->get(name).get_type() == Variant::INT)){
				node->set(name, serialized[name]);
				print_line(std::to_string((int)serialized[name].get_type()).c_str());
				print_line(std::to_string((int)node->get(name).get_type()).c_str());
				print_line("Appled " + name + " property");
			}else {
				print_line(std::to_string((int)serialized[name].get_type()).c_str());
				print_line(std::to_string((int)node->get(name).get_type()).c_str());
				print_line("Ignored " + name + " property");
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
			List<PropertyInfo> animation_plist;
			ClassDB::get_property_list("Animation", &animation_plist, false, animation.ptr());
			for (List<PropertyInfo>::Element *E = animation_plist.front(); E; E = E->next()) {
				String name = E->get().name;
				if (name == "resource_local_to_scene" || name == "resource_path" || name == "resource_name" || 
					name == "current_animation_length" || name == "current_animation_position") continue;
				if (serialized.has(name)){
					if (serialized[name].get_type()==animation->get(name).get_type())
						animation->set(name, serialized[name]);
					else
						print_line("Ignored " + name + " animaation property");
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
				if (track_i_new==-1){
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
			dict_to_node(child, node, p_options, scene_path);
			print_line(node->get_name());
		}
	}
	if (parent==nullptr){
		Ref<PackedScene> scene = memnew(PackedScene);
		Error err = scene->pack(node);
		if (err==OK){
			err = ResourceSaver::save(scene_path, scene);
			if (err==OK){
				EditorNode::get_singleton()->reload_scene(scene_path);
			}
		}
	}
}
/*
void ResourceImporterJSCN::inject_data_to(Array &children, String &scene_path, const Map<StringName, Variant> &p_options, Node *parent, bool parent_is_root){
	int count = children.size();
	Node *node;
	for (int i = 0; i < count; i++){	
		Dictionary child = children[i];
		String type = child["type"];
		String name = child["name"];
		bool need_create = true;
		if (parent==nullptr){
			Ref<PackedScene> scene;
			_File file2Check;
			if (file2Check.file_exists(scene_path)){
				scene = ResourceLoader::load(scene_path, "PackedScene");
				node = scene->instance();
				need_create = false;
			}
		}
		else if (parent->has_node(name)){
			need_create = false;
			node = parent->get_node(name);
		}
		if (need_create){
			if (type=="Node")
				continue;
			if (type=="Node2D")
				node = memnew(Node2D);
			else if (type=="Polygon2D"){
				node = memnew(Polygon2D);
				print_line(type);}
			else if (type=="REDPolygon")
				node = memnew(REDPolygon);
			else if (type=="AnimationPlayer")
				node = memnew(AnimationPlayer);
			else if (type=="Skeleton2D")
				node = memnew(Skeleton2D);
			else if (type=="Bone2D")
				node = memnew(Bone2D);
		}
		if (node==nullptr)
			continue;
		node->set_name(name);
		if (!(parent==nullptr)){
			parent->add_child(node);
			if (parent_is_root)
				node->set_owner(parent);
			else
				node->set_owner(parent->get_owner());
		}
		// mode script meta
		if (node->is_class("Node")){}
		// modulate selfmodulate showbehindparent lightmask
		if (node->is_class("CanvasItem")){
			CanvasItem *canvasitem = (CanvasItem*)node;
			if (child.has("visible")){
				bool visible = false;
				visible = child["visible"];
				canvasitem->set_visible(visible);
			}
		}
		// zasrelative
		if (node->is_class("Node2D")){
			if (p_options["update/position"] || need_create){
				Node2D *node2d = (Node2D*)node;
				node2d->set_position(red::vector2(child["position"]));
				node2d->set_rotation_degrees(child["rotation_degrees"]);
				node2d->set_scale(red::vector2(child["scale"]));
				node2d->set_z_index(child["z_index"]);
			}
		}
		// color offset antialised textureoffset texturescale texturerotation skeleton invertenable invertborder
		if (node->is_class("Polygon2D")){
			Polygon2D *polygon2d = (Polygon2D*)node;
			
			if (child.has("texture")){
				String texture_path = child["texture"];
				Ref<Texture> texture = ResourceLoader::load(ProjectSettings::get_singleton()->localize_path(texture_path), "Texture");
				polygon2d->set_texture(texture);
			}
			if (p_options["update/polygon"] || need_create){
				if (child.has("polygon")){
					PoolVector<Vector2> polygon;
					Array polygon_array = child["polygon"];
					int polygon_count = polygon_array.size();
					for (int i = 0; i < polygon_count; i++)
						polygon.append(red::vector2(polygon_array[i]));
					polygon2d->set_polygon(polygon);
				}
				if (child.has("polygons")){
					Array polygons_array = child["polygons"];
					polygon2d->set_polygons(polygons_array);
				}	
				if (child.has("uv")){
					PoolVector<Vector2> uv;
					Array uv_array = child["uv"];
					int uv_count = uv_array.size();
					for (int i = 0; i < uv_count; i++)	
						uv.append(red::vector2(uv_array[i]));
					polygon2d->set_uv(uv);
				}
			}

			if (child.has("vertex_colors")){
				PoolVector<Color> vertex_colors;
				Array vertex_colors_array = child["vertex_colors"];
				int vertex_colors_count = vertex_colors.size();
				for (int i = 0; i < vertex_colors_count; i++)	
					vertex_colors.append(red::color(vertex_colors_array[i]));
				polygon2d->set_vertex_colors(vertex_colors);
			}

			if (child.has("internal_vertex_count")){
				int internal_vertex_count = child["internal_vertex_count"];
				polygon2d->set_internal_vertex_count(internal_vertex_count);
			}

			if (child.has("weights")){
				Array weights = child["weights"];
				int weights_count = weights.size();

				for (int i = 0; i < weights_count; i++)	
					polygon2d->set_bone_weights(i, weights[i]);
			}
		}
		// color offset antialised textureoffset texturescale texturerotation skeleton invertenable invertborder
		else if (node->is_class("REDPolygon")){
			REDPolygon *redpolygon = (REDPolygon*)node;
			if (child.has("texture")){
				String texture_path = child["texture"];
				Ref<Texture> texture = ResourceLoader::load(ProjectSettings::get_singleton()->localize_path(texture_path), "Texture");
				redpolygon->set_texture(texture);
			}
			if (p_options["update/polygon"] || need_create){
				if (child.has("polygon")){
					PoolVector<Vector2> polygon;
					Array polygon_array = child["polygon"];
					int polygon_count = polygon_array.size();
					for (int i = 0; i < polygon_count; i++)
						polygon.append(red::vector2(polygon_array[i]));
					redpolygon->set_polygon(polygon);
				}
				if (child.has("polygons")){
					Array polygons_array = child["polygons"];
					redpolygon->set_polygons(polygons_array);
				}
				if (child.has("uv")){
					PoolVector<Vector2> uv;
					Array uv_array = child["uv"];
					int uv_count = uv_array.size();
					for (int i = 0; i < uv_count; i++)	
						uv.append(red::vector2(uv_array[i]));
					redpolygon->set_uv(uv);
				}
			}
			if (child.has("vertex_colors")){
				PoolVector<Color> vertex_colors;
				Array vertex_colors_array = child["vertex_colors"];
				int vertex_colors_count = vertex_colors.size();
				for (int i = 0; i < vertex_colors_count; i++)	
					vertex_colors.append(red::color(vertex_colors_array[i]));
				redpolygon->set_vertex_colors(vertex_colors);
			}

			if (child.has("internal_vertex_count")){
				int internal_vertex_count = child["internal_vertex_count"];
				redpolygon->set_internal_vertex_count(internal_vertex_count);
			}

			if (child.has("weights")){
				Array weights = child["weights"];
				int weights_count = weights.size();

				for (int i = 0; i < weights_count; i++)	
					redpolygon->set_bone_weights(i, weights[i]);
			}
		}
		//rootnode currentanimation methodcall proccessmode defaultblendtime speed
		else if (node->is_class("AnimationPlayer")){
			AnimationPlayer *player = (AnimationPlayer*)node;
			Array layers = child["layers"];
			int layers_count = layers.size();
			for (int i = 0; i < layers_count; i++){
				Dictionary layer = layers[i];
				String name = layer["name"];
				Ref<Animation> animation = player->get_animation(name);

				if (animation.is_valid()){
					animation = Ref<Animation>(memnew(Ref<Animation>));
					player->add_animation(name, animation);
				}
				Array keys = layer["keys"];
				int keys_count = keys.size();
				for (int j = 0; j < keys_count; j++){
					Dictionary key = keys[j];
					String track = key["track"];
					Animation::TrackType type = (Animation::TrackType)(int)key["type"];
					Animation::InterpolationType interpolation_type = (Animation::InterpolationType)(int)key["interpolation_type"];
					int idx = animation->find_track(track);
					
					if (idx==-1){
						idx = animation->add_track(type, -1);
						animation->track_set_path(idx, track);
						animation->track_set_interpolation_type(idx, interpolation_type);
					}

					Variant value = key["value"];
					Variant new_value;
					if (value.get_type()==Variant::ARRAY){
						Array value_array = Array(value);
						if(value_array.size()>0){
							if (value_array[0].get_type()==Variant::DICTIONARY){
								Array keys = layer["keys"];
								int value_count = value_array.size();
								PoolVector<Vector2> values;
								for (int z = 0; z < value_count; z++){
									Dictionary value = value_array[z];
									values.append(red::vector2(value));
								}
								new_value = values;
							}
						}
					} else if (value.get_type()==Variant::DICTIONARY)
						new_value = Vector2(red::vector2(value));
					else{
						new_value = value;
					}

					int key_idx = animation->track_find_key (idx, key["time"], true);
					if (key_idx == -1){
						animation->track_insert_key(idx, key["time"], new_value, key["transition"]);
						key_idx = animation->track_find_key(idx, key["time"], true);
					}
					else{
						animation->track_set_key_value (idx, key_idx, new_value);
						animation->track_set_key_transition (idx, key_idx, key["transition"]);
					}
				} 
			}
		}
		else if (node->is_class("Skeleton2D")){
			Skeleton2D *skeleton = (Skeleton2D*)node;
		}
		// rest length
		else if (node->is_class("Bone2D")){
			Bone2D *node2d = (Bone2D*)node;
		}
		if (child.has("children")) {
			Array node_children = child["children"];
			inject_data_to(node_children, scene_path, p_options, node, parent==nullptr);
		}
	}
	if (parent==nullptr){
		Ref<PackedScene> scene = memnew(PackedScene);
		Error err = scene->pack(node);
		if (err==OK){
			err = ResourceSaver::save(scene_path, scene);
			if (err==OK){
				EditorNode::get_singleton()->reload_scene(scene_path);
			}
		}
	}
}
*/
Error ResourceImporterJSCN::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	FileAccess *f = FileAccess::open(p_source_file, FileAccess::READ);
	ERR_FAIL_COND_V(!f, ERR_CANT_OPEN);
	String json_text = f->get_as_utf8_string();

	Variant parse_result;
	String err_string;
	int err_int;
	Error err = REDJSON::parse(json_text, parse_result, err_string, err_int);
	if(err!=OK){
		memdelete(f);
		return ERR_CANT_OPEN;
	}
	Dictionary data = Dictionary(parse_result);
	memdelete(f);
	
	String scene_path = p_source_file.get_basename() + ".tscn";

	if (data.has("root")) {
		Dictionary root_serialized = data["root"];
		dict_to_node(root_serialized, nullptr, p_options, scene_path);
	}
	return OK;
}

ResourceImporterJSCN::ResourceImporterJSCN() {
}
