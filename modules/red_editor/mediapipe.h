/*************************************************************************/
/*  resource_importer_texture_atlas.h                                    */
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

#ifndef MEDIAPIPE
#define MEDIAPIPE

#include "core/math/transform.h"
#include "core/math/vector3.h"
#include "facegen/facegen.h"
#include "facegen/meta.h"
#include "modules/red/red_engine.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/polygon_2d.h"
#include "scene/resources/packed_scene.h"
#include "modules/red/red_transform.h"
#include "modules/red_editor/red_render_data.h"

#include "scene/resources/texture.h"
#include "core/math/geometry.h"

#include <windows.h>
#include <string>
#define DEBUG_MEDIAPIPE
#ifdef DEBUG_MEDIAPIPE
#define ERR_FAIL_COND_V_MSG_CUSTOM(cond, val, err) ERR_FAIL_COND_V_MSG(cond, val, err)
#else
#define ERR_FAIL_COND_V_MSG_CUSTOM(cond, val, err) \
	if (cond) return false;
#endif

struct Part{
	int bones_count;
	int vertex_step;
	int vtxs_count;
	int center_id;
	Vector<PoolVector<float> > weights;
	PoolVector<Vector2> polygon;
	int operator [] (int i) const {return get_ids()[i];}
	Part(int bones_count, int vertex_step, int vtxs_count) : bones_count(bones_count), vertex_step(vertex_step), vtxs_count(vtxs_count) {}
	Part(int bones_count, int vertex_step, int vtxs_count, int center_id) : bones_count(bones_count), vertex_step(vertex_step), vtxs_count(vtxs_count), center_id(center_id) {}
	virtual const int *get_ids() const = 0;
};

struct Brows : public Part{
	const int _ids[21] = {307, 214, 57, 117, 65, 123, 63, 167, 79, 135, 51, 397, 347, 427, 331, 385, 333, 379, 325, 470, 555};
	virtual const int *get_ids() const{return _ids;}
	Brows() : Part(6, 4, 21)  {} 
};

struct EyeL : public Part{
	const int _ids[16] = {431, 425, 334, 383, 330, 357, 322, 323, 433, 321, 428, 358, 329, 382, 335, 324};
	const int *get_ids() const{return _ids;}
    EyeL() : Part(4, 4, 16)  {} 
};

struct EyeR : public Part{
	const int _ids[16] = {171, 165, 66, 121, 62, 91, 54, 55, 173, 53, 168, 92, 61, 120, 67, 56};
	const int *get_ids() const{return _ids;}
    EyeR() : Part(4, 4, 16)  {} 
};

struct Nose : public Part{
	const int _ids[7] = {256, 305, 255, 50, 246, 241, 49};
	const int *get_ids() const{return _ids;}
    Nose() : Part(3, 3, 7)  {} 
};

struct NoseB : public Part{
	const int _ids[9] = {72, 184, 59, 129, 47, 391, 327, 442, 340};
	const int *get_ids() const{return _ids;}
    NoseB() : Part(3, 4, 9)  {} 
};

struct Lips : public Part{
	const int _ids[24] = {103, 104, 102, 156, 101, 130, 100, 392, 366, 416, 367, 369, 368, 372, 371, 417, 370, 393, 105, 131, 106, 157, 107, 108};
	const int *get_ids() const{return _ids;}
	Lips() : Part(4, 6, 24)  {} 
};

struct JawShape : public Part{
	const int _ids[15] = {7, 6, 5, 4, 3, 2, 1, 0, 45, 44, 43, 42, 41, 40, 39};
	const int *get_ids() const{return _ids;}
    JawShape() : Part(3, 7, 15, 0) {} 
};

struct Iris : public Part{
	const int _ids[3] = {176, 192, 188};
	const int *get_ids() const{return _ids;}
    Iris() : Part(3, 1, 3) {} 
};

struct RedFaceMesh { 
	const int iris_ids[49] = {147, 146, 145, 144, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 180, 179, 178, 177, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 163, 162, 161, 160, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 176};
	const int iris_smooth_ids[16] = {131, 130, 129, 128, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132};
	const int vertex_count = 596;
	int parts_size = 9;
	Brows brows = Brows();
	EyeL eye_l = EyeL();
	EyeR eye_r = EyeR();
	Nose nose = Nose();
	NoseB nose_b = NoseB();
	Lips lips = Lips();
	JawShape jaw_shape = JawShape();
	Iris iris_l = Iris();
	Iris iris_r = Iris();
	Part* operator [] (int i) {
		switch (i)
		{
			case 0: return (Part*)&brows;
			case 1: return (Part*)&eye_l;
			case 2: return (Part*)&eye_r;
			case 3: return (Part*)&nose;
			case 4: return (Part*)&nose_b;
			case 5: return (Part*)&lips;
			case 6: return (Part*)&jaw_shape;
			case 7: return (Part*)&iris_l;
			case 8: return (Part*)&iris_r;
			default: return (Part*)&jaw_shape;
		}
	}
};

class Mediapipe : public Node2D {
	GDCLASS(Mediapipe, Node2D);

	static HMODULE facegen_dll;

	Graph *graph;

	bool need_init;
	bool need_polygon_init;
	bool _force_reset_polygon;
	bool render_target_dirty;
	bool render_target_polygon_dirty;

	float eye_size;
	Vector3 eye_offset;

	PoolVector<Vector3> canonical_face_landmarks;
	PoolVector<float> canonical_face_pressence;
	PoolVector<Vector3> canonical_iris_landmarks;
	PoolVector<float> canonical_iris_pressence;
	Transform canonical_pose_transform;

	PoolVector<Vector3> face_landmarks;
	PoolVector<Vector3> face_landmarks_rotated;
	Vector3 face_offset;
	Vector3 face_scale;
	PoolVector<Vector3> iris_l;
	PoolVector<Vector3> iris_l_rotated;
	Vector3 iris_l_offset;
	Vector3 iris_l_scale;
	PoolVector<Vector3> iris_r;
	PoolVector<Vector3> iris_r_rotated;
	Vector3 iris_r_offset;
	Vector3 iris_r_scale;

	PoolVector<Vector3> edited_landmarks;
	PoolVector<Vector3> edited_landmarks_rotated;

	CanonicalMesh face_data;
	RedFaceMesh red_face_data;

 	Ref<PackedScene> face_scene;
	
	bool reinit;
	bool display;
	bool front_view;
	bool edit_uv;
	bool edit_front;

	Array distance_matrix;
	bool in_texture_dirty;
	bool face_texture_dirty;
	bool canonical_face_landmarks_dirty;
	bool canonical_iris_landmarks_dirty;
	bool canonical_face_pose_dirty;
	bool face_landmarks_dirty;
	bool distance_matrix_dirty;
	bool weights_matrix_dirty;
	bool edited_landmarks_dirty;
	bool iris_landmarks_dirty;
	Size2 polygon_size;
	
	NodePath face_path;
	NodePath eye_l_path;
	NodePath blick_l_path;
	NodePath eye_r_path;
	NodePath blick_r_path;

	REDRenderData *texture_renderer;
	Ref<ViewportTexture> render_target;


	bool _init();
	bool _polygon_init();
	bool _texture_init();
	bool _draw();
	void send_name();
protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	static void load_dll();
	static void free_dll();

	void set_eye_offset(const Vector3 &p_eye_offset) {
		eye_offset = p_eye_offset;
		update();
	}

	Vector3 get_eye_offset() {
		return eye_offset;
	}

	void set_eye_size(const float p_eye_size) {
		eye_size = p_eye_size;
		update();
	}

	float get_eye_size() {
		return eye_size;
	}

	void set_canonical_face_landmarks(const PoolVector<Vector3> &p_landmarks) {
		canonical_face_landmarks = p_landmarks;
	}

	PoolVector<Vector3> get_canonical_face_landmarks() {
		return canonical_face_landmarks;
	}

	void set_canonical_iris_landmarks(const PoolVector<Vector3> &p_landmarks) {
		canonical_iris_landmarks = p_landmarks;
	}

	PoolVector<Vector3> get_canonical_iris_landmarks() {
		return canonical_iris_landmarks;
	}

	void set_canonical_pose_transform(const Transform &p_transform) {
		canonical_pose_transform = p_transform;
	}

	Transform get_canonical_pose_transform() {
		return canonical_pose_transform;
	}

	void set_force_reset_polygon(bool p_force_reset_polygon) {
		_force_reset_polygon = p_force_reset_polygon;
		update();
	}

	void set_reinit(bool p_reinit) {
		reinit = p_reinit;
		if (p_reinit)
			update();
	}

	bool get_reinit() const{
		return reinit;
	}

	void set_display(bool p_display) {
		display = p_display;
		if (p_display)
			out_display();
	}

	bool get_display() const{
		return display;
	}

	void set_front_view(bool p_front_view) {
		front_view = p_front_view;
		face_landmarks_dirty = true;
		update();
	}

	bool get_front_view() const{
		return front_view;
	}

	void set_edit_uv(bool p_edit_uv) {
		edit_uv = p_edit_uv;
		update();
	}

