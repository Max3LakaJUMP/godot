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

#ifndef RESOURCE_IMPORTER_PSD
#define RESOURCE_IMPORTER_PSD

#include "core/image.h"
#include "core/io/resource_importer.h"

#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"
#include "modules/red/red_clipper.h"

struct _psd_layer_record;
class Polygon2D;
class Node2D;
class ShaderMaterial;
class REDClipper;
class REDPolygon;
struct _psd_context;
struct _psd_layer_mask_info;

struct Materials{
	Ref<ShaderMaterial> outline_material;
	Ref<ShaderMaterial> material;
	Ref<ShaderMaterial> material_mul;
	Ref<ShaderMaterial> material_add;
	Ref<ShaderMaterial> material_sub;

	Ref<ShaderMaterial> masked_material;
	Ref<ShaderMaterial> masked_material_mul;
	Ref<ShaderMaterial> masked_material_add;
	Ref<ShaderMaterial> masked_material_sub;
	void init(const Map<StringName, Variant> &p_options, Node *node);
};

struct MeshData{
	int vertex_count = 0;
	int faces_count = 0;
	Size2 vtx_size;
	Size2 uv_size;
	Point2 uv_min;
	Point2 uv_max;
	Vector2 vtx_min;
	Vector2 vtx_max;
	Vector2 uv_offset;
	Vector2 vtx_offset;
	Point2 obj_pos;
	void calc(REDPolygon *poly, bool vtx=false, bool uv=false, bool faces=false, bool obj=false);
};


class ResourceImporterPSD : public ResourceImporter {
	GDCLASS(ResourceImporterPSD, ResourceImporter);

public:
	enum TextureScale {
		ORIGINAL,
		DOWNSCALE,
		UPSCALE,
		CLOSEST,
	};
	enum LayerType {
		LAYER_POLYGON2D,
		LAYER_POLYGON,
	};

	enum FolderType {
		FOLDER_NODE2D,
		FOLDER_PARALLAX,
		FOLDER_PAGE,
		FOLDER_FRAME,
		FOLDER_FRAME_EXTERNAL,
	};
	enum UpdateFolderPos {
		FOLDER_POS_IGNORE,
		FOLDER_POS_RESET,
		FOLDER_MOVE_LAYERS,
	};

	enum UpdateLayerPos {
		LAYER_POS_IGNORE,
		LAYER_POS_MOVE,
		LAYER_POS_NEW_UV,
		LAYER_POS_KEEP_UV,
	};

	enum Anchor {
		ANCHOR_TOP_LEFT,
		ANCHOR_CENTER,
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
	
	Vector<Vector2> load_polygon_from_mask(_psd_layer_record *layer, float target_width, int psd_width);
	void simplify_polygon(Vector<Vector2> &p_polygon, float min_dot);
	void save_png(_psd_layer_record *layer, String png_path, int texture_max_size=2048, int texture_scale=0);
	void create_polygon(_psd_layer_record *layer, Ref<ShaderMaterial> material, Vector2 polygon_size, String png_path, Node2D *parent);

	int load_folder(_psd_context *context, String target_dir, int start, Materials &materials, 
					Node *parent, Vector2 parent_post, Vector2 parent_offset, const Map<StringName, Variant> &p_options, bool force_save=false, int counter=0, int folder_level=-1, REDClipper *parent_clipper=nullptr);
	
	void _mask_to_node(_psd_layer_record *layer, float target_width, Node2D *node2d, _psd_context *context, int anchor_second_level);
	Node *_get_root(_psd_context *context, const String &target_dir, bool &force_save, const Map<StringName, Variant> &p_options) const;
	Node *get_edited_scene_root(String p_path) const;
	ResourceImporterPSD();
};

#endif // RESOURCE_IMPORTER_PSD
