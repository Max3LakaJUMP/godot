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
#include "facegen/facegen.h"
#include "mediapipe.h"

// GET_GRAPH get_graph = NULL;
// HMODULE facegen = NULL;

GET_GRAPH get_graph = NULL;
HMODULE Mediapipe::facegen_dll = NULL;

void Mediapipe::load_dll(){
	if (Mediapipe::facegen_dll == NULL){
		HMODULE loaded = LoadLibrary("facegen.dll");
		ERR_FAIL_COND_MSG(loaded == NULL, "facegen.dll load error. Mediapipe node will cause errors.");
		Mediapipe::facegen_dll = loaded;
	}
}

void Mediapipe::free_dll(){
	if (Mediapipe::facegen_dll != NULL){
		FreeLibrary(Mediapipe::facegen_dll);
		Mediapipe::facegen_dll = NULL;
	}
};

bool Mediapipe::_init(){
	bool graph_created = graph != nullptr;
	if (graph_created){
		return graph_created;
	}
	if (!need_init){
		return false;
	}
	need_init = false;
	if (Mediapipe::facegen_dll == NULL){
		ERR_FAIL_COND_V_MSG(Mediapipe::facegen_dll == NULL, false, "facegen.dll load error");
	}
	if (get_graph == NULL){
		get_graph = (GET_GRAPH)GetProcAddress(Mediapipe::facegen_dll, "create_graph");
		ERR_FAIL_COND_V_MSG(get_graph == NULL, false, "create_graph() function load error");
	}
	graph = get_graph();
	return graph != nullptr;
}

bool Mediapipe::_texture_init(){
	if(!calculate_face_texture()){
		return false;
	}
	bool render_target_is_valid = render_target.is_valid() && render_target->get_size().x > 0;
	if(!render_target_is_valid){
		return false;
	}
	if (canonical_face_landmarks.size() == 0 || in_render_target_dirty || reinit){
		if (!in_texture(render_target->get_data()))
			return false;
		if(in_render_target_dirty){
			in_render_target_dirty = false;
		}
		render_target_polygon_dirty = true;
		if(!calculate_canonical_iris_landmarks())
			return false;
		if(!calculate_face_landmarks())
			return false;
		if (display)
			out_display();
	}
	if(render_target_polygon_dirty && _force_reset_polygon){
		if (reset_polygons(true)){
			render_target_polygon_dirty = false;
			_force_reset_polygon = false;
		}
	}
	return true;
}

bool Mediapipe::_polygon_init(){
	if(need_polygon_init){
		need_polygon_init = !reset_polygons(false);
		return !need_polygon_init;
	}else{
		return true;
	}
}

bool Mediapipe::_draw(){
	if (!is_inside_tree())
		return false;
	if(!_texture_init())
		return false;
	_polygon_init();
	if(!calculate_parent())
		return false;
	return true;
}

void Mediapipe::send_name(){
	graph->set_window_name(String(get_name()).utf8().get_data());
}

void Mediapipe::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			_draw();
		} break;
		case NOTIFICATION_READY: {
			_texture_init();
		} break;
		case NOTIFICATION_ENTER_TREE: {
			need_init = true;
			connect("renamed", this, "send_name");
			if (_init()){
				send_name();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			disconnect("renamed", this, "send_name");
		} break;
	}
}