	bool get_edit_uv() {
		return edit_uv;
	}

	void set_graph_path(String p_graph_path) {
		ERR_FAIL_COND_MSG(!_init(), "Graph initialization error");
		graph->set_graph_path(p_graph_path.utf8().get_data());
	}

	String get_graph_path() {
		ERR_FAIL_COND_V_MSG(!_init(), "None", "Graph initialization error");
		return String(graph->get_graph_path());
	}

	void set_face_scene(const Ref<PackedScene> &p_scene) {
		face_scene = p_scene;
		face_landmarks_dirty = true;
		iris_landmarks_dirty = true;
		update();
	}

	Ref<PackedScene> get_face_scene() const {
		return face_scene;
	}
	
	void set_polygon_size(const Size2 &p_size) {
		if (polygon_size == p_size)
			return;
		PoolVector<Vector3>::Write landmarks_w = canonical_face_landmarks.write();
		PoolVector<Vector3>::Read landmarks_r = canonical_face_landmarks.read();
		Vector3 scaler(p_size.x / polygon_size.x, p_size.y / polygon_size.y, (p_size.x + p_size.y) / (polygon_size.x + polygon_size.y));
		Vector2 scaler2d = Vector2(scaler.x, scaler.y);
		for (int i = 0; i < canonical_face_landmarks.size(); i++) {
			landmarks_w[i] = landmarks_r[i] * scaler;
		}
		for (int i = 0; i < red_face_data.parts_size; i++){
			PoolVector<Vector2> poly = _get_polygon(i);
			PoolVector<Vector2>::Write poly_w = poly.write();
			PoolVector<Vector2>::Read poly_r = poly.read();
			
			for (int j = 0; j < poly.size(); j++) {
				poly_w[j] = poly_r[j] * scaler2d;
			}
			_set_polygon(poly, i);
		}
		polygon_size = p_size;
		face_landmarks_dirty = true;
		update();
	}

	Size2 get_polygon_size() {
		return polygon_size;
	}

	// Polygon Nodepaths
	void set_face_path(const NodePath &p_path) {
		face_path = p_path;
		face_texture_dirty = true;
		update();
	}

	NodePath get_face_path() const {
		return face_path;
	}

	void set_eye_l_path(const NodePath &p_path) {
		eye_l_path = p_path;
		face_texture_dirty = true;
		update();
	}

	NodePath get_eye_l_path() const {
		return eye_l_path;
	}

	void set_eye_blick_l_path(const NodePath &p_path) {
		blick_l_path = p_path;
		update();
	}

	NodePath get_eye_blick_l_path() const {
		return blick_l_path;
	}

	void set_eye_r_path(const NodePath &p_path) {
		eye_r_path = p_path;
		face_texture_dirty = true;
		update();
	}

	NodePath get_eye_r_path() const {
		return eye_r_path;
	}

	void set_eye_blick_r_path(const NodePath &p_path) {
		blick_r_path = p_path;
		update();
	}

	NodePath get_eye_blick_r_path() const {
		return blick_r_path;
	}

	// set polygon
	PoolVector<Vector2> get_offsets(int part_id) {
		Part *p = red_face_data[part_id];
		PoolVector<Vector2> offsets;
		offsets.resize(p->bones_count);
		PoolVector<Vector2>::Write offsets_write = offsets.write();
		for (int i = 0; i < p->bones_count; i++){
			offsets_write[i] = Vector2();
		}
		if (p->polygon.size() < p->bones_count)
			return offsets;
		PoolVector<Vector3>::Read landmarks_read;
		PoolVector<Vector2>::Read polygon_read = p->polygon.read();
		if (part_id == red_face_data.parts_size - 2){
			if (!calculate_iris_landmarks())
				return offsets;
			if (canonical_iris_landmarks.size() < 10)
				return offsets;
			landmarks_read = canonical_iris_landmarks.read();
			offsets_write[0] = polygon_read[0] - Vector2(landmarks_read[5].x, landmarks_read[5].y);
			offsets_write[1] = polygon_read[1] - Vector2(landmarks_read[8].x, landmarks_read[8].y);
			offsets_write[2] = polygon_read[2] - Vector2(landmarks_read[9].x, landmarks_read[9].y);
		}else if (part_id == red_face_data.parts_size - 1){
			if (!calculate_iris_landmarks())
				return offsets;
			if (canonical_iris_landmarks.size() < 10)
				return offsets;
			landmarks_read = canonical_iris_landmarks.read();
			offsets_write[0] = polygon_read[0] - Vector2(landmarks_read[0].x, landmarks_read[0].y);
			offsets_write[1] = polygon_read[1] - Vector2(landmarks_read[1].x, landmarks_read[1].y);
			offsets_write[2] = polygon_read[2] - Vector2(landmarks_read[4].x, landmarks_read[4].y);
		}else{
			if (!calculate_face_landmarks())
				return offsets;
			if (face_landmarks_rotated.size() < red_face_data.vertex_count)
				return offsets;
			landmarks_read = face_landmarks_rotated.read();
			for (int i = 0; i < p->bones_count; i++){
				Vector3 landmark = landmarks_read[p->get_ids()[int(i * p->vertex_step)]];
				offsets_write[i] = polygon_read[i] - Vector2(landmark.x, landmark.y);
			}
		}
		return offsets;
	}

	PoolVector<Vector2> reset_polygon(int part_id) {
		Part *p = red_face_data[part_id];
		int bones_count = p->bones_count;
		int vertex_step = p->vertex_step;
		PoolVector<Vector2> polygon;
		PoolVector<Vector3>::Read landmarks_read;
		if (part_id == red_face_data.parts_size - 2){
			if (canonical_iris_landmarks.size() < bones_count)
				return polygon;
			polygon.resize(bones_count);
			PoolVector<Vector2>::Write polygon_write = polygon.write();
			landmarks_read = canonical_iris_landmarks.read();
			polygon_write[0] = Vector2(landmarks_read[5].x, landmarks_read[5].y);
			polygon_write[1] = Vector2(landmarks_read[8].x, landmarks_read[8].y);
			polygon_write[2] = Vector2(landmarks_read[9].x, landmarks_read[9].y);
		}else if (part_id == red_face_data.parts_size - 1){
			if (canonical_iris_landmarks.size() < bones_count)
				return polygon;
			polygon.resize(bones_count);
			PoolVector<Vector2>::Write polygon_write = polygon.write();
			landmarks_read = canonical_iris_landmarks.read();
			polygon_write[0] = Vector2(landmarks_read[0].x, landmarks_read[0].y);
			polygon_write[1] = Vector2(landmarks_read[1].x, landmarks_read[1].y);
			polygon_write[2] = Vector2(landmarks_read[4].x, landmarks_read[4].y);
		}else{
			if (face_landmarks_rotated.size() < red_face_data.vertex_count)
				return polygon;
			polygon.resize(bones_count);
			PoolVector<Vector2>::Write polygon_write = polygon.write();
			landmarks_read = face_landmarks_rotated.read();
			for (int i = 0; i < bones_count; i++){
				Vector3 landmark = landmarks_read[p->get_ids()[int(i * vertex_step)]];
				polygon_write[i] = Vector2(landmark.x, landmark.y);
			}
		}
		return polygon;
	}

	void _set_polygon(const PoolVector<Vector2> &p_polygon, int part_id = 0) {
		Part *p = red_face_data[part_id];
		p->polygon = p_polygon.size() == p->bones_count ? p_polygon : reset_polygon(part_id);
		edited_landmarks_dirty = true;
		update();
	}

	PoolVector<Vector2> _get_polygon(int part_id = 0) {
		return red_face_data[part_id]->polygon;
	}

