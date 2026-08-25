# -*- coding: utf-8 -*-
"""Geometria unica de las marcas de LGA_FolderSwitch.
Todo se define en un espacio de 100x100 y se escala. De aca salen tanto los
PNG (via PIL) como los SVG (para el canvas de diseno), asi no derivan.
"""

# Paleta LGA, medida de los PNG de las apps existentes (no estimada).
CYAN    = (0x26, 0xBA, 0xF1)
MAGENTA = (0xEF, 0x26, 0x9D)
YELLOW  = (0xFF, 0xF4, 0x26)
CORE    = (0x26, 0x26, 0x26)
GLYPH   = (0xD4, 0xD4, 0xD4)  # gris del glifo, medido del "S3" de FileManagerS3

# Desregistro de las 3 planchas: ~5.5% del tamano de la marca, a 120 grados.
# Medido en LGA_Player: magenta arriba, amarillo abajo-izq, cyan abajo-der.
OFFSETS = {
    "magenta": (  0.3, -18.3),
    "yellow":  (-19.7,  11.7),
    "cyan":    ( 19.3,   6.7),
}
OFFSET_SCALE = 1.0 / 362.0  # los valores de arriba son px sobre una marca de 362px


def _rr(x0, y0, x1, y1, r):
    """Rect redondeado como lista de puntos (aprox. con segmentos)."""
    import math
    pts = []
    for cx, cy, a0 in ((x1 - r, y0 + r, -90), (x1 - r, y1 - r, 0),
                       (x0 + r, y1 - r, 90), (x0 + r, y0 + r, 180)):
        for i in range(9):
            a = math.radians(a0 + i * 90 / 8)
            pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


def folder_body(x0=6, y0=24, x1=94, y1=88, tab_w=40, tab_h=10, r=7):
    """Silueta de carpeta gorda: pestana + cuerpo, una sola forma cerrada."""
    import math
    pts = []
    # esquina sup-izq de la pestana
    for i in range(9):
        a = math.radians(180 + i * 90 / 8)
        pts.append((x0 + r + r * math.cos(a), y0 + r + r * math.sin(a)))
    # borde superior de la pestana hasta el quiebre diagonal
    pts.append((x0 + tab_w, y0))
    pts.append((x0 + tab_w + 9, y0 + tab_h))          # diagonal
    # borde superior del cuerpo
    for i in range(9):
        a = math.radians(-90 + i * 90 / 8)
        pts.append((x1 - r + r * math.cos(a), y0 + tab_h + r + r * math.sin(a)))
    for i in range(9):
        a = math.radians(0 + i * 90 / 8)
        pts.append((x1 - r + r * math.cos(a), y1 - r + r * math.sin(a)))
    for i in range(9):
        a = math.radians(90 + i * 90 / 8)
        pts.append((x0 + r + r * math.cos(a), y1 - r + r * math.sin(a)))
    return pts


def arrow(x, y, w, h, direction="right", head=0.52, tail=0.42):
    """Flecha maciza tipo bloque. (x,y) esquina sup-izq del bounding box."""
    hh = h * tail / 2.0
    cy = y + h / 2.0
    hx = x + w * (1 - head) if direction == "right" else x + w * head
    if direction == "right":
        return [(x, cy - hh), (hx, cy - hh), (hx, y), (x + w, cy),
                (hx, y + h), (hx, cy + hh), (x, cy + hh)]
    return [(x + w, cy - hh), (hx, cy - hh), (hx, y), (x, cy),
            (hx, y + h), (hx, cy + hh), (x + w, cy + hh)]


# --- Direcciones -----------------------------------------------------------
# Sin glifo claro: la silueta sola dice el switch, como el triangulo de
# LGA_Player. Proporcion ~0.90 W/H y forma no muy ancha, para que el borde
# inferior no solape cyan+amarillo en una banda verde.

def _tab(x0, y0, tab_w, tab_h, r):
    """Pestana superior izquierda, comun a todas."""
    import math
    pts = []
    for i in range(9):
        a = math.radians(180 + i * 90 / 8)
        pts.append((x0 + r + r * math.cos(a), y0 + r + r * math.sin(a)))
    pts.append((x0 + tab_w, y0))
    pts.append((x0 + tab_w + 8, y0 + tab_h))
    return pts


