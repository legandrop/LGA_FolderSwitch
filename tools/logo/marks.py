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
# Cada una devuelve (solidos, huecos): poligonos que se pintan y que se calan.

def dir_c():
    """C - Carpeta con flecha maciza calada entrando. Linea base."""
    return [folder_body()], [arrow(20, 46, 60, 30, "right")]


def dir_c2():
    """C2 - Flecha gorda dentro del core. Marca elegida.

    La caja de la flecha se ajusta al CORE (la interseccion de las 3 planchas,
    x 11..88 / y 34..83 segun fitcore.py), no a la silueta: si se ajustara a la
    silueta, la punta se saldria del negro y quedaria flotando en los flecos.
    """
    return [folder_body()], [arrow(20, 45, 60, 30, "right")]


def dir_a2():
    """A2 - Doble flecha, pero mucho mas gorda (test del swap legible)."""
    return [folder_body()], [arrow(20, 42, 60, 19, "right", tail=0.60),
                             arrow(20, 66, 60, 19, "left", tail=0.60)]


def dir_e():
    """E - La flecha rompe el borde derecho: el salto sale de la carpeta."""
    f = folder_body()
    a = arrow(30, 45, 74, 32, "right")
    return [f, a], [arrow(20, 52, 30, 18, "right")]


DIRECTIONS = {"C": dir_c, "C2": dir_c2, "A2": dir_a2, "E": dir_e}
