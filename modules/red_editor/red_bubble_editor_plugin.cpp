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

#include "red_bubble_editor_plugin.h"
#include <string> 
Node2D *REDBubbleEditor::_get_node() const {

	return node;
}

void REDBubbleEditor::_set_node(Node *p_line) {
	node = Object::cast_to<REDBubble>(p_line);
}

bool REDBubbleEditor::_is_line() const {

	return false;
}
/*
Variant REDBubbleEditor::_get_polygon(int p_idx) const {
	Node2D *node = _get_node();
	return get_polygon_id() == 0 ? node->get("polygon") : node->get("polygon_tail");
}

void REDBubbleEditor::_set_polygon(int p_idx, const Variant &p_polygon) const {
	Node2D *node = _get_node();
	get_polygon_id() == 0 ? node->set("polygon", p_polygon) : node->set("polygon_tail", p_polygon);
}
 
void REDBubbleEditor::_action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon) {
	Node2D *node = _get_node();
	if (get_polygon_id() == 0){
		undo_redo->add_do_method(node, "set_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_polygon", p_previous);
	}
	else{
		undo_redo->add_do_method(node, "set_polygon_tail", p_polygon);
		undo_redo->add_undo_method(node, "set_polygon_tail", p_previous);

	}
}
 
 
 */
int REDBubbleEditor::_get_polygon_count() const {

	return 2;
}
Variant REDBubbleEditor::_get_polygon(int p_idx) const {
	if (p_idx==0){
		return _get_node()->get("polygon");
	}
	else{
		return _get_node()->get("polygon_tail");
	}
}

void REDBubbleEditor::_set_polygon(int p_idx, const Variant &p_polygon) const {
	if (p_idx==0){
		_get_node()->set("polygon", p_polygon);
	}
	else{
		_get_node()->set("polygon_tail", p_polygon);
	}
}
void REDBubbleEditor::_action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon) {
	Node2D *node = _get_node();

	if (p_idx == 0){
		undo_redo->add_do_method(node, "set_polygon", p_polygon);
		undo_redo->add_undo_method(node, "set_polygon", p_previous);
	}
	else{
		undo_redo->add_do_method(node, "set_polygon_tail", p_polygon);
		undo_redo->add_undo_method(node, "set_polygon_tail", p_previous);

	}
}
void REDBubbleEditor::_action_set_polygon(int p_idx, const Variant &p_polygon) {

	_action_set_polygon(p_idx, _get_polygon(p_idx), p_polygon);
}

void REDBubbleEditor::_action_add_polygon(const Variant &p_polygon) {
	Vector<Vector2> vertices = _get_polygon(0);

	if (vertices.size() == 0)
		_action_set_polygon(0, p_polygon);
	else{
		_action_set_polygon(1, p_polygon);
	}
}

/*
Variant REDBubbleEditor::_get_polygon(int p_idx) const {

	return _get_node()->get("polygon");
}

void REDBubbleEditor::_set_polygon(int p_idx, const Variant &p_polygon) const {

	_get_node()->set("polygon", p_polygon);
}

void REDBubbleEditor::_action_set_polygon(int p_idx, const Variant &p_previous, const Variant &p_polygon) {

	Node2D *node = _get_node();
	undo_redo->add_do_method(node, "set_polygon", p_polygon);
	undo_redo->add_undo_method(node, "set_polygon", p_previous);
}
*/
REDBubbleEditor::REDBubbleEditor(EditorNode *p_editor) :
		AbstractPolygon2DEditor(p_editor) {
	node = NULL;
}

REDBubbleEditorPlugin::REDBubbleEditorPlugin(EditorNode *p_node) :
		AbstractPolygon2DEditorPlugin(p_node, memnew(REDBubbleEditor(p_node)), "REDBubble") {
}
