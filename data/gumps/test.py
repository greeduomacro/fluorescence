# -*- coding: utf-8 -*-

from gumps import *
from data import *

import time

def onButtonClick(button):
	print "onButtonClick"
	button.rgba = (1.0, 0.0, 1.0)
	print button.name

	button.gump.component("i1").hue = 90

	print button.gump.store
	print button.gump.store["i2"]
	button.gump.store["i2"].rgba = rgba(91)

def create():
	g = GumpMenu(30, 30)

	b1 = g.addBackground((0, 0, 500, 500), 3000)
	b1.rgba = rgba("#ff0000")

	# test for texture class
	i1 = g.addImage((0, 0, 100, 100), Texture(TextureSource.THEME, "images/button.png"))
	i1.texture = Texture(TextureSource.GUMPART, 13)
	i1.geometry = (30, 30)
	i1.hue = 13

	i2 = g.addImage((0, 0), Texture(TextureSource.GUMPART, 12))
	i2.rgba = (0.8, 0.2, 0.0)
	i2.alpha = 0.6
	i2.hue = 2

	but = g.addPythonButton((10, 10, 100, 60), Texture(TextureSource.THEME, "images/button.png"), onButtonClick)
	but.mouseover.hue = 13
	but.mousedown.rgba = (1.0, 1.0, 0.0)
	but.text = "foooobert"
	but.mousedown.fontRgba = rgba(4)
	but.mouseover.fontRgba = rgba("#00ffff")
	but.setFont("Arial", 8);
	i1.name = "i1"
	print g.store
	print i2
	g.store["i2"] = i2

	# cliloc test
	str2 = Cliloc(501522)
	str3 = Cliloc(1072058, [ "fo", "awef" ])
	print str2
	print str3

	g.addAlphaRegion((50, 50, 100, 100), 0.9)

	l1 = g.addLabel((300, 300), str2)
	print l1.halign
	l1.halign = HAlign.RIGHT
	print l1.halign

	return g