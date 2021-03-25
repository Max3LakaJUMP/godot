/*************************************************************************/
/*  line_2d_editor_plugin.h                                              */
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

#ifndef RED_ROOT_BONE_EDITOR_PLUGIN_H
#define RED_ROOT_BONE_EDITOR_PLUGIN_H

#include "editor/plugins/abstract_polygon_2d_editor.h"

class RootBone2D;
class CanvasItemEditor;
	
class RootBone2DEditor : public HBoxContainer {
	GDCLASS(RootBone2DEditor, HBoxContainer);
public:
	enum Menu {
		MENU_OPTION_MAKE_REST,
		MENU_OPTION_TO_REST,
		MENU_OPTION_MAKE_PRESUMED,
		MENU_OPTION_TO_PRESUMED,
	};
	enum EditState{
		STOPED,
		MOVING,
		ROTATING
	} edit_state;
	friend class RootBone2DEditorPlugin;
private:
	RootBone2D *node;
	CanvasItemEditor *canvas_item_editor;
	Vector3 edited_point;
	Vector3 pre_move_edit;
	Vector3 pre_rotate_edit;

	MenuButton *options;
protected:
	UndoRedo *undo_redo;

	static void _bind_methods();
	void _node_removed(Node *p_node);

	virtual RootBone2D *_get_node() const;
	virtual void _set_node(Node *p_node);
	bool spatial_pos_is_near(const Vector2 &p_pos) const;
	virtual Vector3 _get_spatial_position() const;
	virtual void _set_spatial_position(const Vector3 &p_polygon) const;
	virtual Vector3 _get_spatial_rotation() const;
	virtual void _set_spatial_rotation(const Vector3 &p_polygon) const;
	virtual void _action_set_spatial_position(const Vector3 &p_previous, const Vector3 &p_polygon);
	virtual void _action_set_spatial_position(const Vector3 &p_polygon);
	virtual void _action_set_spatial_rotation(const Vector3 &p_previous, const Vector3 &p_polygon);
	virtual void _action_set_spatial_rotation(const Vector3 &p_polygon);
	virtual void _commit_action();
	
public:
	void draw_empty_circle(Control *p_overlay, const Vector2 &circle_center, float circle_radius, const Color &color, float p_width, int resolution=1, bool p_antialiased=true);
	bool forward_gui_input(const Ref<InputEvent> &p_event);
	void forward_canvas_draw_over_viewport(Control *p_overlay);
	void edit(Node *p_polygon);
	void _menu_option(int p_option);
	RootBone2DEditor(EditorNode *p_editor);
};

class RootBone2DEditorPlugin : public EditorPlugin {

	GDCLASS(RootBone2DEditorPlugin, EditorPlugin);

	RootBone2DEditor *transform_editor;
	EditorNode *editor;
	String klass;

public:
	virtual bool forward_canvas_gui_input(const Ref<InputEvent> &p_event) { return transform_editor->forward_gui_input(p_event); }
	virtual void forward_canvas_draw_over_viewport(Control *p_overlay) { transform_editor->forward_canvas_draw_over_viewport(p_overlay); }

	bool has_main_screen() const { return false; }
	virtual String get_name() const { return klass; }
	virtual void edit(Object *p_object);
	virtual bool handles(Object *p_object) const;
	virtual void make_visible(bool p_visible);

	RootBone2DEditorPlugin(EditorNode *p_node);
	~RootBone2DEditorPlugin();
};

#endif // RED_ROOT_BONE_EDITOR_PLUGIN_H
