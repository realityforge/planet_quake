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

models/runes/regen
{
  deformVertexes wave 100 sin 3 0 0 0
  {
    map textures/effects/regenmap2.tga
    blendfunc GL_ONE GL_ONE
    tcGen environment
    tcmod scroll 0 1.03
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
    tcMod scroll 1 3
    tcGen environment
    rgbGen identity
  }
}

models/runes/flight
{	
  {
    map textures/effects/envmappurp.jpg
    blendfunc GL_ONE GL_ZERO
    tcMod scroll 1 3
    tcGen environment
    rgbGen identity
  }
}

models/runes/berserk
{	
  {
    map textures/effects/envmapred.jpg
    blendfunc GL_ONE GL_ZERO
    tcMod scroll 0.5 0.5
    tcGen environment
    rgbGen identity
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll -0.5 -0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 2.0 0.0 0.0 )
  }
}

models/runes/berserk_2
{	
  nopicmip
  deformvertexes wave 100 sin 0.5 0 0 0
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 0.5 0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 2.0 0.0 0.0 )
  }
}

models/runes/electric
{	
  {
    map textures/effects/envmaprail.jpg
    blendfunc GL_ONE GL_ZERO
    tcMod scroll 0.5 0.5
    tcGen environment
    rgbGen identity
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll -0.5 -0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 0.0 2.0 0.0 )
  }
}

models/runes/electric_2
{	
  nopicmip
  deformvertexes wave 100 sin 0.5 0 0 0
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 0.5 0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 0.0 2.0 0.0 )
  }
}

models/runes/cloak
{
  {
    map textures/effects/tinfx2c.tga
    blendfunc GL_ONE GL_ONE
    tcGen environment
  }
}

models/runes/divine
{	
  {
    map textures/effects/regenmap2.jpg
    blendfunc GL_ONE GL_ZERO
    tcMod scroll 0.5 0.5
    tcGen environment
    rgbgen const ( 0.5 0.0 0.0 )
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll -0.5 -0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 2.0 0.0 0.0 )
  }
}

models/runes/divine_2
{	
  nopicmip
  deformvertexes wave 100 sin 1 0 0 0
  {
    map models/powerups/kamitrail.tga 
    tcMod scroll 0.5 0.5
    blendFunc Add
    rgbgen const ( 0.8 0.8 0.8 )
  }
  {
    map models/weaphits/kamiwave02.tga 
		blendFunc Add
    tcmod scroll 1 0.5
    rgbgen const ( 0.8 0.8 0.8 )
	}
}

models/runes/death
{	
  {
    map textures/effects/tinfx3.jpg
    blendfunc GL_ONE GL_ZERO
    tcMod scroll 0.5 0.5
    tcGen environment
    rgbgen identity
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll -0.5 -0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 2.5 2.0 0.0 )
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 1 1
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 2.0 2.5 0.0 )
  }
}

models/runes/holo
{	
  {
    map textures/effects/tinfx2c.tga
    blendfunc GL_ONE GL_ONE
    tcGen environment
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll -0.5 -0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 0.0 0.0 2.0 )
  }
}

models/runes/holo_2
{	
  nopicmip
  deformvertexes wave 100 sin 1 0 0 0
  {
    map textures/effects/tinfx2c.tga
    blendfunc GL_ONE GL_ONE
    tcGen environment
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 1 1
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 0.0 0.0 2.0 )
	}
}

models/runes/orb
{	
  {
    map textures/effects/envmapgold.tga
    blendfunc GL_ONE GL_ZERO
    tcMod scroll 1 1
    tcGen environment
    rgbgen const ( 1.0 0.8 0.0 )
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll -0.5 -0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 1.0 0.5 0.0 )
  }
}

models/runes/blink
{
  {
    map textures/effects/envmapblue.jpg
    tcMod scroll 1 1
    blendfunc GL_ONE GL_ONE
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll -0.5 -0.5
    tcMod scale 3 3
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 0.0 1.0 0.5 )
  }
}

models/runes/blink_2
{
  nopicmip
  deformvertexes wave 100 sin 1 0 0 0
  {
    map textures/effects/envmapblue2.jpg
    tcMod scroll 0 0
    blendfunc GL_ONE GL_ONE
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 0.5 0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 0.0 0.5 1.0 )
  }
}

models/runes/camo
{
  {
    map textures/effects/envmapblue2.jpg
    tcMod scroll 0.3 0.3
    blendfunc GL_ONE GL_ONE
  }
}