	void set_brows_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 0);
	}

	PoolVector<Vector2> get_brows_polygon() {
		return red_face_data[0]->polygon;
	}

	void set_eye_l_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 1);
	}

	PoolVector<Vector2> get_eye_l_polygon() {
		return red_face_data[1]->polygon;
	}

	void set_eye_r_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 2);
	}

	PoolVector<Vector2> get_eye_r_polygon() {
		return red_face_data[2]->polygon;
	}

	void set_nose_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 3);
	}

	PoolVector<Vector2> get_nose_polygon() {
		return red_face_data[3]->polygon;
	}

	void set_nose_b_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 4);
	}

	PoolVector<Vector2> get_nose_b_polygon() {
		return red_face_data[4]->polygon;
	}

	void set_lips_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 5);
	}

	PoolVector<Vector2> get_lips_polygon() {
		return red_face_data[5]->polygon;
	}

	void set_jaw_shape_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 6);
	}

	PoolVector<Vector2> get_jaw_shape_polygon() {
		return red_face_data[6]->polygon;
	}

	void set_iris_l_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 7);
	}

	PoolVector<Vector2> get_iris_l_polygon() {
		return red_face_data[7]->polygon;
	}

	void set_iris_r_polygon(const PoolVector<Vector2> &p_polygon) {
		_set_polygon(p_polygon, 8);
	}

	PoolVector<Vector2> get_iris_r_polygon() {
		return red_face_data[8]->polygon;
	}

	// delete not needed
	Vector<int> get_sort_ids(const Array &source, const int *index_list, int bones_count = 8, int vertex_every = 2){
		Vector<int> bone_ids;
		Vector<float> min_distances;
		bone_ids.resize(bones_count);
		min_distances.resize(bones_count);
		for (int i = 0; i < bones_count; i++) {
			int bone_id = index_list[i * vertex_every];
			bone_ids.write[i] = i;
			min_distances.write[i] = source[bone_id];
		}
		int temp_i;
		float temp_d;
		for (int i = 0; i < bones_count - 1; i++) {
			for (int j = 0; j < bones_count - i - 1; j++) {
				if (min_distances[j] > min_distances[j + 1]) {
					temp_d = min_distances[j];
					min_distances.write[j] = min_distances[j + 1];
					min_distances.write[j + 1] = temp_d;

					temp_i = bone_ids[j];
					bone_ids.write[j] = bone_ids[j + 1];
					bone_ids.write[j + 1] = temp_i;
				}
			}
		}
		return bone_ids;
	}

	PoolVector<float> get_point_weights(const Array distances, const int *index_list, int vtxs_count = 14, int bones_count = 8, int vertex_every = 2, float max_distance = 100.0) {
		PoolVector<float> weights;
		float weights_sum = 0.0;
		float max_weight = 0.0;
		int index_prev = 0;
		int index_next = -1;
		{
			weights.resize(bones_count);
			PoolVector<float>::Write weights_w = weights.write();
			for (int i = 0; i < bones_count; i++)
				weights_w[i] = 0.0;
			float min_d = distances[index_list[0]];
			int min_i = 0;
			for (int i = 1; i < vtxs_count; i++) {
				float d = distances[index_list[i]];
				if (d < min_d){
					min_d = d;
					min_i = i;
				}
			}
			index_prev = int(min_i / vertex_every);
			if (min_i % vertex_every != 0){
				index_next = index_prev + 1;
				if (index_next >= bones_count - 0.1){
					index_next = index_prev;
					index_prev = 0;
				}
			}
			if (min_d >= max_distance)
				return weights;
			max_weight = 1.0 - min_d / max_distance;
		}
		{
			PoolVector<float>::Write weights_w = weights.write();
			for (int i = 0; i < 2; i++) {
				int sorted_i = index_prev;
				if (i == 1){
					if(index_next != -1)
						sorted_i = index_next;
					else
						break;
				}
				int bone_id = index_list[sorted_i * int(vertex_every)];
				float d = distances[bone_id];
				if (d >= max_distance)
					continue;
				
				float w = 1.0 - d / max_distance;
				weights_w[sorted_i] = w;
				weights_sum += w;
			}
		}
		{
			PoolVector<float>::Write weights_w = weights.write();
			PoolVector<float>::Read weights_r = weights.read();
			float weights_sum_current = 0.0;
			for (int i = 0; i < 2; i++) {
				int sorted_i = index_prev;
				if (i == 1){
					if(index_next != -1)
						sorted_i = index_next;
					else
						break;
				}
				int bone_id = index_list[sorted_i * int(vertex_every)];
				float w = max_weight * weights_r[sorted_i] / weights_sum;
				w = MIN(w, max_weight - weights_sum_current);
				weights_w[sorted_i] = w;
				weights_sum_current += w;
			}
		}
		return weights;
	}

	Vector3 get_scaler(Vector3 eye_l, Vector3 eye_r, Vector3 jaw, Vector3 eye_l_target, Vector3 eye_r_target, Vector3 jaw_target) {
		Vector3 scaler;
		scaler.x = eye_l_target.distance_to(eye_r_target) / eye_l.distance_to(eye_r);
		Vector3 eye_target = (eye_l_target + eye_r_target) * 0.5f;
		Vector3 eye = (eye_l + eye_r) * 0.5;
		scaler.y = eye_target.distance_to(jaw_target) / eye.distance_to(jaw);
		scaler.z = (scaler.x + scaler.y) * 0.5;
		return scaler;
		// return get_scaler(	Vector2(eye_l.x, eye_l.y), 
		// 					Vector2(eye_r.x, eye_r.y), 
		// 					Vector2(jaw.x, jaw.y), 
		// 					Vector2(eye_l_target.x, eye_l_target.y), 
		// 					Vector2(eye_r_target.x, eye_r_target.y),  
		// 					Vector2(jaw_target.x, jaw_target.y));
	}

	Vector3 get_scaler(Vector2 eye_l, Vector2 eye_r, Vector2 jaw, Vector2 eye_l_target, Vector2 eye_r_target, Vector2 jaw_target) {
		Vector3 scaler;
		scaler.x = eye_l_target.distance_to(eye_r_target) / eye_l.distance_to(eye_r);
		Vector2 eye_target = (eye_l_target + eye_r_target) * 0.5f;
		Vector2 eye = (eye_l + eye_r) * 0.5;
		scaler.y = eye_target.distance_to(jaw_target) / eye.distance_to(jaw);
		scaler.z = (scaler.x + scaler.y) * 0.5;
		return scaler;
	}

	Vector3 get_eye_target(const Vector3 &rotated_eye_target, const Vector3 &eye_position, float eye_radius){
		Vector3 rotated_eye_center(canonical_pose_transform.xform(eye_position) + face_offset);
		Vector3 out(rotated_eye_target);
		Vector3 eye_temp(out - rotated_eye_center);
		out.z = Math::sqrt(MAX(eye_radius * eye_radius - eye_temp.x * eye_temp.x - eye_temp.y * eye_temp.y, 0.0)) + rotated_eye_center.z;
		out = eye_position - canonical_pose_transform.xform_inv(out - face_offset) ;
		return out;
	}

	bool calculate_face_texture() {
		if(!face_texture_dirty)
			return true;
		if(face_path.is_empty())
			return false;
		Polygon2D *head_poly = Object::cast_to<Polygon2D>(get_node(face_path));
		if(!head_poly || head_poly->get_texture().is_null()){
			return false;
		}
		texture_renderer = red::create_node<REDRenderData>(this, "face_texture_generator");
		if(texture_renderer->is_connected("rendered", this, "_render_target_rendered"))
			texture_renderer->disconnect("rendered", this, "_render_target_rendered");
		face_texture_dirty = false;
		float aspect_ratio = polygon_size.y / polygon_size.x;
		Size2 viewport_size = head_poly->get_texture()->get_size();
		viewport_size.y *= aspect_ratio;

		Vector2 k = viewport_size / polygon_size;
		texture_renderer->set_clean_viewport_before_render(true);
		texture_renderer->set_resolution(viewport_size);
		if (render_target.is_null()){
			render_target.instance();
		}
		texture_renderer->set_render_target(render_target);
		//rd->set_delete_after_render(true);
		// Init polygons
		PoolVector<Vector2> uv;
		uv.append(Vector2(0, 0));
		uv.append(Vector2(1.0, 0));
		uv.append(Vector2(1.0, 1.0));
		uv.append(Vector2(0.0, 1.0));
		if (has_node(eye_l_path)){
			Polygon2D *eye_l_poly = Object::cast_to<Polygon2D>(get_node(eye_l_path));
			Size2 eye_l_size = red::get_full_size(eye_l_poly->get_polygon(), eye_l_poly->get_uv()) * k;
			Size2 eye_l_position = (eye_l_poly->get_position() - head_poly->get_position()) * k;
			PoolVector<Vector2> eye_l_polygon;
			eye_l_polygon.append(Vector2(0,0));
			eye_l_polygon.append(Vector2(eye_l_size.x, 0));
			eye_l_polygon.append(eye_l_size);
			eye_l_polygon.append(Vector2(0, eye_l_size.y));
			Polygon2D *eye_l_render_poly = red::create_node<Polygon2D>(texture_renderer, "eye_l");
			eye_l_render_poly->set_polygon(eye_l_polygon);
			eye_l_render_poly->set_uv(uv);
			eye_l_render_poly->set_texture(eye_l_poly->get_texture());
			eye_l_render_poly->set_position(eye_l_position);
		}
		if (has_node(eye_r_path)){
			Polygon2D *eye_r_poly = Object::cast_to<Polygon2D>(get_node(eye_r_path));
			Size2 eye_r_size = red::get_full_size(eye_r_poly->get_polygon(), eye_r_poly->get_uv()) * k;
			Size2 eye_r_position = (eye_r_poly->get_position() - head_poly->get_position()) * k;
			PoolVector<Vector2> eye_r_polygon;
			eye_r_polygon.append(Vector2(0,0));
			eye_r_polygon.append(Vector2(eye_r_size.x, 0));
			eye_r_polygon.append(eye_r_size);
			eye_r_polygon.append(Vector2(0, eye_r_size.y));
			Polygon2D *eye_r_render_poly = red::create_node<Polygon2D>(texture_renderer, "eye_r");
			eye_r_render_poly->set_polygon(eye_r_polygon);
			eye_r_render_poly->set_uv(uv);
			eye_r_render_poly->set_texture(eye_r_poly->get_texture());
			eye_r_render_poly->set_position(eye_r_position);
		}
		PoolVector<Vector2> head_polygon;
		head_polygon.append(Vector2(0,0));
		head_polygon.append(Vector2(viewport_size.x, 0));
		head_polygon.append(viewport_size);
		head_polygon.append(Vector2(0, viewport_size.y));
		Polygon2D *head_render_poly = red::create_node<Polygon2D>(texture_renderer, "head");
		head_render_poly->set_polygon(head_polygon);
		head_render_poly->set_uv(uv);
		head_render_poly->set_texture(head_poly->get_texture());

		
		texture_renderer->render();
		texture_renderer->connect("rendered", this, "_render_target_rendered", varray(), CONNECT_ONESHOT);
		
		canonical_face_landmarks_dirty = true;
		canonical_iris_landmarks_dirty = true;
		canonical_face_pose_dirty = true;
		return true;
	}

	void _render_target_rendered() {
		in_texture_dirty = true;
		render_target_dirty = true;
		update();
	}

	bool calculate_canonical_face_landmarks(const bool pressence_check = false) {
		if (!calculate_face_texture())
			return false;
		if (!canonical_face_landmarks_dirty)
			return true;
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		bool real_pressence_check = canonical_face_pressence.size() == 468 && canonical_face_landmarks.size() == 468 ? pressence_check : false;
		float *polygon_in;
		int polygon_count = 0;
		ERR_FAIL_COND_V_MSG_CUSTOM(!graph->out_landmarks(&polygon_in, &polygon_count), false, "Failed to get canonical_face_landmarks data");
		//ERR_FAIL_COND_V_MSG_CUSTOM(polygon_count == 0, false, "polygon data has 0 size");
		canonical_face_landmarks.resize(polygon_count);
		canonical_face_pressence.resize(polygon_count);

		PoolVector<Vector3>::Write landmarks_w = canonical_face_landmarks.write();
		PoolVector<float>::Write pressence_w = canonical_face_pressence.write();
		PoolVector<float>::Read pressence_r = canonical_face_pressence.read();

		int j = 0;
		for (int i = 0; i < polygon_count; i++) {
			if (real_pressence_check && pressence_r[j] >= polygon_in[j + 3])
				continue;
			float d = -polygon_in[j + 2];
			landmarks_w[i] = Vector3(polygon_in[j] * polygon_size.x, polygon_in[j + 1] * polygon_size.y, d * (polygon_size.x + polygon_size.y) * 0.5);
			pressence_w[i] = polygon_in[j + 3];
			j += 4;
		}
		delete polygon_in;
		canonical_face_landmarks_dirty = false;
		face_landmarks_dirty = true;
		return true;
	}

	bool calculate_canonical_iris_landmarks(const bool pressence_check = false) {
		if (!calculate_face_texture())
			return false;
		if (!canonical_iris_landmarks_dirty)
			return true;
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		bool real_pressence_check = canonical_iris_pressence.size() != 0 && canonical_iris_landmarks.size() != 0 ? pressence_check : false;
		float *polygon_in;
		int polygon_count = 0;
		ERR_FAIL_COND_V_MSG_CUSTOM(!graph->out_iris_landmarks(&polygon_in, &polygon_count), false, "Failed to get iris_landmarks data");
		//ERR_FAIL_COND_V_MSG_CUSTOM(polygon_count == 0, false, "polygon data has 0 size");
		canonical_iris_landmarks.resize(polygon_count);
		canonical_iris_pressence.resize(polygon_count);

		PoolVector<Vector3>::Write landmarks_w = canonical_iris_landmarks.write();
		PoolVector<float>::Write pressence_w = canonical_iris_pressence.write();
		PoolVector<float>::Read pressence_r = canonical_iris_pressence.read();

		int j = 0;
		for (int i = 0; i < polygon_count; i++) {
			if (real_pressence_check && pressence_r[j] >= polygon_in[j + 3])
				continue;
			float d = -polygon_in[j + 2];
			landmarks_w[i] = Vector3(polygon_in[j] * polygon_size.x, polygon_in[j + 1] * polygon_size.y, d * (polygon_size.x + polygon_size.y) * 0.5);
			pressence_w[i] = polygon_in[j + 3];
			j += 4;
		}
		delete polygon_in;
		canonical_iris_landmarks_dirty = false;
		return true;
	}

	bool calculate_canonical_pose_transform() {
		if (!calculate_face_texture())
			return false;
		if (!canonical_face_pose_dirty)
			return true;
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		float *polygon_in;
		int *polygons_in;
		float *transform_matrix;
		int polygon_count = 0;
		int polygons_count = 0;
		ERR_FAIL_COND_V_MSG_CUSTOM(!graph->out_polygon(&polygon_in, &polygons_in, &polygon_count, &polygons_count, &transform_matrix), false, "Failed to get pose matrix");
		//ERR_FAIL_COND_V_MSG_CUSTOM(polygon_count == 0 || polygons_count == 0, false, "polygon data has 0 size");
		canonical_pose_transform = Transform(	transform_matrix[0], transform_matrix[1], transform_matrix[2],
									transform_matrix[4], transform_matrix[5], transform_matrix[6],
									transform_matrix[8], transform_matrix[9], transform_matrix[10],
									transform_matrix[12], transform_matrix[13], transform_matrix[14]);

		delete polygon_in;
		delete polygons_in;
		delete transform_matrix;
		Vector3 rotation = canonical_pose_transform.basis.get_euler_xyz();
		canonical_pose_transform.basis.set_euler_xyz(Vector3(rotation.x, -rotation.y, rotation.z));
		canonical_pose_transform.origin = Vector3(0.0, 0.0, 0.0);
		canonical_face_pose_dirty = false;
		distance_matrix_dirty = true;
		return true;
	}

	bool calculate_face_landmarks() {
		if (!calculate_canonical_face_landmarks()){
			ERR_FAIL_V_MSG(false, "calculate_canonical_face_landmarks in calculate_face_landmarks")
			return false;
		}
		if (!calculate_canonical_pose_transform()){
			ERR_FAIL_V_MSG(false, "calculate_canonical_pose_transform in calculate_face_landmarks")
			return false;
		}
		if (!face_landmarks_dirty){
			return true;
		}
		if (face_scene.is_null()){
			face_scene = ResourceLoader::load("res://redot/polygons/faces/default_face.tscn");
			if (face_scene.is_null())
				return false;
		}
		if(face_path.is_empty())
			return false;
		Polygon2D *polygon_object = Object::cast_to<Polygon2D>(get_node(face_path));
		if (!polygon_object){
			return false;
		}
		if (polygon_object->get_texture().is_null()){
			return false;
		}
		if (canonical_face_landmarks.size() < face_data.vertex_count){
			return false;
		}
		Node *face_scene_instance = face_scene->instance();
		if (!face_scene_instance){
			return false;
		}
		Polygon2D *face_polygon = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("face")));
		if (!face_polygon){
			face_scene_instance->queue_delete();
			return false;
		}
		PoolVector<Vector2> p = face_polygon->get_polygon();
		PoolVector<Color> vc = face_polygon->get_vertex_colors();
		float depth = face_polygon->get_depth_size();
		float depth_offset = face_polygon->get_depth_offset();
		float min_depth = 1000000.0;
		float max_depth = -1000000.0;
		float real_depth = 0.0;
		PoolVector<Vector3> p_temp;
		p_temp.resize(p.size());
		PoolVector<Vector3> p_temp_rotated;
		p_temp_rotated.resize(p.size());
		{
			PoolVector<Vector3>::Write p_temp_write = p_temp.write();
			PoolVector<Vector2>::Read p_read = p.read();
			PoolVector<Color>::Read vc_read = vc.read();
			for (int i = 0; i < p.size(); i++) {
				float depth_val = (vc_read[i].a - depth_offset) * depth;
				p_temp_write[i] = Vector3(p_read[i].x, p_read[i].y, depth_val);
				//p_temp_write[i] = canonical_pose_transform.xform(Vector3(p_read[i].x, p_read[i].y, depth_val));
			}
		}
		float face_height = 0;
		float face_height_target = 0;
		PoolVector<Vector3>::Read lr = canonical_face_landmarks.read();
		{
			PoolVector<Vector3>::Read p_temp_read = p_temp.read();
			Vector3 eye_l_land = canonical_pose_transform.xform_inv(lr[face_data.eye_l[8]]);
			Vector3 eye_r_land = canonical_pose_transform.xform_inv(lr[face_data.eye_r[8]]);
			Vector3 jaw_land = canonical_pose_transform.xform_inv(lr[face_data.jaw_center_id]);
			face_scale = get_scaler(p_temp_read[red_face_data.eye_l[8]], 
									p_temp_read[red_face_data.eye_r[8]], 
									p_temp_read[red_face_data.jaw_shape.center_id], 
									eye_l_land, eye_r_land, jaw_land);
		}
		{
			PoolVector<Vector3>::Write p_temp_write = p_temp.write();
			PoolVector<Vector2>::Read p_read = p.read();
			PoolVector<Color>::Read vc_read = vc.read();
			for (int i = 0; i < p.size(); i++) {
				float depth_val = (vc_read[i].a - depth_offset) * depth;
				p_temp_write[i] = canonical_pose_transform.xform(Vector3(p_read[i].x, p_read[i].y, depth_val) * face_scale);
			}
		}
		{
			PoolVector<Vector3>::Read p_temp_read = p_temp.read();
			Vector3 eye_pos_target = (lr[face_data.eye_l[8]] + lr[face_data.eye_r[8]]) * 0.5f;
			Vector3 eye_pos = (p_temp_read[red_face_data.eye_l[8]] + p_temp_read[red_face_data.eye_r[8]]) * 0.5;
			face_offset = eye_pos_target - eye_pos;
		}
		// Apply
		{
			PoolVector<Vector3>::Write p_temp_write = p_temp.write();
			PoolVector<Vector3>::Write p_temp_rotated_write = p_temp_rotated.write();
			PoolVector<Vector3>::Read p_temp_read = p_temp.read();
			PoolVector<Vector2>::Read p_read = p.read();
			PoolVector<Color>::Read vc_read = vc.read();
			for (int i = 0; i < p.size(); i++) {
				float depth_val = (vc_read[i].a - depth_offset) * depth;
				Vector3 vtx = Vector3(p_read[i].x, p_read[i].y, depth_val) * face_scale;
				p_temp_write[i] = vtx;
				p_temp_rotated_write[i] = canonical_pose_transform.xform(vtx) + face_offset;
			}
		}
		face_landmarks = p_temp;
		face_landmarks_rotated = p_temp_rotated;
		face_scene_instance->queue_delete();
		face_landmarks_dirty = false;
		distance_matrix_dirty = true;
		edited_landmarks_dirty = true;
		return true;
	}

	bool calculate_distance_matrix() {
		if (!calculate_face_landmarks())
			return false;
		if (!distance_matrix_dirty){
			return true;
		}
		if (distance_matrix.size()<red_face_data.vertex_count)
			distance_matrix.resize(red_face_data.vertex_count);
		PoolVector<Vector3>::Read landmarks_read = face_landmarks.read();
		for (int i = 0; i < red_face_data.vertex_count; i++) {
			Vector3 landmark = landmarks_read[i];
			landmark.z = 0.0;
			Array distance_column;
			for (int j = 0; j < red_face_data.vertex_count; j++) {
				Vector3 target_landmark = landmarks_read[j];
				target_landmark.z = 0.0;
				distance_column.append(landmark.distance_to(target_landmark));
			}
			distance_matrix[i] = distance_column;
		}
		// 3d weights
		// for (int i = 0; i < red_face_data.vertex_count; i++) {
		// 	Vector3 landmark = landmarks_read[i];
		// 	Array distance_column;
		// 	for (int j = 0; j < red_face_data.vertex_count; j++) {
		// 		Vector3 target_landmark = landmarks_read[j];
		// 		distance_column.append(landmark.distance_to(target_landmark));
		// 	}
		// 	distance_matrix[i] = distance_column;
		// }
		distance_matrix_dirty = false;
		weights_matrix_dirty = true;
		return true;
	}

	bool calculate_weights_matrix() {
		if (!calculate_distance_matrix()){
			return false;
		}
		if (!weights_matrix_dirty){
			return true;
		}
		for (int j = 0; j < red_face_data.parts_size; j++){
			if (red_face_data[j]->weights.size()<red_face_data.vertex_count)
				red_face_data[j]->weights.resize(red_face_data.vertex_count); // errr
		}
		Array _brows = distance_matrix[red_face_data.brows[0]];
		Array _eyes = distance_matrix[red_face_data.eye_l[0]];
		Array _nose = distance_matrix[red_face_data.nose[6]];
		Array _nose_b = distance_matrix[red_face_data.nose[4]];
		Array _lips = distance_matrix[red_face_data.lips[6]];
		Array _jaw_shape =  distance_matrix[red_face_data.jaw_shape[7]];

		float nose_to_lips_range = _nose[red_face_data.lips[6]];

		float brows_range = _brows[red_face_data.brows[10]];
		float eyes_range = _eyes[red_face_data.eye_r[0]];
		float nose_range = _nose[red_face_data.nose[0]];
		float nose_b_range = _nose_b[red_face_data.nose_b[0]];
		float lips_range = _lips[red_face_data.lips[0]];
		float jaw_shape_range = _jaw_shape[red_face_data.jaw_shape[0]];

		float ranges[7] = {brows_range, eyes_range, eyes_range, nose_range, nose_b_range, lips_range, jaw_shape_range};

		for (int i = 0; i < red_face_data.vertex_count; i++) {
			Array distances = distance_matrix[i];	
			for (int j = 0; j < red_face_data.parts_size; j++){
				Part *p = red_face_data[j];
				p->weights.write[i] = get_point_weights(distances, p->get_ids(), p->vtxs_count, p->bones_count, p->vertex_step, ranges[j]);
			}
			{
				PoolVector<float>::Write brows_weights_write = red_face_data[0]->weights.write[i].write();
				PoolVector<float>::Write eye_l_weights_write = red_face_data[1]->weights.write[i].write();
				PoolVector<float>::Write eye_r_weights_write = red_face_data[2]->weights.write[i].write();
				PoolVector<float>::Write nose_weights_write = red_face_data[3]->weights.write[i].write();
				PoolVector<float>::Write nose_b_weights_write = red_face_data[4]->weights.write[i].write();
				PoolVector<float>::Write lips_weights_write = red_face_data[5]->weights.write[i].write();
				PoolVector<float>::Write jaw_shape_weights_write = red_face_data[6]->weights.write[i].write();

				PoolVector<float>::Read brows_weights_read = red_face_data[0]->weights[i].read();
				PoolVector<float>::Read eye_l_weights_read = red_face_data[1]->weights[i].read();
				PoolVector<float>::Read eye_r_weights_read = red_face_data[2]->weights[i].read();
				PoolVector<float>::Read nose_weights_read = red_face_data[3]->weights[i].read();
				PoolVector<float>::Read nose_b_weights_read = red_face_data[4]->weights[i].read();
				PoolVector<float>::Read lips_weights_read = red_face_data[5]->weights[i].read();
				PoolVector<float>::Read jaw_shape_weights_read = red_face_data[6]->weights[i].read();
				float brows_sum = 0;
				float eyes_sum = 0;
				float nose_sum = 0;
				float nose_b_sum = 0;
				float lips_sum = 0;
				for (int j = 0; j < red_face_data.brows.bones_count; j++)
					brows_sum += brows_weights_read[j];
				for (int j = 0; j < red_face_data.eye_l.bones_count; j++)
					eyes_sum += eye_l_weights_read[j];
				for (int j = 0; j < red_face_data.eye_r.bones_count; j++)
					eyes_sum += eye_r_weights_read[j];
				for (int j = 0; j < red_face_data.nose.bones_count; j++)
					nose_sum += nose_weights_read[j];
				for (int j = 0; j < red_face_data.nose_b.bones_count; j++)
					nose_b_sum += nose_b_weights_read[j];
				for (int j = 0; j < red_face_data.lips.bones_count; j++)
					lips_sum += lips_weights_read[j];
				
				brows_sum = 1.0 - brows_sum * brows_sum;
				eyes_sum = 1.0 - eyes_sum * eyes_sum;
				nose_sum = 1.0 - nose_sum * nose_sum;
				nose_b_sum = 1.0 - nose_b_sum * nose_b_sum;
				lips_sum = 1.0 - lips_sum * lips_sum;
				
				for (int j = 0; j < red_face_data.brows.bones_count; j++){
					brows_weights_write[j] = brows_weights_read[j] * eyes_sum * nose_sum * nose_b_sum * lips_sum;
				}
				for (int j = 0; j < red_face_data.eye_l.bones_count; j++){
					eye_l_weights_write[j] = eye_l_weights_read[j] * nose_b_sum;
				}
				for (int j = 0; j < red_face_data.eye_r.bones_count; j++){
					eye_r_weights_write[j] = eye_r_weights_read[j] * nose_b_sum;
				}
				float nose_mask = 1.0 - CLAMP(-((float)distances[red_face_data.nose[3]]) / nose_to_lips_range - 1.0, 0.0, 1.0);
				for (int j = 0; j < red_face_data.nose.bones_count; j++){
					nose_weights_write[j] = nose_weights_read[j] * eyes_sum * lips_sum * nose_mask;
				}
				for (int j = 0; j < red_face_data.nose_b.bones_count; j++){
					nose_b_weights_write[j] = nose_b_weights_read[j] * nose_sum * lips_sum * nose_mask;
				}
				for (int j = 0; j < red_face_data.lips.bones_count; j++){
					lips_weights_write[j] = lips_weights_read[j] * nose_sum * nose_b_sum;
				}
				float jaw_mask = CLAMP(((float)distances[red_face_data.nose[4]]) / nose_to_lips_range - 1.0, 0.0, 1.0);
				for (int j = 0; j < red_face_data.jaw_shape.bones_count; j++){
					jaw_shape_weights_write[j] = jaw_shape_weights_read[j] * eyes_sum * nose_sum * nose_b_sum * lips_sum * jaw_mask;
				}
			}
		}
		weights_matrix_dirty = false;
		edited_landmarks_dirty = true;
		return true;
	}

	bool calculate_edited_landmarks() {
		if (!calculate_face_landmarks())
			return false;
		if (!calculate_weights_matrix()){
			return false;
		}
		if (!edited_landmarks_dirty){
			return true;
		}
		Vector<PoolVector<Vector2> > offsets;
		offsets.resize(red_face_data.parts_size);
		for (int i = 0; i < red_face_data.parts_size - 2; i++){
			offsets.write[i] = get_offsets(i);
		}
		if (edited_landmarks.size() < red_face_data.vertex_count)
			edited_landmarks.resize(red_face_data.vertex_count);
		if (edited_landmarks_rotated.size() < red_face_data.vertex_count)
			edited_landmarks_rotated.resize(red_face_data.vertex_count);
		PoolVector<Vector3>::Write edited_landmarks_w = edited_landmarks.write();
		PoolVector<Vector3>::Write edited_landmarks_rotated_w = edited_landmarks_rotated.write();
		PoolVector<Vector3>::Read landmarks_read = face_landmarks.read();
		
		Vector2 eye_offset2d = (offsets[1].read()[0]+offsets[2].read()[0]) * 0.5;
		Vector3 eye_offset = Vector3(eye_offset2d.x, eye_offset2d.y, 0.0);
		for (int i = 0; i < red_face_data.vertex_count; i++) {
			Vector3 offsets_sum;
			for (int j = 0; j < red_face_data.parts_size - 2; j++){
				const Part *p = red_face_data[j];
				PoolVector<float>::Read w_read = p->weights[i].read();
				PoolVector<Vector2>::Read o_read = offsets[j].read();
				for (int z = 0; z < p->bones_count; z++){
					Vector2 offset = w_read[z] * o_read[z];
					offsets_sum += Vector3(offset.x, offset.y, 0.0);
				}
			}
			if(edit_front){
				edited_landmarks_w[i] = landmarks_read[i] + offsets_sum + face_offset;
				edited_landmarks_rotated_w[i] = canonical_pose_transform.xform(landmarks_read[i] + offsets_sum) + face_offset;
			}else{
				edited_landmarks_w[i] = landmarks_read[i] + canonical_pose_transform.xform_inv(offsets_sum) + face_offset;
				edited_landmarks_rotated_w[i] = canonical_pose_transform.xform(landmarks_read[i]) + offsets_sum + face_offset;
			}
		}
		edited_landmarks_dirty = false;
		iris_landmarks_dirty = true;
		return true;
	}

	// move iris calculations here
	bool calculate_iris_landmarks() {
		if (!calculate_edited_landmarks())
			return false;
		if (!iris_landmarks_dirty){
			return true;
		}
		if (face_scene.is_null()){
			face_scene = ResourceLoader::load("res://redot/polygons/faces/default_face.tscn");
			if (face_scene.is_null())
				return false;
		}
		Node *face_scene_instance = face_scene->instance();
		if (!face_scene_instance){
			ERR_FAIL_V_MSG(false, "face_scene_instance in calculate_iris_landmarks")
			return false;
		}
		Polygon2D *eye_l = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("eye_l")));
		Polygon2D *eye_r = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("eye_r")));
		if (!eye_l || !eye_r){
			face_scene_instance->queue_delete();
			return false;
		}
		PoolVector<Vector3>::Read canonical_iris_landmarks_read = canonical_iris_landmarks.read();
		for (int i = 0; i < 2; i++)
		{
			Polygon2D *loaded_eye;
			int polygon_size;
			Vector3 iris_ladmark_c;
			Vector3 iris_ladmark_r;
			Vector3 iris_ladmark_d;
			const int *ids;
			PoolVector<Vector3>::Write eye_polygon_write;
			PoolVector<Vector3>::Write eye_polygon_rotated_write;
			if(i == 0){
				loaded_eye = eye_l;
				polygon_size = loaded_eye->get_polygon().size();
				iris_ladmark_c = canonical_iris_landmarks_read[5];
				iris_ladmark_r = canonical_iris_landmarks_read[8];
				iris_ladmark_d = canonical_iris_landmarks_read[9];
				ids = red_face_data.eye_l.get_ids();
				iris_l.resize(polygon_size);
				iris_l_rotated.resize(polygon_size);
				eye_polygon_write = iris_l.write();
				eye_polygon_rotated_write = iris_l_rotated.write();
			}else{
				loaded_eye = eye_r;
				polygon_size = loaded_eye->get_polygon().size();
				iris_ladmark_c = canonical_iris_landmarks_read[0];
				iris_ladmark_r = canonical_iris_landmarks_read[1];
				iris_ladmark_d = canonical_iris_landmarks_read[4];
				ids = red_face_data.eye_r.get_ids();
				iris_r.resize(polygon_size);
				iris_r_rotated.resize(polygon_size);
				eye_polygon_write = iris_r.write();
				eye_polygon_rotated_write = iris_r_rotated.write();
			}
			PoolVector<Vector2>::Read eye_polygon_read = loaded_eye->get_polygon().read();
			PoolVector<Color>::Read eye_color_read = loaded_eye->get_vertex_colors().read();
			float depth_size = loaded_eye->get_depth_size();
			float depth_offset = loaded_eye->get_depth_offset();
			float eye_len = ABS(eye_polygon_read[63].x);
			PoolVector<Vector3>::Read edited_read = edited_landmarks.read();

			float eye_scale = eye_size * ABS((edited_read[ids[8]].x - edited_read[ids[0]].x) / (2.0 * eye_len));
			Vector3 eyelids_position = Vector3(	edited_read[ids[0]].x + eye_offset.x * (edited_read[ids[8]].x - edited_read[ids[0]].x) - face_offset.x, 
												edited_read[ids[12]].y + eye_offset.y * (edited_read[ids[4]].y - edited_read[ids[12]].y) - face_offset.y, 
												2.0 * eye_offset.z * (face_scale.z * loaded_eye->get_depth_position() + eye_len * (face_scale.z - eye_scale)));
			Transform eye_transform = Transform().looking_at(get_eye_target(iris_ladmark_c, eyelids_position, eye_len * eye_scale), Vector3(0, 1, 0));
			for (int i = 0; i < polygon_size; i++){
				float depth_val = (eye_color_read[i].a - depth_offset) * depth_size;
				Vector3 local_pos = Vector3(eye_polygon_read[i].x, eye_polygon_read[i].y, depth_val) * eye_scale;
				eye_polygon_write[i] = local_pos;
				
				Vector3 global_pos = eye_transform.xform(local_pos) + eyelids_position;
				eye_polygon_rotated_write[i] = canonical_pose_transform.xform(global_pos) + face_offset;
			}
		}
		iris_landmarks_dirty = false;
		face_scene_instance->queue_delete();
		return true;
	}

	bool calculate_parent() {
		if (!is_inside_tree()){
			ERR_FAIL_V_MSG(false, "is_inside_tree")
			return false;
		}
		if(face_path.is_empty())
			return false;
		Polygon2D *source_object = Object::cast_to<Polygon2D>(get_node(face_path));
		if (!source_object){
			ERR_FAIL_V_MSG(false, "source_object")
			return false;
		}
		Polygon2D *target_object = Object::cast_to<Polygon2D>(get_parent());
		if (!target_object){
			ERR_FAIL_V_MSG(false, "target_object")
			return false;
		}
		if (!calculate_edited_landmarks()){
			ERR_FAIL_V_MSG(false, "Calculate previous cache error in calculate_parent")
			return false;
		}
		if (edited_landmarks.size() < red_face_data.vertex_count){
			ERR_FAIL_V_MSG(false, "edited_landmarks")
			return false;
		}
		if (face_scene.is_null()){
			face_scene = ResourceLoader::load("res://redot/polygons/faces/default_face.tscn");
			if (face_scene.is_null())
				return false;
		}
		Node *face_scene_instance = face_scene->instance();
		if (!face_scene_instance){
			ERR_FAIL_V_MSG(false, "face_scene_instance")
			return false;
		}
		Polygon2D *face_polygon = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("face")));
		if (!face_polygon){
			face_scene_instance->queue_delete();
			return false;
		}
		Polygon2D *eye_l = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("eye_l")));
		Polygon2D *eye_r = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("eye_r")));
		Polygon2D *eye_blick_l = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("eye_blick_l")));
		Polygon2D *eye_blick_r = Object::cast_to<Polygon2D>(face_scene_instance->get_node(String("eye_blick_r")));
		if (!eye_l || !eye_r || !eye_blick_l || !eye_blick_r){
			face_scene_instance->queue_delete();
			return false;
		}
		NodePath face_transform_path = target_object->get_custom_transform();
		if (front_view){
			REDTransform *head_transform = red::create_node<REDTransform>(target_object, "head_transform");
			REDTransform *face_transform = red::create_node<REDTransform>(head_transform,"face_transform");
			target_object->set_custom_transform(target_object->get_path_to(face_transform));
			face_transform->set_custom_transform(canonical_pose_transform);
			head_transform->set_position(Point2(face_offset.x, face_offset.y));
			head_transform->set_depth_position(face_offset.z);
			PoolVector<Vector3>::Read canonical_iris_landmarks_read = canonical_iris_landmarks.read();
			for (int i = 0; i < 2; i++)
			{
				Polygon2D *loaded_eye;
				Polygon2D *loaded_eye_blick;
				NodePath eye_path;
				NodePath blick_path;
				String eye_name;
				String eye_blick_name;
				String transform_name;
				int face_iris_center_id;
				int face_iris_r_id;
				int offsets_num;
				const int *ids;
				const int *iris_ids;
				if(i == 0){
					eye_path = eye_l_path;
					blick_path = blick_l_path;
					loaded_eye = eye_l;
					loaded_eye_blick = eye_blick_l;
					eye_name = "eye_l";
					eye_blick_name = "eye_blick_l";
					transform_name = "eye_l_transform";
					face_iris_center_id = 5;
					face_iris_r_id = 8;
					offsets_num = 7;
					ids = red_face_data.eye_l.get_ids();
					iris_ids = red_face_data.iris_l.get_ids();
				}else{
					eye_path = eye_r_path;
					blick_path = blick_r_path;
					loaded_eye = eye_r;
					loaded_eye_blick = eye_blick_r;
					eye_name = "eye_r";
					eye_blick_name = "eye_blick_r";
					transform_name = "eye_r_transform";
					face_iris_center_id = 0;
					face_iris_r_id = 1;
					offsets_num = 8;
					ids = red_face_data.eye_r.get_ids();
					iris_ids = red_face_data.iris_r.get_ids();
				}
				Polygon2D *eye_flat_polygon;
				if(!eye_path.is_empty() && has_node(eye_path)){
					eye_flat_polygon = Object::cast_to<Polygon2D>(get_node(eye_path));
				}else{
					continue;
				}
				
				PoolVector<Vector2>::Read eye_polygon_read = loaded_eye->get_polygon().read();
				PoolVector<Vector2>::Read eye_blick_polygon_read = loaded_eye_blick->get_polygon().read();
				PoolVector<Color>::Read eye_color_read = loaded_eye->get_vertex_colors().read();
				int eye_polygon_size = loaded_eye->get_polygon().size();
				int eye_blick_polygon_size = loaded_eye_blick->get_polygon().size();
				float depth_size = loaded_eye->get_depth_size();
				float depth_offset = loaded_eye->get_depth_offset();
				float blick_depth_size = loaded_eye_blick->get_depth_size();
				float blick_depth_offset = loaded_eye_blick->get_depth_offset();
				int eye_max_x_id = 63;
				float eye_len = ABS(eye_polygon_read[eye_max_x_id].x);
				PoolVector<Vector3>::Read edited_read = edited_landmarks.read();

				float eye_scale = eye_size * ABS((edited_read[ids[8]].x - edited_read[ids[0]].x) / (2.0 * eye_len));
				Vector3 eyelids_position = Vector3(	edited_read[ids[0]].x + eye_offset.x * (edited_read[ids[8]].x - edited_read[ids[0]].x) - face_offset.x, 
													edited_read[ids[12]].y + eye_offset.y * (edited_read[ids[4]].y - edited_read[ids[12]].y) - face_offset.y, 
													2.0 * eye_offset.z * (face_scale.z * loaded_eye->get_depth_position() + eye_len * (face_scale.z - eye_scale)));
				REDTransform *eye_transform = red::create_node<REDTransform>(face_transform, transform_name);
				PoolVector2Array eye_offset = get_offsets(offsets_num);
				PoolVector2Array::Read eye_offset_read = eye_offset.read();
				Vector3 iris_offset_c = canonical_iris_landmarks_read[face_iris_center_id] + Vector3(eye_offset_read[0].x, eye_offset_read[0].y, 0.0);
				Vector3 iris_offset_x = canonical_iris_landmarks_read[face_iris_r_id] + Vector3(eye_offset_read[1].x, eye_offset_read[1].y, 0.0);
				Vector3 iris_offset_y = canonical_iris_landmarks_read[face_iris_center_id + 4] + Vector3(eye_offset_read[2].x, eye_offset_read[2].y, 0.0);
				Vector3 eye_c_target = get_eye_target(iris_offset_c, eyelids_position, eye_len * eye_scale);

				Transform eye_tr = Transform().looking_at(eye_c_target, Vector3(0, 1, 0));
				eye_transform->set_custom_transform(eye_tr);
				eye_transform->set_position(Point2(eyelids_position.x, eyelids_position.y));
				eye_transform->set_depth_position(eyelids_position.z);
				// eye mesh init
				{
					Vector3 offseted_x = eye_tr.xform_inv(get_eye_target(iris_offset_x, eyelids_position, eye_len * eye_scale));
					Vector3 offseted_y = eye_tr.xform_inv(get_eye_target(iris_offset_y, eyelids_position, eye_len * eye_scale));
					float iris_len = eye_polygon_read[iris_ids[1]].x;
					Vector2 iris_scale = Vector2(ABS(offseted_x.x / iris_len), ABS(offseted_y.y / iris_len));
					if(target_object->has_node(NodePath(eye_name))){
						loaded_eye = Object::cast_to<Polygon2D>(target_object->get_node(NodePath(eye_name)));
					}else{
						face_scene_instance->remove_child(loaded_eye);
						target_object->add_child(loaded_eye);
						loaded_eye->set_owner(target_object->get_owner());
					}
					PoolVector<Vector2> eye_polygon;
					eye_polygon.resize(eye_polygon_size);
					PoolVector<Vector2>::Write eye_polygon_write = eye_polygon.write();
					for (int i = 0; i < eye_polygon.size(); i++){
						int found = 0;
						for (int j = 0; j < 16; j++){
							if (i == red_face_data.iris_smooth_ids[j]){
								found = 1;
								break;
							}
						}
						for (int j = 0; j < 49; j++){
							if (i == red_face_data.iris_ids[j]){
								found = 2;
								break;
							}
						}
						Vector2 local_pos;
						Vector2 eye_scale2(eye_scale, eye_scale);
						switch (found)
						{
						case 1:
							local_pos = eye_polygon_read[i] * (iris_scale + 0.75 * (eye_scale2 - iris_scale));
							break;
						case 2:
							local_pos = eye_polygon_read[i] * iris_scale;
							break;
						default:
							local_pos = eye_polygon_read[i] * eye_scale;
							break;
						}
						eye_polygon_write[i] = local_pos;
					}
					loaded_eye->set_polygon(eye_polygon);
					loaded_eye->set_depth_size(depth_size * eye_scale);
					loaded_eye->set_texture(eye_flat_polygon->get_texture());
					loaded_eye->set_position(Point2(eyelids_position.x + face_offset.x, eyelids_position.y + face_offset.y));
					loaded_eye->set_depth_position(eyelids_position.z + face_offset.z);
					loaded_eye->set_custom_transform(loaded_eye->get_path_to(eye_transform));
				}
				Polygon2D *blick_flat_polygon;
				if(!blick_path.is_empty() && has_node(blick_path)){
					blick_flat_polygon = Object::cast_to<Polygon2D>(get_node(blick_path));
				}else{
					continue;
				}
				// eye blick mesh init
				{
					if(target_object->has_node(NodePath(eye_blick_name))){
						loaded_eye_blick = Object::cast_to<Polygon2D>(target_object->get_node(NodePath(eye_blick_name)));
					}else{
						face_scene_instance->remove_child(loaded_eye_blick);
						target_object->add_child(loaded_eye_blick);
						loaded_eye_blick->set_owner(target_object->get_owner());
					}
					PoolVector<Vector2> eye_polygon;
					eye_polygon.resize(eye_blick_polygon_size);
					PoolVector<Vector2>::Write eye_polygon_write = eye_polygon.write();
					for (int i = 0; i < eye_polygon.size(); i++)
						eye_polygon_write[i] = eye_blick_polygon_read[i] * eye_scale;
					loaded_eye_blick->set_polygon(eye_polygon);
					loaded_eye_blick->set_depth_size(blick_depth_size * eye_scale);
					loaded_eye_blick->set_texture(blick_flat_polygon->get_texture());
					loaded_eye_blick->set_position(Point2(eyelids_position.x + face_offset.x, eyelids_position.y + face_offset.y));
					loaded_eye_blick->set_depth_position(eyelids_position.z + face_offset.z);
					loaded_eye_blick->set_custom_transform(loaded_eye->get_path_to(eye_transform));
				}
			}
		}
		PoolVector<Vector3>::Read face_landmarks_read = edited_landmarks.read();
		PoolVector<Vector3>::Read face_landmarks_rotated_read = edited_landmarks_rotated.read();
		PoolVector<Vector2> polygon_new;
		PoolVector<Vector2> uv_new;
		PoolVector<Color> vc_new;
		polygon_new.resize(edited_landmarks.size());
		uv_new.resize(face_landmarks.size());
		vc_new.resize(edited_landmarks.size());
		PoolVector<Vector2>::Write polygon_new_write = polygon_new.write();
		PoolVector<Vector2>::Write uv_new_write = uv_new.write();
		PoolVector<Color>::Write vc_new_write = vc_new.write();

		float min_depth = face_landmarks_read[0].z;
		float max_depth = face_landmarks_read[0].z;
		for (int i = 1; i < face_landmarks.size(); i++) {
			float z = face_landmarks_read[i].z;
			if (z > max_depth) 
				max_depth = z; 
			else if (z < min_depth) 
				min_depth = z;
		}
		float real_depth = ABS(max_depth - min_depth);

		for (int i = 0; i < face_landmarks.size(); i++) {
			Vector3 vtx = face_landmarks_read[i];
			Vector3 vtx_rotated = face_landmarks_rotated_read[i];
			polygon_new_write[i] = Vector2(vtx.x, vtx.y);
			uv_new_write[i] = Vector2(vtx_rotated.x / polygon_size.x, vtx_rotated.y / polygon_size.y);
			vc_new_write[i] = Color(1.0, 1.0, 1.0, (vtx.z - min_depth) / real_depth);
		}
		target_object->set_texture(source_object->get_texture());
		target_object->set_polygon(polygon_new);
		target_object->set_vertex_colors(vc_new);
		target_object->set_depth_size(real_depth);
		target_object->set_depth_offset(-min_depth / real_depth);
		if (edit_uv){
			target_object->set_uv(uv_new);
		}
		target_object->set_internal_vertex_count(face_polygon->get_internal_vertex_count());
		target_object->set_polygons(face_polygon->get_polygons());
		face_scene_instance->queue_delete();
		return true;
	}

	bool start() {
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		return graph->start();
	}

	bool start_capture() {
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		return graph->start_capture();
	}

	bool finish() {
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		return graph->finish();
	}

	bool in_camera() {
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		canonical_face_landmarks_dirty = true;
		canonical_iris_landmarks_dirty = true;
		canonical_face_pose_dirty = true;
		return graph->in_camera();
	}

	bool reset_polygons(bool force=false){
		bool success = true;
		for (int i = 0; i < red_face_data.parts_size; i++){
			if (red_face_data[i]->polygon.size() != red_face_data[i]->bones_count || force) 
				_set_polygon(PoolVector<Vector2>(), i);
		}
		for (int i = 0; i < red_face_data.parts_size - 2; i++){
			if (!red_face_data[i]->polygon.size() != red_face_data[i]->bones_count) {
				success = false;
				break;
			}
		}
		_change_notify("brows");
		_change_notify("eye_l");
		_change_notify("eye_r");
		_change_notify("nose");
		_change_notify("nose_b");
		_change_notify("lips");
		_change_notify("jaw_shape");
		_change_notify("iris_l_polygon");
		_change_notify("iris_r_polygon");
		return success;
	}

	bool in_texture(Ref<Image> image) {
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		ERR_FAIL_COND_V_MSG(image.is_null(), false, "Image error");
		Size2 size = image->get_size();
		int len = size.width * size.height * 4;
		uint8_t *pixel_data = (uint8_t *)malloc(len * sizeof(uint8_t));
		{
			PoolVector<uint8_t>::Read imgr = image->get_data().read();
			for (int i = 0; i < len; i++) {
				pixel_data[i] = (uint8_t)imgr[i];
			}
		}
		ERR_FAIL_COND_V_MSG(!graph->in_texture(size.width, size.height, (uint8_t *)pixel_data), false, "Error running in_texture in graph");
		in_texture_dirty = false;
		canonical_face_landmarks_dirty = true;
		canonical_iris_landmarks_dirty = true;
		canonical_face_pose_dirty = true;
		return true;
	}

	bool out_display() {
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		return graph->out_display();
	}
	
	// delete not needed
	void give_depth(const NodePath &path) {
		// Polygon2D *polygon_object = (Polygon2D *)get_node(path);
		// ERR_FAIL_COND_MSG(polygon_object == nullptr, "Wrong polygon");
		// ERR_FAIL_COND_MSG(landmarks_depth.size() == 0, "No depth");
		// PoolVector<Color> colors;

		// PoolVector<float>::Read landmarks_depth_read = landmarks_depth.read();
		// landmarks_max_depth = landmarks_depth_read[0];
		// landmarks_min_depth = landmarks_depth_read[0];
		// for (int i = 1; i < landmarks_depth.size(); i++) {
		// 	float d = landmarks_depth_read[i];
		// 	if (d > landmarks_max_depth) {
		// 		landmarks_max_depth = d;
		// 	} else if (d < landmarks_min_depth) {
		// 		landmarks_min_depth = d;
		// 	}
		// }
		// {
		// 	colors.resize(landmarks_depth.size());
		// 	PoolVector<Color>::Write colors_w = colors.write();
		// 	float depth_size = landmarks_max_depth - landmarks_min_depth;
		// 	for (int i = 0; i < landmarks_depth.size(); i++) {
		// 		Color color(1.0, 1.0, 1.0, 1.0);
		// 		color.a = (landmarks_depth_read[i] - landmarks_min_depth) / depth_size;
		// 		colors_w[i] = color;
		// 	}
		// }
		// polygon_object->set_vertex_colors(colors);
		// polygon_object->set_depth_size((landmarks_max_depth - landmarks_min_depth));
		// polygon_object->set_depth_offset(0.0);

		// Array polygons_array;
		// for (int i = 0; i < face_data.index_buffer_size; i+=3)
		// {
		// 	Array polygons_current;
		// 	for (int j = 0; j < 3; j++)
		// 		polygons_current.push_back(face_data.index_buffer[i+j]);
		// 	polygons_array.push_back(polygons_current);
		// }
		// polygon_object->set_polygons(polygons_array);

	}
	// delete not needed
	bool out_polygon(const Size2 &size = Size2(1, 1)) {
		// ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		// float *polygon_in;
		// int *polygons_in;
		// float *transform_matrix;
		// int polygon_count = 0;
		// int polygons_size = 0;
		// ERR_FAIL_COND_V_MSG_CUSTOM(!graph->out_polygon(&polygon_in, &polygons_in, &polygon_count, &polygons_size, &transform_matrix), false, "Failed to get polygon data");
		// ERR_FAIL_COND_V_MSG_CUSTOM(polygon_count == 0 || polygons_size == 0, false, "polygon data has 0 size");

		// int real_polygon_size = polygon_count / 5;
		// polygon.resize(real_polygon_size);
		// uv.resize(real_polygon_size);
		// polygon_depth.resize(real_polygon_size);
		// polygons.resize(polygons_size);
		// PoolVector<Vector2>::Write polygon_w = polygon.write();
		// PoolVector<Vector2>::Write uv_r = uv.write();
		// PoolVector<float>::Write polygon_depth_w = polygon_depth.write();
		// int j = 0;
		// canonical_pose_transform = Transform(	transform_matrix[0], transform_matrix[1], transform_matrix[2],
		// 							transform_matrix[4], transform_matrix[5], transform_matrix[6],
		// 							transform_matrix[8], transform_matrix[9], transform_matrix[10],
		// 							transform_matrix[12], transform_matrix[13], transform_matrix[14]);
		// //canonical_pose_transform = canonical_pose_transform.affine_inverse();
		// Vector3 rotation = canonical_pose_transform.basis.get_euler_xyz();
		// canonical_pose_transform.basis.set_euler_xyz(Vector3(rotation.x, -rotation.y, rotation.z));
		// // polygon_max_depth = 1;
		// // for (int i = 0; i < real_polygon_size; i++) {
		// 	//uv_r[i] = Vector2(polygon_in[j + 3], polygon_in[j + 4]);
		// 	//Vector3 v3 = (matrix.xform(Vector3(polygon_in[j] * size.x, -polygon_in[j + 1] * size.y, -polygon_in[j + 2])));
		// 	//polygon_w[i] = Vector2(v3.x, v3.y);
		// 	//polygon_depth_w[i] = v3.z;
		// 	//polygon_w[i] = Vector2(polygon_in[j] * size.x, -polygon_in[j + 1] * size.y);
		// 	// if (v3.z > polygon_max_depth) {
		// 	// 	polygon_max_depth = v3.z;
		// 	// }
		// // 	j += 5;
		// // }

		// delete polygon_in;
		// delete polygons_in;
		// delete transform_matrix;
		return false;
	}

	Transform get_pose_transform() {
		calculate_canonical_pose_transform();
		return canonical_pose_transform;
	}

	PoolVector<Vector3> get_landmarks() {
		calculate_edited_landmarks();
		return edited_landmarks;
	}

	bool is_started() {
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		return graph->is_started();
	}

	bool is_key_pressed() {
		ERR_FAIL_COND_V_MSG(!_init(), false, "Graph initialization error");
		return graph->is_key_pressed();
	}

	Mediapipe();
	~Mediapipe();
};

#endif // MEDIAPIPE
