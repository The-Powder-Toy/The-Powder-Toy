# Styled Icons

This directory contains artwork along with platform-specific templates and icons for optional use by downstream distributors.

This README reflects the current contents of this directory. If you add to or update the contents of this directory, please update this README accordingly.

## Icon Components

There are three subfolders here:

1. **Templates:** these files can be used with SVG artwork to create icons styled for different desktop environments.
2. **Contents:** these files are SVG artwork that can be used with different templates for different desktop environments.
3. **Composites:** these files are "contents" files that have already been combined with "templates" files.

> **Note:** at present the macOS Big Sur icon template does not have top-edge glow, but it would probably look better if it had one. I just haven't figured out how to do it manually in SVG, yet.

## Editing and Conversion

These SVG files were created out of raw XML in VS Code with [a particular SVG preview](https://marketplace.visualstudio.com/items?itemName=jock.svg) extension. Not all SVG extensions support all the features of SVG. Because these were written in raw XML, you may create mangled syntax if you try to edit them in a graphical editor. The best editor for not mangling the handwritten syntax *too much* is [Inkscape](https://inkscape.org/), which uses SVG natively and therefore will not lose quality due to format conversion. You should probably save your work under a different filename, though.

The files use a nominal resolution of 32px x 32px in order to keep the pixel grid easy to visualize. For some desktop environments, you may need to export from SVG to PNG. On Linux, you can create 1024px x 1024px PNGs using ImageMagick with the following command:

```bash
$ convert -density 3072 -background none <filename>.svg <filename>.png
```

(Other renderers may support a different subset of SVG features, so you may need to experiment to get the correct result elsewhere.)

## Tweaking "Fireball" Layouts

Where the "contents" files use "fireballs", these fireballs are simple radial gradients that are scaled and overlaid on each other, with specific CSS blending modes and filters applied.

* To move and resize the fireballs, find the tags labeled `<use xlink:href="#fireball" />`. `translate(<x> <y>)` represents the coordinates of the center of the fireball, and `scale(<num>)` is the scale factor (the prototype fireball being 1.5px x 1.25px). These values do not have to be whole numbers, but I used whole numbers to keep the code more legible.

* The "haze" effect comes from a filter translating [Perlin noise](https://en.wikipedia.org/wiki/Perlin_noise) into localized displacement. I used [the example from the MDN](https://developer.mozilla.org/en-US/docs/Web/SVG/Element/feTurbulence), if you want to play around with the parameters.

* The fireballs in the directories at this point use the "lighten" [blending mode](https://en.wikipedia.org/wiki/Blend_modes). The blending mode used by The Powder Toy (e.g. in the historical icon) seems to be the ["color dodge" blending mode](https://en.wikipedia.org/wiki/Blend_modes#Dodge_and_burn). With the corrent color gradients, the "color dodge" blending mode is a bit burnt out, but you might get better results if you change the gradient values. You can find a full list of CSS blend modes [at the MDN here](https://developer.mozilla.org/en-US/docs/Web/CSS/blend-mode).

* The fireball gradient at present is made up of simple ratios of red and yellow. Colors along the [Planckian locus](https://en.wikipedia.org/wiki/Planckian_locus) may be more accurate, but the more cartoonish colors seem to be a better match for the colors used by The Powder Toy.

* At present, the gradient is applied to the fireball prototypes, which are then scaled. The gradient scales with each fireball, which may not be the best representation of how the heat glow of an actual fireball is distributed. For example, the gradient may maintain the same absolute scale aropund the perimeter of the fireball's silhouette, or it may scale logarithmically, idk. Long story short, you may get more realistic results if you copy the gradient code into each fireball separately and tweak the gradient stops to better reflect the fireball's scale. This could be automated in JavaScript, but I really didn't feel like going down the rabbit hole of beginning to create a physics simulation within the icon itself.

* There are XML comments within the SVG files themselves with some more explanation around how they work and how to work with them.

## Credits

GNOME "monitor" frame taken from [GNOME Terminal](https://gitlab.gnome.org/GNOME/gnome-terminal/blob/master/data/icons/hicolor_apps_scalable_org.gnome.Terminal.svg)

Fireballs and compositing by [Elsie Hupp](https://github.com/elsiehupp/), 2021.

(Add your name here if you contribute further.)

## License

SVG files containing elements of the GNOME Terminal icon are GPL-3, per [GNOME Terminal](https://gitlab.gnome.org/GNOME/gnome-terminal/).

Everything else is also GPL-3, per [The Powder Toy](https://github.com/The-Powder-Toy/The-Powder-Toy) as a whole.