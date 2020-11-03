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
#include "modules/red/red_transform.h"
//#include "modules/red/red_polygon.h"
#include "modules/red/red_json.h"
#include "modules/red_editor/red_render_data.h"
#include "mediapipe.h"
#include "modules/red/red_deform.h"

SceneExportEditorPlugin::SceneExportEditorPlugin(EditorNode *p_node) {
	red_menu = memnew(MenuButton);
	red_menu->set_text("RED");
	PopupMenu *popup = red_menu->get_popup();
	popup->add_item("To Maya");
	popup->add_item("Render normals");
	popup->add_item("Mediapipe");
	popup->add_item("Attach transform");
	popup->connect("id_pressed", this, "red_clicked");
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, red_menu);
}

void SceneExportEditorPlugin::red_clicked(const int id) {
	switch (id){
	case 0:
		to_maya();
		break;
	case 1:
		selection_to_normals();
		break;
	case 2:
		create_mediapipe();
		break;
	case 3:
		attach_transform();
		break;
	default:
		break;
	}
}

void SceneExportEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("red_clicked", "id"), &SceneExportEditorPlugin::red_clicked);
	ClassDB::bind_method(D_METHOD("to_maya"), &SceneExportEditorPlugin::to_maya);
	ClassDB::bind_method(D_METHOD("selection_to_normals"), &SceneExportEditorPlugin::selection_to_normals);
	ClassDB::bind_method(D_METHOD("create_mediapipe"), &SceneExportEditorPlugin::selection_to_normals);
	ClassDB::bind_method(D_METHOD("attach_transform"), &SceneExportEditorPlugin::attach_transform);
}

void SceneExportEditorPlugin::to_maya() {
	Node *root = get_tree()->get_edited_scene_root();
	ERR_FAIL_COND_MSG(!root, "No root node")
	String scene_path = root->get_filename();
	String json_path = scene_path.get_basename() + ".json";
	
	Dictionary dict = scene_to_dict(root);

	save_json(dict, json_path);

	String json_global_path = "\'" + red::globalize(json_path) + "\'";

	List<String> arr;

	//print_line("python(\\\"rsptf = " + json_global_path + "\\\");");
	//print_line("python(\\\"cmds.loadModule(load=\'redmaya.redot\')\\\");");
	//print_line("python(\\\"redmaya.redot.godot.load(rsptf)\\\");");

	arr.push_back("python(\\\"rsptf = " + json_global_path + "\\\");");
	arr.push_back("python(\\\"cmds.loadModule(load=\'redmaya.redot.godot\')\\\");");
	arr.push_back("python(\\\"redmaya.redot.godot.load(rsptf)\\\");");
	OS::get_singleton()->execute(red::globalize(String("res://redot/apps/commandPort.exe")), arr, true);
}

