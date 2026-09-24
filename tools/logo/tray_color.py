# -*- coding: utf-8 -*-
"""Icono de bandeja en color: la marca T3 con desregistro CMY ajustado al pixel.

Salen dos juegos de PNG en resources/icons/tray/, uno por tamano (16 a 48 px, escalas 100-300 %):
  tray_white_<n>.png  cuerpo blanco, para la barra de tareas oscura
  tray_dark_<n>.png   cuerpo #262626 (el del app-icon), para la barra clara
La pausa no sale de aca: sigue siendo la silueta monocroma de LGA_FolderSwitch_menubar.png.

Por que PNG y no SVG: QSvgRenderer no implementa mix-blend-mode y las planchas van en multiply.
Por que un PNG por tamano: a 16 px un desfase fraccionario se promedia con el fondo y ensucia el
borde; aca cada plancha se corre en pixeles enteros, round(n / 16) por paso.

Composicion (la misma formula del tray de FrameRev, para que la familia se lea igual): amarillo
quieto, magenta a la derecha, cian abajo. El cuerpo es la triple interseccion, como en el app-icon:
los secundarios caen adentro de la silueta y los primarios afuera.

Todo se compone por sub-pixel (supersample 16x, mascaras binarias) y se promedia, asi el antialias
del contorno sale exacto. Uso, desde tools/logo: python tray_color.py
"""
import os
import numpy as np
from PIL import Image, ImageDraw
import marks

KEY = "T3"
OUT = os.path.join("..", "..", "resources", "icons", "tray")
SIZES = (16, 20, 24, 32, 40, 48)
SS = 16
INK = {"Y": marks.YELLOW, "M": marks.MAGENTA, "C": marks.CYAN}
OFF16 = {"Y": (0, 0), "M": (1, 0), "C": (0, 1)}  # px a 16; escala con round(n / 16)
BODIES = {"white": (255, 255, 255), "dark": (0x26, 0x26, 0x26)}


def _mask(n, dx, dy):
    solids, holes = marks.DIRECTIONS[KEY]()
    W = n * SS
    im = Image.new("1", (W, W), 0)
    d = ImageDraw.Draw(im)
    k = W / 100.0
    for p in solids:
        d.polygon([(x * k + dx * SS, y * k + dy * SS) for x, y in p], fill=1)
    for p in holes:
        d.polygon([(x * k + dx * SS, y * k + dy * SS) for x, y in p], fill=0)
    return np.array(im, dtype=bool)


def tray_icon(n, body):
    step = round(n / 16)
    m = {p: _mask(n, OFF16[p][0] * step, OFF16[p][1] * step) for p in "YMC"}
    W = n * SS
    rgb = np.full((W, W, 3), 255.0)
    cov = np.zeros((W, W), bool)
    for p in "YMC":
        rgb[m[p]] *= np.array(INK[p]) / 255.0
        cov |= m[p]
    rgb[m["Y"] & m["M"] & m["C"]] = body
    a = cov.astype(float)
    pre = (rgb * a[..., None]).reshape(n, SS, n, SS, 3).mean((1, 3))
    al = a.reshape(n, SS, n, SS).mean((1, 3))
    col = np.where(al[..., None] > 0, pre / np.maximum(al[..., None], 1e-9), 0)
    return Image.fromarray(np.dstack([col, al * 255]).round().clip(0, 255).astype(np.uint8), "RGBA")


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for name, body in BODIES.items():
        for n in SIZES:
            tray_icon(n, body).save(os.path.join(OUT, "tray_%s_%d.png" % (name, n)), optimize=True)
    print("tray ->", sorted(os.listdir(OUT)))
