models/runes/resist
{
	{
		map models/runes/resist_fluid.tga
		tcMod scroll 0 2
		blendfunc GL_ONE GL_ONE
	}
	{
		map models/powerups/armor/energy_red1.tga 
    blendFunc GL_ONE GL_ONE
		tcMod scroll 7.6 1.3
		alphaGen lightingSpecular
	}
}

models/runes/magic
{
	{
    map textures/effects/envmapblue2.jpg
    tcMod scroll 0 3
    blendfunc GL_ONE GL_ONE
  }
}

models/runes/reflection
{	
	{
    map textures/effects/tinfxb.jpg
    tcMod scroll 0 3
    blendfunc GL_ONE GL_ONE
  }   
}

powerups/regen
{
  deformVertexes wave 100 sin 3 0 0 0
  {
    map textures/effects/regenmap2.tga
    blendfunc GL_ONE GL_ONE
    tcGen environment
    tcmod scroll 0 1.03
  }
}


models/runes/resist
{
  {
    map models/runes/resist_fluid.tga
    tcMod scroll 0 2
    blendfunc GL_ONE GL_ONE
  }
  {
    map models/powerups/armor/energy_red1.tga 
    blendFunc GL_ONE GL_ONE
    tcMod scroll 7.6 1.3
    alphaGen lightingSpecular
  }
}

models/runes/haste
{
//  {
//    map textures/sfx/firegorre.tga
//    tcMod scroll 0 3
//    blendfunc GL_ONE GL_ONE
//  }
  {
    map textures/effects/envmapblue.tga
    blendfunc GL_ONE GL_ZERO
    tcMod scroll 0 3
    tcGen environment
    rgbGen identity
  }
}

models/runes/enviro
{
  {
    map textures/effects/envmaprail.tga
    blendfunc GL_ONE GL_ZERO
    tcMod scroll 0 3
    tcGen environment
    rgbGen identity
  }
}


models/runes/magic
{
  {
    map textures/effects/envmapblue2.jpg
    tcMod scroll 0 3
    blendfunc GL_ONE GL_ONE
  }
}

models/runes/reflection
{	
  {
    map textures/effects/tinfxb.jpg
    tcMod scroll 0 3
    blendfunc GL_ONE GL_ONE
  }   
}

models/runes/regen
{	
  {
    map textures/effects/envmapgold2.jpg
    tcMod scroll 1 1
    blendfunc GL_ONE GL_ONE
  }
}

models/runes/strength
{
  {
    map textures/sfx/metalfloor_wall_14b.jpg
    blendfunc GL_ONE GL_ZERO
    rgbgen const ( 2.0 1.8 1.7 )
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcgen environment
    tcMod scroll -4 -.5
    tcMod scale 3 3
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 2.0 0.0 0.0 )
  }    
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcgen environment
    tcMod scroll 4 1
    tcMod scale 3 3
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 2.0 0.0 0.0 )
  }    
}

runes/phase
{
  {
    map textures/effects/phasemap.tga
    //map textures/sfx/specular.tga
    blendfunc GL_ONE GL_ONE
    tcMod turb 0 0.15 0 0.25
    tcGen environment
  }
}

runes/weird
{
  deformVertexes wave 100 sin 3 0 0 0
  {
    map textures/effects/weirdmask.tga
    blendfunc GL_ONE GL_ONE
    tcGen environment
    tcmod rotate 30
    //tcMod turb 0 0.2 0 .2
    tcmod scroll 1 .1
  }
}
