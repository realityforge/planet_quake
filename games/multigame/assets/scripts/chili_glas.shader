textures/chili_glas/glas_stainedblue_rgbGen
{
	qer_editorimage textures/chili_light/stained_glass_blue.tga
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm lightfilter

	{
		map textures/chili_light/stained_glass_blue.tga
		blendfunc filter
	}
	{
		map textures/chili_light/stained_glass_blue.tga
		blendfunc add
		rgbGen identity
	}
}

textures/chili_glas/glas_stainedred_rgbGen
{
	qer_editorimage textures/chili_light/stained_glass_red.tga
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm lightfilter

	{
		map textures/chili_light/stained_glass_red.tga
		blendfunc filter
	}
	{
		map textures/chili_light/stained_glass_red.tga
		blendfunc add
		rgbGen identity
	}
}

textures/chili_glas/glas_stainedblue_norgbGen
{
	qer_editorimage textures/chili_light/stained_glass_blue.tga
	surfaceparm nomarks

	{
		map textures/chili_light/stained_glass_blue.tga
		blendfunc filter
	}
}

textures/chili_glas/glas_stainedred_norgbGen
{
	qer_editorimage textures/chili_light/stained_glass_red.tga
	surfaceparm nomarks

	{
		map textures/chili_light/stained_glass_red.tga
		blendfunc filter
	}
}

textures/chili_glas/glass_nolightmap_dark
{
	qer_editorimage textures/base_wall/shiny3.tga
	surfaceparm trans
	cull none
	surfaceparm nolightmap
	qer_trans 0.5
	tesssize 128

	{
		map textures/effects/tinfx3.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
}

textures/chili_glas/glass_1
{
	qer_editorimage textures/sfx/cabletest2.tga
	surfaceparm trans
	cull front
	qer_trans 0.5
	tesssize 128

	{
		map textures/chili_sfx/rain_layer.tga
		tcMod scroll 0 -.5
		blendFunc add
	}
	{
		map textures/sfx/cabletest2.tga
		blendfunc blend
	}
	{
		map textures/chili_glas/envmap_2.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
	}
}

textures/chili_glas/glass_2
{
	qer_editorimage textures/chili_glas/envmap_3.tga
	surfaceparm trans
	cull none
	qer_trans 0.5
	tesssize 128

	{
		map textures/chili_glas/envmap_3.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
	}
}

textures/chili_glas/glass_2_rain
{
	qer_editorimage textures/sfx/cabletest2.tga
	surfaceparm trans
	cull none
	qer_trans 0.5
	tesssize 128

	{
		map textures/chili_glas/envmap_3.tga
		tcgen environment
		rgbGen identity
	}
	{
		map textures/chili_sfx/rain_layer.tga
		tcMod scroll 0 -.5
		blendFunc add
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
	}
}

textures/chili_glas/glass_3
{
	qer_editorimage textures/sfx/cabletest2.tga
	surfaceparm trans
	cull front
	qer_trans 0.5
	tesssize 128

	{
		map textures/chili_sfx/rain_layer.tga
		tcMod scroll 0 -.5
		blendFunc add
	}
	{
		map textures/sfx/cabletest2.tga
		blendfunc blend
	}
	{
		map textures/chili_glas/envmap_5.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
	}
}

textures/chili_glas/glass_4
{
	qer_editorimage textures/sfx/cabletest2.tga
	surfaceparm trans
	cull none
	qer_trans 0.5
	tesssize 128

	{
		map textures/sfx/cabletest2.tga
		blendfunc blend
	}
	{
		map textures/chili_glas/envmap_4.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
	}
}

textures/chili_glas/glass_5
{
	qer_editorimage textures/sfx/cabletest2.tga
	surfaceparm trans
	cull none
	qer_trans 0.5
	tesssize 128

	{
		map textures/chili_sfx/rain_layer.tga
		tcMod scroll 0 -.5
		blendFunc add
	}
	{
		map textures/sfx/cabletest2.tga
		blendfunc blend
	}
	{
		map textures/chili_glas/envmap_6.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
	}
}

textures/chili_glas/glass_nolightmap
{
	qer_editorimage textures/base_wall/shiny3.tga
	surfaceparm trans
	cull none
	surfaceparm nolightmap
	qer_trans 0.5
	tesssize 128

	{
		map textures/effects/tinfx.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
}

textures/chili_glas/glass_tess128
{
	qer_editorimage textures/base_wall/shiny3.tga
	surfaceparm trans
	cull none
	qer_trans 0.5
	tesssize 128

	{
		map textures/effects/tinfx2.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
	}
}