Dictionary SceneExportEditorPlugin::scene_to_dict(Node *root){
	Dictionary result;
	result["name"] = root->get_filename().get_file().get_basename();
	result["path"] = root->get_filename().get_base_dir();
	//result["path_file"] = root->get_filename().get_basename() + ".json";
	result["project_path"] = ProjectSettings::get_singleton()->get_resource_path();
	
	//result["abs_path"] = red::globalize(root->get_filename().get_basename());
	//result["abs_path_file"] = red::globalize(root->get_filename().get_basename()) + ".json";
	result["format"] = "scene";
	result["mode"] = "2D";
	result["version"] = 1.0;

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
				/*if (cl=="REDPolygon"){
					REDPolygon *polygon = (REDPolygon*)node;
					Array bones;
					for (int i = 0; i < polygon->get_bone_count(); i++){
						Dictionary bone;
						bone["path"] = polygon->get_bone_path(i);
						bone["weights"] = polygon->get_bone_weights(i);
						bones.push_back(bone);
					}
					serialized["bones"] = bones;

				} else */
				if (cl=="Polygon2D"){
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

Vector<int> SceneExportEditorPlugin::triangulate(const PoolVector2Array &points, const Array &polygons){
	Vector<int> total_indices;
	for (int i = 0; i < polygons.size(); i++) {
		PoolVector<int> src_indices = polygons[i];
		int ic = src_indices.size();
		if (ic < 3)
			continue;
		PoolVector<int>::Read r = src_indices.read();

		Vector<Vector2> tmp_points;
		tmp_points.resize(ic);

		for (int j = 0; j < ic; j++) {
			int idx = r[j];
			ERR_CONTINUE(idx < 0 || idx >= points.size());
			tmp_points.write[j] = points[r[j]];
		}
		Vector<int> indices = Geometry::triangulate_polygon(tmp_points);
		int ic2 = indices.size();
		const int *r2 = indices.ptr();

		int bic = total_indices.size();
		total_indices.resize(bic + ic2);
		int *w2 = total_indices.ptrw();

		for (int j = 0; j < ic2; j++) {
			w2[j + bic] = r[r2[j]];
		}
	}
	return total_indices;
}

void SceneExportEditorPlugin::selection_to_normals(){
	List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
	bool delete_after_render = selection.size() > 1;
	for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
		Polygon2D *apply_polygon = Object::cast_to<Polygon2D>(E->get());
		if (!apply_polygon){
			REDRenderData *apply_normal_data = Object::cast_to<REDRenderData>(E->get());
			if (!apply_normal_data){
				continue;
			}
			apply_normal_data->render();
			continue;
		}
		if (apply_polygon->has_node(NodePath("generator_container"))){
			REDRenderData *normal_data = Object::cast_to<REDRenderData>(apply_polygon->get_node(NodePath("generator_container")));
			if(normal_data){
				normal_data->render();
				continue;
			}
		}
		Ref<Texture> texture = apply_polygon->get_texture();
		if (texture.is_null())
			continue;
		Size2 polygon_size = red::get_full_size(apply_polygon->get_polygon(), apply_polygon->get_uv());
		Size2 resolution = texture->get_size();
		float bitmap_threshold = 0.2f;
		Size2 bitmap_size(128, 128);
		Ref<Image> texture_image = texture->get_data();
		String diffuse_path = texture->get_path();
		String normal_path = diffuse_path.replace_first(".png", "_normal.png");
		
		Ref<Texture> rim_texture;
		String rim_path = diffuse_path.replace_first(".png", "_rim.png");
		_File file_сheck;
		if (file_сheck.file_exists(rim_path))
			rim_texture = ResourceLoader::load(rim_path, "Texture");

		REDRenderData *normal_data = red::create_node<REDRenderData>(apply_polygon, String("generator_container"), nullptr, true);
		normal_data->set_resolution(resolution);
		normal_data->set_use_background(true);
		normal_data->set_render_path(normal_path);
		normal_data->set_delete_after_render(delete_after_render);
		normal_data->set_clean_viewport_before_render(false);
		normal_data->set_apply_normal(true);
		bool use_real_normal = false;
		if(apply_polygon->get_depth_size() > 0.0f){
			use_real_normal = true;
			Polygon2D* p = red::create_node<Polygon2D>(normal_data, "mesh");
			float depth_size = apply_polygon->get_depth_size();
			float depth_offset = apply_polygon->get_depth_offset();
			PoolVector<Vector2> polygon = apply_polygon->get_polygon();
			PoolVector<Vector2> uvs = apply_polygon->get_uv();
			PoolVector<Color> vertex_colors = apply_polygon->get_vertex_colors();
			Array polygons = apply_polygon->get_polygons();
			int vtxs_size = polygon.size();

			PoolVector<Vector2> polygon_uv;
			{
				polygon_uv.resize(vtxs_size);
				PoolVector<Vector2>::Read uvs_read = uvs.read();
				PoolVector<Vector2>::Write polygon_uv_write = polygon_uv.write();
				for(int i = 0; i < vtxs_size; i++){
					polygon_uv_write[i] = uvs_read[i] * resolution;
				}
			}
			PoolVector<Vector3> vtxs;
			PoolVector<Vector3> vtxs_rotated;
			PoolVector<Vector2> polygon_rotated;
			{
				vtxs.resize(vtxs_size);
				vtxs_rotated.resize(vtxs_size);
				polygon_rotated.resize(vtxs_size);
				PoolVector<Vector2>::Read polygon_read = polygon.read();
				PoolVector<Color>::Read vertex_colors_read = vertex_colors.read();
				PoolVector<Vector3>::Write vtxs_write = vtxs.write();
				PoolVector<Vector3>::Write vtxs_rotated_write = vtxs_rotated.write();
				PoolVector<Vector2>::Write polygon_rotated_write = polygon_rotated.write();
				for(int i = 0; i < vtxs_size; i++){
					Vector3 p(polygon_read[i].x, polygon_read[i].y, (vertex_colors_read[i].a - depth_offset) * depth_size);
					vtxs_write[i] = p;
					vtxs_rotated_write[i] = p;
					polygon_rotated_write[i] = Vector2(p.x, p.y);
				}
			}
			PoolVector<Vector3> normals;
			normals.resize(vtxs_size);
			{
				PoolVector<Vector3>::Read vtxs_read = vtxs.read();
				PoolVector<Vector3>::Write normals_write = normals.write();
				for(int i = 0; i < vtxs_size; i++ ) normals_write[i] = Vector3();
				Vector<int> triangles = triangulate(polygon, polygons);
				for(int i = 2; i < triangles.size(); i += 3){
					const int ia = triangles[i-2];
					const int ib = triangles[i-1];
					const int ic = triangles[i];
					const Vector3 no = (vtxs_read[ia] - vtxs_read[ib]).cross(vtxs_read[ic] - vtxs_read[ib]);
					normals_write[ia] += no;
					normals_write[ib] += no;
					normals_write[ic] += no;
				}
			}
			PoolVector<Color> normal_colors;
			{
				PoolVector<Vector3>::Read normals_read = normals.read();
				PoolVector<Color>::Read vertex_colors_read = vertex_colors.read();
				normal_colors.resize(vtxs_size);
				PoolVector<Color>::Write normals_write = normal_colors.write();

				for(int i = 0; i < vtxs_size; i++) {
					Vector3 n = normals_read[i];
					n.normalize();
					normals_write[i] = Color(-n.x * 0.5 + 0.5, -n.y * 0.5 + 0.5, n.z * 0.5 + 0.5, vertex_colors_read[i].a);
				}
				
			}
			p->set_polygon(polygon_uv);
			p->set_uv(apply_polygon->get_uv());
			p->set_vertex_colors(normal_colors);
			p->set_polygons(polygons);
			p->set_internal_vertex_count(apply_polygon->get_internal_vertex_count());
			p->set_texture(texture);

			Ref<ShaderMaterial> material;
			material.instance();
			material->set_shader(ResourceLoader::load("res://redot/shaders/editor/normal_baker/mesh.shader", "Shader"));
			material->set_shader_param("use_texture_alpha", false);
			p->set_material(material);
		}else{
			Ref<BitMap> bitmap = red::read_bitmap(texture_image, bitmap_threshold, bitmap_size);
			Vector<Polygon2D*> polygons = red::bitmap_to_polygon2d(bitmap, resolution, 1.0, 4.0, false, true);
			for (int i = 0; i < polygons.size(); i++){
				Polygon2D* p = polygons[i];
				Ref<ShaderMaterial> material;
				material.instance();
				material->set_shader(ResourceLoader::load("res://redot/shaders/editor/normal_baker/mesh.shader", "Shader"));
				p->set_material(material);
				p->set_texture(texture);
				normal_data->add_child(p);
				p->set_owner(normal_data->get_owner());
			}
		}
		{
			Ref<ShaderMaterial> material;
			material.instance();
			material->set_shader(ResourceLoader::load("res://redot/shaders/editor/normal_baker/back.shader", "Shader"));
			Ref<Texture> texture = ResourceLoader::load(diffuse_path, "Texture");
			if (texture.is_valid()){
				material->set_shader_param("tex", texture);
			}
			if (use_real_normal){
				material->set_shader_param("back", 1);
			}else{
				material->set_shader_param("back", 0.5);
			}
			normal_data->set_back_material(material);
		}
		Array materials;
		if (!use_real_normal){
			Ref<Shader> shader = ResourceLoader::load("res://redot/shaders/editor/normal_baker/pass1.shader", "Shader");
			Ref<ShaderMaterial> material;
			material.instance();
			material->set_shader(shader);
			material->set_shader_param("amount_k", resolution / polygon_size);
			materials.push_back(material);
		}
		{
			Ref<Shader> shader = ResourceLoader::load("res://redot/shaders/editor/normal_baker/pass2.shader", "Shader");
			Ref<ShaderMaterial> material;
			material.instance();
			material->set_shader(shader);
			material->set_shader_param("amount_k", resolution / polygon_size);
			if (rim_texture.is_valid()){
				material->set_shader_param("tex", rim_texture);
				material->set_shader_param("use_texture", true);
				material->set_shader_param("rim_pixel_size", Size2(1.0f, 1.0f) / rim_texture->get_size());
			}
			if (use_real_normal){
				material->set_shader_param("amount", 4);
			}else{
				material->set_shader_param("amount", 32);
			}
			materials.push_back(material);
		}
		normal_data->set_materials(materials);
		if(apply_polygon){
			apply_polygon->set_light_mask(rim_texture.is_valid() ? 2 : 1);
		}
		normal_data->render();
	}
}

void SceneExportEditorPlugin::create_mediapipe() {
	List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
	String scene_path = "res://redot/polygons/faces/default_face.tscn";
	Ref<PackedScene> face_scene = ResourceLoader::load(scene_path);
	if (face_scene.is_null()){
		return;
	}
	for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
		Polygon2D *apply_polygon = Object::cast_to<Polygon2D>(E->get());
		if (!apply_polygon)
			continue;
		Node *parent = apply_polygon->get_parent();
		Node *face_scene_instance = face_scene->instance();
		Polygon2D *apply_polygon3d = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("face")));
		if (!apply_polygon3d){
			face_scene_instance->queue_delete();
			break;
		}
		face_scene_instance->remove_child(apply_polygon3d);
		parent->add_child_below_node(apply_polygon, apply_polygon3d);
		apply_polygon3d->set_owner(parent->get_owner());
		face_scene_instance->queue_delete();
		apply_polygon3d->set_name(String(apply_polygon->get_name()) + String("3d"));
		apply_polygon3d->set_position(apply_polygon->get_position());

		Size2 polygon_size = red::get_full_size(apply_polygon->get_polygon(), apply_polygon->get_uv());
		Mediapipe *mediapipe = red::create_node<Mediapipe>(apply_polygon3d, "mediapipe");
		mediapipe->set_polygon_size(polygon_size);
		mediapipe->set_face_scene(ResourceLoader::load(scene_path));
		for (int i = 0; i < parent->get_child_count(); i++){
			Polygon2D *child = Object::cast_to<Polygon2D>(parent->get_child(i));
			if (!child)
				continue;
			String name = child->get_name();
			if(name.find("blick") != -1){
				if(name.find("_l") != -1)
					mediapipe->set_eye_blick_l_path(mediapipe->get_path_to(child));
				else if(name.find("_r") != -1)
					mediapipe->set_eye_blick_r_path(mediapipe->get_path_to(child));
			}else if(name.find("eye") != -1){
				if(name.find("_l") != -1)
					mediapipe->set_eye_l_path(mediapipe->get_path_to(child));
				else if(name.find("_r") != -1)
					mediapipe->set_eye_r_path(mediapipe->get_path_to(child));
			}
		}
		mediapipe->set_face_path(mediapipe->get_path_to(apply_polygon));
		mediapipe->set_force_reset_polygon(true);
	}
}

void SceneExportEditorPlugin::attach_transform() {
	REDTransform *apply_transform = nullptr;
	REDDeform *apply_deform = nullptr;
	{
		List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
		for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
			if (!apply_transform)
				apply_transform = Object::cast_to<REDTransform>(E->get());
			if (!apply_deform)
				apply_deform = Object::cast_to<REDDeform>(E->get());
			if(apply_transform && apply_deform)
				break;
		}
	}
	if(!apply_deform && !apply_transform)
		return;
	List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
	for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
		Polygon2D *apply_polygon = Object::cast_to<Polygon2D>(E->get());
		if (!apply_polygon)
			continue;
		if(apply_transform)
			apply_polygon->set_custom_transform(apply_polygon->get_path_to(apply_transform));
		if(apply_deform)
			apply_polygon->set_deform(apply_polygon->get_path_to(apply_deform));
	}
}