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

#ifndef SCENE_EXPORT_EDITOR_PLUGIN_H
#define SCENE_EXPORT_EDITOR_PLUGIN_H

#include "editor/plugins/abstract_polygon_2d_editor.h"
#include "scene/2d/line_2d.h"
#include "scene/gui/menu_button.h"

class SceneExportEditorPlugin : public EditorPlugin {

	GDCLASS(SceneExportEditorPlugin, EditorPlugin);
	MenuButton *red_menu;
protected:
	static void _bind_methods();

public:
	SceneExportEditorPlugin(EditorNode *p_node);
	void red_clicked(const int id);

	void to_maya();
	Dictionary scene_to_dict(Node *root);
	Dictionary node_to_dict(Node *node);
	Error save_json(Dictionary &p_data, String &p_path);
	
	Vector<int> triangulate(const PoolVector2Array &points, const Array &polygons);
	void selection_to_normals();
	
	void create_mediapipe();
	void attach_transform();
};

#endif // SCENE_EXPORT_EDITOR_PLUGIN_H
