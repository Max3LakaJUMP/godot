#include "red_tools_editor_plugin.h"
#include "editor/editor_node.h"
#include "editor/editor_data.h"
#include "core/io/json.h"
#include "core/bind/core_bind.h"
#include "modules/red/red_engine.h"
#include "modules/red/root_bone_2d.h"
#include "modules/red/red_json.h"
#include "modules/red_editor/red_render_data.h"
#include "mediapipe.h"
#include "modules/red/red_deform.h"

REDToolsEditorPlugin::REDToolsEditorPlugin(EditorNode *p_node) {
	maya.instance();
	maya->set_server_port(6000);
	maya->set_maya_port(6001);
	red_menu = memnew(MenuButton);
	red_menu->set_text("RED");
	PopupMenu *popup = red_menu->get_popup();
	popup->add_check_shortcut(ED_SHORTCUT("maya/server", TTR("Maya server")));
	popup->add_check_shortcut(ED_SHORTCUT("maya/sender", TTR("Maya sender")));
	popup->add_item("Send scene");
	popup->add_item("Render normals");
	popup->add_item("Mediapipe");
	popup->add_item("Attach transform");
	popup->connect("id_pressed", this, "red_clicked");
	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, red_menu);
}

void REDToolsEditorPlugin::red_clicked(const int id) {
	switch (id){
	case 0:{
		PopupMenu *popup = red_menu->get_popup();
		int idx = popup->get_item_index(0);
		bool current = !popup->is_item_checked(idx);
		popup->set_item_checked(idx, current);
		if(current)
			maya->start_server();
		else
			maya->stop_server();
	} break;
	case 1:{
		PopupMenu *popup = red_menu->get_popup();
		int idx = popup->get_item_index(1);
		bool current = !popup->is_item_checked(idx);
		popup->set_item_checked(idx, current);
		if(current)
			maya->start_sender();
		else
			maya->stop_sender();
	} break;
	case 2:
		to_maya();
		break;
	case 3:
		selection_to_normals();
		break;
	case 4:
		create_mediapipe();
		break;
	case 5:
		attach_transform();
		break;
	default:
		break;
	}
}

void REDToolsEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("red_clicked", "id"), &REDToolsEditorPlugin::red_clicked);
	ClassDB::bind_method(D_METHOD("to_maya"), &REDToolsEditorPlugin::to_maya);
	ClassDB::bind_method(D_METHOD("selection_to_normals"), &REDToolsEditorPlugin::selection_to_normals);
	ClassDB::bind_method(D_METHOD("create_mediapipe"), &REDToolsEditorPlugin::selection_to_normals);
	ClassDB::bind_method(D_METHOD("attach_transform"), &REDToolsEditorPlugin::attach_transform);
}

void REDToolsEditorPlugin::to_maya() {
	Node *root = get_tree()->get_edited_scene_root();
	ERR_FAIL_COND_MSG(!root, "No root node")
	maya->send_scene(root);
}

Vector<int> REDToolsEditorPlugin::triangulate(const PoolVector2Array &points, const Array &polygons){
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

void REDToolsEditorPlugin::selection_to_normals(){
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

void REDToolsEditorPlugin::create_mediapipe() {
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
		Node2D *new_parent = red::create_node<Node2D>(parent, "head");
		parent->move_child(new_parent, apply_polygon->get_position_in_parent() + 1);
		new_parent->set_position(apply_polygon->get_position());
		Size2 polygon_size = red::get_full_size(apply_polygon->get_polygon(), apply_polygon->get_uv());
		Mediapipe *mediapipe = red::create_node<Mediapipe>(new_parent, "mediapipe");
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

void REDToolsEditorPlugin::attach_transform() {
	RootBone2D *apply_transform = nullptr;
	REDDeform *apply_deform = nullptr;
	{
		List<Node *> selection = EditorInterface::get_singleton()->get_selection()->get_selected_node_list();
		for (List<Node *>::Element *E = selection.front(); E; E = E->next()) {
			if (!apply_transform)
				apply_transform = Object::cast_to<RootBone2D>(E->get());
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
			apply_polygon->set_root_bone_path(apply_polygon->get_path_to(apply_transform));
		if(apply_deform)
			apply_polygon->set_deform_path(apply_polygon->get_path_to(apply_deform));
	}
}