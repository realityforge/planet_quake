textures/portal_room/wall_p
{
	qer_editorimage textures/portal_room/wall.tga
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/portal_room/wall.tga
		blendFunc GL_DST_COLOR GL_ZERO
	}
}

textures/portal_room/floor_p
{
	qer_editorimage textures/portal_room/floor.tga
	surfaceparm nomarks
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/portal_room/floor.tga
		blendFunc GL_DST_COLOR GL_ZERO
	}
}

textures/portal_room/portal_sfx
{
	qer_editorimage textures/portal_room/portal_sfx1.tga
	surfaceparm nolightmap
	{	
		map textures/portal_room/portal_sfx_ring_electric.jpg 
		blendfunc gl_one gl_one
		rgbgen wave inversesawtooth 0 1 .2 .5
		tcmod scroll 0 .5
	}
	{
		map textures/portal_room/portal_sfx1.tga
		blendfunc gl_dst_color gl_zero
		tcMod rotate 360
	}
}

textures/portal_room/jaildr1_3
{
	qer_editorimage textures/base_light/jaildr1_3.tga
	surfaceparm nonsolid
	q3map_surfacelight 1000
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/jaildr1_3.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/base_light/jaildr1_3.blend.tga
		rgbGen wave sin 0.5 0.5 1 1
		blendfunc GL_ONE GL_ONE
	}
}

textures/portal_room/jaildr1_3trim
{
	qer_editorimage textures/base_light/jaildr1_3.tga
	q3map_surfacelight 1000
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/jaildr1_3.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/base_light/jaildr1_3.blend.tga
		rgbGen wave sin 0.5 0.5 1 1
		blendfunc GL_ONE GL_ONE
	}
}

