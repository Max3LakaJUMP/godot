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

#include "resource_importer_psd.h"

#include "core/io/image_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/file_access.h"
#include "editor/editor_atlas_packer.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"
#include "core/bind/core_bind.h"
#include "core/image.h"
#include "core/project_settings.h"

#include "libpsd/include/libpsd.h"
#include "scene/resources/packed_scene.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/polygon_2d.h"
#include "scene/resources/material.h"
#include "scene/main/node.h"
#include "editor/editor_plugin.h" 
#include "modules/red/red_shape_renderer.h" 
#include "modules/red/red_clipper.h" 
#include "modules/red/red_frame.h" 
#include "modules/red/red_page.h" 
#include "scene/resources/bit_map.h"
#include "core/math/math_funcs.h"
#include <string>
#include "core/message_queue.h"
#include "editor/editor_node.h" 
#include "editor/import/resource_importer_texture_atlas.h"
#include "red_render_data.h"

#include "scene/main/viewport.h"
#include "scene/gui/viewport_container.h"
#include "modules/red/red_engine.h"
#include "modules/red/red_parallax_folder.h"
#include "modules/red/red_target.h"
#include "scene/gui/color_rect.h"
#include "scene/2d/back_buffer_copy.h"

void Layer::_texture(){
	if(save_texture){
		apply_scale(layer_image, image_resize, texture_scale_mode, 32, texture_max_size);
		apply_border(layer_image, 0.75f);
		layer_image->save_png(png_path);
		if (!ResourceLoader::is_imported(png_path)){
			ResourceLoader::import(png_path);
		}
	}
	// todo clipped layers support only "rim" layer
	if(save_rim){
		load_clipped_layers(context, final_iter+1, image_data, true);
		if (image_data.rim.is_valid()){
			apply_scale(image_data.rim, image_resize, texture_scale_mode, 32, texture_max_size);
			image_data.rim->save_png(rim_path);
			if (!ResourceLoader::is_imported(rim_path)){
				ResourceLoader::import(rim_path);
			}
		}
	}
	if (update_material){
		Ref<Texture> texture = ResourceLoader::load(png_path, "Texture");
		if (texture.is_valid())
			poly->set_texture(texture);
		psd_layer_record *layers = context->layer_records;
		psd_layer_record *layer = &layers[final_iter];
		materials.apply_material(layer, poly, image_data);
	}
	// memdelete(this);
}

void Layer::_polygon(){
	bool old_move_polygon_with_uv = poly->get_move_polygon_with_uv();
	poly->set_move_polygon_with_uv(true);
	poly->set_move_polygon_with_uv(false);
	if (update_polygon){
		Ref<BitMap> bitmap_poly;
		if(p_options["texture/alpha_to_polygon"])
			bitmap_poly = red::read_bitmap(layer_image, bitmap_threshold, bitmap_size);
		poly->set_polygon(red::bitmap_to_polygon(bitmap_poly, polygon_size, scene_size.width * polygon_grow, 6.0f, true)[0]);
	}
	if (update_polygon_transform){
		Rect2 k = red::get_rect(poly->get_uv());
		k.size = k.size * image_data.count;
		Rect2 real = red::get_rect(poly->get_polygon());
		PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
		Rect2 target = Rect2(polygon_size * k.position, polygon_size * k.size);
		PoolVector<Vector2> new_poly;
		for (int i = 0; i < poly->get_polygon().size(); i++)
			new_poly.append(((polyr[i] - real.position) / real.size) * target.size + target.position);
		poly->set_polygon(new_poly);
		poly->set_position(local_pos);
	}
	else if (update_polygon_pos){
		poly->set_position(local_pos);
	}
	if (update_uv){
		Rect2 poly_rect = red::get_rect(poly->get_polygon());
		Vector2 psd_offset = local_pos - poly->get_position();
		Vector2 uv_offset = poly_rect.position / poly_rect.size;
		Vector<Vector2> uv_temp;
		if (poly->get_uv().size() != poly->get_polygon().size() || reset_uv){
			PoolVector<Vector2>::Read polyr = poly->get_polygon().read();
			for (int i = 0; i < poly->get_polygon().size(); i++)
				uv_temp.push_back((polyr[i]) / poly_rect.size);
		}
		else{
			PoolVector<Vector2>::Read uvr = poly->get_uv().read();

			Rect2 real = red::get_rect(poly->get_uv());
			for (int i = 0; i < poly->get_uv().size(); i++)
				uv_temp.push_back((uvr[i] - real.position) / real.size + uv_offset);
		}
		PoolVector<Vector2> new_uv;
		Vector2 psd_offset_uv = psd_offset / polygon_size;
		Vector2 psd_scale_uv = poly_rect.size / polygon_size;
		for (int i = 0; i < uv_temp.size(); i++)
			new_uv.append((uv_temp[i] * psd_scale_uv - psd_offset_uv) / image_data.count);
		poly->set_uv(new_uv);
	}
	poly->set_move_polygon_with_uv(old_move_polygon_with_uv);
}

void Layer::_reorder(){
	if(reorder){
		psd_layer_record *layers = context->layer_records;
		Node *p = poly->get_parent();
		int extra_nodes = 0;
		for (int i = 0; i < MIN(p->get_child_count(), iter-start+extra_nodes+1); i++)
		{
			Node *child = p->get_child(i);
			bool found = false;
			for (int j = 0; j < names.size(); j++)
			{
				if (child->get_name() == names[j]){
					found = true;
					break;
				}
			}
			if (!found){
				extra_nodes++;
			}
		}
		int cliped_count_order = 0;
		for (int i = start + 1; i < final_iter-1; i++)
		{
			if (layers[i].clipping){
				cliped_count_order += 1;
			}
		}
		p->move_child(poly, MAX(iter - start + extra_nodes - cliped_count_order, p->get_child_count() - 1));
	}
}

