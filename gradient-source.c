#include <obs-module.h>
#include "gradient-source.h"
#include "version.h"

struct gradient_info {
	obs_source_t *source;
	uint32_t cx;
	uint32_t cy;
	gs_texrender_t *render;
	bool rendered;
};

static const char *gradient_get_name(void *type_data)
{
	UNUSED_PARAMETER(type_data);
	return obs_module_text("Gradient");
}

static void gradient_update(void *data, obs_data_t *settings)
{
	struct gradient_info *context = data;
	context->cx = (uint32_t)obs_data_get_int(settings, "width");
	context->cy = (uint32_t)obs_data_get_int(settings, "height");
	struct vec4 from_color;
	vec4_from_rgba(&from_color, (uint32_t)obs_data_get_int(settings, "from_color"));
	from_color.w =
		(float)(obs_data_get_double(settings, "from_opacity") / 100.0);
	struct vec4 to_color;
	vec4_from_rgba(&to_color, (uint32_t)obs_data_get_int(settings, "to_color"));
	to_color.w =
		(float)(obs_data_get_double(settings, "to_opacity") / 100.0);
	
	obs_enter_graphics();
	if (!context->render) {
		context->render = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	} else {
		gs_texrender_reset(context->render);
	}
	if (gs_texrender_begin(context->render, context->cx, context->cy)) {
		gs_blend_state_push();
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
		gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
		gs_eparam_t *color =
			gs_effect_get_param_by_name(solid, "color");
		gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");	
		gs_effect_set_vec4(color, &from_color);
		gs_technique_begin(tech);
		gs_technique_begin_pass(tech, 0);
		struct vec4 cur_color;
		for (uint32_t i = 0; i<context->cx; i++) {
			cur_color.x = from_color.x * (1.0f - ((float)i / (float)context->cx)) + to_color.x * ((float)i / (float)context->cx);
			cur_color.y = from_color.y * (1.0f - ((float)i / (float)context->cx)) + to_color.y * ((float)i / (float)context->cx);
			cur_color.z = from_color.z * (1.0f - ((float)i / (float)context->cx)) + to_color.z * ((float)i / (float)context->cx);
			cur_color.w = from_color.w * (1.0f - ((float)i / (float)context->cx)) + to_color.w * ((float)i / (float)context->cx);
			gs_effect_set_vec4(color, &cur_color);
			gs_draw_sprite(0, 0, 1, context->cy);
			gs_matrix_translate3f(1.0f, 0.0f, 0.0f);			
		}
		gs_technique_end_pass(tech);
		gs_technique_end(tech);
		gs_texrender_end(context->render);
		gs_blend_state_pop();
	}
	obs_leave_graphics();
	
}

static void *gradient_create(obs_data_t *settings, obs_source_t *source)
{
	struct gradient_info *context = bzalloc(sizeof(struct gradient_info));
	context->source = source;
	gradient_update(context, settings);
	return context;
}

static void gradient_destroy(void *data)
{
	struct gradient_info *context = data;
	bfree(context);
}

static void gradient_video_render(void *data, gs_effect_t *effect)
{
	struct gradient_info *context = data;

	if (!context->render)
		return;
	gs_texture_t *tex = gs_texrender_get_texture(context->render);
	gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"),
			      tex);
	gs_draw_sprite(tex, 0, context->cx, context->cy);
}

static obs_properties_t *gradient_properties(void *data)
{
	obs_properties_t *ppts = obs_properties_create();
	obs_property_t *p = obs_properties_add_color(
		ppts, "from_color", obs_module_text("FromColor"));

	p = obs_properties_add_float_slider(ppts, "from_opacity",
					    obs_module_text("Opacity"), 0.0,
					    100.0, 1.0);
	obs_property_float_set_suffix(p, "%");
	p = obs_properties_add_color(ppts, "to_color",
				     obs_module_text("ToColor"));
	p = obs_properties_add_float_slider(ppts, "to_opacity",
					    obs_module_text("Opacity"), 0.0,
					    100.0, 1.0);
	obs_property_float_set_suffix(p, "%");
	obs_properties_add_int(ppts, "width",
			       obs_module_text("Width"), 0, 4096,
			       1);

	obs_properties_add_int(ppts, "height",
			       obs_module_text("Height"), 0, 4096,
			       1);
	return ppts;
}

void gradient_defaults(obs_data_t *settings)
{
	obs_data_set_default_int(settings, "from_color", 0xFFD1D1D1);
	obs_data_set_default_double(settings, "from_opacity", 100.0);
	obs_data_set_default_int(settings, "to_color", 0xFF000000);
	obs_data_set_default_double(settings, "to_opacity", 100.0);
	obs_data_set_default_int(settings, "width", 1920);
	obs_data_set_default_int(settings, "height", 1080);
}
static uint32_t gradient_width(void *data)
{
	struct gradient_info *context = data;
	return context->cx;
}

static uint32_t gradient_height(void *data)
{
	struct gradient_info *context = data;
	return context->cy;
}

struct obs_source_info gradient_source = {
	.id = "gradient_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_OUTPUT_VIDEO,
	.get_name = gradient_get_name,
	.create = gradient_create,
	.destroy = gradient_destroy,
	.load = gradient_update,
	.update = gradient_update,
	.get_width = gradient_width,
	.get_height = gradient_height,
	.video_render = gradient_video_render,
	.get_properties = gradient_properties,
	.get_defaults = gradient_defaults,
	.icon_type = OBS_ICON_TYPE_COLOR,
};

OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR("Exeldro");
OBS_MODULE_USE_DEFAULT_LOCALE("gradient-source", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return obs_module_text("Description");
}

MODULE_EXPORT const char *obs_module_name(void)
{
	return obs_module_text("GradientSource");
}

bool obs_module_load(void)
{
	blog(LOG_INFO, "[Gradient Source] loaded version %s", PROJECT_VERSION);
	obs_register_source(&gradient_source);
	return true;
}