textures/portal_room/glass
{
	qer_editorimage textures/base_wall/shiny3.tga
	surfaceparm trans	
	cull front
	qer_trans 	0.5
    
	{
		map textures/effects/tinfx.tga
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

textures/portal_room/railDiscNPM
{
	qer_editorimage textures/portal_room/raildisc_mono2.tga
	nopicmip
	sort nearest
	cull none
	surfaceparm nonsolid
//	deformVertexes wave 100 sin 0 .5 0 2.4
	deformVertexes autosprite
	{
		clampmap textures/portal_room/raildisc_mono2.tga 
		blendFunc GL_ONE GL_ONE
		rgbGen vertex
                 tcMod rotate -30
	}
}

textures/portal_room/railCoreNPM
{
	qer_editorimage textures/portal_room/railcorethin_mono.tga
	nopicmip
	sort nearest
	cull none
	surfaceparm nonsolid
	deformVertexes autosprite2
	{
		map textures/portal_room/railcorethin_mono.tga
		blendFunc GL_ONE GL_ONE
		rgbGen vertex
		tcMod scroll -1 0
	}
}

textures/portal_room/noise_s
{
	cull none
	surfaceparm nolightmap
//	surfaceparm weaponclip
	surfaceparm nomarks
	surfaceparm nodlight
	noPicMip
	tessSize 32
	deformVertexes wave 100 sin 2 4 0 .5

	qer_editorimage textures/portal_room/noise.tga
	qer_trans 0.4
	{
		map textures/portal_room/noise.tga
		blendfunc add
		rgbgen wave sin 0.8 .2 .2 .5
		tcmod scroll 0.3 0.05
	}
}

textures/portal_room/noise2_s
{
	cull none
	surfaceparm	nonsolid
	surfaceparm nolightmap
	surfaceparm weaponclip
	surfaceparm nomarks
	surfaceparm nodlight
	noPicMip
//	tessSize 64
//	deformVertexes wave 100 cos 2 4 0 .5

	qer_editorimage textures/portal_room/railcorethin_mono2.tga
	qer_trans 0.4
	{
		map textures/portal_room/railcorethin_mono2.tga
		blendfunc add
//		rgbGen vertex
		rgbgen wave inversesawtooth 0.6 .4 .2 .3
		tcMod scroll 0.1 0.3
	}

}

textures/portal_room/weaponclip
{
	surfaceparm	nonsolid
	surfaceparm	nodraw
	surfaceparm weaponclip
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nodlight
}

textures/portal_room/redfog
{
	qer_editorimage textures/sfx/fog_red.tga
	qer_trans 0.4
	surfaceparm	trans
	surfaceparm	nonsolid
	surfaceparm	fog
	surfaceparm	nolightmap

	fogparms ( 0.75 0.38 0 ) 256
}

textures/portal_room/plasma1
{
	nopicmip
	sort nearest
	cull none
	surfaceparm nonsolid
	surfaceparm nolightmap
	deformVertexes autosprite
	qer_editorimage textures/portal_room/plasmaa.tga
	{
		clampmap textures/portal_room/plasmaa.tga
		blendfunc GL_ONE GL_ONE
                tcMod rotate 931
	}
}

textures/portal_room/slime1
{
	qer_editorimage textures/liquids/slime7.tga
	q3map_lightimage textures/liquids/slime7.tga
	q3map_globaltexture
	qer_trans .5

	surfaceparm noimpact
	surfaceparm slime
	surfaceparm nolightmap
	surfaceparm trans		

	q3map_surfacelight 100
	tessSize 32
	cull disable

	deformVertexes wave 100 sin 0 1 .5 .5

	fogparms ( 0.28 0.44 0.11 ) 256

	{
		map textures/liquids/slime7c.tga
		tcMod turb .3 .2 1 .05
		tcMod scroll .01 .01
	}
	
	{
		map textures/liquids/slime7.tga
		blendfunc GL_ONE GL_ONE
		tcMod turb .2 .1 1 .05
		tcMod scale .5 .5
		tcMod scroll .01 .01
	}

	{
		map textures/liquids/bubbles.tga
		blendfunc GL_ZERO GL_SRC_COLOR
		tcMod turb .2 .1 .1 .2
		tcMod scale .05 .05
		tcMod scroll .001 .001
	}		
}

textures/portal_room/noisekill_s
{
	cull none
	surfaceparm nolightmap
//	surfaceparm weaponclip
	surfaceparm nomarks
	surfaceparm nodlight
	noPicMip
	tessSize 32
	deformVertexes wave 100 sin 2 4 0 .5

	qer_editorimage textures/portal_room/noise_red.tga
	qer_trans 0.4
	{
		map textures/portal_room/noise_red.tga
		blendfunc add
		rgbgen wave sin 0.8 .2 .2 .5
		tcmod scroll 0.3 0.05
	}
}

textures/portal_room/noisekill2_s
{
	cull none
	surfaceparm	nonsolid
	surfaceparm nolightmap
	surfaceparm weaponclip
	surfaceparm nomarks
	surfaceparm nodlight
	noPicMip
//	tessSize 64
//	deformVertexes wave 100 cos 2 4 0 .5
	q3map_surfacelight 100

	qer_editorimage textures/portal_room/electricslime.tga
	qer_trans 0.4

	{
		map textures/sfx/tesla1.tga
		blendfunc add
		rgbgen wave sawtooth 0 1 0 5
		tcmod scale 1 .5
		tcmod turb 0 .1 0 1
		tcMod scroll -1 -0.5
	}
 
	
	
	{
		map textures/sfx/electricslime.tga
		blendfunc add
		rgbgen wave sin 0 .5 0 1
		tcmod scale .5 .5
		tcmod turb 0 .1 0 1
		tcmod rotate 180
		tcmod scroll -1 -1.2
	}
	
	{
		map textures/sfx/electricslime2.tga
		blendfunc add
		rgbGen wave square .25 .25 0 2.5
		tcmod scale 1 1
		tcMod scroll 1 0.5
	}



	{
		map textures/sfx/tesla1b.tga
		blendfunc add
		rgbgen wave square 0 1 0 3
		tcmod scale 1 1
		tcMod scroll -2 1.2
	}

}

textures/portal_room/bluefog
{
	qer_editorimage textures/sfx/fog_blue.tga
	qer_trans 0.4
	surfaceparm	trans
	surfaceparm	nonsolid
	surfaceparm	fog
	surfaceparm	nolightmap

	fogparms ( 0.6 0.85 0.9 ) 2048
}
