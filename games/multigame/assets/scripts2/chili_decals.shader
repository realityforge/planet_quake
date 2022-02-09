textures/chili_decals/pool_filter
{
	qer_editorimage textures/chili_liquids/pool_green.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_tcGen ivector ( 256 0 0 ) ( 0 256 0 )
      q3map_nonplanar
	qer_trans 0.5
	polygonoffset

	{
		map textures/chili_liquids/pool_green.tga
		blendFunc GL_dst_color GL_one
		rgbgen identity
		tcmod scale .5 .5
		tcmod scroll .025 .01
	}
	{
		map textures/chili_liquids/pool_green.tga
		blendFunc GL_dst_color GL_one
		tcmod scale -.5 -.5
		tcmod scroll .025 .025
	}
}

textures/chili_decals/clouds_filter
{
	qer_editorimage textures/chili_decals/sky_shadows.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_tcGen ivector ( 512 0 0 ) ( 0 512 0 )
      q3map_nonplanar
      polygonoffset
	qer_trans 0.5

	{
		map textures/chili_decals/sky_shadows.tga
		blendfunc filter
		tcMod scroll -0.005 0.0125
	}
}

textures/chili_decals/neon
{
	qer_editorimage textures/chili_decals/neon.tga
	surfaceparm nolightmap
	surfaceparm trans
      nopicmip
      nomipmaps
      cull disable

	{
		map textures/chili_decals/neon.tga
		blendfunc add
		rgbGen wave sin 0.5 0.5 0 0.5
		tcMod rotate 100
	}
}

textures/chili_decals/penta
{
	qer_editorimage textures/chili_decals/penta.tga
	surfaceparm nolightmap
	surfaceparm trans
      nopicmip
      nomipmaps
      cull disable

	{
		map textures/chili_decals/penta.tga
		blendfunc add
		rgbGen wave sin 0.5 0.5 0 0.5
	}
}

textures/chili_decals/balustrade_decal_1
{
	qer_editorimage textures/chili_decals/balustrade_decal_1.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_nonplanar
      polygonoffset

	{
		map textures/chili_decals/balustrade_decal_1.tga
		blendfunc filter
	}
}

textures/chili_decals/balustrade_decal_2
{
	qer_editorimage textures/chili_decals/balustrade_decal_2.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_nonplanar
      polygonoffset

	{
		map textures/chili_decals/balustrade_decal_2.tga
		blendfunc filter
	}
}

textures/chili_decals/balustrade_decal_3
{
	qer_editorimage textures/chili_decals/balustrade_decal_3.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_nonplanar
      polygonoffset

	{
		map textures/chili_decals/balustrade_decal_3.tga
		blendfunc filter
	}
}

textures/chili_decals/balustrade_decal_4
{
	qer_editorimage textures/chili_decals/balustrade_decal_4.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_nonplanar
      polygonoffset

	{
		map textures/chili_decals/balustrade_decal_4.tga
		blendfunc filter
	}
}

textures/chili_decals/balustrade_decal_5
{
	qer_editorimage textures/chili_decals/balustrade_decal_5.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_nonplanar
      polygonoffset

	{
		map textures/chili_decals/balustrade_decal_5.tga
		blendfunc filter
	}
}

textures/chili_decals/balustrade_decal_6
{
	qer_editorimage textures/chili_decals/balustrade_decal_6.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_nonplanar
      polygonoffset

	{
		map textures/chili_decals/balustrade_decal_6.tga
		blendfunc filter
	}
}

textures/chili_decals/balustrade_decal_7
{
	qer_editorimage textures/chili_decals/balustrade_decal_7.tga
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
      q3map_nonplanar
      polygonoffset

	{
		map textures/chili_decals/balustrade_decal_7.tga
		blendfunc filter
	}
}