void Mediapipe::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_reinit", "reinit"), &Mediapipe::set_reinit);
    ClassDB::bind_method(D_METHOD("get_reinit"), &Mediapipe::get_reinit);
	ClassDB::bind_method(D_METHOD("set_display", "display"), &Mediapipe::set_display);
    ClassDB::bind_method(D_METHOD("get_display"), &Mediapipe::get_display);
	ClassDB::bind_method(D_METHOD("set_front_view", "front_view"), &Mediapipe::set_front_view);
    ClassDB::bind_method(D_METHOD("get_front_view"), &Mediapipe::get_front_view);
	ClassDB::bind_method(D_METHOD("set_edit_uv", "edit_uv"), &Mediapipe::set_edit_uv);
    ClassDB::bind_method(D_METHOD("get_edit_uv"), &Mediapipe::get_edit_uv);
	ClassDB::bind_method(D_METHOD("set_graph_path", "graph_path"), &Mediapipe::set_graph_path);
    ClassDB::bind_method(D_METHOD("get_graph_path"), &Mediapipe::get_graph_path);
	ClassDB::bind_method(D_METHOD("set_eye_size", "eye_size"), &Mediapipe::set_eye_size);
    ClassDB::bind_method(D_METHOD("get_eye_size"), &Mediapipe::get_eye_size);
	ClassDB::bind_method(D_METHOD("set_eye_offset", "eye_offset"), &Mediapipe::set_eye_offset);
    ClassDB::bind_method(D_METHOD("get_eye_offset"), &Mediapipe::get_eye_offset);
	ClassDB::bind_method(D_METHOD("set_anchor", "eye_anchor"), &Mediapipe::set_anchor);
    ClassDB::bind_method(D_METHOD("get_anchor"), &Mediapipe::get_anchor);

	ClassDB::bind_method(D_METHOD("set_face_path", "path"), &Mediapipe::set_face_path);
    ClassDB::bind_method(D_METHOD("get_face_path"), &Mediapipe::get_face_path);
	ClassDB::bind_method(D_METHOD("set_eye_l_path", "path"), &Mediapipe::set_eye_l_path);
    ClassDB::bind_method(D_METHOD("get_eye_l_path"), &Mediapipe::get_eye_l_path);
	ClassDB::bind_method(D_METHOD("set_eye_blick_l_path", "path"), &Mediapipe::set_eye_blick_l_path);
    ClassDB::bind_method(D_METHOD("get_eye_blick_l_path"), &Mediapipe::get_eye_blick_l_path);
	ClassDB::bind_method(D_METHOD("set_eye_r_path", "path"), &Mediapipe::set_eye_r_path);
    ClassDB::bind_method(D_METHOD("get_eye_r_path"), &Mediapipe::get_eye_r_path);
	ClassDB::bind_method(D_METHOD("set_eye_blick_r_path", "path"), &Mediapipe::set_eye_blick_r_path);
    ClassDB::bind_method(D_METHOD("get_eye_blick_r_path"), &Mediapipe::get_eye_blick_r_path);

	ClassDB::bind_method(D_METHOD("set_face_scene", "polygon"), &Mediapipe::set_face_scene);
	ClassDB::bind_method(D_METHOD("get_face_scene"), &Mediapipe::get_face_scene);
	ClassDB::bind_method(D_METHOD("set_polygon_size", "polygon_size"), &Mediapipe::set_polygon_size);
    ClassDB::bind_method(D_METHOD("get_polygon_size"), &Mediapipe::get_polygon_size);

	ClassDB::bind_method(D_METHOD("set_brows_polygon", "polygon"), &Mediapipe::set_brows_polygon);
	ClassDB::bind_method(D_METHOD("get_brows_polygon"), &Mediapipe::get_brows_polygon);

	ClassDB::bind_method(D_METHOD("set_eye_l_polygon", "polygon"), &Mediapipe::set_eye_l_polygon);
	ClassDB::bind_method(D_METHOD("get_eye_l_polygon"), &Mediapipe::get_eye_l_polygon);
	ClassDB::bind_method(D_METHOD("set_eye_r_polygon", "polygon"), &Mediapipe::set_eye_r_polygon);
	ClassDB::bind_method(D_METHOD("get_eye_r_polygon"), &Mediapipe::get_eye_r_polygon);

	ClassDB::bind_method(D_METHOD("set_nose_polygon", "polygon"), &Mediapipe::set_nose_polygon);
	ClassDB::bind_method(D_METHOD("get_nose_polygon"), &Mediapipe::get_nose_polygon);
	ClassDB::bind_method(D_METHOD("set_nose_b_polygon", "polygon"), &Mediapipe::set_nose_b_polygon);
	ClassDB::bind_method(D_METHOD("get_nose_b_polygon"), &Mediapipe::get_nose_b_polygon);

	ClassDB::bind_method(D_METHOD("set_lips_polygon", "polygon"), &Mediapipe::set_lips_polygon);
	ClassDB::bind_method(D_METHOD("get_lips_polygon"), &Mediapipe::get_lips_polygon);
	ClassDB::bind_method(D_METHOD("set_jaw_shape_polygon", "polygon"), &Mediapipe::set_jaw_shape_polygon);
	ClassDB::bind_method(D_METHOD("get_jaw_shape_polygon"), &Mediapipe::get_jaw_shape_polygon);
	
	ClassDB::bind_method(D_METHOD("set_iris_l_polygon", "polygon"), &Mediapipe::set_iris_l_polygon);
	ClassDB::bind_method(D_METHOD("get_iris_l_polygon"), &Mediapipe::get_iris_l_polygon);
	ClassDB::bind_method(D_METHOD("set_iris_r_polygon", "polygon"), &Mediapipe::set_iris_r_polygon);
	ClassDB::bind_method(D_METHOD("get_iris_r_polygon"), &Mediapipe::get_iris_r_polygon);

	ClassDB::bind_method(D_METHOD("start"), &Mediapipe::start);
	ClassDB::bind_method(D_METHOD("start_capture"), &Mediapipe::start_capture);
	ClassDB::bind_method(D_METHOD("finish"), &Mediapipe::finish);

	ClassDB::bind_method(D_METHOD("in_camera"), &Mediapipe::in_camera);
	ClassDB::bind_method(D_METHOD("in_texture"), &Mediapipe::in_texture);
	ClassDB::bind_method(D_METHOD("out_display"), &Mediapipe::out_display);

	ClassDB::bind_method(D_METHOD("is_started"), &Mediapipe::is_started);
	ClassDB::bind_method(D_METHOD("is_key_pressed"), &Mediapipe::is_key_pressed);
	
	ClassDB::bind_method(D_METHOD("_init"), &Mediapipe::_init);
	ClassDB::bind_method(D_METHOD("send_name"), &Mediapipe::send_name);
	ClassDB::bind_method(D_METHOD("get_landmarks"), &Mediapipe::get_landmarks);
	ClassDB::bind_method(D_METHOD("get_pose_transform"), &Mediapipe::get_pose_transform);
	ClassDB::bind_method(D_METHOD("reset_polygon", "force"), &Mediapipe::reset_polygon);
	ClassDB::bind_method(D_METHOD("_render_target_rendered"), &Mediapipe::_render_target_rendered);
	
	ClassDB::bind_method(D_METHOD("set_canonical_face_landmarks", "landmarks"), &Mediapipe::set_canonical_face_landmarks);
	ClassDB::bind_method(D_METHOD("get_canonical_face_landmarks"), &Mediapipe::get_canonical_face_landmarks);
	ClassDB::bind_method(D_METHOD("set_canonical_iris_landmarks", "landmarks"), &Mediapipe::set_canonical_iris_landmarks);
	ClassDB::bind_method(D_METHOD("get_canonical_iris_landmarks"), &Mediapipe::get_canonical_iris_landmarks);
	ClassDB::bind_method(D_METHOD("set_canonical_pose_transform", "transform"), &Mediapipe::set_canonical_pose_transform);
	ClassDB::bind_method(D_METHOD("get_canonical_pose_transform"), &Mediapipe::get_canonical_pose_transform);
	
	ADD_GROUP("Mediapipe", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reinit"), "set_reinit", "get_reinit");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "display"), "set_display", "get_display");
	//ADD_PROPERTY(PropertyInfo(Variant::BOOL, "front_view"), "set_front_view", "get_front_view");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "graph_path"), "set_graph_path", "get_graph_path");
	ADD_GROUP("Face", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "face_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_face_scene", "get_face_scene");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "polygon_size"), "set_polygon_size", "get_polygon_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "anchor", PROPERTY_HINT_ENUM, "Center, Top left"), "set_anchor", "get_anchor");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_uv"), "set_edit_uv", "get_edit_uv");
	ADD_GROUP("Eye", "eye_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "eye_size"), "set_eye_size", "get_eye_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "eye_offset"), "set_eye_offset", "get_eye_offset");

	ADD_GROUP("Paths", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "face_path"), "set_face_path", "get_face_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "eye_l_path"), "set_eye_l_path", "get_eye_l_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "eye_blick_l_path"), "set_eye_blick_l_path", "get_eye_blick_l_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "eye_r_path"), "set_eye_r_path", "get_eye_r_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "eye_blick_r_path"), "set_eye_blick_r_path", "get_eye_blick_r_path");
	ADD_GROUP("Polygons", "");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "brows_polygon"), "set_brows_polygon", "get_brows_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "eye_l_polygon"), "set_eye_l_polygon", "get_eye_l_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "eye_r_polygon"), "set_eye_r_polygon", "get_eye_r_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "nose_polygon"), "set_nose_polygon", "get_nose_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "nose_b_polygon"), "set_nose_b_polygon", "get_nose_b_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "lips_polygon"), "set_lips_polygon", "get_lips_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "jaw_shape_polygon"), "set_jaw_shape_polygon", "get_jaw_shape_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "iris_l_polygon"), "set_iris_l_polygon", "get_iris_l_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "iris_r_polygon"), "set_iris_r_polygon", "get_iris_r_polygon");
	
	ADD_GROUP("Canonical", "canonical_");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR3_ARRAY, "canonical_face_landmarks", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_canonical_face_landmarks", "get_canonical_face_landmarks");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR3_ARRAY, "canonical_iris_landmarks", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_canonical_iris_landmarks", "get_canonical_iris_landmarks");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR3_ARRAY, "canonical_pose_transform", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_canonical_pose_transform", "get_canonical_pose_transform");
	BIND_ENUM_CONSTANT(MEDIAPIPE_ANCHOR_CENTER);
	BIND_ENUM_CONSTANT(MEDIAPIPE_ANCHOR_TOP_LEFT);
}

Mediapipe::Mediapipe() {
	graph = nullptr;
	eye_size = 1.5;
	eye_offset = Vector3(0.35f, 0.5f, 0.5f);

	anchor = MEDIAPIPE_ANCHOR_CENTER;
	reinit = false;
	display = true; // todo out of memory
	front_view = true; // todo broken
	edit_uv = true;
	edit_front = true;

	need_init = true;
	need_polygon_init = true;
	_force_reset_polygon = false;
	render_target_polygon_dirty = false;

	polygon_size = Size2(100, 100);
	in_render_target_dirty = true;
	in_texture_dirty = true;
	face_texture_dirty = true;
	canonical_face_landmarks_dirty = true;
	canonical_iris_landmarks_dirty = true;
	canonical_face_pose_dirty = true;
	face_landmarks_dirty = true;
	distance_matrix_dirty = true;
	weights_matrix_dirty = true;
	edited_landmarks_dirty = true;
	iris_landmarks_dirty = true;
}

Mediapipe::~Mediapipe() {
	if (graph != nullptr){
		graph->finish();
		delete graph;
		graph = nullptr;
	}
}