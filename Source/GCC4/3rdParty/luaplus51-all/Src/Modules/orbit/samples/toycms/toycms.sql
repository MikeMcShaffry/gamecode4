# Sequel Pro dump
# Version 1630
# http://code.google.com/p/sequel-pro
#
# Host: 127.0.0.1 (MySQL 5.1.41)
# Database: toycms-devel
# Generation Time: 2010-02-24 10:52:59 -0300
# ************************************************************

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


# Dump of table node
# ------------------------------------------------------------

DROP TABLE IF EXISTS `node`;

CREATE TABLE `node` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` varchar(50) NOT NULL,
  `nice_id` varchar(250) DEFAULT NULL,
  `title` varchar(250) DEFAULT NULL,
  `body` text,
  `created_at` datetime NOT NULL,
  `post_byline` varchar(250) DEFAULT NULL,
  `post_full_body` text,
  `post_published_at` datetime DEFAULT NULL,
  `poll_total` int(11) DEFAULT NULL,
  `poll_closed` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `nice_id` (`nice_id`),
  KEY `created_at` (`created_at`),
  KEY `poll_closed` (`poll_closed`)
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8;

LOCK TABLES `node` WRITE;
/*!40000 ALTER TABLE `node` DISABLE KEYS */;
INSERT INTO `node` (`id`,`type`,`nice_id`,`title`,`body`,`created_at`,`post_byline`,`post_full_body`,`post_published_at`,`poll_total`,`poll_closed`)
VALUES
	(1,'post','/post/foo','Fooo','<p>Lorem ipsum dolor</p>','2010-02-01 00:00:00','Fabio Mascarenhas','Full body for post 1.','2010-02-03 02:00:00',NULL,NULL),
	(2,'post',NULL,'Bar','<p>Foo bar baz bloop</p> <p>Boo.</p>','2010-01-31 00:00:00','Fabio Mascarenhas','Full body for post 2.','2010-02-09 02:00:00',NULL,NULL),
	(3,'post','/post/bar-blaz','Bar Blaz','<p>Bar foo</p><ol><li>Foo</li><li>Bar</li></ol>','2010-01-30 00:00:00','Andre Carregal','Full body for post 3.','2010-01-30 00:00:00',NULL,NULL),
	(4,'poll',NULL,'Poll 1','<p>Are you going to buy an iPad?</p>','2010-01-31 00:00:00',NULL,NULL,NULL,802,1),
	(5,'poll','nil','Poll 2','<p>What is the best phone OS?</p>','2010-01-03 00:00:00',NULL,NULL,NULL,1,0),
	(8,'page','/home','Home Page',NULL,'2010-02-07 00:00:00',NULL,NULL,NULL,NULL,NULL),
	(13,'post',NULL,'New Post #2','Foo bar.','2010-02-08 16:39:01','Fabio','Foo bar.<div><br></div><div>Lorem Ipsum.</div>','2010-02-05 00:00:00',NULL,NULL),
	(15,'post',NULL,'Foo Bar','<meta charset=\"utf-8\"><div>Foo Bar in Home.</div><div><br></div>','2010-02-08 17:38:20','Fabio Mascarenhas','<meta charset=\"utf-8\"><div>Foo Bar in Home, with changes.</div><div><br></div><div>Lorem Ipsum in Home.</div>','2010-02-22 03:00:00',NULL,NULL);

/*!40000 ALTER TABLE `node` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table node_term
# ------------------------------------------------------------

DROP TABLE IF EXISTS `node_term`;

CREATE TABLE `node_term` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `node_id` int(11) NOT NULL,
  `term_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `node_id` (`node_id`,`term_id`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8;

LOCK TABLES `node_term` WRITE;
/*!40000 ALTER TABLE `node_term` DISABLE KEYS */;
INSERT INTO `node_term` (`id`,`node_id`,`term_id`)
VALUES
	(11,2,1),
	(3,3,1),
	(14,15,1);

/*!40000 ALTER TABLE `node_term` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table poll_option
# ------------------------------------------------------------

DROP TABLE IF EXISTS `poll_option`;

CREATE TABLE `poll_option` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(250) DEFAULT NULL,
  `weight` smallint(6) DEFAULT NULL,
  `votes` int(11) DEFAULT NULL,
  `poll_id` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `poll_id` (`poll_id`)
) ENGINE=InnoDB AUTO_INCREMENT=38 DEFAULT CHARSET=utf8;

LOCK TABLES `poll_option` WRITE;
/*!40000 ALTER TABLE `poll_option` DISABLE KEYS */;
INSERT INTO `poll_option` (`id`,`name`,`weight`,`votes`,`poll_id`)
VALUES
	(1,'Yes',NULL,101,4),
	(2,'No',NULL,300,4),
	(3,'I don\'t know',NULL,401,4),
	(33,'Touch OS',20,1,5),
	(34,'Android',10,0,5),
	(35,'Symbian',5,0,5),
	(36,'None',1,0,5),
	(37,'Windows Mobile',15,0,5);

/*!40000 ALTER TABLE `poll_option` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table role
# ------------------------------------------------------------

DROP TABLE IF EXISTS `role`;

CREATE TABLE `role` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(250) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

LOCK TABLES `role` WRITE;
/*!40000 ALTER TABLE `role` DISABLE KEYS */;
INSERT INTO `role` (`id`,`name`)
VALUES
	(1,'Anonymous User'),
	(2,'Authenticated User');

/*!40000 ALTER TABLE `role` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table term
# ------------------------------------------------------------

DROP TABLE IF EXISTS `term`;

CREATE TABLE `term` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `vocabulary_id` int(11) NOT NULL,
  `parent_id` int(11) DEFAULT NULL,
  `name` varchar(250) DEFAULT NULL,
  `display_name` varchar(250) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `vocabulary_id` (`vocabulary_id`),
  KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

LOCK TABLES `term` WRITE;
/*!40000 ALTER TABLE `term` DISABLE KEYS */;
INSERT INTO `term` (`id`,`vocabulary_id`,`parent_id`,`name`,`display_name`)
VALUES
	(1,1,NULL,'home','Home Page'),
	(3,1,NULL,'foo','Foo');

/*!40000 ALTER TABLE `term` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table user
# ------------------------------------------------------------

DROP TABLE IF EXISTS `user`;

CREATE TABLE `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `login` varchar(50) DEFAULT NULL,
  `password` varchar(250) DEFAULT NULL,
  `name` varchar(250) DEFAULT NULL,
  `root` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `login` (`login`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

LOCK TABLES `user` WRITE;
/*!40000 ALTER TABLE `user` DISABLE KEYS */;
INSERT INTO `user` (`id`,`login`,`password`,`name`,`root`)
VALUES
	(1,'admin',NULL,'Site Administrator',1);

/*!40000 ALTER TABLE `user` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table user_workgroup
# ------------------------------------------------------------

DROP TABLE IF EXISTS `user_workgroup`;

CREATE TABLE `user_workgroup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) DEFAULT NULL,
  `workgroup_id` int(11) DEFAULT NULL,
  `role` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `pair` (`user_id`,`workgroup_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;



# Dump of table vocabulary
# ------------------------------------------------------------

DROP TABLE IF EXISTS `vocabulary`;

CREATE TABLE `vocabulary` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(50) DEFAULT NULL,
  `display_name` varchar(250) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

LOCK TABLES `vocabulary` WRITE;
/*!40000 ALTER TABLE `vocabulary` DISABLE KEYS */;
INSERT INTO `vocabulary` (`id`,`name`,`display_name`)
VALUES
	(1,'visibility','Visibility');

/*!40000 ALTER TABLE `vocabulary` ENABLE KEYS */;
UNLOCK TABLES;


# Dump of table workgroup
# ------------------------------------------------------------

DROP TABLE IF EXISTS `workgroup`;

CREATE TABLE `workgroup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(250) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

LOCK TABLES `workgroup` WRITE;
/*!40000 ALTER TABLE `workgroup` DISABLE KEYS */;
INSERT INTO `workgroup` (`id`,`name`)
VALUES
	(1,'General');

/*!40000 ALTER TABLE `workgroup` ENABLE KEYS */;
UNLOCK TABLES;





/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