models/runes/action
{
  {
    map textures/effects/envmapmach.tga
    tcGen environment
    blendfunc GL_ONE GL_ZERO
    rgbgen const ( 1.0 0.0 0.0 )
    tcmod Scroll 1 1
  }
}

models/runes/jump
{
  deformVertexes move 0 0 3  sin 0 5 0 1.5
  {
    map textures/effects/envmapyel.tga
    tcGen environment
    blendfunc GL_ONE GL_ZERO
    rgbGen identity
    tcmod Scroll 2 2
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 0.5 0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 1.0 1.0 1.0 )
  }
}

models/runes/vampire
{
  deformVertexes move 0 0 3  sin 0 5 0 0.1
  {
    map textures/effects/envmapmach.tga
    tcGen environment
    blendfunc GL_ONE GL_ZERO
    rgbgen const ( 1.0 0.0 0.0 )
    tcmod Scroll 1 1
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll -0.5 -0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 1.0 1.0 1.0 )
  }
}

models/runes/shield
{
  {
    map textures/effects/envmapgold.tga
    blendfunc GL_ONE GL_ZERO
    tcGen environment
    rgbgen const ( 1.0 0.8 0.0 )
  }
}

models/runes/health
{
  {
    map textures/effects/envmapred.tga
    tcGen environment
    blendfunc GL_ONE GL_ZERO
  }
  {
    map textures/sfx/kenelectric.tga
    tcmod scale 2 2
    tcmod rotate 333
    tcmod scroll 9 9
    blendfunc GL_ONE GL_ONE
  }
}

models/runes/health_2
{
  deformvertexes wave 100 sin 0 0 0 0
  {
    map textures/effects/tinfx2b.tga
    tcmod scroll 1 1
    blendfunc GL_ONE GL_ONE
  }
}


models/runes/radio
{
  deformvertexes wave 100 sin 0 0 0 0
  {
    map textures/effects/tinfx2b.tga
    tcmod scroll 1 1
    blendfunc GL_ONE GL_ZERO
    rgbgen const ( 8.0 1.0 0.0 )
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 0.5 0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 0.0 1.0 0.0 )
  }
}

models/runes/switch
{
  deformVertexes move -2 -2 0  sin 0 2 0 0.5
  {
    map textures/effects/envmapyel.tga
    tcGen environment
    blendfunc GL_ONE GL_ZERO
    rgbGen identity
    tcmod Scroll 2 2
  }
}

models/runes/switch_2
{
  deformVertexes move 2 2 0  sin 0 2 0 0.5
  {
    map textures/effects/tinfx2c.tga
    tcGen environment
    blendfunc GL_ONE GL_ONE
    rgbGen identity
    tcmod Scroll 2 2
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 0.5 0.5
    tcMod scale 4 4
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 1.0 1.0 0.0 )
  }
}

models/runes/icetrap
{
  {
    map textures/effects/envmapblue2.tga
    blendfunc GL_ONE GL_ONE
    rgbGen identity
    tcmod Scroll 2 2
  }
}

models/runes/gravity
{
  {
    map textures/effects/envmaproc.tga
    blendfunc GL_ONE GL_ONE
    rgbGen identity
    tcmod Scroll 1 1
  }
}

models/runes/vengeance
{
  {
    map textures/effects/envmapred.tga
    blendfunc GL_ONE GL_ZERO
    rgbGen identity
    tcmod Scroll 1 1
  }
}

models/runes/impact
{
  {
    map textures/effects/envmapred.tga
    blendfunc GL_ONE GL_ONE
    rgbGen identity
    tcmod Scroll 1 1
  }
}

models/runes/tele
{
  deformVertexes move -2 -2 0  sin 0 2 0 0.5
  {
    map textures/effects/envmapyel.tga
    tcGen environment
    blendfunc GL_ONE GL_ONE
    rgbGen identity
    tcmod Scroll 1 1
  }
}

models/runes/tele_2
{
  deformVertexes move 2 2 0  sin 0 2 0 0.5
  {
    map textures/effects/tinfx2c.tga
    tcGen environment
    blendfunc GL_ONE GL_ONE
    rgbgen const ( 1.0 1.0 0.0 )
    tcmod Scroll 1 1
  }
}

models/runes/recall
{
  {
    map textures/effects/envmapblue.jpg
    tcMod scroll 1 3
    blendfunc GL_ONE GL_ONE
  }
  {
    map models/mapobjects/bitch/hologirl2.tga
    tcMod scroll 0.5 0.5
    tcMod scale 3 3
    blendFunc GL_ONE GL_ONE
    rgbgen const ( 0.0 0.0 2.0 )
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
