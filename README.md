# Projet-C

## Morpion en C (Tic-Tac-Toe en C)

Ce programme est une implémentation en ligne de commande du jeu classique du Morpion (Tic-Tac-Toe), écrit entièrement en langage C. Il a été adapté à partir d'une version C++ initiale, en utilisant les idiomes et bibliothèques standard du C. Le jeu se joue directement dans le terminal.  
Il offre différents modes de jeu, y compris contre un autre joueur ou contre une intelligence artificielle de difficulté variable.

## Fonctionnalités

* **Interface en Console :** Le jeu se déroule entièrement dans le terminal texte.
* **Modes de jeu :**
    * **Joueur vs Joueur :** Deux joueurs humains s'affrontent sur un plateau 3x3.
    * **Joueur vs IA Facile :** Un joueur humain affronte une IA qui joue des coups aléatoires sur les cases vides (plateau 3x3).
    * **Joueur vs IA Difficile :** Un joueur humain affronte une IA plus avancée utilisant l'algorithme Minimax (avec une profondeur limitée pour la performance) sur un **plateau 5x5**. La condition de victoire sur ce plateau est d'aligner 4 symboles.
* **Curseur de Sélection :** Un curseur `[]` indique la case actuellement sélectionnée sur le plateau.
* **Compatibilité Multiplateforme :** Le code utilise des directives de préprocesseur (`#ifdef _WIN32`) pour gérer les différences entre Windows et les systèmes POSIX (Linux, macOS) pour l'entrée non bufferisée (`getch`) et la fonction de pause (`sleep`).

## Bibliothèques Utilisées (Headers C Standard et Spécifiques)

Le programme utilise les bibliothèques C suivantes :

* **Bibliothèques Standard C :**
    * `stdio.h` : Pour les fonctions d'entrée/sortie standard (`printf`, `scanf`, `getchar`, etc.).
    * `stdlib.h` : Pour l'allocation dynamique de mémoire (`malloc`, `free`), la génération de nombres aléatoires (`rand`, `srand`), et la fonction `system` (utilisée pour effacer l'écran sous Windows).
    * `stdbool.h` : Pour utiliser le type de données `bool` et les constantes `true`/`false` (Standard C99).
    * `time.h` : Pour initialiser le générateur de nombres aléatoires (`srand(time(NULL))`).
    * `limits.h` : Pour les constantes `INT_MIN` et `INT_MAX`, utilisées dans l'algorithme Minimax.

* **Bibliothèques Spécifiques à la Plateforme :**
    * **Windows :**
        * `conio.h` : Pour la fonction `_getch()` qui permet de lire un caractère sans attendre la touche Entrée.
        * `windows.h` : Pour la fonction `Sleep()` qui met en pause l'exécution.
    * **POSIX (Linux, macOS) :**
        * `termios.h` : Pour contrôler les attributs du terminal afin d'implémenter une lecture de caractère non bufferisée et sans écho (similaire à `getch`).
        * `unistd.h` : Pour la fonction `read()` (utilisée dans l'implémentation `getch_custom`) et `usleep()` (équivalent de `Sleep` avec une résolution en microsecondes).

## Commandes du Jeu

### Menu Principal :

* Tapez le numéro correspondant au mode de jeu désiré (1, 2, ou 3) et appuyez sur `Entrée`.
* Tapez `4` et appuyez sur `Entrée` pour quitter le programme.

### En Cours de Partie :

* **Flèches Directionnelles** (Haut, Bas, Gauche, Droite) : Utilisez les flèches de votre clavier pour déplacer le curseur `[]` sur la case souhaitée du plateau de jeu.
* **Touche `e`** : Appuyez sur `e` pour placer votre symbole (`X` ou `O`) sur la case actuellement sélectionnée par le curseur. Le coup n'est accepté que si la case est vide.
* **Touche `q`** : Appuyez sur `q` pour abandonner la partie en cours et revenir au menu principal.

## Compilation

Pour compiler ce programme, vous avez besoin d'un compilateur C (comme GCC). Ouvrez un terminal dans le répertoire contenant le fichier source (par exemple, `morpion.c`) et exécutez la commande suivante :

```bash
gcc morpion.c -o morpion_c