void Layer::load(){
	psd_layer_record *layers = context->layer_records;
	psd_layer_record *layer = &layers[final_iter];
	String name = reinterpret_cast<char const*>(layer->layer_name);
	print_verbose("Loading " + name);
	int len = layer->width * layer->height;
	if (len == 0) {
		memdelete(this);
		return;
	}
	_File file_сheck;
	int folder_pos = p_options["update/folder_pos"];
	bool update_pos_move_layers = updateble && folder_pos == ResourceImporterPSD::FOLDER_MOVE_LAYERS;
	
	png_path = target_dir + "/" + name + ".png";
	normal_path = target_dir + "/" + name + "_normal.png";
	rim_path = target_dir + "/" + name + "_rim.png";
	save_texture = !file_сheck.file_exists(png_path) || (p_options["update/texture"] && updateble);
	save_rim = !file_сheck.file_exists(rim_path) || (p_options["update/texture"] && updateble);
	int update_pos = p_options["update/layer_pos"];

	update_material = false;
	update_polygon = false;
	update_polygon_pos = (updateble && (update_pos == ResourceImporterPSD::LAYER_MOVE_AND_RESET_UV || 
											 update_pos == ResourceImporterPSD::LAYER_MOVE_AND_MOVE_UV)) || update_pos_move_layers;
	update_polygon_transform = updateble && update_pos == ResourceImporterPSD::LAYER_MOVE_AND_SCALE;
	update_uv = updateble && (update_pos == ResourceImporterPSD::LAYER_RESET_UV ||
									update_pos == ResourceImporterPSD::LAYER_MOVE_UV ||
									update_pos == ResourceImporterPSD::LAYER_MOVE_AND_RESET_UV || 
									update_pos == ResourceImporterPSD::LAYER_MOVE_AND_MOVE_UV);
	reset_uv = update_pos == ResourceImporterPSD::LAYER_RESET_UV || update_pos == ResourceImporterPSD::LAYER_MOVE_AND_RESET_UV;
	reorder =  updateble && p_options["update/order"];
	bool need_create = parent->has_node(name) ? false : true;
	if (need_create){
		Node *owner = parent->get_owner();
		poly = red::create_node<Polygon2D>(parent, name, owner ? owner : parent);
		poly->set_draw_behind_parent(true);
		if (parent_clipper != nullptr)
			poly->set_clipper(poly->get_path_to(parent_clipper));
		update_material = true;
		update_polygon = true;
		update_polygon_pos = true;
		update_polygon_transform = false;
		update_uv = true;
		reset_uv = true;
		reorder = true;
	} 
	else {
		poly = (Polygon2D*)parent->get_node(name);
	}
	if (poly == nullptr){
		memdelete(this);
		return;
	}

	if (!update_polygon && poly->get_polygon().size() == 0)
		update_polygon = true;
	polygon_grow = p_options["texture/polygon_grow"];	// grow per scene width
	float alpha_grow = p_options["texture/alpha_grow"];	// grow per scene width
	bitmap_threshold = 0.2f;
	bitmap_size = Size2(p_options["texture/bitmap_size"], p_options["texture/bitmap_size"]);
	texture_max_size = Size2(p_options["texture/max_size"], p_options["texture/max_size"]);
	texture_scale_mode = p_options["texture/scale"];

	layer_image = read_image(layer);
	Vector2 layer_image_size = layer_image->get_size();
	polygon_size = layer_image_size * image_size_to_scene_size;
	Ref<BitMap> bitmap = red::read_bitmap(layer_image, bitmap_threshold, bitmap_size);
	// Rect2 crop_rect = get_crop(bitmap, alpha_grow * bitmap->get_size() * scene_size / polygon_size);
	Rect2 crop_rect_bitmap = get_crop(bitmap, bitmap->get_size() * scene_size * alpha_grow / 128.0);
	Rect2 crop_rect;
	{
		image_data.img = layer_image;
		image_data.bitmap = bitmap;
		Vector2 bitmap_k = layer_image_size / bitmap->get_size();
		crop_rect.position = Point2(Math::round(crop_rect_bitmap.position.x * bitmap_k.x), Math::round(crop_rect_bitmap.position.y * bitmap_k.y));
		crop_rect.size = Size2(Math::round(crop_rect_bitmap.size.width * bitmap_k.x), Math::round(crop_rect_bitmap.size.height * bitmap_k.y));
		image_data.img_rect = Rect2(Point2(layer->left, layer->top) + crop_rect.position, crop_rect.size);
		image_data.polygon_name = name;
	}
	polygon_size = image_data.img_rect.size * image_size_to_scene_size;
	global_pos = image_data.img_rect.position * image_size_to_scene_size;
	local_pos = global_pos - parent_pos - parent_offset - poly->get_offset();
	if (save_texture || update_polygon){
		layer_image->crop_from_point(crop_rect.position.x, crop_rect.position.y, crop_rect.size.width, crop_rect.size.height);
	}
	_reorder();
	_polygon();
	_texture();
}

Ref<Image> Layer::read_image(_psd_layer_record *layer){
	Ref<Image> img;
	int len = layer->width * layer->height;
	psd_argb_color *pixels = layer->image_data;
	PoolVector<uint8_t> data;
	data.resize(len * 4);
	String name = reinterpret_cast<char const*>(layer->layer_name);
	{
		PoolVector<uint8_t>::Write w = data.write();
		int j = 0;
		for (int i = 0; i < len; i++) {
			unsigned int pixel = pixels[i];
			w[j] = (uint8_t) ((pixel & 0x00FF0000) >> 16);
			w[j+1] = (uint8_t) ((pixel & 0x0000FF00) >> 8);
			w[j+2] = (uint8_t) ((pixel & 0x000000FF));
			uint8_t alpha = (uint8_t) ((pixel & 0xFF000000) >> 24);
			if (alpha > ((uint8_t) 32)){
				w[j+3] = alpha;
			}else{
				w[j+3] = ((uint8_t) 0);
			}
			j += 4;
		}
	}
	img.instance();
	img->create(layer->width, layer->height, false, Image::FORMAT_RGBA8, data);
	return img;
}

