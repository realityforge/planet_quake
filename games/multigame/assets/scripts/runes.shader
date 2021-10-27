
powerups/runes/strength
{
  deformVertexes wave 100 sin 0.5 0 0 0
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

powerups/runes/regen
{
	deformVertexes wave 100 sin 3 0 0 0
	{
		map textures/effects/regenmap2.tga
		blendfunc GL_ONE GL_ONE
		tcGen environment
                tcmod rotate 30
		//tcMod turb 0 0.2 0 .2
                tcmod scroll 1 .1
	}
}
