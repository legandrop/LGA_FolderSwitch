# -*- coding: utf-8 -*-
"""Emite SVG (para diseno) y los assets de produccion desde la misma geometria."""
import os
from PIL import Image
import marks, render, oncanvas

OUT_ICONS = os.path.join("..", "..", "resources", "icons")


def _path_d(poly):
    d = "M %.2f %.2f " % poly[0]
    for x, y in poly[1:]:
        d += "L %.2f %.2f " % (x, y)
    return d + "Z"


def svg_mark(key, size=256, plate=None, mono=None):
    """SVG de la marca. mono=color -> silueta plana (tray). plate -> fondo."""
    solids, holes = marks.DIRECTIONS[key]()
    d = " ".join(_path_d(p) for p in solids + holes)   # silueta calada (mono/tray)
    d_solid = " ".join(_path_d(p) for p in solids)     # planchas macizas (color)
    d_glyph = " ".join(_path_d(p) for p in holes)      # glifo encima, en gris
    bg = '<rect width="100" height="100" fill="%s"/>' % plate if plate else ""
    if mono:
        body = '<path d="%s" fill="%s" fill-rule="evenodd"/>' % (d, mono)
        iso = ""
    else:
        layers = []
        for name, col in (("yellow", marks.YELLOW), ("magenta", marks.MAGENTA), ("cyan", marks.CYAN)):
            dx, dy = marks.OFFSETS[name]
            dx *= marks.OFFSET_SCALE * 100
            dy *= marks.OFFSET_SCALE * 100
            layers.append(
                '<g transform="translate(%.2f %.2f)" style="mix-blend-mode: multiply">'
                '<path d="%s" fill="#%02X%02X%02X" fill-rule="evenodd"/></g>' % ((dx, dy, d_solid) + col))
        if d_glyph:
            layers.append('<path d="%s" fill="#%02X%02X%02X" fill-rule="evenodd"/>'
                          % ((d_glyph,) + marks.GLYPH))
        body = "".join(layers)
        iso = ' style="isolation: isolate"'
    return ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100" '
            'width="%d" height="%d"%s>%s%s</svg>' % (size, size, iso, bg, body))


if __name__ == "__main__":
    os.makedirs("svg", exist_ok=True)
    for key in marks.DIRECTIONS:
        open("svg/%s.svg" % key, "w").write(svg_mark(key))
        open("svg/%s_mono.svg" % key, "w").write(svg_mark(key, mono="#000000"))
    print("svg/ ->", sorted(os.listdir("svg")))

    KEY = "T3"  # marca elegida
    # PNG grande, RGBA con alfa real (transparente donde no hay tinta).
    big = oncanvas.cmy_rgba(1024, KEY)
    big.save(os.path.join(OUT_ICONS, "LGA_FolderSwitch_1024.png"))
    big.resize((512, 512), Image.LANCZOS).save(os.path.join(OUT_ICONS, "LGA_FolderSwitch.png"))
    # ICO multi-tamano, cada tamano renderizado directo (no escalado) para aprovechar el supersampling propio.
    ico = [oncanvas.cmy_rgba(s, KEY) for s in (256, 128, 64, 48, 32, 24, 16)]
    ico[0].save(os.path.join(OUT_ICONS, "LGA_FolderSwitch.ico"),
                sizes=[(i.width, i.height) for i in ico], append_images=ico[1:])
    # Tray: silueta negra con alpha; la app la tintea segun el tema.
    render.mono_mark(512, KEY, (0, 0, 0)).save(os.path.join(OUT_ICONS, "LGA_FolderSwitch_menubar.png"))
    print("assets ->", sorted(os.listdir(OUT_ICONS)))
