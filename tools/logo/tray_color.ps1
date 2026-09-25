# Gemelo de tray_color.py para Windows, sin numpy: la misma formula compuesta con System.Drawing.
# Arma resources/icons/tray/tray_<n>.png (16 a 48 px). Uso, desde la raiz del repo:
#   powershell -ExecutionPolicy Bypass -File tools\logo\tray_color.ps1
# La geometria la exporta marks.py (Python sin dependencias); si cambia la formula, cambiar los dos.
$ErrorActionPreference = 'Stop'
$logo = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent (Split-Path -Parent $logo)
$out = Join-Path $root 'resources\icons\tray'

Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @'
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;

public static class TrayColor {
    const int SS = 16;

    static bool[] Mask(double[][] poly, int n, double dx, double dy) {
        int W = n * SS;
        using (var bmp = new Bitmap(W, W, PixelFormat.Format32bppArgb))
        using (var g = Graphics.FromImage(bmp)) {
            g.SmoothingMode = SmoothingMode.None;
            g.Clear(Color.Black);
            double k = W / 100.0;
            var pts = new PointF[poly.Length];
            for (int i = 0; i < poly.Length; i++)
                pts[i] = new PointF((float)(poly[i][0] * k + dx), (float)(poly[i][1] * k + dy));
            g.FillPolygon(Brushes.White, pts);
            var d = bmp.LockBits(new Rectangle(0, 0, W, W), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
            var buf = new byte[W * W * 4];
            Marshal.Copy(d.Scan0, buf, 0, buf.Length);
            bmp.UnlockBits(d);
            var m = new bool[W * W];
            for (int i = 0; i < m.Length; i++) m[i] = buf[i * 4] > 127;
            return m;
        }
    }

    // off: desfase de Y, M y C en pasos (1 paso = round(n / 16) px); shift: corrimiento global en
    // unidades de 100. Un medio paso cae en pixel entero salvo cuando el paso es impar (16 y 20 px).
    public static void Icon(double[][] poly, int n, double[] off, double[] shift, int[] ink, int[] body, string path) {
        int step = (int)Math.Round(n / 16.0, MidpointRounding.ToEven);  // igual que round() de Python
        // El centrado tambien va en pixeles enteros: un corrimiento fraccionario ensucia el borde.
        double sx = Math.Round(shift[0] * n / 100.0) * SS, sy = Math.Round(shift[1] * n / 100.0) * SS;
        var mY = Mask(poly, n, sx + off[0] * step * SS, sy + off[1] * step * SS);
        var mM = Mask(poly, n, sx + off[2] * step * SS, sy + off[3] * step * SS);
        var mC = Mask(poly, n, sx + off[4] * step * SS, sy + off[5] * step * SS);
        int W = n * SS;
        var px = new byte[n * n * 4];
        for (int py = 0; py < n; py++)
        for (int qx = 0; qx < n; qx++) {
            double r = 0, gr = 0, b = 0, a = 0;
            for (int j = 0; j < SS; j++)
            for (int i = 0; i < SS; i++) {
                int p = (py * SS + j) * W + qx * SS + i;
                bool y = mY[p], m = mM[p], c = mC[p];
                if (!(y || m || c)) continue;
                double cr = 255, cg = 255, cb = 255;
                if (y && m && c) { cr = body[0]; cg = body[1]; cb = body[2]; }
                else {
                    if (y) { cr *= ink[0] / 255.0; cg *= ink[1] / 255.0; cb *= ink[2] / 255.0; }
                    if (m) { cr *= ink[3] / 255.0; cg *= ink[4] / 255.0; cb *= ink[5] / 255.0; }
                    if (c) { cr *= ink[6] / 255.0; cg *= ink[7] / 255.0; cb *= ink[8] / 255.0; }
                }
                r += cr; gr += cg; b += cb; a += 1;
            }
            int o = (py * n + qx) * 4;
            if (a > 0) {
                px[o + 2] = (byte)Math.Round(r / a); px[o + 1] = (byte)Math.Round(gr / a);
                px[o] = (byte)Math.Round(b / a); px[o + 3] = (byte)Math.Round(a * 255.0 / (SS * SS));
            }
        }
        using (var bmp = new Bitmap(n, n, PixelFormat.Format32bppArgb)) {
            var d = bmp.LockBits(new Rectangle(0, 0, n, n), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
            Marshal.Copy(px, 0, d.Scan0, px.Length);
            bmp.UnlockBits(d);
            bmp.Save(path, ImageFormat.Png);
        }
    }
}
'@

# Geometria y tintas salen de marks.py, la fuente unica.
$py = Get-Command python3.10, python3, python -ErrorAction SilentlyContinue | Select-Object -First 1
Push-Location $logo
try {
    $json = & $py.Source -c "import json,marks;s,h=marks.DIRECTIONS['T3']();print(json.dumps({'p':s[0],'ink':marks.YELLOW+marks.MAGENTA+marks.CYAN,'body':marks.CORE}))"
} finally { Pop-Location }
$j = $json | ConvertFrom-Json
$poly = [double[][]]@($j.p | ForEach-Object { ,([double[]]@($_[0], $_[1])) })

# Misma direccion que el app-icon: amarillo a la izquierda, magenta arriba, cian abajo a la derecha.
# El cian va a medio paso: con un paso entero el borde de abajo y el de la derecha pesaban el doble.
$off = [double[]]@(-1, 0, 0, -1, 0.5, 0.5)
$shift = [double[]]@(-2.5, -4.0)   # centra en el canvas el conjunto de las tres planchas
New-Item -ItemType Directory -Force $out | Out-Null
foreach ($n in 16, 20, 24, 32, 40, 48) {
    [TrayColor]::Icon($poly, $n, $off, $shift, [int[]]$j.ink, [int[]]$j.body, (Join-Path $out "tray_$n.png"))
}
Get-ChildItem $out -Filter 'tray_*.png' | ForEach-Object { $_.Name }
