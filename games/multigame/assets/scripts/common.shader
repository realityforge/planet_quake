textures/common/areaportal
{
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm areaportal
}

textures/common/botclip
{
	qer_trans 0.40
	qer_editorimage textures/common/botclip
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm botclip
	surfaceparm noimpact
}

textures/common/caulk
{
	surfaceparm nodraw
	surfaceparm nomarks
	surfaceparm nolightmap
}

textures/common/clip
{
	qer_trans 0.40
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm playerclip
	surfaceparm noimpact
}

textures/common/clusterportal
{
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm detail
	surfaceparm clusterportal
}

textures/common/cushion
{
	qer_nocarve
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nomarks
	surfaceparm nodamage
	surfaceparm trans
}

textures/common/donotenter
{
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm detail
	surfaceparm donotenter
}

textures/common/energypad
{
	qer_editorimage textures/common/bluegoal.tga
	surfaceparm nolightmap
	cull twosided

	{
		map textures/common/bluegoal.tga
		blendFunc GL_ONE GL_SRC_ALPHA
		tcGen environment
		tcMod turb 0 0.25 0 0.05
	}
}

textures/common/full_clip
{
	qer_trans 0.40
	surfaceparm nodraw
	surfaceparm playerclip
}

textures/common/hint
{
	qer_nocarve
	qer_trans 0.30
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
	surfaceparm noimpact
}

textures/common/invisible
{
	surfaceparm nolightmap

	{
		map textures/common/invisible.tga
		alphaFunc GE128
		depthWrite
		rgbGen vertex
	}
}

textures/common/lightgrid
{
	qer_trans 0.40
	qer_editorimage textures/common/lightgrid
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm lightgrid
	surfaceparm noimpact
}

textures/common/mirror1
{
	qer_editorimage textures/common/qer_mirror.tga
	surfaceparm nolightmap
	portal

	{
		map textures/common/mirror1.tga
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}
}

textures/common/mirror2
{
	qer_editorimage textures/common/qer_mirror.tga
	surfaceparm nolightmap
	portal

	{
		map textures/common/mirror1.tga
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}
	{
		map textures/sfx/mirror.tga
		blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
	}
}

textures/common/missileclip
{
	qer_trans 0.40
	surfaceparm nodamage
	surfaceparm nomarks
	surfaceparm nodraw
	surfaceparm playerclip
}

textures/common/nodraw
{
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
}

textures/common/nodrawnonsolid
{
	surfaceparm nonsolid
	surfaceparm nodraw
}

textures/common/nodrop
{
	qer_nocarve
	qer_trans 0.5
	surfaceparm trans
	surfaceparm nonsolid
	surfaceparm nomarks
	surfaceparm nodrop
	surfaceparm nolightmap
	surfaceparm nodraw
	cull disable
}

textures/common/noimpact
{
	surfaceparm noimpact
}

textures/common/nolightmap
{
	surfaceparm nolightmap
}

textures/common/origin
{
	qer_nocarve
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm origin
}

textures/common/portal
{
	qer_editorimage textures/common/qer_portal.tga
	surfaceparm nolightmap
	portal

	{
		map textures/common/mirror1.tga
		tcMod turb 0 0.25 0 0.05
		blendfunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
		depthWrite
	}
}

textures/common/skip
{
	qer_nocarve
	qer_trans 0.40
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm structural
	surfaceparm trans
}

textures/common/slick
{
	qer_trans 0.50
	surfaceparm nodraw
	surfaceparm nomarks
	surfaceparm trans
	surfaceparm slick
}

textures/common/teleporter
{
	surfaceparm nolightmap
	surfaceparm noimpact
	q3map_lightimage textures/sfx/powerupshit.tga
	q3map_surfacelight 800

	{
		map textures/sfx/powerupshit.tga
		tcGen environment
		tcMod turb 0 0.015 0 0.3
	}
}

textures/common/timportal
{
	qer_editorimage textures/common/qer_portal.tga
	portal
	surfaceparm nolightmap

	{
		map textures/common/portal.tga
		tcMod turb 0 0.25 0 0.05
		blendFunc GL_ONE GL_SRC_ALPHA
		depthWrite
	}
}

textures/common/trigger
{
	qer_trans 0.50
	qer_nocarve
	surfaceparm nodraw
}

textures/common/weapclip
{
	qer_trans 0.40
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nodraw
}
