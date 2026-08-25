# -*- coding: utf-8 -*-
"""Compone la marca sobre capturas reales de la barra y el tray."""
from PIL import Image, ImageDraw
import numpy as np, marks, render

def cmy_rgba(size, key):
    """Marca con alfa real: transparente donde no hay tinta."""
    solids, holes = marks.DIRECTIONS[key]()
    rgb = np.ones((size, size, 3), dtype=float)
    alpha = np.zeros((size, size), dtype=float)
    for name, col in (("yellow", marks.YELLOW), ("magenta", marks.MAGENTA), ("cyan", marks.CYAN)):
        dx, dy = marks.OFFSETS[name]
        dx *= marks.OFFSET_SCALE * size
        dy *= marks.OFFSET_SCALE * size
        sh = lambda ps: [[(x + dx * 100.0 / size, y + dy * 100.0 / size) for x, y in p] for p in ps]
        lay = render._draw(size, sh(solids), sh(holes), col)
        a = np.asarray(lay.getchannel("A"), dtype=float) / 255.0
        c = np.asarray(col, dtype=float) / 255.0
        rgb *= (1.0 - a[..., None]) + a[..., None] * c[None, None, :]
        alpha = alpha + a - alpha * a
    out = Image.fromarray(np.uint8(np.clip(rgb, 0, 1) * 255))
    out.putalpha(Image.fromarray(np.uint8(alpha * 255)))
    return out


def mono_rgba(size, key, color=(255, 255, 255)):
    return render.mono_mark(size, key, color)


def place(bg_path, spots, size, mode, key="C2", zoom=3):
    bg = Image.open(bg_path).convert("RGBA")
    mark = cmy_rgba(size, key) if mode == "cmy" else mono_rgba(size, key)
    for (x, y) in spots:
        bg.alpha_composite(mark, (x, y))
    return bg.resize((bg.width * zoom, bg.height * zoom), Image.NEAREST)
