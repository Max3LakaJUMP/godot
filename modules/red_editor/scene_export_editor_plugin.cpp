/*************************************************************************/
/*  line_2d_editor_plugin.cpp                                            */
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

#include "scene_export_editor_plugin.h"
#include "editor/editor_node.h"
#include "core/io/json.h"
#include "core/bind/core_bind.h"
#include "modules/red/red_engine.h"
#include "modules/red/red_polygon.h"
#include "modules/red/red_json.h"

SceneExportEditorPlugin::SceneExportEditorPlugin(EditorNode *p_node) {
	maya_button = memnew(Button);
	maya_button->connect("pressed", this, "to_maya");
	maya_button->set_text("MAYA");
	buttons = memnew(HBoxContainer);
	buttons->add_child(maya_button);
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, buttons);
}

void SceneExportEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("to_maya"), &SceneExportEditorPlugin::to_maya);
}

void SceneExportEditorPlugin::to_maya() {
	Node *root = get_tree()->get_edited_scene_root();
	if (!root){
		return;
	}

	String scene_path = root->get_filename();
	String json_path = scene_path.get_basename() + ".json";
	
	Dictionary dict = scene_to_dict(root);

	save_json(dict, json_path);

	String json_global_path = "\'" + red::globalize(json_path) + "\'";

	List<String> arr;

	//String arr1 = "python(\\\"cmds.loadModule(load=\'redmaya\')\\\");";
	//String arr2 = "python(\\\"get+godot_scene(\'" + json_global_path + "\')\\\");";
	
	print_line("python(\\\"rsptf = " + json_global_path + "\\\");");
	print_line("python(\\\"cmds.loadModule(load=\'redmaya.redot\')\\\");");
	print_line("python(\\\"redmaya.redot.godot.load(rsptf)\\\");");

	arr.push_back("python(\\\"rsptf = " + json_global_path + "\\\");");
	arr.push_back("python(\\\"cmds.loadModule(load=\'redmaya.redot.godot\')\\\");");
	arr.push_back("python(\\\"redmaya.redot.godot.load(rsptf)\\\");");
	OS::get_singleton()->execute(red::globalize(String("res://redot/apps/commandPort.exe")), arr, true);
}

Dictionary SceneExportEditorPlugin::scene_to_dict(Node *root){
	Dictionary result;
	result["name"] = root->get_filename().get_file().get_basename();
	result["path"] = red::globalize(root->get_filename().get_basename()) + ".json";
	result["format"] = "scene";
	result["version"] = "1.0";

	result["root"] = node_to_dict(root);
	return result;
}