Ref<Image> Layer::read_atlas(_psd_layer_record *layer, ImgData &atlas_img){
	Ref<Image> img;
	{
		PoolVector<uint8_t> data = atlas_img.img->get_data();
		int j = data.size();
		data.resize(data.size() + data.size() / atlas_img.count.width / atlas_img.count.height);
		PoolVector<uint8_t>::Write w = data.write();
		atlas_img.count.height += 1;

		Point2 offset = Point2(layer->left - atlas_img.img_rect.position.x, layer->top - atlas_img.img_rect.position.y);
		Point2 size = Point2(atlas_img.img_rect.position.x - layer->right, atlas_img.img_rect.position.y - layer->bottom);
		int x = -1;
		int y = 0;
		int len = data.size();
		int p = 0;
		p += MAX(layer->width * (-offset.y), 0);
		p += MAX(-offset.x, 0);
		psd_argb_color *pixels = layer->image_data;
		while(true){
			x++;
			if (x == atlas_img.img->get_width()){
				if (y >= offset.y && y < size.y){
					p += MAX(-offset.x, 0);
					p += MAX(layer->right - (atlas_img.img_rect.position.x + atlas_img.img_rect.size.width), 0);
				}
				x = 0;
				y++;
			}
			if (y == atlas_img.img_rect.size.height){
				break;
			}
			if (x >= offset.x && y >= offset.y && x < size.x && y < size.y){
				unsigned int pixel = pixels[p];
				w[j] = (uint8_t) ((pixel & 0x00FF0000) >> 16);
				w[j+1] = (uint8_t) ((pixel & 0x0000FF00) >> 8);
				w[j+2] = (uint8_t) ((pixel & 0x000000FF));
				w[j+3] = (uint8_t) ((pixel & 0xFF000000) >> 24);
				j += 4;
				p++;
			}
			else{
				w[j] = (uint8_t) 0;
				w[j+1] = (uint8_t) 0;
				w[j+2] = (uint8_t) 0;
				w[j+3] = (uint8_t) 0;
				j+=4;
			}
		}
		img.instance();
		img->create(atlas_img.img_rect.size.width*atlas_img.count.width, atlas_img.img_rect.size.height*atlas_img.count.height, false, Image::FORMAT_RGBA8, data);
	}
	return img;
}

Ref<Image> Layer::read_rim(_psd_layer_record *layer, ImgData &atlas_img){
	Ref<Image> img;
	int target_width = atlas_img.img_rect.size.width;
	int target_height = atlas_img.img_rect.size.height;
	PoolVector<uint8_t> data;
	int data_size = target_width * target_height * 4;
	data.resize(data_size);
	PoolVector<uint8_t>::Write w = data.write();
	Point2 offset = Point2(layer->left - atlas_img.img_rect.position.x, layer->top - atlas_img.img_rect.position.y);
	Point2 size = Point2(layer->right - atlas_img.img_rect.position.x, layer->bottom - atlas_img.img_rect.position.y);
	
	int j = 0;
	int x = -1;
	int y = 0;
	
	int p = 0;

	p += MAX(layer->width * (-offset.y), 0);
	p += MAX(-offset.x, 0);
	float p_add = MAX(-offset.x, 0) + MAX(layer->right - (atlas_img.img_rect.position.x + atlas_img.img_rect.size.width), 0);
	psd_argb_color *pixels = layer->image_data;
	while(true){
		x++;
		if (x == target_width){
			if (y >= offset.y && y < size.y){
				p += p_add;
			}
			x = 0;
			y++;
		}
		if (y == target_height || j >= data_size - 3){
			break;
		}
		if (x >= offset.x && y >= offset.y && x < size.x && y < size.y){
			unsigned int pixel = pixels[p];
			w[j] = (uint8_t) ((pixel & 0x00FF0000) >> 16);
			w[j+1] = (uint8_t) ((pixel & 0x0000FF00) >> 8);
			w[j+2] = (uint8_t) ((pixel & 0x000000FF));
			w[j+3] = (uint8_t) ((pixel & 0xFF000000) >> 24);
			j += 4;
			p++;
		}
		else{
			w[j] = (uint8_t) 0;
			w[j+1] = (uint8_t) 0;
			w[j+2] = (uint8_t) 0;
			w[j+3] = (uint8_t) 0;
			j += 4;
		}
	}
	img.instance();
	img->create(atlas_img.img_rect.size.width, atlas_img.img_rect.size.height, false, Image::FORMAT_RGBA8, data);
	return img;
}

void Layer::load_clipped_layers(_psd_context *context, int start, ImgData &atlas, bool load_layer_data){
	int end = context->layer_count;
	psd_layer_record *layers = context->layer_records;
	for (int iter = start; iter < end; iter++){	
		_psd_layer_record *layer = &layers[iter];
		if (layer->layer_type != psd_layer_type::psd_layer_type_normal){
			continue;
		}
		if (!layer->clipping){
			break;
		}
		int len = layer->width * layer->height;
		if (len == 0) 
			continue;
		String name = reinterpret_cast<char const*>(layer->layer_name);

		if (load_layer_data){
			if(name == "rim"){
				atlas.rim = read_rim(layer, atlas);
			}else{
				atlas.img = read_atlas(layer, atlas);
			}
		}
		else{
			if(name == "rim"){
				
			}else{
				atlas.count.height += 1;
			}
		}
	}
}

