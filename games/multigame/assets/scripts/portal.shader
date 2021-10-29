textures/portal/portal_sfx_blue2
{
  qer_editorimage textures/common/qer_mirror.tga
  surfaceparm nolightmap
  portal

  {
    map textures/common/mirror1.tga
    blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
    depthWrite
    alphagen portal 1024
  }
}

textures/portal/portal_sfx_blue
{
	portal
	surfaceparm nolightmap
	deformVertexes wave 100 sin 0 2 0 .5

	{
		map textures/portal/blue/portal_sfx3.tga
    alphagen portal 1024
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}

	{
		map textures/portal/blue/portalfog.jpg
		blendfunc GL_SRC_ALPHA GL_ONE
		tcmod rotate .1 //.1
		tcmod scroll .01 .03
	}

	{
		map textures/portal/blue/portal_sfx1.tga
		blendfunc gl_dst_color gl_zero
		tcMod rotate 360
	}

	{
		map textures/portal/blue/portal_sfx.jpg
		blendfunc gl_one gl_one
		rgbgen wave inversesawtooth 0 .5 .2 .7
	}
}

textures/portal/portal_sfx_ring_blue
{
	deformVertexes wave 100 sin 0 2 0 .5
	surfaceparm nolightmap

	{
		map textures/portal/blue/portal_sfx_ring_blue1.tga 
		blendfunc gl_src_alpha gl_one_minus_src_alpha
	}

	
	{	
		map textures/portal/blue/portal_sfx_ring_electric.jpg 
		blendfunc gl_one gl_one
		rgbgen wave inversesawtooth 0 1 .2 .5
		tcmod scroll 0 .5

	}

	{
		map textures/portal/blue/portal_sfx1.tga
		blendfunc gl_dst_color gl_zero
		tcMod rotate 360
	}

	{
		map textures/portal/blue/portal_sfx_ring.jpg
		blendfunc gl_one gl_one
		rgbgen wave inversesawtooth 0 .5 .2 .7
	}

}

textures/portal/portal_sfx_red
{
	portal
	surfaceparm nolightmap
	deformVertexes wave 100 sin 0 2 0 .5

	{
		map textures/portal/red/portal_sfx3.tga
    alphagen portal 1024
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}

  {
		map textures/portal/red/portalfog.jpg
    blendfunc GL_SRC_ALPHA GL_ONE
		tcmod rotate .1 //.1
		tcmod scroll .01 .03
	}

	{
		map textures/portal/red/portal_sfx1.tga
		blendfunc gl_dst_color gl_zero
		tcMod rotate 360
	}

	{
		map textures/portal/red/portal_sfx.jpg
		blendfunc gl_one gl_one
		rgbgen wave inversesawtooth 0 .5 .2 .7
	}
}

textures/portal/portal_sfx_ring_red
{
	deformVertexes wave 100 sin 0 2 0 .5
	surfaceparm nolightmap
	
	{
		map textures/portal/red/portal_sfx_ring_blue1.tga 
		blendfunc gl_src_alpha gl_one_minus_src_alpha
	}

	
	{	
		map textures/portal/red/portal_sfx_ring_electric.jpg 
		blendfunc gl_one gl_one
		rgbgen wave inversesawtooth 0 1 .2 .5
		tcmod scroll 0 .5

	}

	{
		map textures/portal/red/portal_sfx1.tga
		blendfunc gl_dst_color gl_zero
		tcMod rotate 360
	}

	{
		map textures/portal/red/portal_sfx_ring.jpg
		blendfunc gl_one gl_one
		rgbgen wave inversesawtooth 0 .5 .2 .7
	}

}