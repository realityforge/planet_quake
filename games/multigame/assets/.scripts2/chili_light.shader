textures/chili_light/kerze
{
	qer_editorimage textures/chili_light/kerze.tga
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm trans
	cull disable
	deformVertexes autosprite2

	{
		clampmap textures/chili_light/kerze.tga
		blendfunc add
	}
	{
		clampmap textures/chili_light/kerzenschein.tga
		blendfunc add
		rgbGen wave noise 0.7 0.1 0.2 -8
	}
}

textures/chili_light/light_base
{
	qer_editorimage textures/chili_light/light_base.tga
	surfaceparm nolightmap
	surfaceparm nomarks

	{
		map textures/chili_light/light_base.tga
		rgbGen identity
	}
}

textures/chili_light/light_blue
{
	qer_editorimage textures/chili_light/light_blue.tga
	surfaceparm nomarks
	q3map_surfacelight 1500

	{
		map textures/chili_light/light_blue.tga
		rgbGen identity
	}
	{
		map textures/chili_light/light_blue.tga
		blendfunc add
	}
}

textures/chili_light/lightclip
{
	qer_editorimage textures/colors/nopass.tga
	surfaceparm nolightmap
	surfaceparm nonsolid
	surfaceparm nomarks

	{
		map textures/colors/black.tga
		blendfunc add
	}
}

textures/chili_light/neonroerhe
{
	qer_editorimage textures/chili_light/bulb.tga
	surfaceparm nomarks

	{
		map textures/effects/tinfx.tga
		rgbGen identity
		tcGen environment
	}
	{
		map textures/chili_light/bulb.tga
		blendfunc blend
	}
}

textures/chili_light/neonroerhe_flackernd
{
	qer_editorimage textures/chili_light/bulb.tga
	surfaceparm nomarks
	q3map_surfacelight 500

	{
		map textures/chili_sfx/black.tga
		rgbGen identity
		tcGen environment
	}
	{
		map textures/chili_light/bulb.tga
		blendfunc blend
		rgbGen wave triangle 0.5 0.05 0 10
	}
	{
		map textures/chili_light/bulb.tga
		blendfunc add
		rgbGen wave triangle -0.5 -0.05 0 -10
	}
}

textures/chili_light/pulse_blue
{
	q3map_lightimage textures/chili_light/lightimage_blue.tga
	q3map_surfacelight 2000

	{
		map textures/chili_light/pulse_blue.tga
		tcMod scale 0.035 1
		tcMod scroll -0.65 0
	}
}

textures/chili_light/stained_glass_blue
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

textures/chili_light/stained_glass_red
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

textures/chili_light/white
{
	qer_editorimage textures/colors/white.tga
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm trans

	{
		map textures/colors/white.tga
	}
}
