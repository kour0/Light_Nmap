# Light NMAP

Ce projet est un scanner réseau simple configurable à distance via un gestionnaire. Il utilise un couple client-serveur TCP et permet de réaliser la cartographie d'un réseau local en déterminant les machines opérationnelles et les ports ouverts sur une machine donnée.

## Compilation

Pour compiler le projet, utilisez CMake en suivant les étapes suivantes :

1. Créez un répertoire de build : `mkdir build && cd build`
2. Lancez CMake : `cmake ..`
3. Compilez le projet : `make`

## Utilisation

Le programme est constitué d'un serveur et d'un client à lancer séparément. Le serveur doit être lancé avec les droits administrateur (sudo) car il utilise des sockets RAW.

### Lancement du serveur

Lancez le serveur en utilisant la commande suivante :
```
sudo ./server <port>
```
Remplacez `<port>` par le numéro de port sur lequel le serveur doit écouter les connexions entrantes.

### Lancement du client

Lancez le client en utilisant la commande suivante :
```
./client <ip> <port>
```
Remplacez `<ip>` par l'adresse IP du serveur et `<port>` par le numéro de port sur lequel le serveur écoute les connexions entrantes.

Une fois connecté au serveur, le client peut utiliser les commandes suivantes :

* `help [-d]` : affiche la liste des commandes disponibles. Utilisez l'option `-d` pour afficher une description détaillée de chaque commande.
* `scanip [-f]` : scanne les adresses IP du réseau pour déterminer les machines opérationnelles. Utilisez l'option `-f` pour utiliser la version avec les fork, plus rapide.
* `scanport [-f] <ip> [port_debut] [port_fin]` : scanne les ports d'une machine donnée. Utilisez l'option `-f` pour utiliser les threads, plus rapide. Vous pouvez également spécifier un port de début et un port de fin pour limiter la plage de ports à scanner.
* `ping <ip>` : envoie une requête ping à une adresse IP donnée.

## Avertissement

Veillez à bien réaliser les tests dans un environnement où vous y êtes autorisé (salles TP). L'utilisation de ce scanner réseau sans autorisation peut être illégale et entraîner des conséquences graves.