def _corner(cx, cy, r, a0):
    import math
    return [(cx + r * math.cos(math.radians(a0 + i * 90 / 8)),
             cy + r * math.sin(math.radians(a0 + i * 90 / 8))) for i in range(9)]


def folder(x0=17, y0=14, x1=83, y1=94, tab_w=34, tab_h=13, r=8):
    """Carpeta clasica, proporcion ~0.85 W/H. La pestana franca es lo que hace
    que lea como carpeta y no como rectangulo redondeado."""
    pts = _tab(x0, y0, tab_w, tab_h, r)
    yb = y0 + tab_h
    pts += _corner(x1 - r, yb + r, r, -90)
    pts += _corner(x1 - r, y1 - r, r, 0)
    pts += _corner(x0 + r, y1 - r, r, 90)
    return pts


def _chevron(xc, yc, w, h, t):
    """Chevron macizo ">" centrado en (xc, yc)."""
    return [(xc - w / 2, yc - h / 2), (xc - w / 2 + t, yc - h / 2),
            (xc + w / 2, yc), (xc - w / 2 + t, yc + h / 2), (xc - w / 2, yc + h / 2),
            (xc + w / 2 - t, yc)]


def dir_t1():
    """T1 - Doble pestana: dos carpetas insinuadas en el borde superior.
    Todo masa, sin calado: el switch vive en la silueta."""
    import math
    x0, y0, x1, y1, r = 17, 12, 83, 94, 8
    s1_w, s1_h, s2_w, s2_h = 26, 9, 52, 9
    pts = _tab(x0, y0, s1_w, s1_h, r)          # primera pestana
    pts.append((x0 + s2_w, y0 + s1_h))          # escalon a la segunda
    pts.append((x0 + s2_w + 8, y0 + s1_h + s2_h))
    yb = y0 + s1_h + s2_h
    pts += _corner(x1 - r, yb + r, r, -90)
    pts += _corner(x1 - r, y1 - r, r, 0)
    pts += _corner(x0 + r, y1 - r, r, 90)
    return [pts], []


def dir_t2():
    """T2 - Calado chico tipo badge, abajo a la derecha. El desregistro queda
    contenido, como los agujeros de MediaTools."""
    return [folder()], [_chevron(64, 74, 18, 22, 7)]


def dir_t3():
    """T3 - Carpeta maciza con el lado derecho en punta."""
    x0, y0, y1, r, tab_w, tab_h = 17, 14, 94, 8, 34, 13
    shoulder, tip = 74, 88
    pts = _tab(x0, y0, tab_w, tab_h, r)
    yb = y0 + tab_h
    pts += _corner(shoulder - r, yb + r, r, -90)
    pts += [(tip, (yb + y1) / 2.0)]
    pts += _corner(shoulder - r, y1 - r, r, 0)
    pts += _corner(x0 + r, y1 - r, r, 90)
    return [pts], []


def dir_t4():
    """T4 - Carpeta inclinada: movimiento sin calar nada."""
    sk = 0.18
    base = folder(x0=14, y0=14, x1=78, y1=94)
    cy = 54.0
    return [[(x + (cy - y) * sk, y) for x, y in base]], []


def dir_t5():
    """T5 - Inclinada y en punta: la suma de T3 y T4."""
    x0, y0, y1, r, tab_w, tab_h = 14, 14, 94, 8, 32, 13
    shoulder, tip, sk, cy = 70, 84, 0.16, 54.0
    pts = _tab(x0, y0, tab_w, tab_h, r)
    yb = y0 + tab_h
    pts += _corner(shoulder - r, yb + r, r, -90)
    pts += [(tip, (yb + y1) / 2.0)]
    pts += _corner(shoulder - r, y1 - r, r, 0)
    pts += _corner(x0 + r, y1 - r, r, 90)
    return [[(x + (cy - y) * sk, y) for x, y in pts]], []


DIRECTIONS = {"T3": dir_t3, "T4": dir_t4, "T1": dir_t1, "T2": dir_t2}
