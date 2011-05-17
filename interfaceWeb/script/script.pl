#!/usr/bin/perl -w

use strict ; 



##########################################################
### DOCUMENTATION
# Ce script est ecrit par l'équipe de développement
# du groupe 22 du projet s2.
# Il est diffusé sous une licence libre GNU PL
# Il sert à manipuler des commandes d'administrations
# d'un serveur svn comme la création de dépot, 
# des utilisateurs...
##########################################################
#    Valeurs de retour:
# 0: success
# 1: echec de l'initialisation
# 2: Mauvais paramètres







#############################################
### CONFIGURATION VARIABLES
############################################
my $svnPath="svn";
my $svnAdminPath="svnadmin";
my $svnDir="/var/www/webprojetdev/script/";
my $tmpDir="/tmp/";
my $tmpFileName="resultSvnScript";
my $sortie=" >/dev/null 2>&1";




#############################################
### LECTURE DES PARAMETRES
#############################################
use utf8;

my $request="";

my $svnServer="";
my $svnPort="3690";
my $svnLogin="";
my $svnPassword="";

my $userLogin="";
my $userPassword="";

my $depotName="";


my $i;
for($i=0;$i<=$#ARGV;$i++)
{
	my $item=$ARGV[$i];
	if($item eq "--request")
	{
		$i++;
		$request=$ARGV[$i];
		next;
	}
	elsif($item eq "--svnServer")
	{
		$i++;
		$svnServer=$ARGV[$i];
		if(!($svnServer =~ m/\/$/)) {
			$svnServer=$svnServer."/";
		}
		next;
	}
	elsif($item eq "--svnPort")
	{
		$i++;
		$svnPort=$ARGV[$i];
		next;
	}
	elsif($item eq "--svnLogin")
	{
		$i++;
		$svnLogin=$ARGV[$i];
		next;
	}
	elsif($item eq "--depotName")
	{
		if(!($depotName =~ m/\//)) {
			$i++;
			$depotName=$ARGV[$i];
			next;
		}
	}
	elsif($item eq "--userLogin")
	{
		$i++;
		$userLogin=$ARGV[$i];
		next;
	}
	elsif($item eq "--userPassword")
	{
		$i++;
		$userPassword=$ARGV[$i];
		next;
	}
	else
	{
		die("Parametre ".$ARGV[$i]." inconnu");
	}
}



if(!($svnDir =~ m/\/$/)) {
	$svnDir=$svnDir."/";
}
if(!( -w $svnDir)) {
	print("Impossible de démarrer le script: Le chemin des dépots svn n'est pas accéssible en écriture\n");
	exit(1);
}
if(!($tmpDir =~ m/\/$/)) {
	$tmpDir=$tmpDir."/";
}
if(!( -w $tmpDir)) {
	print("Impossible de démarrer le script: Le chemin temporaire n'est pas accéssible en écriture\n");
	exit(1);
}




#############################################
### CREER UN DEPOT
#############################################

if($request eq "createDepot")
{
	if($depotName eq "")
	{
		print("Paramètres incorrects\n");
		exit(2);
	}
	my $depotPath=$svnDir.$depotName;

	if( -e $depotPath) {
		print("Ce dépot existe déjà\n");
		exit(3);
	}
	my $cmd="$svnAdminPath create $depotPath";
	system($cmd);
	if(!( -e $depotPath)) {
		print("Echec à la création du dépot.");
		exit(4);
	}

	&configureNewDepot($depotPath);
	exit(0);
}



#############################################
### SUPPRIMER UN DEPOT
#############################################

elsif($request eq "removeDepot")
{
	if($depotName eq "")
	{
		print("Paramètres incorrects\n");
		exit(2);
	}
	my $depotPath=$svnDir.$depotName;

	if(!( -e $depotPath)) {
		print("Ce dépot n'existe pas\n");
		exit(3);
	}
	my $cmd="rm -rf $depotPath";
	system($cmd);
	if( -e $depotPath) {
		print("Echec de la suppression du dépot\n");
		exit(5);
	}
	exit(0);
}



#############################################
### AJOUTER UN UTILISATEUR
#############################################

elsif($request eq "addUser")
{
	if(($depotName eq "") or ($userLogin eq "") or ($userPassword eq ""))
	{
		print("Paramètres incorrects\n");
		exit(2);
	}
	my $depotPath=$svnDir.$depotName;
	
	if(!( -e $depotPath)) {
		print("Ce dépot n'existe pas\n");
		exit(3);
	}
	&addDepotUser($depotPath,$userLogin,$userPassword);
	exit(0);
}





#############################################
### SUPPRIMER UN UTILISATEUR
#############################################

elsif($request eq "deleteUser")
{
	if(($depotName eq "") or ($userLogin eq ""))
	{
		print("Paramètres incorrects\n");
		exit(2);
	}
	my $depotPath=$svnDir.$depotName;

	if(!( -e $depotPath)) {
		print("Ce dépot n'existe pas\n");
		exit(3);
	}
	&deleteDepotUser($depotPath,$userLogin);
	exit(0);
}





#############################################
### CHANGER LE MOT DE PASSE D'UN UTILISATEUR
#############################################

elsif($request eq "updatePasswordUser")
{
	if(($depotName eq "") or ($userLogin eq "") or ($userPassword eq ""))
	{
		print("Paramètres incorrects\n");
		exit(2);
	}
	my $depotPath=$svnDir.$depotName;

	if(!( -e $depotPath)) {
		print("Ce dépot n'existe pas\n");
		exit(3);
	}
	&updateDepotUserPassword($depotPath,$userLogin,$userPassword);
	exit(0);
}





#############################################
### L'HISTORIQUE DU DEPOT
#############################################

elsif($request eq "history")
{
	if(($svnServer eq "") or ($depotName eq ""))
	{
		print("Paramètres incorrects\n");
		exit(2);
	}
	if ( -e $tmpDir.$depotName ) {
		my $cmd2="$svnPath update --username $svnLogin --password $svnPassword $tmpDir"."$depotName $sortie";
		system($cmd2);
	}
	else {
		my $cmd2="mkdir $tmpDir"."$depotName; $svnPath checkout --username $svnLogin --password $svnPassword $svnServer"."$depotName $tmpDir"."$depotName $sortie";
		system($cmd2);
	}
	my $cmd3="$svnPath diff --summarize --xml --depth infinity -r 0:HEAD $tmpDir"."$depotName > $tmpDir".$tmpFileName;
	system($cmd3);
	
	use XML::Simple;
	my $parser = XML::Simple->new( KeepRoot => 1 );
	my $doc = $parser->XMLin($tmpDir.$tmpFileName);
	foreach my $path ( @{ $doc->{diff}->{paths}->{path} } ) {
		print $path->{item}."\n";
		print substr($path->{content},length($tmpDir))."\n";
		print $path->{kind}."\n";
	}
	system("rm $tmpDir".$tmpFileName);
	exit(0);
}



#############################################
### COMMANDE INCONNUE
#############################################

else
{
	print("Requete $request inconnue\n");
	exit(2);
}








#############################################
### DES FONCTIONS TRES UTILES
#############################################


# Retourne le chemin du fichier de configuration du dépot
sub getDepotConfFilePath {
	my $depotPath=$_[0];
	if(!($depotPath =~ m/\/$/)) {
		$depotPath=$depotPath."/";
	}
	my $confPath=$depotPath."conf/svnserve.conf";
	if(!( -w $confPath)) {
		print("Le fichier de configuration du dépot $depotPath est inacessible en écriture\n");
		exit(6);
	}
	return $confPath;
}



# Retourne le chemin du fichier de mots de passes du dépot
sub getDepotPasswordFilePath {
	my $depotPath=$_[0];
	if(!($depotPath =~ m/\/$/)) {
		$depotPath=$depotPath."/";
	}
	my $confPath=&getDepotConfFilePath($depotPath);
	open(FILE,$confPath);
	my $passwordFilePath="";
	while (<FILE>) {
		if( $_ =~ m/^\s*password-db\s*=\s*(\S*)\s*/) {
			$passwordFilePath=$depotPath."conf/".$1;
		}
	}
	close(FILE);
	if(!( -w $passwordFilePath)) {
		print("Le fichier de mots de passes du dépot $depotPath est inacessible en écriture\n");
		exit(7);
	}
	return $passwordFilePath;
}




# Retourne les utilisateurs du dépot passé en paramètre
sub getDepotUsers {
	my $depotPath=$_[0];
	if(!($depotPath =~ m/\/$/)) {
		$depotPath=$depotPath."/";
	}
	my $passwordFilePath=&getDepotPasswordFilePath($depotPath);
	open(FILE,$passwordFilePath);
	my @users;
	while (<FILE>) {
		if( $_ =~ m/^\s*(\S*)\s*=\s*(\S*)\s*/) {
			push @users, {'login' => $1, 'password' => $2};
		}
	}
	close(FILE);
	return @users;
}





# Teste si le login passé correspond à utilisateur du dépot
sub isDepotUserLoginExists {
	my $depotPath=$_[0];
	my $login=$_[1];
	my @users=&getDepotUsers($depotPath);
	for (my $i=0;$i<=$#users;$i++) {
		if($users[$i]{'login'} eq $login) {
			return 0;
		}
	}
	return 1;
}



# Teste si les identifiants (login, password) passés en parametre correspondent
sub isDepotUserLoginPasswordExists {
	my $depotPath=$_[0];
	my $login=$_[1];
	my $password=$_[2];
	my @users=&getDepotUsers($depotPath);
	for (my $i=0;$i<=$#users;$i++) {
		if(($users[$i]{'login'} eq $login) and ($users[$i]{'password'} eq $password)) {
			return 0;
		}
	}
	return 1;
}




# Ajoute un utilisateur (login,password) au dépot
sub addDepotUser {
	my $depotPath=$_[0];
	my $login=$_[1];
	my $password=$_[2];
	if(!($depotPath =~ m/\/$/)) {
		$depotPath=$depotPath."/";
	}
	my $result=&isDepotUserLoginExists($depotPath,$login);
	if($result == 0) {
		print("Cet utilisateur existe déjà dans ce dépot\n");
		exit(8);
	}
	my $passwordFilePath=&getDepotPasswordFilePath($depotPath);
	open(FILE,">>$passwordFilePath");
	print FILE "\n$login = $password";
	close(FILE);
	$result=&isDepotUserLoginPasswordExists($depotPath,$login,$password);
	if($result != 0) {
		print("Erreur lors de l'ajout de l'utilisateur\n");
		exit(9);
	}
}




# Supprime un utilisateur (login,password) du dépot
sub deleteDepotUser {
	my $depotPath=$_[0];
	my $login=$_[1];
	if(!($depotPath =~ m/\/$/)) {
		$depotPath=$depotPath."/";
	}
	if(&isDepotUserLoginExists($depotPath,$login) != 0) {
		print("Cet utilisateur n'existe pas dans ce dépot\n");
		exit(10);
	}
	my $passwordFilePath=&getDepotPasswordFilePath($depotPath);
	open(FILE,"$passwordFilePath");
	my $content;
	while (<FILE>) {
		if(!( $_ =~ m/\s*$login\s*=\s*(\S*)\s*/)) {
			$content=$content.$_;
		}
	}
	close(FILE);
	open(FILE,">$passwordFilePath");
	print FILE $content;
	close(FILE);
	if(&isDepotUserLoginExists($depotPath,$login) == 0) {
		print("Echec lors de la suppression de l'utilisateur\n");
		exit(11);
	}
}







# Modifie le mdp d'un utilisateur du dépot
sub updateDepotUserPassword {
	my $depotPath=$_[0];
	my $login=$_[1];
	my $password=$_[2];
	if(!($depotPath =~ m/\/$/)) {
		$depotPath=$depotPath."/";
	}
	&deleteDepotUser($depotPath,$login);
	&addDepotUser($depotPath,$login,$password);
	close(FILE);
}





# Pour configurer un nouveau dépot créé
sub configureNewDepot {
	my $depotPath=$_[0];
	if(!($depotPath =~ m/\/$/)) {
		$depotPath=$depotPath."/";
	}
	my $confPath=&getDepotConfFilePath($depotPath);
	open(FILE,$confPath);
	my $content="";
	my $passwordFileName="passwd";
	while (<FILE>) {
		if( $_ =~ m/\s*password-db\s*=\s*(\S*)\s*/) {
			$passwordFileName=$1;
		}
		elsif(!( $_ =~ m/\s*anon-access\s*=\s*(\S*)\s*/) && !( $_ =~ m/\s*auth-access\s*=\s*(\S*)\s*/ ) ) {
			$content=$content.$_;
		}
	}
	close(FILE);
	open(FILE,">$confPath");
	print FILE $content;
	print FILE "\n password-db = ".$passwordFileName;
	print FILE "\n anon-access = none";
	print FILE "\n auth-access = write";
	close(FILE);
	open(FILE,">$depotPath"."conf/".$passwordFileName);
	print FILE "[users]\n";
	close(FILE);
}



