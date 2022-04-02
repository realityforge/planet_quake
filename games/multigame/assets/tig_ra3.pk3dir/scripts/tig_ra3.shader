// ---------------.shader for Certain Ruin by Tigger-oN-----------------
//
// if you use ANY of the graphics from tig_ra3 please move them to 
// your OWN folder to stop any conflicts - do NOT use a /tig_ra3 folder
// and do NOT include a tig_ra3.shader in your level - make your own
//
// A lot of the textures have come from evil_lair's HFX site - planetquake.com/hfx 
// A number of textures are from Mr.cleaN - planetquake.com/mrclean
// A few from Japan Castles by g1zm0 - www.members.home.net/mburbidge/
// The skyboxes where custom made for tig_ra3 by mandog (mandog@ebom.org)
//
// As far as I know you are to use the textures but you MUST move them 
// to a new folder! /textures/yourmapname/ for example and the same 
// goes for this .shader - scripts/yourmapname/yourmapname.shader is fine
//
// Thanx! - Tigger-oN
//
// ------------------------------------------------------18.mar.2001----

// TODO: add a remap texture into the map file for this :(
//textures/gothic_door/door02_i_ornate5_fin
textures/gothic_door/door02_portal
{
	portal
	surfaceparm nolightmap
  surfaceparm nomarks
  surfaceparm nodamage

	qer_editorimage textures/gothic_door/door02_i_ornate5_fin.tga
	//{
	//	map $lightmap
	//	rgbgen identity      
	//}
	
	{
		map textures/gothic_door/door02_i_ornate5_fin.tga
		blendfunc GL_SRC_ALPHA GL_ONE
		//blendFunc GL_DST_COLOR GL_SRC_ALPHA
		//rgbGen identity
		//alphaGen lightingSpecular
    alphagen portal 1024
  	//depthWrite
	}
}

//graphic originaly from t8dm5 by Mr.cleaN
textures/tig_ra3/t8dm5_goop_bounce2
{
	qer_editorimage textures/tig_ra3/tig_bird_bounce.tga
	surfaceparm nodamage
	q3map_lightimage textures/tig_ra3/tig_bird_jump7.tga
	q3map_surfacelight 200

	{
		map textures/tig_ra3/tig_bird_bounce.tga
		rgbGen identity
	}
	{
		map $lightmap 
		blendfunc filter
		rgbGen identity
		tcGen lightmap 
	}
	{
		map textures/tig_ra3/tig_bird_jump7.tga
		blendFunc Add
		rgbGen wave sin .2 .1 0 .2
	}
}


textures/tig_ra3/tig_evil_bounce
{
	qer_editorimage textures/tig_ra3/evil_confllrtile2pad.jpg
	surfaceparm nodamage
	q3map_lightimage textures/tig_ra3/t8dm5_jumpt8sm.tga
	q3map_surfacelight 100

	{
		map textures/tig_ra3/evil_confllrtile2pad.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}
	{
		clampmap textures/tig_ra3/tig_jump_red_lt.tga
		blendfunc gl_one gl_one
		tcMod stretch sin 1.2 1. 0 1.5
		rgbGen wave square .5 .5 .25 1.5
	}
}


textures/tig_ra3/tig_ivy_left2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_left2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_left2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ivy_left_bot2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_left_bot2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_left_bot2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ivy_left_top2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_left_top2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_left_top2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ivy_right2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_right2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_right2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ivy_right_bot2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_right_bot2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_right_bot2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ivy_right_top2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_right_top2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_right_top2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ivy_tile_top2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_tile_top2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_tile_top2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ivy_tile2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_tile2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_tile2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ivy_tile_bot2a
{
	qer_editorimage textures/tig_ra3/tig_ivy_tile_bot2.tga
	cull disable
	surfaceparm alphashadow
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_ivy_tile_bot2.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc filter
		depthFunc equal
	}
}

textures/tig_ra3/tig_ra3_skybox01
{
	qer_editorimage textures/tig_ra3/overcast_bk
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm sky
	q3map_globaltexture
	q3map_lightsubdivide 256
	q3map_surfacelight 50
	q3map_sun 0.2 0.4 0.6 100 82 45
	skyparms env/tig_ra3/overcast - -
}

textures/tig_ra3/tig_ra3_skybox00
{
	qer_editorimage textures/tig_ra3/mistyvalley_bk
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm sky
	q3map_globaltexture
	q3map_lightsubdivide 256
	q3map_surfacelight 100
	q3map_sun	0.4 0.2 0.0 100	45 45
	skyparms env/tig_ra3/mistyvalley - -
}

