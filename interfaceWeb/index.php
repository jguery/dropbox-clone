<?php
//Pour activer les messages d'erreur et de warnings
//Celà sert pour le debuggage
ini_set('display_errors', 1);
ini_set('log_errors', 1);
error_reporting(E_ALL);
?>

<?php

///////////////////////////////////////////////////////////////
///                 CONFIGURATION                           ///
///////////////////////////////////////////////////////////////
// You need to configure these options before lauching

$ApplicationName="CloneDropBox";                	    //Give a name to the application

$DataBasePath="bdd/serverDB";				//The sqlite database relative path
$DataBaseUser="serverUser";				//A login to access to database
$DataBasePassword="serverPassword";			//A password for the login

$ImgPath="img/";					//The relative dir path where images files are stored
$ScriptPath="script/script.pl";				//The relative path of the script file



///////////////////////////////////////////////////////////////////
///    STARTING SESSIONS AND HTML HEADER                        ///
///////////////////////////////////////////////////////////////////

session_start();
?>

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="fr" lang="fr">
<head>
<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />
<title><?php echo $ApplicationName." - Interface web"; ?></title>

<style type="text/css">
/*************************************************************/
/*             CSS STYLE SHEET                               */
/*************************************************************/
body
{
	margin: 10px;
	padding: 0px;
	font-family: Arial, Helvetica, sans-serif;
	font-size: 14px;
	color: #000000;
	background-color: #e0ebf6;
}
a
{
	color: #03F;
	text-decoration: none;
	cursor :pointer;
}
img
{
	border:none;
}
a:hover
{
	color: #06F;
	text-decoration: underline;
}
</style>
</head>

<?php

///////////////////////////////////////////////////
/////       RECUPERATION DES VARIABLES         ////
///////////////////////////////////////////////////

//Les variables de session
if(!isset($_SESSION['login'])) $_SESSION['login']='';
if(!isset($_SESSION['session'])) $_SESSION['session']='';
if(!isset($_SESSION['depotname'])) $_SESSION['depotname']='';

//Le nom de la page
$info = pathinfo($_SERVER['PHP_SELF']);
$thisName = $info['basename'];
define("PAGE", $thisName);

//Les paramètres passés par GET
$action=isset($_GET['action'])?$_GET['action']:'acceuil';
$session=isset($_GET['session'])?$_GET['session']:'';





///////////////////////////////////////////////////
/////       AUTHENTIFICATION
//////////////////////////////////////////////////

if($action=="authentificate")
{
	$showForm=true;
	$error="";
	if($_SESSION['login']!='')
	{
		$action='acceuil';
		$showForm=false;
	}
	else if(isset($_POST['login']) and isset($_POST['password']) and ($session!="admin" or isset($_POST['depotname'])))
	{
		$login=$_POST['login'];
		$password=$_POST['password'];
		$depotname=isset($_POST['depotname'])?$_POST['depotname']:'';
		if($login!="" and $password!="" and ($session!="admin" or $depotname!=""))
		{
			try{
				$dbHandle = new PDO("sqlite:$DataBasePath");
				$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($login)." and password=".$dbHandle->quote($password);
				$resultats=$dbHandle->query($sqlRequest);
				if($result=$resultats->fetch(PDO::FETCH_BOTH))
				{
					if($session=="user")
					{
						$action="acceuil";
						$_SESSION['login']=$login;
						$_SESSION['session']='user';
						$showForm=false;
					}
					else if($session=="admin")
					{
						$sqlRequest = "SELECT * FROM admin WHERE login=".$dbHandle->quote($login)." and depotname=".$dbHandle->quote($depotname);
						$resultats=$dbHandle->query($sqlRequest);
						if($result=$resultats->fetch())
						{
							$action="acceuil";
							$_SESSION['login']=$login;
							$_SESSION['depotname']=$depotname;
							$_SESSION['session']='admin';
							$showForm=false;
						}
						else $error="Vous n'&ecirc;tes pas administrateur de ce d&eacute;pot.";
					}
					else if($session=="superadmin")
					{
						$sqlRequest = "SELECT * FROM superadmin WHERE login=".$dbHandle->quote($login);
						$resultats=$dbHandle->query($sqlRequest);
						if($result=$resultats->fetch())
						{
							$action="acceuil";
							$_SESSION['login']=$login;
							$_SESSION['session']='superadmin';
							$showForm=false;
						}
						else $error="Vous n'&ecirc;tes pas super-administrateur.";
					}
					else $error="Variable session incorrecte.";
				}
				else $error="Ce compte n'existe pas dans la base de donn&eacute;es.";
			}
			catch( PDOException $exception ){
				$error='Error connecting to database'.$exception->getMessage();
			}
		}
		else $error="Veuillez remplir tous les champs.";
	}
	if($showForm)
	{
	if($session=='user')
	{
		$name="utilisateur";
		$img=$ImgPath.'user.png';
	}
	else if($session=='admin')
	{
		$name="administrateur";
		$img=$ImgPath.'admin.png';
	}
	else if($session=='superadmin')
	{
		$name="super-administrateur";
		$img=$ImgPath.'superadmin.png';
	}
	else
	{
		echo entete($error);
		echo "Erreur paramètre session=$session incorrect";
		$showForm=false;
	}
	if($showForm)
	{
	echo entete($name);
	echo $error;
	$form ='<form action="'.PAGE.'?action=authentificate&session='.$session.'" method="POST"><table>';
	$form.='<tr><td><label for="login">Login</label></td><td><input type="text" name="login" id="login"/></td></tr>';
	$form.='<tr><td><label for="password">Password</label></td><td><input type="password" name="password" id="password"/></td></tr>';
	if($session=="admin") $form.='<tr><td><label for="depotname">D&eacute;pot</label></td><td><input type="text" name="depotname" id="depotname"/></td></tr>';
	$form.='<tr><td align="right" colspan="2"><input style="width:100px;" type="submit" value="Valider"/></td></tr>';
	$form.='</table></form>';
	echo '<div style="margin-top:50px;"><table align="center"><tr><td><img src="'.$img.'"/></td><td>'.$form.'</td></tr></table></div>';
	}
	}
}



///////////////////////////////////////////////////
/////       DECONNEXION
//////////////////////////////////////////////////

if($action=="deconnexion")
{
	if($_SESSION['login']!='')
	{
		$_SESSION["login"]="";
		$_SESSION["session"]="";
		$_SESSION["depotname"]="";
	}
	$action="acceuil";
}




///////////////////////////////////////////////////
/////      MODIF INFOS
//////////////////////////////////////////////////

