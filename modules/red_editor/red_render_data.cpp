#include "red_render_data.h"

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
#include "facegen/facegen.h"
#include "mediapipe.h"

#include "scene/main/viewport.h"
#include "scene/gui/viewport_container.h"
#include "modules/red/red_engine.h"
#include "modules/red/red_parallax_folder.h"
#include "modules/red/red_target.h"
#include "scene/gui/color_rect.h"
#include "scene/2d/back_buffer_copy.h"

void REDRenderData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_resolution", "resolution"), &REDRenderData::set_resolution);
    ClassDB::bind_method(D_METHOD("get_resolution"), &REDRenderData::get_resolution);

    ClassDB::bind_method(D_METHOD("set_render_path", "render_path"), &REDRenderData::set_render_path);
    ClassDB::bind_method(D_METHOD("get_render_path"), &REDRenderData::get_render_path);
    
	ClassDB::bind_method(D_METHOD("set_apply_normal", "apply_normal"), &REDRenderData::set_apply_normal);
    ClassDB::bind_method(D_METHOD("get_apply_normal"), &REDRenderData::get_apply_normal);
    ClassDB::bind_method(D_METHOD("set_delete_after_render", "delete_after_render"), &REDRenderData::set_delete_after_render);
    ClassDB::bind_method(D_METHOD("get_delete_after_render"), &REDRenderData::get_delete_after_render);
    ClassDB::bind_method(D_METHOD("set_clean_viewport_before_render", "clean_viewport_before_render"), &REDRenderData::set_clean_viewport_before_render);
    ClassDB::bind_method(D_METHOD("get_clean_viewport_before_render"), &REDRenderData::get_clean_viewport_before_render);

	ClassDB::bind_method(D_METHOD("set_render_target", "render_target"), &REDRenderData::set_render_target);
    ClassDB::bind_method(D_METHOD("get_render_target"), &REDRenderData::get_render_target);
	ClassDB::bind_method(D_METHOD("set_back_material", "back_material"), &REDRenderData::set_back_material);
    ClassDB::bind_method(D_METHOD("get_back_material"), &REDRenderData::get_back_material);
	ClassDB::bind_method(D_METHOD("set_materials", "materials"), &REDRenderData::set_materials);
    ClassDB::bind_method(D_METHOD("get_materials"), &REDRenderData::get_materials);
	ClassDB::bind_method(D_METHOD("_render0"), &REDRenderData::_render0);
	ClassDB::bind_method(D_METHOD("_render"), &REDRenderData::_render);
	ClassDB::bind_method(D_METHOD("_normal_to_polygon"), &REDRenderData::_normal_to_polygon);

	ADD_SIGNAL(MethodInfo("rendered"));
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "resolution"), "set_resolution", "get_resolution");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "render_path"), "set_render_path", "get_render_path");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "render_target", PROPERTY_HINT_RESOURCE_TYPE, "ViewportTexture"), "set_render_target", "get_render_target");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "back_material", PROPERTY_HINT_RESOURCE_TYPE, "ShaderMaterial"), "set_back_material", "get_back_material");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "materials", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_materials", "get_materials");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "apply_normal"), "set_apply_normal", "get_apply_normal");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "delete_after_render"), "set_delete_after_render", "get_delete_after_render");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clean_viewport_before_render"), "set_clean_viewport_before_render", "get_clean_viewport_before_render");
}

void REDRenderData::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
            //_init();
		} break;
	}
}

void REDRenderData::set_resolution(const Size2 &p_resolution){
	resolution = p_resolution;
	if (viewport){
		viewport->set_size(p_resolution);
		update();
	}
}

Size2 REDRenderData::get_resolution() const{
	return resolution;
}

void REDRenderData::set_render_path(const String &p_render_path){
	render_path = p_render_path;
}

String REDRenderData::get_render_path() const{
	return render_path;
}

void REDRenderData::set_delete_after_render(bool p_delete_after_render){
	_delete_after_render = p_delete_after_render;
}

bool REDRenderData::get_delete_after_render() const{
	return _delete_after_render;
}

void REDRenderData::set_clean_viewport_before_render(bool p_clean_viewport_before_render){
	_clean_viewport_before_render = p_clean_viewport_before_render;
}

bool REDRenderData::get_clean_viewport_before_render() const{
	return _clean_viewport_before_render;
}

void REDRenderData::set_apply_normal(bool p_apply_normal){
	apply_normal = p_apply_normal;
}

bool REDRenderData::get_apply_normal() const{
	return apply_normal;
}

void REDRenderData::set_use_background(bool p_use_background){
	use_background = p_use_background;
}

bool REDRenderData::get_use_background() const{
	return use_background;
}

void REDRenderData::set_render_target(const Ref<ViewportTexture> p_render_target){
	render_target = p_render_target;
}

Ref<ViewportTexture> REDRenderData::get_render_target() const{
	return render_target;
}
void REDRenderData::set_back_material(const Ref<ShaderMaterial> p_material){
	back_material = p_material;
}
Ref<ShaderMaterial> REDRenderData::get_back_material() const{
	return back_material;
}
void REDRenderData::set_materials(const Array &p_materials){
	materials = p_materials;
}

