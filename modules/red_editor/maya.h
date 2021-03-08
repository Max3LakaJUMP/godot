#ifndef RED_EDITOR_ENGINE_H
#define RED_EDITOR_ENGINE_H

#include "scene/main/timer.h"
#include "core/io/stream_peer_tcp.h"
#include "core/io/tcp_server.h"
#include "scene/resources/packed_scene.h"

#define MAX_POCKET_SIZE 262144

class Timer;
class Tween;
class AnimationPlayer;

class Maya : public Reference{
	GDCLASS(Maya, Reference);

	Ref<StreamPeerTCP> maya_tcp_client;
	Ref<TCP_Server> maya_tcp_server;
	Vector<Ref<StreamPeerTCP> > maya_tcp_clients;
	Vector<String> maya_tcp_commands;
	Vector<String> ignore_nodes_to_send;

	Timer *server_timer;
	Timer *sender_timer;
	Tween *receiver_tween;
	
	String maya_ip;
	int server_port;
	int maya_port;
	float sending_fps;
	float receiver_fps;
	bool echo;

	bool maya_tcp_server_is_listening;

	char current[MAX_POCKET_SIZE];
protected:
	static void _bind_methods();

public:
	// Core
	Variant property_to_dict(Node *node, const String &property_name);
	void animation_player_to_dict(AnimationPlayer *player, Dictionary &properties);
	void node_to_dict(Node *node, Node *root, Array &nodes, bool child=true, const Array &properties=Array());
	void dict_to_property(Node *node, const String &property_name, const Variant &value, bool force=false);
	void dict_to_animation_player(AnimationPlayer *player, const Dictionary &serialized);
	Node *dict_to_node(const Dictionary &serialized, Node *root);
	// Send
	void _send_node(Node *node, bool full_scene=false, bool use_file=false, const Array &properties=Array());
	void _send_nodes(const Array &nodes, bool full_scene, bool use_file, const Array &properties=Array());
	void send_node_properties(Node *node, const Array &properties=Array(), bool use_file=false);
	void send_nodes_properties(const Array &nodes, const Array &properties, bool use_file);
	void send_node(Node *node, bool use_file=false);
	void send_selected_nodes(bool use_file=false);
	void send_nodes(const Array &nodes, bool use_file=false);
	void send_scene(Node *node, bool use_file=true);
	// Load
	Error load_file(const String &p_source_filea);
	Error load(const String &json_text);
	// Servers
	void start_server();
	void stop_server();
	void _server_process();
	void start_sender();
	void stop_sender();
	void _sender_process();

	// Misc
	Dictionary get_scene(const Node *root);
	Error save_json(const Array &p_data, const String &p_path);
	void python(const String &command);
	void mel(const String &command);
	void _send_text(const String &command);
	void send_text(const String &command);
	bool check_serialized(const Array &serialized) const;
	void set_echo(bool p_echo=true);
	bool get_echo();
	void set_server_port(int p_server_port);
	int get_server_port();
	void set_maya_port(int p_maya_port);
	int get_maya_port();
	Maya();
	~Maya();
};
#endif // RED_EDITOR_ENGINE_H
