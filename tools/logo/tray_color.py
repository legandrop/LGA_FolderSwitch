# -*- coding: utf-8 -*-
"""Icono de bandeja en color: la marca T3 con desregistro CMY ajustado al pixel.

Sale un juego de PNG en resources/icons/tray/, uno por tamano (16 a 48 px, escalas 100-300 %):
  tray_<n>.png  cuerpo #262626 (el del app-icon), igual sobre barra oscura y clara
Sobre la barra oscura el cuerpo casi se pierde y la forma la dibuja el borde de color, como en el
tray de LGA_NukeShortcuts. La pausa no sale de aca: sigue siendo la silueta monocroma de
LGA_FolderSwitch_menubar.png.

Por que PNG y no SVG: QSvgRenderer no implementa mix-blend-mode y las planchas van en multiply.
Por que un PNG por tamano: a 16 px un desfase fraccionario se promedia con el fondo y ensucia el
borde; aca cada plancha se corre en pixeles enteros, round(n / 16) por paso.

Composicion: la misma direccion que el app-icon (amarillo a la izquierda, magenta arriba, cian abajo a
la derecha), asi tres lados del borde quedan en tintas puras, las que se leen sobre la barra oscura.
El cuerpo es la triple interseccion: los secundarios caen adentro de la silueta y los primarios afuera.
SHIFT centra en el canvas el conjunto de las tres planchas, redondeado a pixeles enteros.

Todo se compone por sub-pixel (supersample 16x, mascaras binarias) y se promedia, asi el antialias
del contorno sale exacto. Uso, desde tools/logo: python tray_color.py
Gemelo sin numpy para Windows: tray_color.ps1 (misma formula; si cambia una, cambiar las dos).
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
OFF16 = {"Y": (-1, 0), "M": (0, -1), "C": (1, 1)}  # px a 16; escala con round(n / 16)
SHIFT = (-2.5, -4.0)  # en unidades de 100
BODY = marks.CORE


def _mask(n, dx, dy):
    solids, holes = marks.DIRECTIONS[KEY]()
    W = n * SS
    im = Image.new("1", (W, W), 0)
    d = ImageDraw.Draw(im)
    k = W / 100.0
    # El centrado tambien va en pixeles enteros: un corrimiento fraccionario ensucia el borde.
    ox = (round(SHIFT[0] * n / 100) + dx) * SS
    oy = (round(SHIFT[1] * n / 100) + dy) * SS
    for p in solids:
        d.polygon([(x * k + ox, y * k + oy) for x, y in p], fill=1)
    for p in holes:
        d.polygon([(x * k + ox, y * k + oy) for x, y in p], fill=0)
    return np.array(im, dtype=bool)


def tray_icon(n):
    step = round(n / 16)
    m = {p: _mask(n, OFF16[p][0] * step, OFF16[p][1] * step) for p in "YMC"}
    W = n * SS
    rgb = np.full((W, W, 3), 255.0)
    cov = np.zeros((W, W), bool)
    for p in "YMC":
        rgb[m[p]] *= np.array(INK[p]) / 255.0
        cov |= m[p]
    rgb[m["Y"] & m["M"] & m["C"]] = BODY
    a = cov.astype(float)
    pre = (rgb * a[..., None]).reshape(n, SS, n, SS, 3).mean((1, 3))
    al = a.reshape(n, SS, n, SS).mean((1, 3))
    col = np.where(al[..., None] > 0, pre / np.maximum(al[..., None], 1e-9), 0)
    return Image.fromarray(np.dstack([col, al * 255]).round().clip(0, 255).astype(np.uint8), "RGBA")


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for n in SIZES:
        tray_icon(n).save(os.path.join(OUT, "tray_%d.png" % n), optimize=True)
    print("tray ->", sorted(os.listdir(OUT)))