Rect2 Layer::get_crop(Ref<BitMap> bitmap, const Vector2 &grow){
	Size2 size = bitmap->get_size();
	int x_min = size.width;
	int x_max = 0;
	int y_min = size.height;
	int y_max = 0;
	Point2 xt = Point2(0.f, 0.f);
	for (int x = 0; x < size.width; x++) {
		xt.x = x;
		for (int y = 0; y < size.height; y++) {
			xt.y = y;
			if (bitmap->get_bit(xt)){
				x_min = MIN(x_min, x - grow.x);
				x_max = MAX(x_max, x + grow.x);
				y_min = MIN(y_min, y - grow.y);
				y_max = MAX(y_max, y + grow.y);
			}
		}
	}
	x_min = MAX(x_min, 0);
	x_max = MIN(x_max, bitmap->get_size().x - 1);
	y_min = MAX(y_min, 0);
	y_max = MIN(y_max, bitmap->get_size().y - 1);
	return Rect2(x_min, y_min, x_max - x_min + 1, y_max - y_min + 1);
}

void Layer::apply_scale(ImgData &atlas_img, double scale, int texture_scale_mode, int texture_min_size, const Size2 &texture_max_size){
	Size2 n(atlas_img.img->get_width()/atlas_img.count.width, atlas_img.img->get_height()/atlas_img.count.height);
	if (scale != 0)
		n = n * scale;
	switch (texture_scale_mode)
	{
	case DOWNSCALE:
		n.width = previous_power_of_2(n.width);
		n.height = previous_power_of_2(n.height);
		break;
	case UPSCALE:
		n.width = next_power_of_2(n.width);
		n.height = next_power_of_2(n.height);
		break;
	case CLOSEST:
		n.width = closest_power_of_2(n.width);
		n.height = closest_power_of_2(n.height);
		break;
	default:
		break;
	}
	n.width = CLAMP(n.width * atlas_img.count.width, texture_min_size, texture_max_size.width);
	n.height = CLAMP(n.height * atlas_img.count.height, texture_min_size, texture_max_size.height);
	if (n.width != atlas_img.img->get_width() || n.height != atlas_img.img->get_height())
		atlas_img.img->resize(n.width, n.height, Image::INTERPOLATE_LANCZOS);
}

void Layer::apply_scale(Ref<Image> img, double scale, int texture_scale_mode, int texture_min_size, const Size2 &texture_max_size){
	Size2 n(img->get_width(), img->get_height());
	if (scale != 0)
		n = n * scale;
	switch (texture_scale_mode)
	{
	case DOWNSCALE:
		n.width = previous_power_of_2(n.width);
		n.height = previous_power_of_2(n.height);
		break;
	case UPSCALE:
		n.width = next_power_of_2(n.width);
		n.height = next_power_of_2(n.height);
		break;
	case CLOSEST:
		n.width = closest_power_of_2(n.width);
		n.height = closest_power_of_2(n.height);
		break;
	default:
		break;
	}
	n.width = CLAMP(n.width, texture_min_size, texture_max_size.width);
	n.height = CLAMP(n.height, texture_min_size, texture_max_size.height);
	if (n.width != img->get_width() || n.height != img->get_height())
		img->resize(n.width, n.height, Image::INTERPOLATE_LANCZOS);
}

void Layer::apply_border(Ref<Image> img, float p_threshold){
	int new_width = img->get_width();
	int new_height = img->get_height();
	int last = new_height - 1;
	img->lock();
	for (int x = 0; x < new_width; x++) {
		Color pixel = img->get_pixel(x, 0);
		if (pixel.a < p_threshold){
			pixel.a = 0.0;
			img->set_pixel(x, 0, pixel);
		}
		pixel = img->get_pixel(x, last);
		if (pixel.a < p_threshold){
			pixel.a = 0.0;
			img->set_pixel(x, last, pixel);
		}
	}
	last = new_width - 1;
	for (int y = 0; y < new_height; y++) {
		Color pixel = img->get_pixel(0, y);
		if (pixel.a < p_threshold){
			pixel.a = 0.0;
			img->set_pixel(0, y, pixel);
		}
		pixel = img->get_pixel(last, y);
		if (pixel.a < p_threshold){
			pixel.a = 0.0;
			img->set_pixel(last, y, pixel);
		}
	}
	img->unlock();
}

void Layer::_bind_methods() {
	ClassDB::bind_method("_texture", &Layer::_texture);
	ClassDB::bind_method("load", &Layer::load);
}

String ResourceImporterPSD::get_importer_name() const {

	return "psd_frame";
}

String ResourceImporterPSD::get_visible_name() const {

	return "PSD file";
}
void ResourceImporterPSD::get_recognized_extensions(List<String> *p_extensions) const {

	p_extensions->push_back("psd");
}

String ResourceImporterPSD::get_save_extension() const {
	return "";
}


String ResourceImporterPSD::get_resource_type() const {

	return "PSD";
}

bool ResourceImporterPSD::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {

	return true;
}

int ResourceImporterPSD::get_preset_count() const {
	return 0;
}
String ResourceImporterPSD::get_preset_name(int p_idx) const {

	return String();
}

void ResourceImporterPSD::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "main/scene_width"), 1000.0f));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "main/canvas_width"), 2048));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/import"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/update"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "main/update_only_editor"), true));

	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/texture"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/page_height"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/frame_targets"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/folder_mask"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "update/order"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "update/folder_pos", PROPERTY_HINT_ENUM, "Ignore, Move, Move layers"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "update/layer_pos", PROPERTY_HINT_ENUM, "Ignore, Move and scale, Reset UV, Move UV, Move and reset UV, Move and move UV"), 3));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/first_level", PROPERTY_HINT_ENUM, "Top left, Center"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/second_level", PROPERTY_HINT_ENUM, "Top left, Center"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "anchor/folder", PROPERTY_HINT_ENUM, "Top left, Center"), 1));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/root", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame"), 2));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/first_level", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 4));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/second_level", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 1));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "type/folder", PROPERTY_HINT_ENUM, "Node2D, Parallax, Page, Frame, Frame external"), 0));

	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/max_size"), 1024));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/scale", PROPERTY_HINT_ENUM, "Original, Downscale, Upscale, Closest"), 2));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "texture/bitmap_size"), 128));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "texture/alpha_grow"), 0.005));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "texture/polygon_grow"), 0.005));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "texture/alpha_to_polygon"), true));

	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/default", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/default.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/default_mul", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/default_mul.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/default_add", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/default_add.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/default_sub", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/default_sub.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/rim", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/default/rim.material")));
	r_options->push_back(ImportOption(PropertyInfo(Variant::OBJECT, "materials/frame_outline", PROPERTY_HINT_RESOURCE_TYPE, "Material"), ResourceLoader::load("res://redot/materials/frame_outline.material")));
}

