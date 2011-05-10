#include "ciphertool.h"

CipherTool::CipherTool(const QString &cle):
		QCA::Cipher("aes128",QCA::Cipher::CBC,QCA::Cipher::DefaultPadding)
{
	// Création du tableau qui recevra le hash de "cle"
	QCA::SecureArray hash;
	// Instanciation du hasher
	QCA::Hash hasher("sha1");

	// On ajoute la chaîne cle aux données à hasher
	hasher.update (cle.toLocal8Bit());
	// On écrit dans le tableau le hash
	hash = hasher.final();

	//Forme la clé, et un vecteur d'initialisation à partir du hash créé
	key = new QCA::SymmetricKey(hash);
	iv = new QCA::InitializationVector(hash);


}


QByteArray * CipherTool::encrypteByteArray(const QCA::SecureArray &data)
{
	//Prépare l'objet à encoder
	setup(QCA::Encode, *key, *iv);

	//Retourne directement le contenu codé
	QCA::SecureArray dataCoded = process(data);
	QByteArray * dataCoded2 = new QByteArray(dataCoded.toByteArray());
	return dataCoded2;

}


QCA::SecureArray * CipherTool::decrypteByteArray(const QByteArray &data)
{
	//Prépare l'objet à décoder
	setup(QCA::Decode, *key, *iv);

	QCA::SecureArray *dataDecoded = new QCA::SecureArray(process((QCA::SecureArray)data));
	return dataDecoded;

}