if($action=="modifyInfos")
{
	$showForm=true;
	$error="";
	if($_SESSION['login']=='')
	{
		$showForm=false;
		$action="acceuil";
		$error="Vous n'&ecirc;tes pas identifi&eacute;";
	}
	else if(isset($_POST['firstname']) and isset($_POST['lastname']))
	{
		$firstname=$_POST['firstname'];
		$lastname=$_POST['lastname'];
		$login=$_SESSION['login'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "UPDATE user SET firstname=".$dbHandle->quote($firstname).", lastname=".$dbHandle->quote($lastname)." WHERE login=".$dbHandle->quote($login);
			$dbHandle->query($sqlRequest);
			$showForm=false;
			$action="acceuil";
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	if($showForm)
	{
	echo entete("Modifier mes infos");
	echo $error;
	$form ='<form action="'.PAGE.'?action=modifyInfos" method="POST"><table>';
	$form.='<tr><td><label for="lastname">Nom</label></td><td><input type="text" name="lastname" id="lastname"/></td></tr>';
	$form.='<tr><td><label for="firstname">Pr&eacute;nom</label></td><td><input type="text" name="firstname" id="firstname"/></td></tr>';
	$form.='<tr><td align="right" colspan="2"><input style="width:100px;" type="submit" value="Valider"/></td></tr>';
	$form.='</table></form>';
	echo '<div style="margin-top:50px;"><table align="center"><tr><td>'.$form.'</td></tr></table></div>';
	}
}




///////////////////////////////////////////////////
/////      MODIF PASSWORD
//////////////////////////////////////////////////

if($action=="modifyPassword")
{
	$showForm=true;
	$error="";
	if($_SESSION['login']=='')
	{
		$showForm=false;
		$action="acceuil";
		$error="Vous n'&ecirc;tes pas identifi&eacute;";
	}
	else if(isset($_POST['oldpassword']) and isset($_POST['newpassword']) and isset($_POST['newpassword2']))
	{
		$oldpassword=$_POST['oldpassword'];
		$newpassword=$_POST['newpassword'];
		$newpassword2=$_POST['newpassword2'];
		$login=$_SESSION['login'];
		if($newpassword==$newpassword2 and $newpassword!="" and $oldpassword!="")
		{
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($login)." and password=".$dbHandle->quote($oldpassword);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
				$sqlRequest = "UPDATE user SET password=".$dbHandle->quote($newpassword)." WHERE login=".$dbHandle->quote($login);
				$dbHandle->query($sqlRequest);
				$ret=0;
				$resp=array();
				$sqlRequest = "SELECT depotname FROM utilisation WHERE login=".$dbHandle->quote($login);
				$resultats=$dbHandle->query($sqlRequest);
				while($result=$resultats->fetch(PDO::FETCH_BOTH))
				{
					exec("$ScriptPath --request updatePasswordUser --depotName ".$result['depotname']." --userLogin $login --userPassword $newpassword",$resp,$ret);
					if($ret!=0) foreach($resp as $r) $error=$error."\n".$r;
				}
				$showForm=false;
				$action="acceuil";
			}
			else $error="Ancien mot de passe incorrect.";
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
		}
		else $error='Formulaire mal rempli.';
	}
	if($showForm)
	{
	echo entete("Modifier mon mot de passe");
	echo $error;
	$form ='<form action="'.PAGE.'?action=modifyPassword" method="POST"><table>';
	$form.='<tr><td><label for="oldpassword">Ancien mot de passe</label></td><td><input type="password" name="oldpassword" id="oldpassword"/></td></tr>';
	$form.='<tr><td><label for="newpassword">Nouveau mot de passe</label></td><td><input type="password" name="newpassword" id="newpassword"/></td></tr>';
	$form.='<tr><td><label for="newpassword2">Re-entrez le mot de passe</label></td><td><input type="password" name="newpassword2" id="newpassword2"/></td></tr>';
	$form.='<tr><td align="right" colspan="2"><input style="width:100px;" type="submit" value="Valider"/></td></tr>';
	$form.='</table></form>';
	echo '<div style="margin-top:50px;"><table align="center"><tr><td>'.$form.'</td></tr></table></div>';
	}
}





///////////////////////////////////////////////////
/////      AJOUTER MEMBRE
//////////////////////////////////////////////////

if($action=="ajouterMembre")
{
	if($_SESSION['login']!='' and ($_SESSION['session']=='admin' or $_SESSION['session']=='superadmin'))
	{
	if(isset($_POST['login']))
	{
		$login=$_POST['login'];
		$depotname=$_SESSION['depotname'];
		$readonly=(isset($_POST['readonly']) and $_POST['readonly']=='on')?'true':'false';
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($login);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
				$password=$result['password'];
				$sqlRequest = "SELECT * FROM utilisation WHERE depotname=".$dbHandle->quote($depotname)." and login=".$dbHandle->quote($login);
				$resultats=$dbHandle->query($sqlRequest);
				if(!($result=$resultats->fetch(PDO::FETCH_BOTH)))
				{
					$sqlRequest = "INSERT INTO utilisation(login,depotname,readonly,inscriptiondate) VALUES(".$dbHandle->quote($login).",".$dbHandle->quote($depotname).",".$dbHandle->quote($readonly).",datetime())";
					$dbHandle->query($sqlRequest);
					$error="";
					$ret=0;
					$resp=array();
					exec("$ScriptPath --request addUser --depotName $depotname --userLogin $login --userPassword $password",$resp,$ret);
					if($ret!=0) foreach($resp as $r) $error=$error."\n".$r;
					$showForm=false;
				}
				else $error='Cet utilisateur est d&eacute;j&agrave; ajout&eacute;.';
			}
			else $error='Ce compte n\'existe pas.';
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	else $error="Formulaire mal rempli";
	}
	else $error="Vous n'&ecirc;tes pas identifi&eacute;";
	$action="acceuil";
	if($_SESSION['session']=='superadmin')
	{
		$action='modifyDepot';
		$_GET['depotname']=isset($_SESSION['depotname'])?$_SESSION['depotname']:'';
	}
}






///////////////////////////////////////////////////
/////      MODIFIER MEMBRE
//////////////////////////////////////////////////

if($action=="modifyMembre")
{
	$showForm=true;
	$error="";
	if($_SESSION['login']!='' and ($_SESSION['session']=='admin' or $_SESSION['session']=='superadmin'))
	{
	if(isset($_POST['login']))
	{
		$login=$_POST['login'];
		$readonly=(isset($_POST['readonly']) and $_POST['readonly']=='on')?'true':'false';
		$depotname=$_SESSION['depotname'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "UPDATE utilisation SET readonly=".$dbHandle->quote($readonly)." WHERE depotname=".$dbHandle->quote($depotname)." and login=".$dbHandle->quote($login);
			$resultats=$dbHandle->query($sqlRequest);
			$dbHandle->query($sqlRequest);
			$action="acceuil";
			if($_SESSION['session']=='superadmin')
			{
				$action='modifyDepot';
				$_GET['depotname']=isset($_SESSION['depotname'])?$_SESSION['depotname']:'';
			}
			$showForm=false;
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	}
	else
	{
		$showForm=false;
		$action="accueil";
		if($_SESSION['session']=='superadmin')
		{
			$action='modifyDepot';
			$_GET['depotname']=isset($_SESSION['depotname'])?$_SESSION['depotname']:'';
		}
		$error="Vous n'&ecirc;tes pas identifi&eacute;";
	}
	if($showForm)
	{
	echo entete("Droits de l'utilisateur ".$_GET['login']);
	echo $error;
	$form ='<form action="'.PAGE.'?action=modifyMembre" method="POST"><table>';
	$form.='<tr><td><label for="login">Login</label></td><td><input type="text" name="login" id="login" value="'.$_GET['login'].'"/></td></tr>';
	$form.='<tr><td><label for="readonly">Lecture seule</label></td><td><input type="checkbox" name="readonly" id="readonly"/></td></tr>';
	$form.='<tr><td align="right" colspan="2"><input style="width:100px;" type="submit" value="Valider"/></td></tr>';
	$form.='</table></form>';
	echo '<div style="margin-top:50px;"><table align="center"><tr><td>'.$form.'</td></tr></table></div>';
	}
}





///////////////////////////////////////////////////
/////      SUPRIMER MEMBRE
//////////////////////////////////////////////////

if($action=="deleteMembre")
{
	if($_SESSION['login']!='' and ($_SESSION['session']=='admin' or $_SESSION['session']=='superadmin'))
	{
	if(isset($_GET['login']))
	{
		$login=$_GET['login'];
		$depotname=$_SESSION['depotname'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM utilisation WHERE depotname=".$dbHandle->quote($depotname)." and login=".$dbHandle->quote($login);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
				$sqlRequest = "DELETE FROM utilisation WHERE login=".$dbHandle->quote($login)." AND depotname=".$dbHandle->quote($depotname);
				$dbHandle->query($sqlRequest);
				$error="";
				$ret=0;
				$resp=array();
				exec("$ScriptPath --request deleteUser --depotName $depotname --userLogin $login",$resp,$ret);
				if($ret!=0) foreach($resp as $r) $error=$error."\n".$r;
			}
			else $error='Cet utilisateur n\'est pas ajout&eacute; &agrave; ce projet.';
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	else $error="Formulaire mal rempli";
	}
	else $error="Vous n'&ecirc;tes pas identifi&eacute;";
	$action="acceuil";
	if($_SESSION['session']=='superadmin')
	{
		$action='modifyDepot';
		$_GET['depotname']=isset($_SESSION['depotname'])?$_SESSION['depotname']:'';
	}
}





///////////////////////////////////////////////////
/////      AJOUTER ADMIN
//////////////////////////////////////////////////

if($action=="ajouterAdmin")
{
	if($_SESSION['login']!='' and ($_SESSION['session']=='admin' or $_SESSION['session']=='superadmin'))
	{
	if(isset($_POST['login']))
	{
		$login=$_POST['login'];
		$depotname=$_SESSION['depotname'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($login);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
				$sqlRequest = "SELECT * FROM admin WHERE depotname=".$dbHandle->quote($depotname)." and login=".$dbHandle->quote($login);
				$resultats=$dbHandle->query($sqlRequest);
				if(!($result=$resultats->fetch(PDO::FETCH_BOTH)))
				{
					$sqlRequest = "INSERT INTO admin(login,depotname,inscriptiondate) VALUES(".$dbHandle->quote($login).",".$dbHandle->quote($depotname).",datetime())";
					$dbHandle->query($sqlRequest);
				}
				else $error='Cet utilisateur est d&eacute;j&agrave; administrateur..';
			}
			else $error='Ce compte n\'existe pas.';
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	else $error="Formulaire mal rempli";
	}
	else $error="Vous n'&ecirc;tes pas identifi&eacute;";
	$action="acceuil";
	if($_SESSION['session']=='superadmin')
	{
		$action='modifyDepot';
		$_GET['depotname']=isset($_SESSION['depotname'])?$_SESSION['depotname']:'';
	}
}






///////////////////////////////////////////////////
/////      SUPRIMER ADMIN
//////////////////////////////////////////////////

if($action=="deleteAdmin")
{
	if($_SESSION['login']!='' and ($_SESSION['session']=='admin' or $_SESSION['session']=='superadmin'))
	{
	if(isset($_GET['login']))
	{
		$login=$_GET['login'];
		$depotname=$_SESSION['depotname'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM admin WHERE depotname=".$dbHandle->quote($depotname)." and login=".$dbHandle->quote($login);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
				$sqlRequest = "DELETE FROM admin WHERE login=".$dbHandle->quote($login)." AND depotname=".$dbHandle->quote($depotname);
				$dbHandle->query($sqlRequest);
			}
			else $error='Cet utilisateur n\'est pas administrateur de ce projet.';
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	else $error="Formulaire mal rempli";
	}
	else $error="Vous n'&ecirc;tes pas identifi&eacute;";
	$action="acceuil";
	if($_SESSION['session']=='superadmin')
	{
		$action='modifyDepot';
		$_GET['depotname']=isset($_SESSION['depotname'])?$_SESSION['depotname']:'';
	}
}





///////////////////////////////////////////////////
/////      AJOUTER SUPER ADMIN
//////////////////////////////////////////////////

if($action=="ajouterSuperAdmin")
{
	if($_SESSION['login']!='' and $_SESSION['session']=='superadmin')
	{
	if(isset($_POST['login']))
	{
		$login=$_POST['login'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($login);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
				$sqlRequest = "SELECT * FROM superadmin WHERE login=".$dbHandle->quote($login);
				$resultats=$dbHandle->query($sqlRequest);
				if(!($result=$resultats->fetch(PDO::FETCH_BOTH)))
				{
					$sqlRequest = "INSERT INTO superadmin(login,inscriptiondate) VALUES(".$dbHandle->quote($login).",datetime())";
					$dbHandle->query($sqlRequest);
				}
				else $error='Cet utilisateur est d&eacute;j&agrave; super-administrateur.';
			}
			else $error='Ce compte n\'existe pas.';
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	else $error="Formulaire mal rempli";
	}
	else $error="Vous n'&ecirc;tes pas identifi&eacute;";
	$action="acceuil";
}





///////////////////////////////////////////////////
/////      SUPRIMER SUPER ADMIN
//////////////////////////////////////////////////

if($action=="deleteSuperAdmin")
{
	if($_SESSION['login']!='' and $_SESSION['session']=='superadmin')
	{
	if(isset($_GET['login']))
	{
		$login=$_GET['login'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM superadmin WHERE login=".$dbHandle->quote($login);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
				$sqlRequest = "DELETE FROM superadmin WHERE login=".$dbHandle->quote($login);
				$dbHandle->query($sqlRequest);
			}
			else $error='Cet utilisateur n\'est pas super-administrateur.';
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	else $error="Formulaire mal rempli";
	}
	else $error="Vous n'&ecirc;tes pas identifi&eacute;";
	$action="acceuil";
}








///////////////////////////////////////////////////
/////     HISTORIQUE DEPOT 
//////////////////////////////////////////////////

if($action=="historique")
{
	$depotname=$_GET['depotname'];
	$startHistory=isset($_GET['startHistory'])?$_GET['startHistory']:0;
	$numberHistory=30;
	$svnServer="NotFoundSvnServer";
	$error="";
	echo entete("Historique du d&eacute;pot ".$depotname);
	try{
		$dbHandle = new PDO("sqlite:$DataBasePath");
		$sqlRequest = "SELECT * FROM infos";
		$resultats=$dbHandle->query($sqlRequest);
		if($result=$resultats->fetch(PDO::FETCH_BOTH))
		{
			$svnServer=$result['svnAddress'];
		}
	}
	catch( PDOException $exception ){
		$error='Error connecting to database'.$exception->getMessage();
	}
	$ret=0;
	$resp=array();
	exec("$ScriptPath --request history --depotName $depotname --svnServer $svnServer",$resp,$ret);
	echo '<div style="margin-top:50px"><table><tr><td><table width="100%" border="1" align="center"><tr><th>Type</th><th>Media</th><th>Action</th></tr>';
	if($ret!=0)
	{
		echo "<tr><td colspan='5'>$error";
		foreach($resp as $r) echo "$r<br/>";
		echo "</td></tr>";
	}
	else
	{
		$types=array('file'=>'<img src="'.$ImgPath.'file.png"/>','dir'=>'<img src="'.$ImgPath.'dir.png"/>');
		$actions=array('added'=>'Ajout&eacute;','modified'=>'Modifi&eacute;','deleted'=>'Supprim&eacute;');
		for($i=sizeof($resp)-1;$i>=0;$i--)
		{
			echo "<tr>";
			echo "<td>".$types[$resp[$i]]."</td>";$i--;
			echo "<td>".mb_convert_encoding($resp[$i], 'UTF-8', 'HTML-ENTITIES')."</td>";$i--;
			echo "<td>".$actions[$resp[$i]]."</td>";
			echo "</tr>";
		}
		if($i==0) echo "<tr><td colspan='5' align='center'>Aucun historique trouv&eacute; pour ce d&eacute;pot</td></tr>";
	}
	echo '</table></td><td></td></tr></table></div>';
}






///////////////////////////////////////////////////
/////      MODIFIER DEPOT
//////////////////////////////////////////////////

if($action=="modifyDepot")
{
	if($_SESSION['login']!='' and ($_SESSION['session']=='superadmin'))
	{
	if(isset($_GET['depotname']) and $_GET['depotname']!='')
	{
		$depotname=$_GET['depotname'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM depot WHERE depotname=".$dbHandle->quote($depotname);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
			echo entete("Super-Administrateur");
			echo output_h2("Administration du d&eacute;pot ".$depotname);
			echo '<table width="100%"><tr><th>Membres</th><th> </th></tr>';
			echo '<tr><td valign="top" align="center">';
			modify_delete_user_admin_depot($depotname);
			echo '</td><td align="center" valign="top">';
			add_user_admin_depot($depotname);
			echo '</td></tr><tr><td colspan="2" align="center"><a href="'.PAGE.'?action=historique&depotname='.$depotname.'"><h3>Voir l\'historique de '.$depotname.'</h3></a></td></tr></table>';
			$_SESSION['depotname']=$depotname;
			}
			else
			{
				$error='D&eacute;pot introuvable.';
				$action='acceuil';
			}
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
			$action="acceuil";
		}
	}
	else
	{
		$action="acceuil";
		$error="D&eacute;pot introuvable.";
	}
	}
	else
	{
		$action="acceuil";
		$error="Formulaire mal rempli";
	}
}







///////////////////////////////////////////////////
/////      AFFICHER DEPOT
//////////////////////////////////////////////////

if($action=="showDepot")
{
	if($_SESSION['login']!='')
	{
	if(isset($_GET['depotname']) and $_GET['depotname']!='')
	{
		$depotname=$_GET['depotname'];
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM depot WHERE depotname=".$dbHandle->quote($depotname);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{
			echo entete("Infos d&eacute;pot ".$depotname);
			echo '<table width="100%"><tr><th>Membres</th><th> </th></tr>';
			echo '<tr><td valign="top" align="center">';
			list_user_admin_depot($depotname);
			echo '</td></tr><tr><td colspan="2" align="center"><a href="'.PAGE.'?action=historique&depotname='.$depotname.'"><h3>Voir l\'historique de '.$depotname.'</h3></a></td></tr></table>';
			$_SESSION['depotname']=$depotname;
			}
			else
			{
				$error='D&eacute;pot introuvable.';
				$action='acceuil';
			}
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
			$action="acceuil";
		}
	}
	else
	{
		$action="acceuil";
		$error="Depot introuvable";
	}
	}
	else
	{
		$action="acceuil";
		$error="Vous n'&ecirc;tes pas identifi&eacute;";
	}
}







///////////////////////////////////////////////////
/////      SUPPRIMER DEPOT
//////////////////////////////////////////////////

if($action=="deleteDepot")
{
	$showForm=true;
	$error="";
	$depotname=isset($_POST['depotname'])?$_POST['depotname']:(isset($_GET['depotname'])?$_GET['depotname']:'');
	if($_SESSION['login']!='' and ($_SESSION['session']=='superadmin'))
	{
	if(isset($_POST['depotname']) and $_POST['depotname']!='')
	{
		try{
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM depot WHERE depotname=".$dbHandle->quote($depotname);
			$resultats=$dbHandle->query($sqlRequest);
			if($result=$resultats->fetch(PDO::FETCH_BOTH))
			{	
				$sqlRequest = "DELETE from utilisation where depotname=".$dbHandle->quote($depotname);
				$resultats=$dbHandle->query($sqlRequest);
				$sqlRequest = "DELETE from admin where depotname=".$dbHandle->quote($depotname);
				$resultats=$dbHandle->query($sqlRequest);
				$sqlRequest = "DELETE from depot where depotname=".$dbHandle->quote($depotname);
				$resultats=$dbHandle->query($sqlRequest);
				$error="";
				$ret=0;
				$resp=array();
				exec("$ScriptPath --request removeDepot --depotName $depotname",$resp,$ret);
				if($ret!=0) foreach($resp as $r) $error=$error."\n".$r;
				$showForm=false;
				$action="acceuil";
			}
			else
			{
				$error="D&eacute;pot introuvable.";
			}
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	}
	else
	{
		$action="acceuil";
		$error="Vous n'&ecirc;tes pas identifi&eacute;";
	}
	if($showForm)
	{
		echo entete("Super-Administrateur");
		echo $error;
		echo output_h2("Suppression du d&eacute;pot ".$depotname);
		$form ='<form action="'.PAGE.'?action=deleteDepot" method="POST"><table>';
		$form.='<tr><td><label for="depotname">D&eacute;pot</label></td><td><input type="text" name="depotname" id="depotname" value="'.$depotname.'"/></td></tr>';
		$form.='<tr><td align="right" colspan="2"><input type="submit" value="Supprimer d&eacute;finitivement"/></td></tr>';
		$form.='</table></form>';
	echo '<div style="margin-top:50px;"><table align="center"><tr><td>'.$form.'</td></tr></table></div>';
	}	
}








///////////////////////////////////////////////////
/////      CREER DEPOT
//////////////////////////////////////////////////

if($action=="creerDepot")
{
	$action="acceuil";
	$error="";
	if($_SESSION['login']!='' and ($_SESSION['session']=='superadmin'))
	{
	if(isset($_POST['depotname']) and $_POST['depotname']!='')
	{
		try{
			$depotname=$_POST['depotname'];
			$login=isset($_POST['login'])?$_POST['login']:'';
			$dbHandle = new PDO("sqlite:$DataBasePath");
			$sqlRequest = "SELECT * FROM depot WHERE depotname=".$dbHandle->quote($depotname);
			$resultats=$dbHandle->query($sqlRequest);
			if(!($result=$resultats->fetch(PDO::FETCH_BOTH)))
			{
				$sqlRequest = "Insert into depot(depotname, inscriptiondate) VALUES(".$dbHandle->quote($depotname).",datetime())";
				$resultats=$dbHandle->query($sqlRequest);
				$error="";
				$ret=0;
				$resp=array();
				exec("$ScriptPath --request createDepot --depotName $depotname",$resp,$ret);
				if($ret!=0) foreach($resp as $r) $error=$error."\n".$r;
				if($login!='')
				{
					$sqlRequest = "Select * from user where login=".$dbHandle->quote($login);
					$resultats=$dbHandle->query($sqlRequest);
					if($result=$resultats->fetch(PDO::FETCH_BOTH))
					{
						$sqlRequest = "Insert into admin(login,depotname,inscriptiondate) VALUES(".$dbHandle->quote($login).",".$dbHandle->quote($depotname).",datetime())";
						$resultats=$dbHandle->query($sqlRequest);
					}
					else $error='L\'administrateur est introuvable.';
				}
			}
			else
			{
				$error="Le d&eacute;pot existe d&eacute;j&agrave;.";
			}
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	else $error="Formulaire mal rempli";
	}
	else $error="Vous n'&ecirc;tes pas identifi&eacute;";
}




///////////////////////////////////////////////////
/////    GERER LES COMPTES USERTMP
//////////////////////////////////////////////////

if($action=="gestionUserTmp")
{
	if($_SESSION['login']!='' and ($_SESSION['session']=='admin' or $_SESSION['session']=='superadmin'))
	{
	$error="Cliquez sur accepter ou rejeter la demande de l'utilisateur.";
	if(isset($_GET['login']) and isset($_GET['decision']))
	{
		try{
			$accept=($_GET['decision']=='accept');
			$login=$_GET['login'];
			$dbHandle = new PDO("sqlite:$DataBasePath");
			if($accept)
			{
				$sqlRequest = "SELECT * FROM usertmp WHERE login=".$dbHandle->quote($login);
				$resultats=$dbHandle->query($sqlRequest);
				$sqlRequest0 = "SELECT * FROM user WHERE login=".$dbHandle->quote($login);
				$resultats0=$dbHandle->query($sqlRequest0);
				if(($result=$resultats->fetch(PDO::FETCH_BOTH)) and (!($result0=$resultats0->fetch(PDO::FETCH_BOTH))))
				{
					$sqlRequest = "Insert into user(login,password,firstname,lastname,inscriptiondate) VALUES(".$dbHandle->quote($result['login']).",".$dbHandle->quote($result['password']).",".$dbHandle->quote($result['firstname']).",".$dbHandle->quote($result['lastname']).",datetime())";
					$resultats=$dbHandle->query($sqlRequest);
					$sqlRequest = "DELETE FROM usertmp WHERE login=".$dbHandle->quote($login);
					$resultats=$dbHandle->query($sqlRequest);
				}
				else $error="Un conflit a &eacute;t&eacute; d&eacute;tect&eacute; lors de l'ajout.";
			}
			else
			{
				$sqlRequest = "DELETE FROM usertmp WHERE login=".$dbHandle->quote($login);
				$resultats=$dbHandle->query($sqlRequest);
			}
		}
		catch( PDOException $exception ){
			$error='Error connecting to database'.$exception->getMessage();
		}
	}
	echo entete("Gerer les demandes de comptes");
	echo '<p>'.$error.'</p>';
	echo '<table style="margin-top:30px" width="100%" border="2" align="center">';
	echo '<tr><th> </th><th> </th><th>Login</th><th>Nom</th><th>Pr&eacute;nom</th><th>Demande d\'inscription le</th></tr>';
	try{
		$dbHandle = new PDO("sqlite:$DataBasePath");
		$sqlRequest = "SELECT * FROM usertmp";
		$resultats=$dbHandle->query($sqlRequest);
		$n=0;
		while($result=$resultats->fetch(PDO::FETCH_BOTH))
		{
			echo '<tr><th><a href="'.PAGE.'?action=gestionUserTmp&amp;login='.$result['login'].'&amp;decision=accept"><img src="'.$ImgPath.'added.png"/></a></th><th><a href="'.PAGE.'?action=gestionUserTmp&amp;login='.$result['login'].'&amp;decision=reject"><img src="'.$ImgPath.'removed.png"/></a></th><th>'.$result['login'].'</th><th>'.$result['lastname'].'</th><th>'.$result['firstname'].'</th><th>'.$result['inscriptiondate'].'</th></tr>';
			$n++;
			
		}
		if($n==0) echo '<tr><td> </td><td> </td><td colspan="4">Aucune demande de compte &agrave; traiter</td></tr>';
	}
	catch( PDOException $exception ){
		$error='Error connecting to database'.$exception->getMessage();
	}
	echo '</table>';
	}
	else 
	{
		$error="Vous n'&ecirc;tes pas identifi&eacute;";
		$action="acceuil";
	}
}







///////////////////////////////////////////////////
/////       ACCEUIL
//////////////////////////////////////////////////

if($action=="acceuil")
{
if($_SESSION['login']=='')
{
	echo entete("Acceuil");
	echo '<table style="margin-top:30px" width="100%">';
	echo '<tr>';
	echo '<td align="center"><a href="'.PAGE.'?action=authentificate&session=user"><img src="'.$ImgPath.'user.png"/></a></td>';
	echo '<td align="center"><a href="'.PAGE.'?action=authentificate&session=admin"><img src="'.$ImgPath.'admin.png"/></a></td>';
	echo '<td align="center"><a href="'.PAGE.'?action=authentificate&session=superadmin"><img src="'.$ImgPath.'superadmin.png"/></a></td>';
	echo '</tr>';
	echo '<tr>';
	echo '<td align="center"><h3><a href="'.PAGE.'?action=authentificate&session=user">Authentification utilisateur</a></h3></td>';
	echo '<td align="center"><h3><a href="'.PAGE.'?action=authentificate&session=admin">Authentification administrateur</a></h3></td>';
	echo '<td align="center"><h3><a href="'.PAGE.'?action=authentificate&session=superadmin">Authentification super-administrateur</a></h3></td>';
	echo '<tr/>';
	echo '<tr><td colspan="3" align="center"><div style="width:500px;border:solid">';
	if(isset($_POST['login']) and isset($_POST['password']))
	{
		$error="";
		$login=$_POST['login'];
		$password=$_POST['password'];
		$firstname=isset($_POST['firstname'])?$_POST['firstname']:'';
		$lastname=isset($_POST['lastname'])?$_POST['lastname']:'';
		if($login!="" and $password!="")
		{
			try {
				$dbHandle = new PDO("sqlite:$DataBasePath");
				$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($login);
				$resultats=$dbHandle->query($sqlRequest);
				if(!($result=$resultats->fetch(PDO::FETCH_BOTH)))
				{
					$sqlRequest = "SELECT * FROM usertmp WHERE login=".$dbHandle->quote($login);
					$resultats=$dbHandle->query($sqlRequest);
					if(!($result=$resultats->fetch(PDO::FETCH_BOTH)))
					{
						$sqlRequest = "INSERT INTO usertmp(login,password,firstname,lastname,inscriptiondate) VALUES(".$dbHandle->quote($login).",".$dbHandle->quote($password).",".$dbHandle->quote($firstname).",".$dbHandle->quote($lastname).",datetime())";
						$resultats=$dbHandle->query($sqlRequest);
						$error="Faites valider votre compte par un administrateur";
					}
					else $error="Login d&eacute;j&agrave; utilis&eacute;";
				}
				else $error="Login d&eacute;j&agrave; utilis&eacute;";
			}
			catch( PDOException $exception ){
				echo 'Error connecting to database'.$exception->getMessage();
				exit();
			}
		}
		else $error="Formulaire mal rempli";
		echo "<p>$error</p>";
	}
	else
	{
	echo '<p>Cr&eacute;ez votre compte ici et faites le valider par un administrateur.</p>';
	}
	echo '<form action="'.PAGE.'?action=acceuil" method="POST"><table><tr><td>';
	echo '<img src="'.$ImgPath.'adduser.png"/></td><td><table width="90%">';
	echo '<tr><td><label for="login">Login</label></td><td><input name="login" id="login"/></td></tr>';
	echo '<tr><td><label for="password">Password</label></td><td><input type="password" name="password" id="password"/></td></tr>';
	echo '<tr><td><label for="lastname">Nom</label></td><td><input name="lastname" id="lastname"/></td></tr>';
	echo '<tr><td><label for="firstname">Prenom</label></td><td><input name="firstname" id="firstname"/></td></tr>';
	echo '<tr><td colspan="2" align="right"><input type="submit" value="Valider"/></td></tr>';
	echo '</table></td></tr></table></form></div></td></tr>';
	echo '</table>';
}
else
{




if($_SESSION['session']=='user')
{
	echo entete("Utilisateur");
	try {
	$dbHandle = new PDO("sqlite:$DataBasePath");
	$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($_SESSION['login']);
	$resultats=$dbHandle->query($sqlRequest);
	$result=$resultats->fetch(PDO::FETCH_BOTH);
	$name=$_SESSION['login'];
	if($result['firstname']!='' or $result['lastname']!='') $name=$result['firstname']." ".$result['lastname'];
	echo output_h2("Bienvenue $name");
	}
	catch( PDOException $exception ){
		echo 'Error connecting to database'.$exception->getMessage();
		exit();
	}
	$login=($result['login']!='')?$result['login']:'Unknown';
	$firstname=($result['firstname']!='')?$result['firstname']:'Unknown';
	$lastname=($result['lastname']!='')?$result['lastname']:'Unknown';
	$inscriptiondate=($result['inscriptiondate']!='')?$result['inscriptiondate']:'Unknown';
	echo '<table align="center" cellspacing="10"><tr><th></th><th>Infos perso</div></th><th width="60%">Liste de mes projets</th></tr><tr>';
	echo '<td><img src="'.$ImgPath.'user.png"/></td><td width="250px">';
	show_user_info($login,$firstname,$lastname,$inscriptiondate);
	echo '</td>';
	$sqlRequest = "SELECT * FROM utilisation WHERE login=".$dbHandle->quote($_SESSION['login']);
	$resultats=$dbHandle->query($sqlRequest);
	echo '<td valign="top"><table width="100%" border="1">';
	$n=0;
	while($result=$resultats->fetch(PDO::FETCH_BOTH)) {
		echo '<tr><td><a href="'.PAGE.'?action=showDepot&depotname='.$result['depotname'].'"><b>'.$result['depotname'].'</b>: Membre depuis le '.$result['inscriptiondate'].'</a></td></tr>';
		$n=$n+1;
	}
	if($n==0) echo '<tr><td>Vous n\'&ecirc;tes inscrit &agrave; aucun projet.</td></tr>';
	echo '</table></td>';
	
echo '</tr></table>';
}





else if($_SESSION['session']=='admin')
{
	echo entete("Administration du dépot ".$_SESSION['depotname']);
	if(isset($error)) echo $error;
	try {
	$dbHandle = new PDO("sqlite:$DataBasePath");
	$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($_SESSION['login']);
	$resultats=$dbHandle->query($sqlRequest);
	$result=$resultats->fetch(PDO::FETCH_BOTH);
	$name=$_SESSION['login'];
	if($result['firstname']!='' or $result['lastname']!='') $name=$result['firstname']." ".$result['lastname'];
	echo output_h2("Bienvenue $name");
	}
	catch( PDOException $exception ){
		echo 'Error connecting to database'.$exception->getMessage();
		exit();
	}
	$login=($result['login']!='')?$result['login']:'Unknown';
	$firstname=($result['firstname']!='')?$result['firstname']:'Unknown';
	$lastname=($result['lastname']!='')?$result['lastname']:'Unknown';
	$inscriptiondate=($result['inscriptiondate']!='')?$result['inscriptiondate']:'Unknown';
	echo '<table align="center" width="100%" cellspacing="1"><tr><th>Administrateur</th><th>Liste des membres</th><th> </th></tr><tr><td valign="top">';
	show_user_info($login,$firstname,$lastname,$inscriptiondate);	
	echo '</td><td align="center" valign="top">';
	modify_delete_user_admin_depot($_SESSION['depotname']);
	echo '</td><td align="center" valign="top">';
	add_user_admin_depot();
	echo '<p><a href="'.PAGE.'?action=gestionUserTmp">Gestion des demandes de comptes</a></p>';
	$depotname=$_SESSION['depotname'];
	echo '</td></tr><tr><td colspan="3" align="center"><a href="'.PAGE.'?action=historique&depotname='.$depotname.'"><h3>Voir l\'historique de '.$depotname.'</h3></a></td></tr></table>';
}




else if($_SESSION['session']=='superadmin')
{
	echo entete("Super-Administrateur");
	if(isset($error)) echo $error;
	$_SESSION['depotname']='';
	try {
	$dbHandle = new PDO("sqlite:$DataBasePath");
	$sqlRequest = "SELECT * FROM user WHERE login=".$dbHandle->quote($_SESSION['login']);
	$resultats=$dbHandle->query($sqlRequest);
	$result=$resultats->fetch(PDO::FETCH_BOTH);
	$name=$_SESSION['login'];
	if($result['firstname']!='' or $result['lastname']!='') $name=$result['firstname']." ".$result['lastname'];
	echo output_h2("Bienvenue $name");
	}
	catch( PDOException $exception ){
		echo 'Error connecting to database'.$exception->getMessage();
		exit();
	}
	$login=($result['login']!='')?$result['login']:'Unknown';
	$firstname=($result['firstname']!='')?$result['firstname']:'Unknown';
	$lastname=($result['lastname']!='')?$result['lastname']:'Unknown';
	$inscriptiondate=($result['inscriptiondate']!='')?$result['inscriptiondate']:'Unknown';
	echo '<table align="center" width="100%" cellspacing="1"><tr><th>Super-Administrateur</th><th>Liste des projets</th><th> </th></tr><tr><td valign="top">';
	show_user_info($login,$firstname,$lastname,$inscriptiondate);
	echo '</td>';
	$sqlRequest = "SELECT depotname, inscriptiondate FROM depot";
	$resultats=$dbHandle->query($sqlRequest);
	echo '<td valign="top"><table width="100%" border="1"><tr><th> </th><th> </th><th>Nom du d&eacute;pot</th><th>Inscrit le</th><th>Nombre users</th></tr>';
	$n=0;
	while($result=$resultats->fetch(PDO::FETCH_BOTH)) {
		$sqlRequest = "SELECT count(login) as nbadmins FROM admin where depotname=".$dbHandle->quote($result['depotname']);
		$resultats2=$dbHandle->query($sqlRequest);
		$result2=$resultats2->fetch(PDO::FETCH_BOTH);
		$sqlRequest = "SELECT count(login) as nblogins FROM utilisation where depotname=".$dbHandle->quote($result['depotname']);
		$resultats2=$dbHandle->query($sqlRequest);
		$result3=$resultats2->fetch(PDO::FETCH_BOTH);
		echo '<tr><td><a href="'.PAGE.'?action=modifyDepot&depotname='.$result['depotname'].'"><img src="'.$ImgPath.'modify.png"/></a></td><td><a href="'.PAGE.'?action=deleteDepot&depotname='.$result['depotname'].'"><img src="'.$ImgPath.'delete.png"/></a></td><td>'.$result['depotname'].'</td><td>'.$result['inscriptiondate'].'</td><td align="center">'.$result3['nblogins'].' users<br/>'.$result2['nbadmins'].' admins</td></tr>';
		$n=$n+1;
	}
	if($n==0) echo '<tr><td> </td><td> </td><td colspan="3">Aucun projet n\'est cr&eacute;&eacute;.</td></tr>';
	echo '</table>';
	echo '<table width="100%" border="1" style="margin-top:20px"><caption><b>Liste des super-admins</b></caption><tr><th> </th><th>Login</th><th>Super-admin depuis le</th></tr>';

	$sqlRequest = "SELECT login, inscriptionDate FROM superadmin";
	$resultats=$dbHandle->query($sqlRequest);
	$n=0;
	while($result=$resultats->fetch(PDO::FETCH_BOTH)) {
		echo '<tr><td><a href="'.PAGE.'?action=deleteSuperAdmin&login='.$result['login'].'"><img src="'.$ImgPath.'delete.png"/></a></td><td>'.$result['login'].'</td><td>'.$result['inscriptiondate'].'</td></tr>';
		$n=$n+1;
	}
	if($n==0) echo '<tr><td> </td><td colspan="2">Aucun super-administrateur.</td></tr>';

	echo '</table>';
	echo '</td><td valign="top" align="right"><table><form action="'.PAGE.'?action=creerDepot" method="POST"><tr><td colspan="2" align="right">Cr&eacute;er un projet<hr/></td></tr>';
	echo '<tr><td><label for="depotname">Nom</label></td><td align="right"><input type="text" name="depotname" id="depotname"/></td></tr>';
	echo '<tr><td><label for="login">Login admin</label></td><td align="right"><input type="text" name="login" id="login"/></td></tr>';
	echo '<tr><td colspan="2" align="right"><input type="submit" value="Valider"/></td></tr></form>';


	echo '</table><table style="margin-top:20px"><form action="'.PAGE.'?action=ajouterSuperAdmin" method="POST"><tr><td colspan="2" align="right">Ajouter un super-admin<hr/></td></tr>';
	echo '<tr><td><label for="login">Login</label></td><td align="right"><input type="text" name="login" id="login"/></td></tr>';
	echo '<tr><td colspan="2" align="right"><input type="submit" value="Valider"/></td></tr></form>';

	echo '</table>';
	echo '<p><a href="'.PAGE.'?action=gestionUserTmp">Gestion des demandes de comptes</a></p>';
	echo '</td></tr></table>';
}



}
}

echo pieddepage();


/*************************************************************/
/*                  FONCTIONS                                */
/*************************************************************/
function entete($text)
{
	global $ApplicationName;
	global $ImgPath;
	global $_SESSION;
	$deconnexion="";
	if($_SESSION['login']!="") $deconnexion='<a href='.PAGE.'?action=deconnexion><img style="margin-right:30px" src="'.$ImgPath.'logout.png"/></a>'; 
	$icones='<td align="right">'.$deconnexion.'<a href='.PAGE.'><img src="'.$ImgPath.'home.png"/></a></td>';
	$str='<div style="margin-top:40px;"><table width="100%" style="margin:0px;"><tr><td align="left"><h1>'.$ApplicationName.' - '.$text.'</h1></td>'.$icones.'</tr></table></div><hr/>';
	return $str;
}

function output_h1($text)
{
	return '<h1 style="margin-top:40px;">'.$text.'</h1><hr/>';
}

function pieddepage()
{
	global $ApplicationName;
//	return '<div align="center" style="width:90%;font-size:15px"><a>Powered by '.$ApplicationName.'</a></p>';
}



function show_user_info($login,$lastname,$firstname,$inscriptiondate)
{
	echo '<table border="1">';
	echo '<tr><td><b>Pseudo</b></td><td>'.$login.'</td></tr>';
	echo '<tr><td><b>Nom</b></td><td>'.$lastname.'</td></tr>';
	echo '<tr><td><b>Pr&eacute;nom</b></td><td>'.$firstname.'</td></tr>';
	echo '<tr><td><b>Inscrit le</b></td><td>'.$inscriptiondate.'</td></tr>';
	echo '<tr><td align="center" colspan="2"><a href="'.PAGE.'?action=modifyInfos">Modifier mes coordonn&eacute;es.</td></tr>';
	echo '<tr><td align="center" colspan="2"><a href="'.PAGE.'?action=modifyPassword">Modifier mon mot de passe.</td></tr>';
	echo '</table>';
}

function modify_delete_user_admin_depot($depot)
{
	global $dbHandle;
	global $ImgPath;
	
	$sqlRequest = "SELECT * FROM utilisation WHERE depotname=".$dbHandle->quote($depot);
	$resultats=$dbHandle->query($sqlRequest);
	echo '<table border="1" width="100%"><tr><th> </th><th> </th><th>login</th><th>Inscrit au projet le</th><th>Droits</th></tr>';
	$n=0;
	while($result=$resultats->fetch(PDO::FETCH_BOTH)) {
		echo '<tr><td><a href="'.PAGE.'?action=modifyMembre&login='.$result['login'].'"><img src="'.$ImgPath.'modify.png"/></a></td><td><a href="'.PAGE.'?action=deleteMembre&login='.$result['login'].'"><img src="'.$ImgPath.'delete.png"/></a></td><td>'.$result['login'].'</td><td>'.$result['inscriptiondate'].'</td><td>'.(($result['readonly']=='true')?'lecture seule':'lecture &eacute;criture').'</td></tr>';
		$n=$n+1;
	}
	if($n==0) echo '<tr><td> </td><td> </td><td colspan="3">Aucun utilisateur dans ce projet.</td></tr>';
	echo '</table><table width="100%" border="1" style="margin-top:20px"><caption><b>Liste des admins</b></caption><tr><th> </th><th>Login</th><th>Admin depuis le</th></tr>';
	$sqlRequest = "SELECT * FROM admin WHERE depotname=".$dbHandle->quote($depot);
	$resultats=$dbHandle->query($sqlRequest);
	$n=0;
	while($result=$resultats->fetch(PDO::FETCH_BOTH)) {
		echo '<tr><td><a href="'.PAGE.'?action=deleteAdmin&login='.$result['login'].'"><img src="'.$ImgPath.'delete.png"/></a></td><td>'.$result['login'].'</td><td>'.$result['inscriptiondate'].'</td></tr>';
		$n=$n+1;
	}
	if($n==0) echo '<tr><td> </td><td colspan="2">Aucun administrateur dans ce projet.</td></tr>';
	echo '</table>';
}


function list_user_admin_depot($depot)
{
	global $dbHandle;
	global $ImgPath;
	
	$sqlRequest = "SELECT * FROM utilisation WHERE depotname=".$dbHandle->quote($depot);
	$resultats=$dbHandle->query($sqlRequest);
	echo '<table border="1" width="100%"><tr><th>login</th><th>Inscrit au projet le</th><th>Droits</th></tr>';
	$n=0;
	while($result=$resultats->fetch(PDO::FETCH_BOTH)) {
		echo '<tr><td>'.$result['login'].'</td><td>'.$result['inscriptiondate'].'</td><td>'.(($result['readonly']=='true')?'lecture seule':'lecture &eacute;criture').'</td></tr>';
		$n=$n+1;
	}
	if($n==0) echo '<tr><td> </td><td> </td><td colspan="3">Aucun utilisateur dans ce projet.</td></tr>';
	echo '</table><table width="100%" border="1" style="margin-top:20px"><caption><b>Liste des admins</b></caption><tr><th>Login</th><th>Admin depuis le</th></tr>';
	$sqlRequest = "SELECT * FROM admin WHERE depotname=".$dbHandle->quote($depot);
	$resultats=$dbHandle->query($sqlRequest);
	$n=0;
	while($result=$resultats->fetch(PDO::FETCH_BOTH)) {
		echo '<tr><td>'.$result['login'].'</td><td>'.$result['inscriptiondate'].'</td></tr>';
		$n=$n+1;
	}
	if($n==0) echo '<tr><td> </td><td colspan="2">Aucun administrateur dans ce projet.</td></tr>';
	echo '</table>';
}


function add_user_admin_depot()
{
	echo '<table><form action="'.PAGE.'?action=ajouterMembre" method="POST"><tr><td colspan="2" align="right"><b>Ajouter un membre existant</b><hr/></td></tr>';
	echo '<tr><td><label for="login">login</label></td><td align="right"><input type="text" name="login" id="login"/></td></tr>';
	echo '<tr><td align="right" colspan="2"><label for="readonly">Lecture seule</label><input type="checkbox" name="readonly" id="readonly"/></td></tr>';
	echo '<tr><td colspan="2" align="right"><input type="submit" value="Valider"/></td></tr></form>';
	echo '<form action="'.PAGE.'?action=ajouterAdmin" method="POST"><tr><td colspan="2" align="right"><b>Ajouter un administrateur</b><hr/></td></tr>';
	echo '<tr><td><label for="login">login</label></td><td align="right"><input type="text" name="login" id="login"/></td></tr>';
	echo '<tr><td colspan="2" align="right"><input type="submit" value="Valider"/></td></tr></form></table>';

}

function output_h2($text)
{
	return '<h3 align="right" style="margin-top:20px;">'.$text.'</h3>';
}

function output_box($content)
{
	return $content;
}



?>
