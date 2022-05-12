#pragma once

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <istream>
#include <string>
#include <list>
#include <utility>

bool	parse_config_file(const std::string& file_name);

/* PARSING *
 * Parsing lexical
 * Parsing sytaxique
 * Créer nos objets servers
 * Trier les membres (ports, ips, paires ips-ports) de nos objets
 * Créer les sockets et les binds à des ports. (Si un port n'a pas d'ip assignée)
 * - alors ouvrir le port et le bind a ADDR_ANY sinon si le port a une ip assignée
 * - mais a aussi un listen sans aucune ip assignée alors ouvrir le port et le bind
 * - a ADDR_ANY sinon si il n y a qu'une adresse ip assignée à une ip alors 
 * - ouvrir le port et le bind à l'adresse assignée sinon si qu'une adresse
 * - ip est spécifiée alors la bind au port 80.
 * exemples des trois cas cités ci-dessus:
 *  1- Listen 80;
 *  2- Listen localhost:9191; listen 9191;
 *  3- listen bidule:8888;
 *  4- listen localhost;
 */

/*
 * CREATE SERVER
 * Tous les ports et toutes les ips dans listen du server
 * ensuite faire le tri pour laisser les paires et les ports les plus pertinent.

*/

/*
 * READ FROM SERVER *
 * on reçoit une requête :
 * case 1: Les 

*/