Dictionary SceneExportEditorPlugin::node_to_dict(Node *node){
	List<PropertyInfo> plist;
	Dictionary serialized;
	Variant name_property;
	String cl = node->get_class();

	ClassDB::get_property_list(cl, &plist, false, node);
	if (ClassDB::get_property(node, "name", name_property)){
		serialized["name"] = name_property;
	}
	serialized["type"] = cl;

	for (List<PropertyInfo>::Element *E = plist.front(); E; E = E->next()) {
		String name = E->get().name;
		if (name == "name" || name == "owner" || name == "_import_path" || 
			name == "custom_multiplayer" || name == "multiplayer" || 
			name == "global_scale" || name == "global_position" || 
			name == "global_rotation_degrees" || name == "global_rotation" || 
			name == "transform" || name == "global_transform" ||
			name == "current_animation" || name == "assigned_animation") 
			continue;
		Variant property;
		if (ClassDB::get_property(node, name, property))
			if (name == "texture"){
				RES r = property;
				if (r.is_valid())
					serialized[name] = r->get_path();
			}else if (name == "material"){
				Ref<ShaderMaterial> r = property;
				if (r.is_valid())
					serialized[name] = r->get_shader()->get_path();
			}else if (name == "bones"){
				if (cl=="REDPolygon"){
					REDPolygon *polygon = (REDPolygon*)node;
					Array bones;
					for (int i = 0; i < polygon->get_bone_count(); i++){
						Dictionary bone;
						bone["path"] = polygon->get_bone_path(i);
						bone["weights"] = polygon->get_bone_weights(i);
						bones.push_back(bone);
					}
					serialized["bones"] = bones;

				} else if (cl=="Polygon2D"){
					Polygon2D *polygon = (Polygon2D*)node;
					Array bones;
					for (int i = 0; i < polygon->get_bone_count(); i++){
						Dictionary bone;
						bone["path"] = polygon->get_bone_path(i);
						bone["weights"] = polygon->get_bone_weights(i);
						bones.push_back(bone);
					}
					serialized["bones"] = bones;
				}
			}else {
				serialized[name] = property;
			}
			
	}
	
	if (node->is_class("AnimationPlayer")){
		AnimationPlayer *player = (AnimationPlayer*)node;
		Array animations;
		List<StringName> animation_names;
		player->get_animation_list(&animation_names);
		animations.resize(animation_names.size());
		int animation_i=0;
		for (List<StringName>::Element *E = animation_names.front(); E; E = E->next()) {
			String animation_name = E->get();
			Ref<Animation> animation = player->get_animation(animation_name);
			Dictionary animation_dict;
			int track_count = animation->get_track_count();
			
			animation_dict["name"] = animation_name;

			List<PropertyInfo> animation_plist;
			ClassDB::get_property_list("Animation", &animation_plist, false, animation.ptr());
			for (List<PropertyInfo>::Element *E = animation_plist.front(); E; E = E->next()) {
				String name = E->get().name;
				if (name == "resource_local_to_scene" || name == "resource_path" || name == "resource_name") continue;
				Variant property;
				if (ClassDB::get_property(animation.ptr(), name, property))
					animation_dict[name] = property;
			}

			Array tracks;
			tracks.resize(track_count);
			for (int track_i = 0; track_i < track_count; track_i++){
				Dictionary track;
				int keys_count = animation->track_get_key_count(track_i);
				track["path"] = animation->track_get_path(track_i);
				track["length"] = keys_count;
				track["type"] = animation->track_get_type(track_i);
				track["interpolation_type"] = animation->track_get_interpolation_type(track_i);
				Array keys;
				keys.resize(keys_count);
				for (int key_i = 0; key_i < keys_count; key_i++){
					Variant value = animation->track_get_key_value(track_i, key_i);
					Dictionary key;
					key["value"] = value;
					key["time"] = animation->track_get_key_time (track_i, key_i);
					key["transition"] = animation->track_get_key_transition(track_i, key_i);
					keys[key_i] = key;
				}
				track["keys"] = keys;
				tracks[track_i] = track;
			}
			animation_dict["tracks"] = tracks;
			animations[animation_i] = animation_dict;
			animation_i++;
		}
		serialized["animations"] = animations;
	}

	/*
	if (node->is_class("AnimationPlayer")){
		AnimationPlayer *player = (AnimationPlayer*)node;
		Array layers;
		List<String> layer_names;
		player->get_animation_list(layer_names);
		int layers_count = layers.size();
		layers.resize(layers_count);
		for (int i = 0; i < layers_count; i++){
			String layer_name = layer_names[i];
			Ref<Animation> animation = player->get_animation(layer_name);
			Dictionary layer;
			Array keys;
			int track_count = animation->get_track_count();
			for (int track_i = 0; track_i < track_count; track_i++){
				Animation::TrackType type = animation->track_get_type(track_i);
				int keys_count = keys.size();
				for (int key_i = 0; key_i < keys_count; key_i++){
					Variant variant_value = animation->track_get_key_value(track_i, key_i);
					Dictionary key;

					key["layer"] = layer_name;
					key["track"] = layer_name;
					key["type"] = layer_name;
					key["layer"] = layer_name;
					key["interpolation_type"] = layer_name;
					key["transition"] = layer_name;
					key["value"] = layer_name;
					key["time"] = layer_name;
				}
				serialized["layers"][-1]["keys"].append(key)
			}
			layer["name"] = layer_name;
			layer["keys"] = keys;
			layers[i] = layer;
		}
		
		serialized["layers"] = layers;
	}*/

	int count = node->get_child_count();
	Array children;
	children.resize(count);
	for (int i = 0; i < count; i++){
		Node *child = node->get_child(i);
		children[i] = node_to_dict(child);
	}
	serialized["children"] = children;
	return serialized;
}
/*
Dictionary SceneExportEditorPlugin::node_to_dict(Node *node){
	Dictionary serialized;
	serialized["name"] = node->get_name();
	serialized["type"] = node->get_class();

	// mode script meta
	if (node->is_class("Node")){}
	// modulate selfmodulate showbehindparent lightmask
	if (node->is_class("CanvasItem")){
		CanvasItem *canvasitem = (CanvasItem*)node;
		serialized["visible"] = canvasitem->is_visible();
	}
	// zasrelative
	if (node->is_class("Node2D")){
		Node2D *node2d = (Node2D*)node;

		serialized["position"] = red::dict(node2d->get_position(), 0.0f);
		serialized["rotation_degrees"] = node2d->get_rotation_degrees();
		serialized["scale"] = red::dict(node2d->get_scale());
		serialized["z_index"] = node2d->get_z_index();
	}
	// color offset antialised textureoffset texturescale texturerotation skeleton invertenable invertborder
	if (node->is_class("Polygon2D")){
		Polygon2D *polygon = (Polygon2D*)node;

		{
			Ref<Texture> texture = polygon->get_texture();
			if (texture.is_valid()){
				serialized["texture"] = red::globalize(texture->get_path());
				serialized["texture_size"] = red::dict(texture->get_size());
			}
			else:
				serialized["texture"] = NULL;
				serialized["texture_size"] = {"w": 32, "h": 32};
		}
		{
			Array array;
			int count = polygon->get_polygon().size();
			PoolVector<Vector2>::Read r = polygon->get_polygon().read();
			array.resize(count);
			for (int i = 0; i < count; i++)
				array[i] = red::dict(r[i]);
			serialized["polygon"] = array;
		}
		{
			serialized["polygons"] = polygon->get_polygons();
		}
		{
			Array array;
			int count = polygon->get_uv().size();
			PoolVector<Vector2>::Read r = polygon->get_uv().read();
			array.resize(count);
			for (int i = 0; i < count; i++)
				array[i] = red::dict(r[i]);
			serialized["uv"] = array;
		}
		{
			Array array;
			int count = polygon->get_vertex_colors().size();
			PoolVector<Color>::Read r = polygon->get_vertex_colors().read();
			array.resize(count);
			for (int i = 0; i < count; i++)
				array[i] = red::dict(r[i]);
			serialized["vertex_colors"] = array;
		}
		{
			serialized["internal_vertex_count"] = polygon->get_internal_vertex_count();
		}
		{
			Array array_bones;
			Array array_weights;

			int count = polygon->get_bone_count();
			array_bones.resize(count);
			array_weights.resize(count);
			for (int i = 0; i < count; i++)
				array_bones[i] = polygon->get_bone_path(i);
				array_weights[i] = polygon->get_bone_weights(i);
			serialized["bones"] = array_bones;
			serialized["weights"] = array_weights;
	}
	// color offset antialised textureoffset texturescale texturerotation skeleton invertenable invertborder
	if (node->is_class("REDPolygon")){
		REDPolygon *polygon = (REDPolygon*)node;
		{
			Ref<Texture> texture = polygon->get_texture();
			if (texture.is_valid()){
				serialized["texture"] = red::globalize(texture->get_path());
				serialized["texture_size"] = red::dict(texture->get_size());
			}
			else:
				serialized["texture"] = NULL;
				serialized["texture_size"] = {"w": 32, "h": 32};
		}
		{
			Array array;
			int count = polygon->get_polygon().size();
			PoolVector<Vector2>::Read r = polygon->get_polygon().read();
			array.resize(count);
			for (int i = 0; i < count; i++)
				array[i] = red::dict(r[i]);
			serialized["polygon"] = array;
		}
		{
			serialized["polygons"] = polygon->get_polygons();
		}
		{
			Array array;
			int count = polygon->get_uv().size();
			PoolVector<Vector2>::Read r = polygon->get_uv().read();
			array.resize(count);
			for (int i = 0; i < count; i++)
				array[i] = red::dict(r[i]);
			serialized["uv"] = array;
		}
		{
			Array array;
			int count = polygon->get_vertex_colors().size();
			PoolVector<Color>::Read r = polygon->get_vertex_colors().read();
			array.resize(count);
			for (int i = 0; i < count; i++)
				array[i] = red::dict(r[i]);
			serialized["vertex_colors"] = array;
		}
		{
			serialized["internal_vertex_count"] = polygon->get_internal_vertex_count();
		}
		{
			Array array_bones;
			Array array_weights;

			int count = polygon->get_bone_count();
			array_bones.resize(count);
			array_weights.resize(count);
			for (int i = 0; i < count; i++)
				array_bones[i] = polygon->get_bone_path(i);
				array_weights[i] = polygon->get_bone_weights(i);
			serialized["bones"] = array_bones;
			serialized["weights"] = array_weights;
		}
	}
	//rootnode currentanimation methodcall proccessmode defaultblendtime speed
	if (node->is_class("AnimationPlayer")){
		AnimationPlayer *player = (AnimationPlayer*)node;
		Array layers;
		List<String> layer_names;
		player->get_animation_list(layer_names);
		int layers_count = layers.size();
		layers.resize(layers_count);
		for (int i = 0; i < layers_count; i++){
			String layer_name = layer_names[i];
			Ref<Animation> animation = player->get_animation(layer_name);
			Dictionary layer;
			Array keys;

			int track_count = animation->get_track_count();
			for (int track_i = 0; track_i < track_count; track_i++){
				Animation::TrackType type = animation->track_get_type(track_i);
				int keys_count = keys.size();
				for (int key_i = 0; key_i < keys_count; key_i++){
					Variant variant_value = animation->track_get_key_value(track_i, key_i);
					Variant::type = variant_value.get_type();
					if (type>=20){
						PoolVector<Vector2> value = variant_value;
						Array temp = red::dict(value);
						variant_value = temp;
					}
					else if (variant_value.get_type()==Variant::VECTOR2){
						PoolVector<Vector2> value = variant_value;
						Array temp = red::dict(value);
						variant_value = temp;
					}
				}
			}
			layer["name"] = layer_name;
			layer["keys"] = keys;
			layers[i] = layer;
		}
		serialized["layers"] = layers;
	}
}
*/

Error SceneExportEditorPlugin::save_json(Dictionary &p_data, String &p_path){
	Error err;
	FileAccess *f = FileAccess::open(p_path, FileAccess::WRITE, &err);
	if (err)
		return err;

	REDJSON *parser = memnew(REDJSON);
	f->store_string(parser->print(p_data, "    ", false, true));
		memdelete(parser);
	memdelete(f);

	return OK;
}