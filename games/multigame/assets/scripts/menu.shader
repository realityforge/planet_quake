// menu shader




models/mapobjects/banner/q3banner04
{      
	cull disable 
	nomipmaps

	{
	//map models/mapobjects/banner/q3banner04.tga
		map textures/sfx/firegorre2.tga
		blendFunc GL_ONE GL_ZERO
		tcmod scale .05 .1
		tcMod turb 0 .25 0 .6
		tcmod scroll .4 .3
	//rgbGen wave sin .5 .5 0 .1
	}
	{
		map textures/sfx/bolts.tga
		tcmod scale .2 .2
		tcmod rotate 999
		tcmod scroll 9 9
		blendfunc add 
		rgbGen wave sin .5 .5 0 .2
	}  
	{
		map textures/sfx/firegorre2.tga
		//map models/mapobjects/banner/q3banner04.tga
		blendFunc add
		tcGen environment
		tcmod scale 5  5 
		tcmod scroll 0.09 0.04
		//rgbGen wave sin .5 .5 0 .1
	}    

}

models/mapobjects/banner/q3banner02
{      
	cull disable
	nomipmaps
	//deformVertexes wave 70 sin 0 .7 0 .4

	{
		map models/mapobjects/banner/q3banner02.tga
		blendFunc add
		tcmod scale  2  1
		tcmod scroll .33 0
		rgbGen wave sin .5 .5 0 .2
	}
	{
		map models/mapobjects/banner/q3banner02.tga
		blendFunc add
		tcmod scale  3  1
		tcmod scroll -.45 0
		rgbGen wave sin .5 .5 0 .2
	} 
	{
		map models/mapobjects/banner/q3banner02x.tga
		blendFunc add
		tcmod scale  4  1
		tcmod scroll 1 0
		// rgbGen wave sin .5 .5 0 -.2
	} 
}

gfx/2d/bigchars
{
	nopicmip
	nomipmaps
	{
		map gfx/2d/bigchars.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}









