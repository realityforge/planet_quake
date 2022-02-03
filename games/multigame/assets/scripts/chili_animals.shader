textures/chili_animals/schmetterling
{
	qer_editorimage textures/chili_animals/butterfly.tga
	surfaceparm nodamage
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
	deformVertexes wave 6 sin 0 9 0 1.6
	deformVertexes move 2 2 2 sin 0 .5 0.75 0.5 
	tessSize 16
      cull disable

	{
		map textures/chili_animals/butterfly.tga
		alphaFunc GE128
	}
}

textures/chili_animals/fledermaus
{
	qer_editorimage textures/chili_animals/bat.tga
	surfaceparm nodamage
	surfaceparm noimpact
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans
	deformVertexes wave 12 sin 0 18 0 3.2
	deformVertexes move 4 4 4 sin 0 1 0.75 0.5 
	tessSize 16
      cull disable

	{
		map textures/chili_animals/bat.tga
		alphaFunc GE128
	}
}

textures/chili_animals/libellen
{
      qer_editorimage textures/chili_animals/dragonfly.tga
      surfaceparm trans	
      surfaceparm nomarks	
      surfaceparm nodamage        
      surfaceparm nonsolid
      surfaceparm nolightmap
      deformVertexes move 1 1.5 .7  sin 0 2.5 0 0.15
      deformVertexes wave 10 sin 0 4 0 .1
      cull none

      {
	      map textures/chili_animals/dragonfly.tga
            tcMod Scroll -5 0.1
            tcMod turb .3 .25 0 .1
            alphaFunc GE128
     }

     {
           map textures/chili_animals/dragonfly.tga
            tcMod Scroll 4 -0.5
            tcMod turb .1 .25 0 -.1
            alphaFunc GE128
     }
}