textures/tig_ra3/tig_ra3_skybox02
{
	qer_editorimage textures/tig_ra3/mistyvalley_bk
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm sky
	q3map_globaltexture
	q3map_lightsubdivide 256
	q3map_surfacelight 450
	q3map_sun	0.1 0.9 0.3 100	45 45
	skyparms env/tig_ra3/mistyvalley - -
}

textures/tig_ra3/tig_ra3_skybox03
{
	qer_editorimage textures/tig_ra3/mistyvalley_bk
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm sky
	q3map_globaltexture
	q3map_lightsubdivide 256
	q3map_surfacelight 40
	q3map_sun	0.5 0.4 0.1 100	45 45
	skyparms env/tig_ra3/mistyvalley - -
}

textures/tig_ra3/tig_ra3_skybox04
{
	qer_editorimage textures/tig_ra3/overcast_bk
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm sky
	q3map_globaltexture
	q3map_lightsubdivide 256
	q3map_surfacelight 150
	q3map_sun	0.6 0.1 0.7 100	45 45
	skyparms env/tig_ra3/overcast - -
}

textures/tig_ra3/tig_spiderweb4
{
	qer_editorimage textures/tig_ra3/tig_spiderweb2.tga
	qer_trans .8
	cull disable
	surfaceparm trans		
	surfaceparm nodamage
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_spiderweb2.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
		rgbGen		vertex
		alphaGen	vertex
	}
}

textures/tig_ra3/tig_spiderweb5
{
	qer_editorimage textures/tig_ra3/tig_spiderweb_big.tga
	qer_trans .8
	cull disable
	surfaceparm trans		
	surfaceparm nodamage
	surfaceparm nomarks
	surfaceparm nolightmap
	surfaceparm nonsolid
        nopicmip

	{
		map textures/tig_ra3/tig_spiderweb_big.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen identity
	}
}


//arena teleport shots
textures/tig_ra3/tig_ra3_a1
{
	qer_editorimage textures/tig_ra3/tig_ra3_a1.tga
	surfaceparm nolightmap
	cull none
	portal
	{
		map textures/tig_ra3/tig_ra3_a1.tga
		blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    alphagen portal 512
		depthWrite
	}

}

textures/tig_ra3/tig_ra3_a2
{
	qer_editorimage textures/tig_ra3/tig_ra3_a2.tga
	surfaceparm nolightmap
	cull none
	portal
	{
		map textures/tig_ra3/tig_ra3_a2.tga
		blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    alphagen portal 512
		depthWrite
	}

}

textures/tig_ra3/tig_ra3_a3
{
	qer_editorimage textures/tig_ra3/tig_ra3_a3.tga
	surfaceparm nolightmap
	cull none
	portal
	{
		map textures/tig_ra3/tig_ra3_a3.tga
		blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    alphagen portal 512
		depthWrite
	}

}

textures/tig_ra3/tig_ra3_a4
{
	qer_editorimage textures/tig_ra3/tig_ra3_a4.tga
	surfaceparm nolightmap
	cull none
	portal
	{
		map textures/tig_ra3/tig_ra3_a4.tga
		blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    alphagen portal 512
		depthWrite
	}

}

//one texture does both sides
textures/tig_ra3/tig_wood_pnl_drty2
{
	qer_editorimage textures/tig_ra3/tig_evilwood_pnl_drty.tga
	cull none
	{
		map textures/tig_ra3/tig_evilwood_pnl_drty.tga
	}

}

