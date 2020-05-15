/*************************************************************************/
/*  line_2d_editor_plugin.cpp                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "mediapipe_editor_plugin.h"


Node2D *MediapipeEditor::_get_node() const {

	return node;
}

void MediapipeEditor::_set_node(Node *p_line) {

	node = Object::cast_to<Mediapipe>(p_line);
}

bool MediapipeEditor::_is_line() const {

	return true;
}

int MediapipeEditor::_get_polygon_count() const {

	return 9;
}

Variant MediapipeEditor::_get_polygon(int p_idx) const {
	switch (p_idx)
	{
	case 0:
		return node->get("brows_polygon");
	case 1:
		return node->get("eye_l_polygon");
	case 2:
		return node->get("eye_r_polygon");
	case 3:
		return node->get("nose_polygon");
	case 4:
		return node->get("nose_b_polygon");
	case 5:
		return node->get("lips_polygon");
	case 6:
		return node->get("jaw_shape_polygon");
	case 7:
		return node->get("iris_l_polygon");
	case 8:
		return node->get("iris_r_polygon");
	default:
		return node->get("jaw_shape_polygon");
	}
}

void MediapipeEditor::_set_polygon(int p_idx, const Variant &p_polygon) const {
	switch (p_idx)
	{
	case 0:
		node->set("brows_polygon", p_polygon);
		break;
	case 1:
		node->set("eye_l_polygon", p_polygon);
		break;
	case 2:
		node->set("eye_r_polygon", p_polygon);
		break;
	case 3:
		node->set("nose_polygon", p_polygon);
		break;
	case 4:
		node->set("nose_b_polygon", p_polygon);
		break;
	case 5:
		node->set("lips_polygon", p_polygon);
		break;
	case 6:
		node->set("jaw_shape_polygon", p_polygon);
		break;
	case 7:
	case 8:{
		PoolVector<Vector2> old_poly;
		if(p_idx == 7){
			old_poly = node->get("iris_l_polygon");
		}else{
			old_poly = node->get("iris_r_polygon");
		}
		PoolVector<Vector2> new_poly = p_polygon;
		if (old_poly.size() == new_poly.size() || old_poly.size() > 0){
			int poly_size = old_poly.size();
			PoolVector<Vector2>::Read old_poly_read = old_poly.read();
			PoolVector<Vector2>::Read new_poly_read = new_poly.read();
			Vector2 mover = new_poly_read[0] - old_poly_read[0];
			PoolVector<Vector2> override_poly;
			override_poly.resize(poly_size);
			PoolVector<Vector2>::Write override_poly_write = override_poly.write();
			override_poly_write[0] = new_poly_read[0];
			for (int i = 1; i < poly_size; i++){
				override_poly_write[i] = new_poly_read[i] + mover;
			}
			if(p_idx == 7){
				node->set("iris_l_polygon", Variant(override_poly));
			}else{
				node->set("iris_r_polygon", Variant(override_poly));
			}
		}else{
			if(p_idx == 7){
				node->set("iris_l_polygon", p_polygon);
			}else{
				node->set("iris_r_polygon", p_polygon);
			}
		}
	} break;
	default:
		node->set("jaw_shape_polygon", p_polygon);
		break;
	}
}

void MediapipeEditor::_action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon) {
	Node2D *node = _get_node();
	switch (p_idx)
	{
	case 0:
		undo_redo->add_do_method(node, "set_brows_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_brows_polygon", p_previous);
		break;
	case 1:
		undo_redo->add_do_method(node, "set_eye_l_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_eye_l_polygon", p_previous);
		break;
	case 2:
		undo_redo->add_do_method(node, "set_eye_r_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_eye_r_polygon", p_previous);
		break;
	case 3:
		undo_redo->add_do_method(node, "set_nose_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_nose_polygon", p_previous);
		break;
	case 4:
		undo_redo->add_do_method(node, "set_nose_b_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_nose_b_polygon", p_previous);
		break;
	case 5:
		undo_redo->add_do_method(node, "set_lips_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_lips_polygon", p_previous);
		break;
	case 6:
		undo_redo->add_do_method(node, "set_jaw_shape_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_jaw_shape_polygon", p_previous);
		break;
	case 7:
		undo_redo->add_do_method(node, "set_iris_l_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_iris_l_polygon", p_previous);
		break;
	case 8:
		undo_redo->add_do_method(node, "set_iris_r_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_iris_r_polygon", p_previous);
		break;
	default:
		undo_redo->add_do_method(node, "set_jaw_shape_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_jaw_shape_polygon", p_previous);
		break;
	}
}

MediapipeEditor::MediapipeEditor(EditorNode *p_editor) :
		AbstractPolygon2DEditor(p_editor) {
	node = NULL;
}

MediapipeEditorPlugin::MediapipeEditorPlugin(EditorNode *p_node) :
		AbstractPolygon2DEditorPlugin(p_node, memnew(MediapipeEditor(p_node)), "Mediapipe") {
}
