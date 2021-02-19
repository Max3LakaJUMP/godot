#include "maya.h"
#include "scene/animation/animation_player.h"
#include "scene/resources/material.h"
#include "scene/2d/polygon_2d.h"
#include "modules/red/red_engine.h"
#include "modules/red/red_json.h"
#include "editor/editor_plugin.h"
#include "scene/animation/tween.h"
#include "editor/editor_data.h"
#include "scene/main/viewport.h"
#include "scene/2d/skeleton_2d.h"
#include "modules/red/red_shape_renderer.h"
#include "modules/red/red_clipper.h"
#include "modules/red/red_frame.h"
#include "modules/red/red_page.h"
#include "modules/red/red_target.h"
#include "modules/red/red_transform.h"
#include "modules/red/red_parallax_folder.h"
#include "core/bind/core_bind.h"
#include "core/variant.h"
#include "editor/editor_node.h"

// Core
Variant Maya::property_to_dict(Node *node, const String &property_name){
	Variant out;
	if (!node)
		return out;
	Variant property;
	if(!ClassDB::get_property(node, property_name, property))
		return out;
	if (property_name == "texture"){
		RES r = property;
		if (r.is_valid())
			out = r->get_path();
	}else if (property_name == "material"){
		Ref<ShaderMaterial> r = property;
		if (r.is_valid())
			out = r->get_path();
			// out = r->get_shader()->get_path();
	}else if (property_name == "bones"){
		Polygon2D *polygon = Object::cast_to<Polygon2D>(node);
		if (polygon){
			Array bones;
			for (int i = 0; i < polygon->get_bone_count(); i++){
				Dictionary bone;
				bone["path"] = polygon->get_bone_path(i);
				bone["weights"] = polygon->get_bone_weights(i);
				bones.push_back(bone);
			}
			out = bones;
		}
	}else {
		out = property;
	}
	return out;
}

void Maya::animation_player_to_dict(AnimationPlayer *player, Dictionary &properties){
	Array animations;
	List<StringName> animation_names;
	player->get_animation_list(&animation_names);
	animations.resize(animation_names.size());
	int animation_i = 0;
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
	properties["animations"] = animations;
}

