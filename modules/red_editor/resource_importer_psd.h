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
class REDClipper;
class REDFrame;
class REDPolygon;
class ViewportContainer;
class Viewport;
class BackBufferCopy;
class Camera2D;
class ColorRect;
struct _psd_context;
struct _psd_layer_mask_info;

struct ImgData{
	Ref<Image> img;
	Ref<BitMap> bitmap;

	Ref<Image> rim;

	Rect2 img_rect;
	Rect2 crop_rect;
	Rect2 crop_rect_bitmap;
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

	void init(const Map<StringName, Variant> &p_options, Node *node);
	void apply_material(const _psd_layer_record *layer, Node2D *node, ImgData &atlas_img);
};

class Layer : public Object {
	GDCLASS(Layer, Object);
protected:
	static void _bind_methods();
public:
	enum TextureScale {
		ORIGINAL,
		DOWNSCALE,
		UPSCALE,
		CLOSEST,
	};
	Map<StringName, Variant> p_options;
	_psd_context *context;
	int final_iter;
	int iter;
	int start;

	String target_dir;
	bool updateble;
	Materials materials;
	Vector<String> names;

	Node *parent;
	REDClipper *parent_clipper;
	Vector2 parent_pos; 
	Vector2 parent_offset;

	Size2 context_size;
	double context_aspect_ratio;
	Size2 scene_size;
	double scene_k;
	Size2 canvas_size;
	double image_resize;
	double image_size_to_scene_size;

	Polygon2D *poly;
	Vector2 polygon_size;
	Point2 global_pos;
	Point2 local_pos;
	float polygon_grow;
	Ref<Image> layer_image;
	float bitmap_threshold;
	Size2 bitmap_size;
	Size2 texture_max_size;
	int texture_scale_mode;
	String png_path;
	String normal_path;
	String rim_path;
	ImgData image_data;
	bool save_texture;
	bool save_rim;
	bool update_material;
	bool update_polygon;
	bool update_polygon_pos;
	bool update_polygon_transform;
	bool update_uv;
	bool reset_uv;
	bool reorder;

	void _texture();
	void _polygon();
	void _uv();
	void _reorder();
	void load();

	Ref<Image> read_image(_psd_layer_record *layer);
	Ref<Image> read_atlas(_psd_layer_record *layer, ImgData &atlas_img);
	Ref<Image> read_rim(_psd_layer_record *layer, ImgData &atlas_img);
	void load_clipped_layers(_psd_context *context, int start, ImgData &atlas, bool load_layer_data);
	Rect2 get_crop(Ref<BitMap> bitmap, const Vector2 &grow);
	void apply_scale(Ref<Image> img, double scale=1.0, int texture_scale_mode=ORIGINAL, int texture_min_size=8, const Size2 &texture_max_size=Size2(4096, 4096));
	void apply_scale(ImgData &atlas, double scale=1.0, int texture_scale_mode=ORIGINAL, int texture_min_size=8, const Size2 &texture_max_size=Size2(4096, 4096));
	void apply_border(Ref<Image> img, float p_threshold = 0.75f);

	Layer(){};
	~Layer(){};
};

class ResourceImporterPSD : public ResourceImporter {
	GDCLASS(ResourceImporterPSD, ResourceImporter);
	Vector<Layer*> _layers;

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

	Ref<Image> read_mask(_psd_layer_record *layer);

	//void _read_image_thread(void *p_ud);
	// Ref<Image> read_image(_psd_layer_record *layer, const int threads_count=4);
	uint8_t *read_image_to_mediapipe(_psd_layer_record *layer);
	void create_polygon(_psd_layer_record *layer, Ref<ShaderMaterial> material, Vector2 polygon_size, String png_path, Node2D *parent);
	int load_folder(_psd_context *context, const String &scene_path, int start, Materials &materials, 
					Node *parent, Vector2 parent_post, Vector2 parent_offset, const Map<StringName, Variant> &p_options, 
					bool force_save=false, int counter=0, int folder_level=-1, REDClipper *parent_clipper=nullptr);
	Node *_get_root(_psd_context *context, const String &scene_path, bool &force_save, const Map<StringName, Variant> &p_options) const;
	Node *get_edited_scene_root(const String &p_path) const;
	Vector3 find_normal_border(const Ref<BitMap> bitmap, const Point2 &point, int max_radius=32);

	void render_normal(Ref<BitMap> bitmap, String &diffuse_path, String &normal_path, Size2 &resolution, Ref<Image> rim_texture=Ref<Image>(), Polygon2D *apply_polygon=nullptr);
	void normal_to_polygon(Node *apply_polygon, const String &normal_path);


	ResourceImporterPSD();
};

#endif // RESOURCE_IMPORTER_PSD