textures/tig_ra3/tig_proto_fence
{
	qer_editorimage textures/base_trim/proto_fence.tga
	//surfaceparm trans
	surfaceparm metalsteps
	cull none
        nopicmip

	{
		map textures/base_trim/proto_fence.tga
		tcMod scale 3 3
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

//a grey to match the fog
textures/tig_ra3/tig_grey
{
	qer_editorimage textures/tig_ra3/tig_grey.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	{
		map textures/tig_ra3/tig_grey.tga
	}
}


// grey fog on arena 2
textures/tig_ra3/tig_grey_fog
	{
		qer_editorimage textures/sfx/fog_grey.tga
		surfaceparm	trans
		surfaceparm	nonsolid
		surfaceparm	fog
		surfaceparm	nolightmap
		surfaceparm	nodrop
		qer_nocarve				
		fogparms ( 0.61 0.61 0.61 ) 1300
	}

//evil's very cool jumppad (shader by maj)
textures/tig_ra3/tig_e6launchengine_s
{
	qer_editorimage textures/tig_ra3/tig_e6launchengine.tga
	q3map_lightimage textures/tig_ra3/tig_e6launchengine_glow.tga
	q3map_surfacelight 400
	surfaceparm nomarks

	{
		map textures/tig_ra3/tig_e6launchengine.tga
	}
	{
		map $lightmap
		blendfunc filter
	}
	{
		map textures/tig_ra3/tig_e6launchengine_glow.tga
		blendfunc add
		rgbgen wave sin .5 .8 0 1.5
	}
	{
		clampmap textures/tig_ra3/tig_e6launchengine_fx.tga
		blendfunc add
		tcMod stretch sin 1.2 .9 0 1.5
		rgbGen wave square .5 .5 .25 1.5
	}
}

textures/tig_ra3/tig_a2pool
	{
		qer_editorimage textures/liquids/pool3d_4b2
		qer_trans .5

		//surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
		surfaceparm nolightmap
		surfaceparm nomarks

		//q3map_surfacelight 50
		q3map_globaltexture

		cull disable
		//tesssize 64
		tesssize 128
		deformVertexes wave 64 sin 0.25 0.25 0 .5
		{ 
			map textures/liquids/pool3d_6c2.tga
			blendFunc GL_DST_COLOR GL_ZERO
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 0 1.5 1 1.5 2 1
			tcmod scroll .025 .001
		}
		{ 
			map textures/liquids/pool3d_3c2.tga
			blendFunc GL_DST_COLOR GL_ZERO
			rgbgen identity
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}
		{
			map textures/liquids/pool3d_4b2.tga
			blendfunc add
			rgbgen identity
			tcmod scale .125 .125	
			tcmod scroll -.005 .015 
		}
	}

// flowing water, shader taken from 'Altanis.shader' (the water fall one)
textures/tig_ra3/tig_atlatis_flow
{
	//qer_editorimage textures/tig_ra3/tig_atlatis_water04
	qer_editorimage textures/tig_ra3/tig_atlatis_blue
	qer_trans .5
	surfaceparm trans	
	surfaceparm nonsolid
	surfaceparm water
	//surfaceparm nolightmap
	surfaceparm nomarks

	deformVertexes wave 100 sin 0 1 0 1.5
	cull none
	{
		//map textures/tig_ra3/tig_atlatis_water04.tga
		map textures/tig_ra3/tig_atlatis_blue.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		//blendFunc blend
		tcmod scroll 0 -2.5
		rgbGen		vertex
		alphaGen	vertex
	}
}

textures/tig_ra3/tig_atlatis_flow2
{
	//qer_editorimage textures/tig_ra3/tig_atlatis_water04
	qer_editorimage textures/tig_ra3/tig_atlatis_blue
	qer_trans .5

	surfaceparm trans	
	surfaceparm nonsolid
	surfaceparm water
	//surfaceparm nolightmap
	surfaceparm nomarks

	deformVertexes wave 100 sin 0 1 0 1.5
	cull none
	{
		//map textures/tig_ra3/tig_atlatis_water04.tga
		map textures/tig_ra3/tig_atlatis_blue.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcmod scroll 0 -1.0
		rgbGen		vertex
		alphaGen	vertex
	}
}


// roof_b and roof_r are from g1zm0's Japan Castle level(s)
// however I wanted metal sounds when people are walking them :]
textures/tig_ra3/roof_b
{
	surfaceparm metalsteps
	{
		map textures/tig_ra3/roof_b.tga
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		//rgbGen identity
	}
}

textures/tig_ra3/roof_r
{
	surfaceparm metalsteps
	{
		map textures/tig_ra3/roof_r.tga
	}
        {
		map $lightmap
                blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
		//rgbGen identity
	}
}

textures/tig_ra3/tig_pool2
	{
		qer_editorimage textures/liquids/pool3d_4b2
		qer_trans .5

		surfaceparm trans
		surfaceparm nonsolid
		surfaceparm water
		surfaceparm nolightmap
		surfaceparm nomarks

		//q3map_surfacelight 50
		q3map_globaltexture

		cull disable
		//tesssize 64
		tesssize 128
		deformVertexes wave 64 sin 0.25 0.25 0 .5
		{ 
			map textures/liquids/pool3d_6c2.tga
			blendFunc GL_DST_COLOR GL_ZERO
			rgbgen identity
			tcmod scale .5 .5
			tcmod transform 0 1.5 1 1.5 2 1
			tcmod scroll .025 .001
		}
		{ 
			map textures/liquids/pool3d_3c2.tga
			blendFunc GL_DST_COLOR GL_ZERO
			rgbgen identity
			tcmod scale .25 .5
			tcmod scroll .001 .025
		}
		{
			map textures/liquids/pool3d_4b2.tga
			blendfunc add
			rgbgen identity
			tcmod scale .125 .125	
			tcmod scroll -.005 .015
				
		}
	}

textures/tig_ra3/tig_weapclip_metal
{
	qer_trans 0.40
	surfaceparm metalsteps
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nodraw
}

textures/tig_ra3/tig_clip_metalstep
{
	qer_trans 0.40
	surfaceparm metalsteps
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm playerclip
	surfaceparm noimpact
}
