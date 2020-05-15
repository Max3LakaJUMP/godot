#ifndef RED_LINE_BUILDER_H
#define RED_LINE_BUILDER_H

#include "core/color.h"
#include "core/math/vector2.h"
#include "red_line.h"
#include "scene/resources/gradient.h"

class REDLineBuilder {
public:

	// TODO Move in a struct and reference it
	// Input
	Vector<Vector2> points;
	REDLine::LineJointMode joint_mode;
	REDLine::LineCapMode begin_cap_mode;
	REDLine::LineCapMode end_cap_mode;
	float width;
	Curve *width_curve;
	Color default_color;
	Gradient *gradient;
	REDLine::LineTextureMode texture_mode;
	float sharp_limit;
	int round_precision;
	float tile_aspect; // w/h
	// TODO offset_joints option (offers alternative implementation of round joints)

	// TODO Move in a struct and reference it
	// Output
	Vector<Vector2> vertices;
	Vector<Color> colors;
	Vector<Vector2> uvs;
	Vector<int> indices;

	REDLineBuilder();
    Vector<float> width_list;
    bool is_closed;
	void build();
	void clear_output();

private:
	enum Orientation {
		UP = 0,
		DOWN = 1
	};

	// Triangle-strip methods
	void strip_begin(Vector2 up, Vector2 down, Color color, float uvx);
	void strip_new_quad(Vector2 up, Vector2 down, Color color, float uvx);
	void strip_add_quad(Vector2 up, Vector2 down, Color color, float uvx);
	void strip_add_tri(Vector2 up, Orientation orientation);
	void strip_add_arc(Vector2 center, float angle_delta, Orientation orientation);

	void new_arc(Vector2 center, Vector2 vbegin, float angle_delta, Color color, Rect2 uv_rect);
private:
	bool _interpolate_color;
	int _last_index[2]; // Index of last up and down vertices of the strip
};

#endif // RED_LINE_BUILDER_H
