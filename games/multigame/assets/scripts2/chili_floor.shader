textures/chili_floor/concrete_pool_floor
{
	qer_editorimage textures/base_wall/concrete_dark.tga

	{
		map textures/liquids/pool3d_5e.tga
		tcmod scale -.25 -.25
		tcmod scroll .025 .025
		rgbgen wave sin .75 0 0 0
	}
	{
		map textures/liquids/pool3d_6e.tga
		blendFunc GL_one GL_ONE
		tcmod scale .25 .25
		tcmod scroll .025 .025
		rgbgen wave sin .75 0 0 0
	}
	{
		map textures/base_wall/concrete_dark.tga
		blendFunc GL_one GL_src_color
		rgbgen identity
	}
	{
		rgbGen identity
		map $lightmap
		blendfunc gl_dst_color gl_zero
	}
}

textures/chili_floor/gfloor3
{
	qer_editorimage textures/chili_floor/gfloor.tga
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm alphashadow
	cull disable

	{
		map textures/chili_floor/gfloor.tga
		alphaFunc GE128
	}
}

textures/chili_floor/icemirror
{
	qer_editorimage textures/photorealistic_ground/snow009.tga
	surfaceparm nolightmap
	portal

	{
		map textures/chili_sfx/mirror.tga
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}
	{
		map textures/photorealistic_ground/snow009.tga
		blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
	}
}