Ref<Image> ResourceImporterPSD::read_mask(_psd_layer_record *layer){
	PoolVector<uint8_t> data;
	int len = layer->layer_mask_info.width * layer->layer_mask_info.height;
	if (layer->layer_mask_info.width < 2 || layer->layer_mask_info.height < 2){
		Ref<BitMap> bitmap;
		return bitmap;
	}
	data.resize(len * 2);
	PoolVector<uint8_t>::Write w = data.write();
	int j = 0;
	for (int i = 0; i < len; i++) {
		uint8_t alpha = (uint8_t)((char)(layer->layer_mask_info.mask_data[i]));
		w[j] = alpha;
		w[j+1] = alpha;
		j+=2;
	}
	Ref<Image> img;
	img.instance();
	img->create(layer->layer_mask_info.width, layer->layer_mask_info.height, false, Image::FORMAT_LA8, data);
	return img;
}

void ResourceImporterPSD::_bind_methods() {

}

Node *ResourceImporterPSD::get_edited_scene_root(const String &p_path) const{
	EditorData &editor_data = EditorNode::get_editor_data();
	for (int i = 0; i < editor_data.get_edited_scene_count(); i++) {
		if (editor_data.get_scene_path(i) == p_path)
			return editor_data.get_edited_scene_root(i);
	}
	return nullptr;
}

Node *ResourceImporterPSD::_get_root(_psd_context *context, const String &scene_path, bool &force_save, const Map<StringName, Variant> &p_options) const{
	_psd_layer_record *last_layer = &(context->layer_records[context->layer_count-1]);
	Node *root = nullptr;
	bool b_update = p_options["main/update"];
	bool b_update_only_editor = p_options["main/update_only_editor"];
	_File file_сheck;
	if (file_сheck.file_exists(scene_path)){
		force_save = false;
		if (!b_update){
			return nullptr;
		}
	}
	else{
		red::scene_loader(scene_path);
	}
	if (!EditorNode::get_singleton()->is_scene_open(scene_path)){
		EditorNode::get_singleton()->load_scene(scene_path);
	}
	root = get_edited_scene_root(scene_path);
	return root;
}

void Materials::init(const Map<StringName, Variant> &p_options, Node *node){
	Vector<Node*> nodes;
	red::get_all_children(node, nodes);
	int count = nodes.size();

	frame_outline = p_options["materials/frame_outline"];
	material = p_options["materials/default"];
	material_mul = p_options["materials/default_mul"];
	material_add = p_options["materials/default_add"];
	material_sub = p_options["materials/default_sub"];

	rim = p_options["materials/rim"];
}

void Materials::apply_material(const _psd_layer_record *layer, Node2D *node, ImgData &atlas_img){
	switch(layer->blend_mode){
		case psd_blend_mode_multiply:{
			if (material_mul.is_valid())
				node->set_material(material_mul);
			break;
		}
		case psd_blend_mode_linear_dodge:{
			if (material_add.is_valid())
				node->set_material(material_add);
			break;
		}
		case psd_blend_mode_difference:{
			if (material_sub.is_valid())
				node->set_material(material_sub);
			break;	
		}
		default:{
			if (atlas_img.count.height == 2 && rim.is_valid()){
				node->set_material(rim);
			}
			else{
				if (material.is_valid())
					node->set_material(material);
			}
			break;
		}
	}
}

void ResourceImporterPSD::normal_to_polygon(Node *apply_polygon, const String &normal_path){
	Polygon2D *p = Object::cast_to<Polygon2D>(apply_polygon);
	if(p){
		Ref<Texture> normal = ResourceLoader::load(normal_path, "Texture");
		if (normal.is_valid()){
			p->set_normalmap(normal);
		}
	}
}

