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

#ifndef RESOURCE_IMPORTER_JSCN
#define RESOURCE_IMPORTER_JSCN

#include "core/image.h"
#include "core/io/resource_importer.h"


#include "editor/editor_atlas_packer.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

struct _psd_layer_record;
class Polygon2D;
class Node2D;
class ShaderMaterial;
class REDFrame;
struct _psd_context;
struct _psd_layer_mask_info;

class ResourceImporterJSCN : public ResourceImporter {
	GDCLASS(ResourceImporterJSCN, ResourceImporter);

	struct PackData {
		Rect2 region;
		bool is_mesh;
		Vector<int> chart_pieces; //one for region, many for mesh
		Vector<Vector<Vector2> > chart_vertices; //for mesh
		Ref<Image> image;
	};

public:
	enum ImportMode {
		IMPORT_MODE_REGION,
		IMPORT_MODE_2D_MESH
	};

	enum LayerType {
		LAYER_NODE2D,
		LAYER_POLYGON2D,
		LAYER_POLYGON,
	};

	enum FolderType {
		FOLDER_NODE2D,
		FOLDER_PAGE,
		FOLDER_FRAME,
		FOLDER_FRAME_EXTERNAL
	};

	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;

	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;
	virtual bool get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const;

	virtual Error import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files = NULL, Variant *r_metadata = NULL);

	Node *get_edited_scene_root(const String &p_path) const;
	void dict_to_node(Dictionary &serialized, Node *parent, const Map<StringName, Variant> &p_options, String &scene_path, bool parent_is_root=false);
	Error create_dir(String dir);

	ResourceImporterJSCN();
};

#endif // RESOURCE_IMPORTER_JSCN
