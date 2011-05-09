#include "ciphertool.h"

CipherTool::CipherTool(QString &cle):
		QCA::Cipher("aes128",QCA::Cipher::CBC,QCA::Cipher::DefaultPadding)
{
	//Initialise l'API QCA
	QCA::Initializer init;

	//Forme la clé, et un vecteur d'initialisation
	key = new QCA::SymmetricKey(cle.toLocal8Bit());
	iv = new QCA::InitializationVector(cle.toLocal8Bit());


}


QByteArray * CipherTool::EncrypteByteArray(QCA::SecureArray &data)
{
	//Prépare l'objet à encoder
	setup(QCA::Encode, *key, *iv);

	//Retourne directement le contenu codé
	QCA::SecureArray dataCoded = process(data);
	QByteArray * dataCoded2 = new QByteArray(dataCoded.toByteArray());
	return dataCoded2;

}


QCA::SecureArray * CipherTool::DecrypteByteArray(QByteArray &data)
{
	//Prépare l'objet à encoder
	setup(QCA::Decode, *key, *iv);

	//Retourne directement le contenu codé
	return ( new QCA::SecureArray(process(data)) );

}