void Maya::node_to_dict(Node *node, Node *root, Array &nodes, bool child, const Array &properties){
	if (!root)
		return;
	Dictionary serialized;
	Variant name_property;
	if (ClassDB::get_property(node, "name", name_property))
		serialized["name"] = name_property;
	else
		return;
	if(node == root)
		serialized["parent"] = "";
	else{
		Node *root_parent = root->get_parent();
		if(!root_parent){
			return;
		}
		serialized["parent"] = root_parent->get_path_to(node->get_parent());
	}
	String type = node->get_class();
	serialized["type"] = type;
	int prop_size = properties.size();
	if(properties.size() > 0){
		for (int i = 0; i < prop_size; i++){
			String property = properties[i];
			Variant new_value = property_to_dict(node, property);
			if(!new_value.is_zero())
				serialized[property] = new_value;
		}
	}
	else{
		List<PropertyInfo> plist;
		ClassDB::get_property_list(type, &plist, false, node);
		for (List<PropertyInfo>::Element *E = plist.front(); E; E = E->next()) {
			String property = E->get().name;
			if (property == "property" || property == "owner" || property == "_import_path" || 
				property == "custom_multiplayer" || property == "multiplayer" || 
				property == "global_scale" || property == "global_position" || 
				property == "global_rotation_degrees" || property == "global_rotation" || 
				property == "transform" || property == "global_transform" ||
				property == "current_animation" || property == "assigned_animation") 
				continue;
			Variant new_value = property_to_dict(node, property);
			if(!new_value.is_zero())
				serialized[property] = new_value;
		}
	}
	AnimationPlayer *player = Object::cast_to<AnimationPlayer>(node);
	if (player){
		animation_player_to_dict(player, serialized);
		// AnimationPlayer *player = (AnimationPlayer*)node;
		
		// Array animations;
		// List<StringName> animation_names;
		// player->get_animation_list(&animation_names);
		// animations.resize(animation_names.size());
		// int animation_i=0;
		// for (List<StringName>::Element *E = animation_names.front(); E; E = E->next()) {
		// 	String animation_name = E->get();
		// 	Ref<Animation> animation = player->get_animation(animation_name);
		// 	Dictionary animation_dict;
		// 	int track_count = animation->get_track_count();
			
		// 	animation_dict["name"] = animation_name;

		// 	List<PropertyInfo> animation_plist;
		// 	ClassDB::get_property_list("Animation", &animation_plist, false, animation.ptr());
		// 	for (List<PropertyInfo>::Element *E = animation_plist.front(); E; E = E->next()) {
		// 		String name = E->get().name;
		// 		if (name == "resource_local_to_scene" || name == "resource_path" || name == "resource_name") continue;
		// 		Variant property;
		// 		if (ClassDB::get_property(animation.ptr(), name, property))
		// 			animation_dict[name] = property;
		// 	}

		// 	Array tracks;
		// 	tracks.resize(track_count);
		// 	for (int track_i = 0; track_i < track_count; track_i++){
		// 		Dictionary track;
		// 		int keys_count = animation->track_get_key_count(track_i);
		// 		track["path"] = animation->track_get_path(track_i);
		// 		track["length"] = keys_count;
		// 		track["type"] = animation->track_get_type(track_i);
		// 		track["interpolation_type"] = animation->track_get_interpolation_type(track_i);
		// 		Array keys;
		// 		keys.resize(keys_count);
		// 		for (int key_i = 0; key_i < keys_count; key_i++){
		// 			Variant value = animation->track_get_key_value(track_i, key_i);
		// 			Dictionary key;
		// 			key["value"] = value;
		// 			key["time"] = animation->track_get_key_time (track_i, key_i);
		// 			key["transition"] = animation->track_get_key_transition(track_i, key_i);
		// 			keys[key_i] = key;
		// 		}
		// 		track["keys"] = keys;
		// 		tracks[track_i] = track;
		// 	}
		// 	animation_dict["tracks"] = tracks;
		// 	animations[animation_i] = animation_dict;
		// 	animation_i++;
		// }
		// serialized["animations"] = animations;
	}
	nodes.append(serialized);
	if(child){
		int count = node->get_child_count();
		for (int i = 0; i < count; i++)
			node_to_dict(node->get_child(i), root, nodes, true);
	}
}