void ResourceImporterPSD::render_normal(Ref<BitMap> bitmap, String &diffuse_path, String &normal_path, Size2 &resolution, Ref<Image> rim_image, Polygon2D *apply_polygon){
	Node *normal_data_parent = EditorNode::get_editor_data().get_edited_scene_root();
	REDRenderData *normal_data = red::create_node<REDRenderData>(normal_data_parent, String("generator_container"), nullptr, true);
	normal_data->set_resolution(resolution);
	normal_data->set_use_background(true);
	normal_data->set_render_path(normal_path);
	normal_data->set_delete_after_render(true);
	{
		Vector<Polygon2D*> polygons = red::bitmap_to_polygon2d(bitmap, resolution, 1.0f, 3.0f, false, true);
		red::print(polygons.size());
		for (int i = 0; i < polygons.size(); i++){
			Polygon2D* p = polygons[i];
			Ref<ShaderMaterial> material;
			material.instance();
			material->set_shader(ResourceLoader::load("res://redot/shaders/editor/normal_baker/mesh.shader", "Shader"));
			p->set_material(material);
			Ref<Texture> texture = ResourceLoader::load(diffuse_path, "Texture");
			if (texture.is_valid())
				p->set_texture(texture);
			normal_data->add_child(polygons[i]);
			polygons[i]->set_owner(normal_data->get_owner());
		}
	}
	{
		Array materials;
		for (int i = 0; i < 3; i++){
			String shader_path;
			switch (i){
			case 0: shader_path = "res://redot/shaders/editor/normal_baker/back.shader"; break;
			case 1: shader_path = "res://redot/shaders/editor/normal_baker/pass1.shader"; break;
			default: shader_path = "res://redot/shaders/editor/normal_baker/pass2.shader"; break;
			}
			Ref<Shader> shader = ResourceLoader::load(shader_path, "Shader");
			Ref<ShaderMaterial> material;
			material.instance();
			material->set_shader(shader);
			materials.push_back(material);
		}
		Ref<Texture> texture = ResourceLoader::load(diffuse_path, "Texture");
		if (texture.is_valid()){
			Ref<ShaderMaterial> m = materials[0];
			m->set_shader_param("tex", texture);
		}
		if (rim_image.is_valid()){
			Ref<ShaderMaterial> m = materials[2];
			Ref<ImageTexture> rim_texture;
			rim_texture.instance();
			rim_texture->create_from_image(rim_image);
			m->set_shader_param("tex", rim_texture);
			m->set_shader_param("use_texture", true);
		}
		normal_data->set_materials(materials);
	}
	int light_mask = rim_image.is_valid() ? 2 : 1;
	if(apply_polygon){
		apply_polygon->set_light_mask(rim_image.is_valid() ? 2 : 1);
	}
	normal_data->connect("rendered", this, "normal_to_polygon", varray(apply_polygon, normal_path), CONNECT_ONESHOT);
	normal_data->render();
}

