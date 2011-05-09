#ifndef CIPHERTOOL_H
#define CIPHERTOOL_H

#include <QtCrypto/qca.h>
#include <QFile>

/* Classe qui crypte et décrypte les fichiers de config, pour qu'ils ne soient pas visibles
   par qui le veut
   Utilise l'algo AES128-CBS avec padding
*/


class CipherTool: public QCA::Cipher
{
public:
    CipherTool(QString &cle);

    QByteArray * EncrypteByteArray(QCA::SecureArray &data);
    QCA::SecureArray * DecrypteByteArray(QByteArray &data);

private:

    QCA::SymmetricKey *key;		//Clé de cryptage/décryptage
    QCA::InitializationVector *iv;	//Vecteur d'initialisation

};

#endif // CIPHERTOOL_H
