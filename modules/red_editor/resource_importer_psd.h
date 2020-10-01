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
#include "scene/gui/viewport_container.h"

struct _psd_layer_record;
class Polygon2D;
class Node2D;
class ShaderMaterial;
class REDFrame;
class REDPolygon;
class ViewportContainer;
class Viewport;
class BackBufferCopy;
class Camera2D;
class ColorRect;
struct _psd_context;
struct _psd_layer_mask_info;

class RenderData : public ViewportContainer {
	GDCLASS(RenderData, ViewportContainer);
	Viewport *viewport;
	Camera2D *camera;
	BackBufferCopy *bb;
	Polygon2D *back_mesh;
	Polygon2D *pass1_mesh;
	Polygon2D *pass2_mesh;
protected:
	static void _bind_methods();
public:
	String png_path;
	String render_path;
	Ref<Image> rim_image;
	Ref<BitMap> bitmap;
	Size2 resolution = Size2(128, 128);

	void init();
	PoolColorArray get_normals(PoolVector2Array vtxs);
	void generate_polygons();
	void generate_pp();
	void render();
	void _render_png();

	RenderData();
};

struct img_data{
	Ref<Image> img;
	Ref<BitMap> bitmap;

	Ref<Image> normal;

	Rect2 img_rect;
	Rect2 crop_rect;
	Size2 count = Size2(1, 1);
	Size2 polygon_size = Size2(1, 1);
	bool use = false;
	bool create = false;
	bool save = false;
	bool polygon = false;
	String polygon_name;
	String atlas_path;
};

struct Materials{
	Ref<ShaderMaterial> frame_outline;
	Ref<ShaderMaterial> rim;
	Ref<ShaderMaterial> material;
	Ref<ShaderMaterial> material_mul;
	Ref<ShaderMaterial> material_add;
	Ref<ShaderMaterial> material_sub;

	// Ref<ShaderMaterial> masked_material;
	// Ref<ShaderMaterial> masked_material_mul;
	// Ref<ShaderMaterial> masked_material_add;
	// Ref<ShaderMaterial> masked_material_sub;
	void init(const Map<StringName, Variant> &p_options, Node *node);
	void apply_material(_psd_layer_record *layer, Node2D *node, REDFrame *parent_clipper, img_data &atlas_img);
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
	
	//void calc(REDPolygon *poly, bool vtx=false, bool uv=false, bool faces=false, bool obj=false);
};


class ResourceImporterPSD : public ResourceImporter {
	GDCLASS(ResourceImporterPSD, ResourceImporter);
	//RenderData render_data;
protected:
	static void _bind_methods();
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
		LAYER_MOVE_AND_SCALE,
		LAYER_RESET_UV,
		LAYER_MOVE_UV,
		LAYER_MOVE_AND_RESET_UV,
		LAYER_MOVE_AND_MOVE_UV,
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
	Vector<Vector2> simplify_polygon_corner(Vector<Vector2> &p_polygon, float min_distance = 2.0f);
	Vector<Vector2> simplify_polygon_distance(Vector<Vector2> &p_polygon, float min_distance = 4.0f, Size2 &size=Size2(128.0f, 128.0f));
	Vector<Vector2> simplify_polygon_direction(Vector<Vector2> &p_polygon, float max_dot = 0.75f);
	Rect2 get_crop(Ref<BitMap> bitmap, const Vector2 &grow);
	Ref<Image> read_mask(_psd_layer_record *layer);
	Ref<BitMap> read_bitmap(Ref<Image> p_img, float p_threshold, Size2 max_size=Size2(128.0f, 128.0f));
	Ref<BitMap> read_bitmap(_psd_layer_record *layer, float p_threshold = 0.1f, Size2 max_size=Size2(128.0f, 128.0f));
	Ref<Image> read_atlas(_psd_layer_record *layer, img_data &atlas_img);
	Ref<Image> read_image(_psd_layer_record *layer);
	Ref<Image> read_rim(_psd_layer_record *layer, img_data &atlas_img);
	void apply_scale(Ref<Image> img, double scale=1.0, int texture_scale_mode=ORIGINAL, int texture_min_size=8, const Size2 &texture_max_size=Size2(4096, 4096));
	void apply_scale(img_data &atlas, double scale=1.0, int texture_scale_mode=ORIGINAL, int texture_min_size=8, const Size2 &texture_max_size=Size2(4096, 4096));
	void apply_border(Ref<Image> img, float p_threshold = 0.75f);
	void create_polygon(_psd_layer_record *layer, Ref<ShaderMaterial> material, Vector2 polygon_size, String png_path, Node2D *parent);
	void load_clipped_layers(_psd_context *context, int start, img_data &atlas, bool load_layer_data);
	int load_folder(_psd_context *context, const String &scene_path, int start, Materials &materials, 
					Node *parent, Vector2 parent_post, Vector2 parent_offset, const Map<StringName, Variant> &p_options, 
					bool force_save=false, int counter=0, int folder_level=-1, REDFrame *parent_clipper=nullptr);
	Node *_get_root(_psd_context *context, const String &scene_path, bool &force_save, const Map<StringName, Variant> &p_options) const;
	Node *get_edited_scene_root(const String &p_path) const;
	Vector3 find_normal_border(const Ref<BitMap> bitmap, const Point2 &point, int max_radius=32);
	Vector<PoolVector<Vector2> > bitmap_to_polygon(Ref<BitMap> bitmap_mask, Size2 &polygon_size, float polygon_grow=0, float epsilon=4.0, bool single=true);

	//void _render_viewport();

	Vector<Vector2> ramer_douglas_peucker(Vector<Vector2> &pointList, double epsilon=2, bool is_closed=false);
	ResourceImporterPSD();
};

#endif // RESOURCE_IMPORTER_PSD
