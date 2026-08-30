#ifndef FOLDERSWITCH_VERSIONCOMPARE_H
#define FOLDERSWITCH_VERSIONCOMPARE_H

#include <QString>

// Comparacion de versiones LGA, portada de FM_VersionCompare (LGA_FileManagerS3).
// Fuente unica de verdad para decidir si el tag de un release es mas nuevo que la
// version instalada (FOLDERSWITCH_VERSION).
//
// Por que no QVersionNumber crudo: el versionado LGA escribe el minor con ancho fijo
// (las apps Qt usan "X.YYY"), asi que "1.5" equivale a "1.500" y NO es menor que "1.05".
// QVersionNumber::fromString("1.5") da [1,5], que compara mal contra [1,50] o [1,500].
namespace VersionCompare {

// Ancho fijo del minor para las apps Qt de LGA (esta entre ellas).
inline constexpr int kMinorWidth = 3;

// True si candidateVersion es mas nueva que referenceVersion. Ambas toleran el
// prefijo "v" y espacios. Si alguna no tiene la forma "X.Y" cae a QVersionNumber
// crudo en vez de rechazarla, para no dejar de reconocer un esquema imprevisto.
bool isNewer(const QString &candidateVersion, const QString &referenceVersion);

} // namespace VersionCompare

#endif // FOLDERSWITCH_VERSIONCOMPARE_H