void Maya::dict_to_property(Node *node, const String &property_name, const Variant &new_value, bool force){
	if (!node)
		return;
	if (property_name == "name")
		return;
	Variant old_value = node->get(property_name);
	int old_type = old_value.get_type();
	int new_type = new_value.get_type();
	if ((property_name == "position" || property_name == "rotation_degrees" || property_name == "scale") && !force){
		if(!receiver_tween)
			receiver_tween = red::create_node<Tween>(EditorInterface::get_singleton()->get_tree()->get_root(), "ReceiverTween");
		receiver_tween->stop(node, property_name);
		receiver_tween->interpolate_property(node, property_name, old_value, new_value, 0.99f / receiver_fps);
		receiver_tween->start();
	}
	else if (property_name == "texture" && force){// || update_texture)){
		String global_path = new_value;
		node->set(property_name, ResourceLoader::load(global_path, "Texture"));
	}
	else if (property_name == "material" && force){
		String global_path = new_value;
		Ref<ShaderMaterial> material = ResourceLoader::load(global_path, "Material");
		// String global_path = new_value;
		// Ref<ShaderMaterial> material;
		// material.instance();
		// Ref<Shader> shader = ResourceLoader::load(global_path, "Shader");
		// material->set_shader(shader);
		node->set(property_name, material);
	}else if (property_name == "bones"){
		Polygon2D *polygon = Object::cast_to<Polygon2D>(node);
		if (polygon){
			Array bones = new_value;
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
	}else if (old_type == new_type || 
			(old_type == Variant::INT && new_type == Variant::REAL) || 
			(old_type == Variant::REAL && new_type == Variant::INT)){
		node->set(property_name, new_value);
	}else if (old_type == Variant::NODE_PATH && new_type == Variant::STRING){
		node->set(property_name, NodePath(new_value));
	}else if(echo){
		print_line("Ignored " + property_name + " property");
		print_line(Variant(old_type));
		print_line(Variant(new_type));
	}
}

void Maya::dict_to_animation_player(AnimationPlayer *player, const Dictionary &serialized){
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

Node *Maya::dict_to_node(const Dictionary &serialized, Node *root){
	Node *node = nullptr;
	Node *parent = nullptr;
	String name = serialized["name"];
	String parent_path = serialized["parent"];
	String type = serialized["type"];
	bool need_create = true;

	if(parent_path == ""){
		node = root;
		need_create = false;
	}else{
		Node *root_parent = root->get_parent();
		if(!root_parent){
			root = nullptr;
			return node;
		}
		parent = root_parent->get_node(NodePath(parent_path));
		ERR_FAIL_COND_V_MSG(!parent, node, "Parent error")
		if (parent->has_node(name)){
			node = parent->get_node(name);
			need_create = false;
		}
	}
	if (need_create){
		if (type == "Node")
			node = red::create_node<Node>(parent, name);
		else if (type == "Node2D")
			node = red::create_node<Node2D>(parent, name);
		else if (type == "Polygon2D")
			node = red::create_node<Polygon2D>(parent, name);
		else if (type == "REDFrame")
			node = red::create_node<REDFrame>(parent, name);
		else if (type == "REDPage")
			node = red::create_node<REDPage>(parent, name);
		else if (type == "REDParallaxFolder")
			node = red::create_node<REDParallaxFolder>(parent, name);
		else if (type == "REDTarget")
			node = red::create_node<REDTarget>(parent, name);
		else if (type == "REDTransform")
			node = red::create_node<REDTransform>(parent, name);
		else if (type == "AnimationPlayer")
			node = red::create_node<AnimationPlayer>(parent, name);
		else if (type == "Skeleton2D")
			node = red::create_node<Skeleton2D>(parent, name);
		else if (type == "Bone2D")
			node = red::create_node<Bone2D>(parent, name);
		else if (type == "REDShapeRenderer")
			node = red::create_node<REDShapeRenderer>(parent, name);
		else if (type == "REDClipper")
			node = red::create_node<REDClipper>(parent, name);
		else{
			return node;
		}
	}
	List<Variant> plist;
	serialized.get_key_list(&plist);
	for (List<Variant>::Element *E = plist.front(); E; E = E->next()){
		String property = E->get();
		dict_to_property(node, property, serialized[property], need_create);
	}
	AnimationPlayer *player = Object::cast_to<AnimationPlayer>(node);
	if (player)
		dict_to_animation_player(player, serialized);
	return node;
}

// Sender
void Maya::_send_node(Node *node, bool full_scene, bool use_file, const Array &properties) {
	// python("cmds.loadModule(load='redmaya.redot.godot')");
	if(!node)
		return;
	Node *root = node->get_tree()->get_edited_scene_root();
	String scene_path = root->get_filename();
	ERR_FAIL_COND_MSG(!root, "No root node")
	ERR_FAIL_COND_MSG(scene_path == "", "Save scene!")
	Array array;
	array.append(get_scene(root));
	node_to_dict(node, root, array, full_scene, properties);

	if(use_file && false){
		String scene_path = root->get_filename();
		String json_path = scene_path.get_basename() + ".json";
		save_json(array, json_path);
		String json_global_path = red::globalize(json_path);
		// python("redmaya.redot.godot.load('" + json_global_path + "')");
		send_text(json_global_path);
	}
	else{
		REDJSON *parser = memnew(REDJSON);
		// String scene_text = parser->print(array, "", false, true).replace("\"", "'").replace("\n", " ").replace("true", "True").replace("false", "False");
		String scene_text = parser->print(array, "", false, true);
		memdelete(parser);
		// python("redmaya.redot.godot.load(" + scene_text + ")");
		send_text(scene_text);
	}
}

void Maya::_send_nodes(const Array &nodes, bool full_scene, bool use_file, const Array &properties) {
	// python("cmds.loadModule(load='redmaya.redot.godot')");
	if(nodes.size() == 0)
		return;
	Node *node = nodes[0];
	if(!node)
		return;
	Node *root = node->get_tree()->get_edited_scene_root();
	String scene_path = root->get_filename();
	ERR_FAIL_COND_MSG(!root, "No root node")
	ERR_FAIL_COND_MSG(scene_path == "", "Save scene!")
	Array array;
	array.append(get_scene(root));

	for (int i = 0; i < nodes.size(); i++){
		node = nodes[i];
		if(!node)
			continue;
		node_to_dict(node, root, array, full_scene, properties);
	}
	if(use_file && false){
		String scene_path = root->get_filename();
		String json_path = scene_path.get_basename() + ".json";
		save_json(array, json_path);
		String json_global_path = red::globalize(json_path);
		// python("redmaya.redot.godot.load('" + json_global_path + "')");
		send_text(json_global_path);
	}
	else{
		REDJSON *parser = memnew(REDJSON);
		//String scene_text = parser->print(array, "", false, true).replace("\"", "'").replace("\n", " ").replace("true", "True").replace("false", "False");
		String scene_text = parser->print(array, "", false, true);
		memdelete(parser);
		// python("redmaya.redot.godot.load(" + scene_text + ")");
		send_text(scene_text);
	}
}

void Maya::send_node_properties(Node *node, const Array &properties, bool use_file) {
	_send_node(node, false, use_file, properties);
}

void Maya::send_nodes_properties(const Array &nodes, const Array &properties, bool use_file) {
	_send_nodes(nodes, false, use_file, properties);
}

void Maya::send_node(Node *node, bool use_file) {
	_send_node(node, false, use_file);
}

void Maya::send_selected_nodes(bool use_file) {
	Array nodes;
	List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
	for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
		Node *node = E->get();
		nodes.append(node);
	}
	_send_nodes(nodes, false, use_file);
}

void Maya::send_nodes(const Array &nodes, bool use_file) {
	_send_nodes(nodes, false, use_file);
}

void Maya::send_scene(Node *node, bool use_file) {
	_send_node(node, true, use_file);
}

// Load
Error Maya::load_file(const String &p_source_file) {
	FileAccess *f = FileAccess::open(p_source_file, FileAccess::READ);
	ERR_FAIL_COND_V(!f, ERR_CANT_OPEN);
	String json_text = f->get_as_utf8_string();
	memdelete(f);
	return load(json_text);
}

Error Maya::load(const String &json_text) {
	Variant parse_result;
	String err_string;
	int err_int;
	Error err = REDJSON::parse(json_text, parse_result, err_string, err_int);
	ERR_FAIL_COND_V_MSG(err != OK, ERR_INVALID_DATA, "Can't parse json")
	Array serialized = parse_result;
	ERR_FAIL_COND_V_MSG(!check_serialized(serialized), FAILED, "Serialization error")
	Dictionary scene_data = serialized[0];
	String path = String(scene_data["path"]);
	String name = String(scene_data["name"]);
	const String scene_path = red::localize(path) + name + ".tscn";
	_File file_сheck;
	bool force_save;
	bool b_update = true;
	if (file_сheck.file_exists(scene_path)){
		if (!b_update)
			return OK;
	}
	else{
		red::scene_loader(scene_path);
		force_save = true;
	}
	if (!EditorNode::get_singleton()->is_scene_open(scene_path)){
		EditorNode::get_singleton()->load_scene(scene_path);
	}
	Node *root = nullptr;
	int current_edited_scene = -1;
	EditorData &editor_data = EditorNode::get_editor_data();
	for (int i = 0; i < editor_data.get_edited_scene_count(); i++) {
		if (editor_data.get_scene_path(i) == scene_path){
			current_edited_scene = i;
			root = editor_data.get_edited_scene_root(i);
		}
	}
	ERR_FAIL_COND_V_MSG(current_edited_scene == -1, FAILED, "Can't get root node in edited scene")
	editor_data.set_edited_scene(current_edited_scene);
	for (int i = 1; i < serialized.size(); i++){
		if(root == nullptr || root != editor_data.get_edited_scene_root(current_edited_scene) || !root->is_inside_tree())
			break;
		Dictionary d = serialized[i];
		dict_to_node(d, root);
	}
	if(root == nullptr || root != editor_data.get_edited_scene_root(current_edited_scene))
		return OK;
	if (force_save){
		Ref<PackedScene> scene = memnew(PackedScene);
		Error err = scene->pack(root);
		if (err == OK)
			err = ResourceSaver::save(scene_path, scene);
	}
	return OK;
}

// Servers
void Maya::start_server() {
    // acceptor.listen(max_connections);
// 	m_acceptor.async_accept(m_socket, [this](std::error_code ec) {
// 		if (!ec) {
// 			std::make_shared<Maya>(std::move(std::move(m_socket)))->start();
// 		}
// 		std::cout << "accept new client" << std::endl;
// 		_server_process();
// 	});
//   }

    // std::cout << "Run server on " << port << " port with "
    //           << max_connections << " max connections and "
    //           << max_threads << " max threads." << std::endl;
	// std::thread thread = std::bind(&Maya::start_server, this);
	// thread.join();

    

	// if (!maya_tcp_server_is_listening){
	// 	maya_tcp_server_is_listening = true;
	// 	maya_tcp_server->startWorkerThread(1);
	// 	auto enterCallback = [](const TcpConnection::Ptr& session) {
	// 		total_client_num++;

	// 		session->setDataCallback([session](brynet::base::BasePacketReader& reader) {
	// 			session->send(reader.begin(), reader.size());
	// 			TotalRecvSize += reader.size();
	// 			total_packet_num++;
	// 			reader.consumeAll();
	// 		});

	// 		session->setDisConnectCallback([](const TcpConnection::Ptr& session) {
	// 			(void) session;
	// 			total_client_num--;
	// 		});
	// 	};
	// 	wrapper::ListenerBuilder listener;
	// 	listener.configureService(maya_tcp_server)
	// 			.configureSocketOptions({[](TcpSocket& socket) {
	// 				socket.setNodelay();
	// 			}})
	// 			.configureConnectionOptions({brynet::net::AddSocketOption::WithMaxRecvBufferSize(1024 * 1024),
	// 										brynet::net::AddSocketOption::AddEnterCallback(enterCallback)})
	// 			.configureListen([=](wrapper::BuildListenConfig config) {
	// 				config.setAddr(false, "0.0.0.0", 6001);
	// 			})
	// 			.asyncRun();
	// }
		
	if(!server_timer){
		server_timer = red::create_node<Timer>(EditorInterface::get_singleton()->get_tree()->get_root(), "ServerTimer");
		server_timer->connect("timeout", this, "_server_process");
	}
	if (server_timer->is_stopped()){
		maya_tcp_server->listen(6001);
		server_timer->start(0.01);
	}
}

void Maya::stop_server() {
	if(server_timer && !server_timer->is_stopped()){
		server_timer->stop();
	}
	if (maya_tcp_server.is_valid() && maya_tcp_server->is_listening()){
		maya_tcp_server->stop();
	}
}

void Maya::_server_process() {
	
	if(maya_tcp_server->is_connection_available()){
		Ref<StreamPeerTCP> client = maya_tcp_server->take_connection();
		client->set_no_delay(true);
		maya_tcp_clients.push_back(client);
		maya_tcp_commands.push_back(String(""));
	}
	Vector<int> pop_ids;
	int size = maya_tcp_clients.size();
	for (int i = 0; i < size; i++){
		Ref<StreamPeerTCP> client = maya_tcp_clients[i];
		if (client->is_connected_to_host()){
			int available_bytes = client->get_available_bytes();
			if(available_bytes > 0){
				maya_tcp_commands.write[i] += client->get_string(available_bytes);
			}
			client->put_8(1);
		}else{
			load(maya_tcp_commands[i]);
			if (echo)
				print_line(String("Got packet of size ") + Variant(maya_tcp_commands[i].size()) + String("bytes"));
			pop_ids.push_back(i);
		}
	}
	size = pop_ids.size();
	for (int i = 0; i < size; i++){
		int pop_id = pop_ids[i];
		maya_tcp_clients.remove(pop_id);
		maya_tcp_commands.remove(pop_id);
	}
}

void Maya::start_sender(){
	// sending = true;
	start_server();
	if(!sender_timer){
		sender_timer = red::create_node<Timer>(EditorInterface::get_singleton()->get_tree()->get_root(), "SenderTimer");
		sender_timer->connect("timeout", this, "_sender_process");
	}
	if (sender_timer->is_stopped())
		sender_timer->start(1.0 / sending_fps);
}

void Maya::stop_sender(){
	if((sender_timer) && !sender_timer->is_stopped())
		sender_timer->stop();
	if (maya_tcp_client->is_connected_to_host()){
		maya_tcp_client->disconnect_from_host();
	}
		
}

void Maya::_sender_process(){
	send_selected_nodes(false);
}

void Maya::start_receiver(){
	// receiving = true;
	start_server();
	if(!receiver_timer){
		receiver_timer = red::create_node<Timer>(EditorInterface::get_singleton()->get_tree()->get_root(), "ReceiverTimer");
		receiver_timer->connect("timeout", this, "_receiver_process");
	}
	if (receiver_timer->is_stopped())
		receiver_timer->start(1.0 / receiver_fps);
}

void Maya::stop_receiver(){
	stop_server();
	if((receiver_timer) && !receiver_timer->is_stopped())
		receiver_timer->stop();
}

void Maya::_receiver_process(){
	python("redmaya.redot.godot.send_selected_nodes()");
}

// Misc
Dictionary Maya::get_scene(const Node *root){
	Dictionary scene;
	if (!root)
		return scene;
	scene["name"] = root->get_filename().get_file().get_basename();
	scene["path"] = root->get_filename().get_base_dir();
	scene["project_path"] = ProjectSettings::get_singleton()->get_resource_path();
	return scene;
}

Error Maya::save_json(const Array &p_data, const String &p_path){
	Error err;
	FileAccess *f = FileAccess::open(p_path, FileAccess::WRITE, &err);
	ERR_FAIL_COND_V_MSG(err!=OK, err, "Can't save")
	REDJSON *parser = memnew(REDJSON);
	f->store_string(parser->print(p_data, "    ", false, true));
	memdelete(parser);
	memdelete(f);
	return OK;
}

void Maya::python(const String &command) {
	send_text("python(\"" + command + "\");");
}

void Maya::mel(const String &command) {
	PoolByteArray ascii = red::to_ascii(command);
	int len = ascii.size();
	if (!maya_tcp_client->is_connected_to_host())
		ERR_FAIL_COND_MSG(maya_tcp_client->connect_to_host(IP_Address("127.0.0.1"), 6000) != OK, "Can't connect to Maya")
	if(echo)
		print_line(command);
	PoolByteArray::Read r = ascii.read();
	maya_tcp_client->put_data(&r[0], len);
}

void Maya::send_text(const String &command) {
	if (!maya_tcp_client->is_connected_to_host())
		ERR_FAIL_COND_MSG(maya_tcp_client->connect_to_host(IP_Address("127.0.0.1"), 6000) != OK, "Can't connect to Maya")
	if(echo)
		print_line(command);
	maya_tcp_client->put_string(command + "%end");
}

bool Maya::check_serialized(const Array &serialized) const{
	ERR_FAIL_COND_V_MSG(serialized.size() == 0, false, "No nodes to load")
	Dictionary scene_data = serialized[0];

	ERR_FAIL_COND_V_MSG(!scene_data.has("name"), false, "Scene name undefined")
	ERR_FAIL_COND_V_MSG(!scene_data.has("path"), false, "Scene path undefined")
	ERR_FAIL_COND_V_MSG(!scene_data.has("project_path"), false, "Project path undefined")
	return true;
}

void Maya::set_echo(bool p_echo){
	echo = p_echo;
}

bool Maya::get_echo(){
	return echo;

}
// Core
void Maya::_bind_methods() {
	// Send
	ClassDB::bind_method(D_METHOD("send_node_properties", "nodes", "properties", "use_file"), &Maya::send_node_properties);
	ClassDB::bind_method(D_METHOD("send_nodes_properties", "nodes", "properties", "use_file"), &Maya::send_nodes_properties);
	ClassDB::bind_method(D_METHOD("send_node", "use_file"), &Maya::send_node);
	ClassDB::bind_method(D_METHOD("send_selected_nodes", "use_file"), &Maya::send_selected_nodes);
	ClassDB::bind_method(D_METHOD("send_nodes", "use_file"), &Maya::send_nodes);
	ClassDB::bind_method(D_METHOD("send_scene", "use_file"), &Maya::send_scene);

	ClassDB::bind_method(D_METHOD("load_file", "source"), &Maya::load_file);
	ClassDB::bind_method(D_METHOD("load", "json"), &Maya::load);

	ClassDB::bind_method(D_METHOD("set_echo", "echo"), &Maya::set_echo);
	ClassDB::bind_method(D_METHOD("get_echo"), &Maya::get_echo);
	ClassDB::bind_method(D_METHOD("python", "command"), &Maya::python);
	ClassDB::bind_method(D_METHOD("mel", "command"), &Maya::mel);
	ClassDB::bind_method(D_METHOD("send_text", "text"), &Maya::send_text);

	ClassDB::bind_method(D_METHOD("start_server"), &Maya::start_server);
	ClassDB::bind_method(D_METHOD("stop_server"), &Maya::stop_server);
	ClassDB::bind_method(D_METHOD("_server_process"), &Maya::_server_process);
	ClassDB::bind_method(D_METHOD("start_sender"), &Maya::start_sender);
	ClassDB::bind_method(D_METHOD("stop_sender"), &Maya::stop_sender);
	ClassDB::bind_method(D_METHOD("_sender_process"), &Maya::_sender_process);
	ClassDB::bind_method(D_METHOD("start_receiver"), &Maya::start_receiver);
	ClassDB::bind_method(D_METHOD("stop_receiver"), &Maya::stop_receiver);
	ClassDB::bind_method(D_METHOD("_receiver_process"), &Maya::_receiver_process);
	
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "echo"), "set_echo", "get_echo");
}
Maya::Maya() {
	maya_port = 6001;

	echo = false;
	sending_fps = 60;
	receiver_fps = 3;

	server_timer = nullptr;
	sender_timer = nullptr;
	receiver_timer = nullptr;
	maya_tcp_client.instance();
	maya_tcp_server.instance();
}

Maya::~Maya() {
	stop_server();
	stop_sender();
	stop_receiver();
	if(server_timer != nullptr){
		server_timer->disconnect("timeout", this, "_server_process");
		server_timer->queue_delete();
	}
	if(sender_timer != nullptr){
		sender_timer->disconnect("timeout", this, "_sender_process");
		sender_timer->queue_delete();
	}
	if(receiver_timer != nullptr){
		receiver_timer->disconnect("timeout", this, "_receiver_process");
		receiver_timer->queue_delete();
	}
}