int ResourceImporterPSD::load_folder(_psd_context *context, const String &scene_path, int start, Materials &materials, 
									 Node *parent, Vector2 parent_pos, Vector2 parent_offset, const Map<StringName, Variant> &p_options, 
									 bool force_save, int counter, int folder_level, REDClipper *parent_clipper){
	psd_layer_record *layers = context->layer_records;
	int count = 0;
	int offset = 0;
	int final_iter = 0;
	int end = context->layer_count;
	bool loop = true;

	Size2 context_size = Size2(context->width, context->height);
	double context_aspect_ratio = context->height / context->width;
	Size2 scene_size = Size2(p_options["main/scene_width"], 0.0f);
	double scene_k = (scene_size.width * 1.0) / context_size.width;
	scene_size.height = context->height * scene_k;
	Size2 canvas_size = Size2(p_options["main/canvas_width"], 0.0f);
	canvas_size.y = canvas_size.x * context_aspect_ratio;
	double image_resize = (canvas_size.width * 1.0) / context->width;
	double image_size_to_scene_size = (scene_size.width * 1.0) / context->width;

	bool b_update = p_options["main/update"];
	bool b_update_only_editor = p_options["main/update_only_editor"];
	bool updateble = true;
	if (parent->get_owner())
		updateble = b_update && (!b_update_only_editor || EditorNode::get_singleton()->is_scene_open(parent->get_owner()->get_filename()));
	else
		updateble = b_update && (!b_update_only_editor || EditorNode::get_singleton()->is_scene_open(parent->get_filename()));
	bool saveble = force_save || (!b_update_only_editor && b_update);
	Vector<String> names;
	for (int iter = start; iter + offset < end && loop; iter++)
		names.push_back(reinterpret_cast<char const*>(layers[iter + offset].layer_name));
	String target_dir = scene_path.get_basename().get_base_dir();
	red::create_dir(target_dir);
	_File file_сheck;
	bool first = true;
	for (int iter = start; iter + offset < end && loop; iter++){	
		final_iter = iter + offset; count++;
		_psd_layer_record *layer = &layers[final_iter];
		if (layer->clipping && !first)
			continue;
		first = false;
		if (!layer->visible && psd_layer_type::psd_layer_type_normal)
			continue;
		switch (layer->layer_type){
			case psd_layer_type::psd_layer_type_folder:{
				if (counter > 0)
					loop = false;
			} break;
			case psd_layer_type::psd_layer_type_normal:{
				Layer *l = memnew(Layer);
				l->p_options = p_options;
				l->context = context;
				l->final_iter = final_iter;
				l->iter = iter;
				l->start = start;

				l->target_dir = target_dir;
				l->updateble = updateble;
				l->materials = materials;
				l->names = names;

				l->parent = parent;
				l->parent_clipper = parent_clipper;
				l->parent_pos = parent_pos;
				l->parent_offset = parent_offset;

				l->context_size = context_size;
				l->context_aspect_ratio = context_aspect_ratio;
				l->scene_size = scene_size;
				l->scene_k = scene_k;
				l->canvas_size = canvas_size;
				l->image_resize = image_resize;
				l->image_size_to_scene_size = image_size_to_scene_size;
				l->load();
				memdelete(l);
			} break;
			case psd_layer_type::psd_layer_type_hidden:{
				_psd_layer_record *layer_folder;
				{
					int child_folder_count = 0;
					int child_end_count = 0;
					for (int j = final_iter + 1; j < end; j++){
						if (layers[j].layer_type == psd_layer_type::psd_layer_type_hidden)
							child_folder_count++;
						if (layers[j].layer_type == psd_layer_type::psd_layer_type_folder)
							child_end_count++;
						if (child_end_count > child_folder_count){
							layer_folder = &layers[j];
							break;
						}
					}
				}
				String folder_name = reinterpret_cast<char const*>(layer_folder->layer_name);
				int new_folder_level = folder_level + 1;
				int mode = 0;
				int anchor = 0;
				switch (new_folder_level)
				{
				case 0:
					mode = p_options["type/root"];
					anchor = ANCHOR_TOP_LEFT;
					break;
				case 1:
					mode = p_options["type/first_level"];
					anchor = p_options["anchor/first_level"];
					break;
				case 2:
					mode = p_options["type/second_level"];
					anchor = p_options["anchor/second_level"];
					break;
				default:
					mode = p_options["type/folder"];
					anchor = p_options["anchor/folder"];
					break;
				}
				if (folder_name == ""){
					folder_name = "root";
				}
				else{
					if ((mode == FOLDER_FRAME_EXTERNAL || mode == FOLDER_FRAME) && folder_name.find("_internal") != -1){
						mode = FOLDER_FRAME;
						folder_name = folder_name.replace_first("_internal", "");
					}
					if ((mode == FOLDER_FRAME_EXTERNAL || mode == FOLDER_FRAME) && folder_name.find("_external") != -1){
						mode = FOLDER_FRAME_EXTERNAL;
						folder_name = folder_name.replace_first("_external", "");
					}
					if (folder_name.find("_center") != -1){
						anchor = ANCHOR_CENTER;
						folder_name = folder_name.replace_first("_center", "");
					}
				}
				String new_target_dir = mode != FOLDER_PAGE ? target_dir + "/" + folder_name : target_dir;
				String new_scene_path = new_target_dir + "/" + folder_name + ".tscn";
				bool new_force_save = false;
				
				bool need_create = parent->has_node(folder_name) ? false : true;
				bool external_frame_created = false;
				Node *new_parent = nullptr;
				Node2D *position_object = nullptr;
				REDPage *page = nullptr;
				REDFrame *frame = nullptr;
				REDClipper *clipper = parent_clipper;
				REDShapeRenderer *outline = nullptr;
				switch (mode){
					case FOLDER_NODE2D:{
						position_object = need_create ? red::create_node<Node2D>(parent, folder_name) : (Node2D*)parent->get_node(folder_name);
						new_parent = (Node*)position_object;
					} break;
					case FOLDER_PAGE:{
						page = need_create ? red::create_node<REDPage>(parent, folder_name) : (REDPage*)parent->get_node(folder_name);
						position_object = (Node2D*)page;
						new_parent = (Node*)position_object;
					} break;
					case FOLDER_FRAME:{
						frame = need_create ? red::create_node<REDFrame>(parent, folder_name) : (REDFrame*)parent->get_node(NodePath(folder_name));
						if(frame){
							clipper = red::create_node<REDClipper>(frame, "content");
							outline = red::create_node<REDShapeRenderer>(frame, "outline");
							position_object = (Node2D*)frame;
							new_parent = (Node*)clipper;
						}
					} break;
					case FOLDER_FRAME_EXTERNAL:{
						red::create_dir(new_target_dir);
						if (!file_сheck.file_exists(new_scene_path))
							new_force_save = true;
						red::scene_loader(new_scene_path);
						if (need_create){
							RES res = ResourceLoader::load(new_scene_path);
							Ref<PackedScene> scene = Ref<PackedScene>(Object::cast_to<PackedScene>(*res));
							Node *instanced_scene = scene->instance(PackedScene::GEN_EDIT_STATE_INSTANCE);
							instanced_scene->set_name(folder_name);
							parent->add_child(instanced_scene);
							instanced_scene->set_owner(parent->get_owner());
						}
						Node *root_node = nullptr;
						if (!EditorNode::get_singleton()->is_scene_open(new_scene_path)){
							EditorNode::get_singleton()->load_scene(new_scene_path);
						}
						root_node = get_edited_scene_root(new_scene_path);
						if (root_node == nullptr){
							Ref<PackedScene> scene = ResourceLoader::load(new_scene_path, "PackedScene");
							root_node = scene->instance();
						}
						if (!root_node->has_node(folder_name)){
							frame = red::create_node<REDFrame>(root_node, folder_name);
							external_frame_created = true;
						}
						else
							frame = (REDFrame*)root_node->get_node(folder_name);
						if(frame){
							clipper = red::create_node<REDClipper>(frame, "content");
							outline = red::create_node<REDShapeRenderer>(frame, "outline");
							position_object = (Node2D*)parent->get_node(folder_name);
							new_parent = (Node*)clipper;
						}
					} break;
					default:
					case FOLDER_PARALLAX:{
						position_object = need_create ? (Node2D*)red::create_node<REDParallaxFolder>(parent, folder_name) : (Node2D*)parent->get_node(folder_name);
						new_parent = (Node*)position_object;
					} break;
				}
				if (!position_object || (!frame && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL)) || (!page && (mode == FOLDER_PAGE))){
					ERR_PRINTS("Error can't create node " + folder_name);
					continue;
				}
				bool folder_mask = p_options["update/folder_mask"];
				bool frame_targets = p_options["update/frame_targets"];
				bool page_height = p_options["update/page_height"];
				int folder_pos = p_options["update/folder_pos"];
				bool apply_mask = (updateble && folder_mask) && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL) && frame;
				bool update_targets = (updateble && frame_targets) && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
				bool update_page_height = updateble && page_height && mode == FOLDER_PAGE;
				bool update_outline = false;
				bool update_frame_params = updateble && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
				bool update_pos_reset = updateble && folder_pos == FOLDER_POS_RESET;
				bool update_pos_move_layers = updateble && folder_pos == FOLDER_MOVE_LAYERS;
				if (need_create){
					apply_mask = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL) && frame;
					update_targets = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
					update_page_height = page_height && mode == FOLDER_PAGE;
					update_frame_params = (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL);
					update_outline = outline;
					update_pos_reset = true;
					update_pos_move_layers = false;
				}
				if (apply_mask){
					Ref<BitMap> bitmap_poly;
					Ref<Image> bitmap_img = read_mask(layer_folder);
					Size2 image_size;
					Size2 polygon_size = image_size * scene_k;
					if (bitmap_img.is_valid())
						image_size = bitmap_img->get_size();
					if (polygon_size.width > 0 && polygon_size.height > 0)
						bitmap_poly = red::read_bitmap(bitmap_img, 0.2f, Vector2(MIN(128, image_size.x), MIN(128, image_size.y)));
					else
						polygon_size = scene_size;
					frame->set_polygon(red::bitmap_to_polygon(bitmap_poly, polygon_size, 0.0, 4.0, true)[0]);
				}
				Vector2 half_frame = (frame) ? frame->_edit_get_rect().size / 2.0 : Vector2(0,0);
				Vector2 frame_anchor_offset = (anchor == ANCHOR_CENTER) ? -half_frame : Vector2(0,0);
				Vector2 local_pos = Vector2(layer_folder->layer_mask_info.left * scene_k, 
											layer_folder->layer_mask_info.top * scene_k);
				Vector2 new_parent_pos = parent_pos + local_pos;
				Vector2 new_parent_offset = -frame_anchor_offset;
				if (update_pos_reset){
					if (frame && (mode == FOLDER_FRAME || mode == FOLDER_FRAME_EXTERNAL)){
						frame->set_offset(frame_anchor_offset);
						frame->set_position(-frame_anchor_offset);
					}
					if (position_object && mode != FOLDER_FRAME_EXTERNAL){
						position_object->set_position(local_pos - parent_pos - parent_offset - frame_anchor_offset);
					}
				}
				else if (update_pos_move_layers){
					if (mode == FOLDER_FRAME_EXTERNAL){
						new_parent_offset += position_object->get_position();// - frame_anchor_offset;
					}
					else{
						new_parent_offset -= (local_pos - parent_pos - parent_offset - frame_anchor_offset) - position_object->get_position();
					}
				}
				if (update_page_height){
					page->set_size(scene_size);
				}
				if (update_frame_params){
					parent->move_child(position_object, parent->get_child_count() - 1);
					if (frame){
						Node2D *targets = red::create_node<Node2D>(frame, "targets");
						if(targets){
							REDTarget *camera_pos = red::create_node<REDTarget>(targets, "camera");
							REDTarget *camera_pos_in = red::create_node<REDTarget>(targets, "camera_in");
							REDTarget *camera_pos_out = red::create_node<REDTarget>(targets, "camera_out");
							REDTarget *parallax_pos = red::create_node<REDTarget>(targets, "parallax");
							REDTarget *parallax_pos_in = red::create_node<REDTarget>(targets, "parallax_in");
							REDTarget *parallax_pos_out = red::create_node<REDTarget>(targets, "parallax_out");
							frame->set_camera_pos_path(frame->get_path_to(camera_pos));
							frame->set_camera_pos_in_path(frame->get_path_to(camera_pos_in));
							frame->set_camera_pos_out_path(frame->get_path_to(camera_pos_out));
							frame->set_parallax_pos_path(frame->get_path_to(parallax_pos));
							frame->set_parallax_pos_in_path(frame->get_path_to(parallax_pos_in));
							frame->set_parallax_pos_out_path(frame->get_path_to(parallax_pos_out));
							camera_pos->set_position(half_frame + frame_anchor_offset);
							camera_pos_in->set_position(half_frame + frame_anchor_offset);
							camera_pos_out->set_position(half_frame + frame_anchor_offset);
							parallax_pos->set_position(half_frame + frame_anchor_offset);
							parallax_pos_in->set_position(half_frame + frame_anchor_offset);
							parallax_pos_out->set_position(half_frame + frame_anchor_offset);
						}
					}
				}
				if(update_outline){
					outline->set_render_mode(REDShapeRenderer::RENDER_LINE);
					outline->set_color(Color(0,0,0,1));
					outline->set_texture(ResourceLoader::load("res://redot/textures/outline/16.png", "Texture"));
					//outline->set_material(materials.frame_outline);
				}
				if (new_parent != nullptr)
					offset += load_folder(context, new_scene_path, final_iter+1, materials, new_parent, new_parent_pos, new_parent_offset, p_options, new_force_save, counter+1, new_folder_level, clipper);
				else
					ERR_PRINTS("Error can't create node " + folder_name);
			} break;
			default:
				break;
		}
		
	}
	if (saveble){
		int mode = 0;
		if (folder_level == 0){
			mode = p_options["type/root"];
		}
		else if (folder_level == 1){
			mode = p_options["type/first_level"];
		}
		else if (folder_level == 2){
			mode = p_options["type/second_level"];
		}
		else{
			mode = p_options["type/folder"];
		}
		if(folder_level == -1 || (mode == FOLDER_FRAME_EXTERNAL)){
			Ref<PackedScene> scene = memnew(PackedScene);
			Node *owner = parent->get_owner();
			if (!owner){
				owner = parent;
			}
			Error err = scene->pack(owner);
			if (err == OK){
				err = ResourceSaver::save(scene_path, scene);
			}
		}
	}
	return count + offset;
}

Error ResourceImporterPSD::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	bool b_import = p_options["main/import"];
	if (!b_import)
		return OK;
	ERR_FAIL_COND_V_MSG(!ProjectSettings::get_singleton(), OK, "Error can't read path: " + p_source_file);
	char *global = _strdup(ProjectSettings::get_singleton()->globalize_path(p_source_file).utf8().get_data());
	String scene_path = p_source_file.get_basename() + ".tscn";
	_psd_context * context = NULL;
	psd_status status = psd_image_load(&context, global);
	ERR_FAIL_COND_V_MSG(status != psd_status::psd_status_done, ERR_FILE_CANT_READ, 
						"Error can't read file: " + ProjectSettings::get_singleton()->globalize_path(p_source_file));
	bool force_save = true;
	Node *parent = _get_root(context, scene_path, force_save, p_options);
	ERR_FAIL_NULL_V(parent, OK);
	Materials mats;
	mats.init(p_options, parent);
	
	load_folder(context, scene_path, 0, mats, parent, Vector2(0,0), Vector2(0,0), p_options, force_save, 0);
	psd_image_free(context);
	return OK;
}

ResourceImporterPSD::ResourceImporterPSD() {
}
