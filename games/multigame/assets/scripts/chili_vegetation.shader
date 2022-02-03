textures/chili_vegetation/veg_lilypad
{
	qer_editorimage textures/chili_vegetation/seerose.tga
	surfaceparm nomarks
      surfaceparm trans
      surfaceparm alphashadow
	cull disable

	{
		clampmap textures/chili_vegetation/seerose.tga
		alphaFunc GE128
	}
}

textures/chili_vegetation/veg_lilypadflower
{
	qer_editorimage textures/chili_vegetation/seerosenbluete.tga
	surfaceparm nomarks
      surfaceparm nonsolid
      surfaceparm trans
	cull disable

	{
		map textures/chili_vegetation/seerosenbluete.tga
		alphaFunc GE128
	}
}

textures/chili_vegetation/veg_ivy
{
	qer_editorimage textures/chili_vegetation/efeu.tga
	surfaceparm nomarks
      surfaceparm nonsolid
      surfaceparm trans
      surfaceparm alphashadow
	cull disable

	{
		clampmap textures/chili_vegetation/efeu.tga
		alphaFunc GE128
	}
}

textures/chili_vegetation/veg_weed
{
	qer_editorimage textures/chili_vegetation/gras.tga
	surfaceparm nomarks
      surfaceparm nonsolid
      surfaceparm trans
	cull disable

	{
		clampmap textures/chili_vegetation/gras.tga
		alphaFunc GE128
	}
}

textures/chili_vegetation/veg_leaves
{
	qer_editorimage textures/chili_vegetation/blaetter.tga
	surfaceparm nomarks
      surfaceparm nonsolid
      surfaceparm trans
      deformVertexes autosprite
	cull disable

	{
		clampmap textures/chili_vegetation/blaetter.tga
		alphaFunc GE128
	}
}
