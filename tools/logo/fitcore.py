# -*- coding: utf-8 -*-
"""Calcula la bbox del core (interseccion de las 3 planchas) en unidades 0-100."""
from PIL import Image, ImageDraw
import numpy as np, marks

def core_bbox(key, N=1200):
    solids, _ = marks.DIRECTIONS[key]()
    inter = None
    for name in ("yellow", "magenta", "cyan"):
        dx, dy = marks.OFFSETS[name]
        dx *= marks.OFFSET_SCALE * 100
        dy *= marks.OFFSET_SCALE * 100
        im = Image.new("L", (N, N), 0)
        d = ImageDraw.Draw(im)
        for p in solids:
            d.polygon([((x + dx) * N / 100.0, (y + dy) * N / 100.0) for x, y in p], fill=255)
        a = np.asarray(im) > 127
        inter = a if inter is None else (inter & a)
    ys, xs = np.nonzero(inter)
    return (xs.min() * 100.0 / N, ys.min() * 100.0 / N,
            xs.max() * 100.0 / N, ys.max() * 100.0 / N)

if __name__ == "__main__":
    for k in ("C2",):
        x0, y0, x1, y1 = core_bbox(k)
        print(f"{k}: core x {x0:.1f}..{x1:.1f} (w {x1-x0:.1f})  y {y0:.1f}..{y1:.1f} (h {y1-y0:.1f})")