Array REDRenderData::get_materials() const{
	return materials;
}

void REDRenderData::render(){
	render(resolution);
}

void REDRenderData::render(const Size2 &p_resolution_override){
	// Init viewport
	set_visible(true);
	if (!viewport){
		viewport = red::create_node<Viewport>(this, "generator");
	}
	if(_clean_viewport_before_render){
		for (int i = viewport->get_child_count()-1; i > -1; i--){
			Node *child = viewport->get_child(i);
			if(child && !child->is_queued_for_deletion()){
				child->queue_delete();
				viewport->remove_child(child);
			}
		}
	}
	viewport->set_update_mode(Viewport::UpdateMode::UPDATE_ALWAYS);
	viewport->set_usage(Viewport::USAGE_2D);
	viewport->set_use_own_world(true);
	viewport->set_vflip(true);
	viewport->set_size(p_resolution_override);
	viewport->set_transparent_background(true);
	camera = red::create_node<Camera2D>(viewport, "camera");
	camera->make_current();
	camera->set_anchor_mode(Camera2D::ANCHOR_MODE_FIXED_TOP_LEFT);
	camera->set_visible(false);

	// Init polygons
	PoolVector<Vector2> polygon;
	PoolVector<Vector2> uv;
	polygon.append(Vector2(0,0));
	polygon.append(Vector2(p_resolution_override.x, 0));
	polygon.append(p_resolution_override);
	polygon.append(Vector2(0, p_resolution_override.y));
	uv.append(Vector2(0, 0));
	uv.append(Vector2(1.0, 0));
	uv.append(Vector2(1.0, 1.0));
	uv.append(Vector2(0.0, 1.0));
	for (int i = -1; i < materials.size(); i++)
	{
		if ((i > 0)){
			BackBufferCopy *bb = red::create_node<BackBufferCopy>(viewport, "bb");
			bb->set_copy_mode(BackBufferCopy::COPY_MODE_VIEWPORT);
		}
		Polygon2D *pass_mesh;
		if ((i == -1 && use_background) || i > -1){
			pass_mesh = red::create_node<Polygon2D>(viewport, "pass_mesh", nullptr, true);
			pass_mesh->set_polygon(polygon);
			pass_mesh->set_uv(uv);
			if (i == -1){
				viewport->move_child(pass_mesh, 0);
				if(back_material.is_valid())
					pass_mesh->set_material(back_material);
			}else{
				pass_mesh->set_material(materials[i]);
			}
			Ref<Texture> texture = ResourceLoader::load("res://redot/textures/default/blue.png", "Texture");
			if (texture.is_valid())
				pass_mesh->set_texture(texture);
		}
		if (i == -1){
			for (int j = 0; j < get_child_count(); j++){
				Polygon2D *child = Object::cast_to<Polygon2D>(get_child(j));
				if(child){
					remove_child(child);
					viewport->add_child(child);
					child->set_owner(viewport->get_owner());
					j--;
				}
			}
		}
	}
	if(!_clean_viewport_before_render){
		back_material.unref();
		materials.resize(0);
		set_use_background(false);
	}

	// Render
	_need_render = true;
	update();
	VisualServer *server = VisualServer::get_singleton();
	server->draw();
	server->connect("frame_post_draw", this, "_render0");
}

void REDRenderData::_render0(){
	if(_need_render){
		_need_render = false;
		VisualServer *server = VisualServer::get_singleton();
		server->disconnect("frame_post_draw", this, "_render0");
		if(!is_queued_for_deletion()){
			update();
			server->draw();
			server->connect("frame_post_draw", this, "_render");
		}
	}
}

void REDRenderData::_render(){
	VisualServer::get_singleton()->disconnect("frame_post_draw", this, "_render");
	if(!is_queued_for_deletion()){
		if(viewport){
			if(render_target.is_valid())
				render_target->set_viewport_path_in_scene(viewport->get_owner()->get_path_to(viewport));
			Ref<Image> normal_texture = viewport->get_texture()->get_data();
			if(render_path != "ignore" && render_path != ""){
				normal_texture->save_png(render_path);
				if (ResourceLoader::import){
					ResourceLoader::import(render_path);
				}
				if(apply_normal){
					_normal_to_polygon();
				}
			}
		}
		emit_signal("rendered");
		if (_delete_after_render)
			queue_delete();
		else{
			set_visible(false);
		}
	}
}

void REDRenderData::_normal_to_polygon(){
	Polygon2D *p = Object::cast_to<Polygon2D>(get_parent());
	ERR_FAIL_COND_MSG(!p, "Object is not a polygon")
	Ref<Texture> normal = ResourceLoader::load(render_path, "Texture");
	ERR_FAIL_COND_MSG(normal.is_null(), "Normal is not rendered")
	p->set_normalmap(normal);
	p->_change_notify("normalmap");
}

REDRenderData::REDRenderData(){
	viewport = nullptr;
	camera = nullptr;

	render_path = "";
	resolution = Size2(1024, 1024);
	
	apply_normal = false;
	use_background = false;
	_need_render = false;
	_delete_after_render = false;
	_clean_viewport_before_render = false;
}