# -*- coding: utf-8 -*-
"""Rasteriza las marcas: prueba de legibilidad a tamano chico + assets."""
from PIL import Image, ImageDraw
import marks

SS = 8  # supersampling

def _draw(size, solids, holes, fill, ss=SS):
    n = size * ss
    im = Image.new("L", (n, n), 0)
    d = ImageDraw.Draw(im)
    for p in solids:
        d.polygon([(x * n / 100.0, y * n / 100.0) for x, y in p], fill=255)
    for p in holes:
        d.polygon([(x * n / 100.0, y * n / 100.0) for x, y in p], fill=0)
    mask = im.resize((size, size), Image.LANCZOS)
    out = Image.new("RGBA", (size, size), fill + (0,))
    out.putalpha(mask)
    return out


def cmy_mark(size, key, bg=(255, 255, 255)):
    """Las 3 planchas CMY desplazadas y multiplicadas.

    El glifo NO se cala: va encima en gris claro, como el "S3" de FileManagerS3.
    Calarlo dejaria flecos de color, porque el hueco de cada plancha cae
    desplazado respecto al de las otras dos.
    """
    solids, holes = marks.DIRECTIONS[key]()
    base = Image.new("RGB", (size, size), bg)
    for name, color in (("yellow", marks.YELLOW), ("magenta", marks.MAGENTA), ("cyan", marks.CYAN)):
        dx, dy = marks.OFFSETS[name]
        dx *= marks.OFFSET_SCALE * size
        dy *= marks.OFFSET_SCALE * size
        # El calado es parte de la SILUETA (no un glifo encima): se cala en cada
        # plancha y se desregistra con ella, como los agujeros de MediaTools.
        sh = lambda ps: [[(x + dx * 100.0 / size, y + dy * 100.0 / size) for x, y in p] for p in ps]
        layer = _draw(size, sh(solids), sh(holes), color)
        flat = Image.new("RGB", (size, size), (255, 255, 255))
        flat.paste(layer, (0, 0), layer)
        # multiply
        base = Image.eval(Image.merge("RGB", [
            Image.fromarray(__import__("numpy").uint8(
                __import__("numpy").asarray(b, dtype=float) * __import__("numpy").asarray(f, dtype=float) / 255.0))
            for b, f in zip(base.split(), flat.split())]), lambda v: v)
    return base


def mono_mark(size, key, color=(0, 0, 0)):
    """Variante plana para el tray: una sola silueta, sin desregistro."""
    solids, holes = marks.DIRECTIONS[key]()
    return _draw(size, solids, holes, color)


if __name__ == "__main__":
    sizes = [16, 20, 24, 32, 48, 128]
    pad, gap, label = 14, 18, 26
    cols = len(sizes)
    rowh = 128 + label + gap
    sheet = Image.new("RGB", (pad * 2 + sum(sizes) + gap * (cols - 1) + 260,
                              pad * 2 + rowh * len(marks.DIRECTIONS) * 2), (245, 245, 245))
    dr = ImageDraw.Draw(sheet)
    y = pad
    for key in sorted(marks.DIRECTIONS):
        for mode in ("cmy", "mono"):
            dr.text((pad, y + 4), f"{key} - {mode}", fill=(20, 20, 20))
            x = pad + 90
            for s in sizes:
                img = cmy_mark(s, key) if mode == "cmy" else None
                if mode == "mono":
                    m = mono_mark(s, key)
                    img = Image.new("RGB", (s, s), (255, 255, 255))
                    img.paste(m, (0, 0), m)
                sheet.paste(img, (x, y + label + (128 - s) // 2))
                dr.text((x, y + label + 128 + 2), str(s), fill=(90, 90, 90))
                x += s + gap
            y += rowh
    sheet.save("contact_sheet.png")
    print("contact_sheet.png", sheet.size)
