# Projet C : Morpion (Tic-Tac-Toe)

Ce fichier fournit les informations essentielles concernant le jeu de Morpion en C développé précédemment. Il inclut une description, les fonctionnalités clés, les instructions d'utilisation et les commandes.

## Sommaire

1.  [Description Brève](#description-brève)
2.  [Points Importants et Fonctionnalités](#points-importants-et-fonctionnalités)
3.  [Mode d'Emploi](#mode-demploi)
    * [Prérequis](#prérequis)
    * [Compilation](#compilation)
    * [Exécution](#exécution)
4.  [Commandes en Jeu](#commandes-en-jeu)

---

## 1. Description <a name="description-brève"></a>

Ce programme est une implémentation en langage C d'un jeu de Morpion jouable entièrement dans un terminal (console).

Il offre plusieurs modes de jeu :
* **Joueur vs Joueur** sur une grille classique de 3x3.
* **Joueur vs IA Facile** sur une grille 3x3, où l'IA effectue des mouvements aléatoires.
* **Joueur vs IA Difficile** sur une grille étendue de 5x5, où l'IA utilise l'algorithme **Minimax** pour tenter de jouer le meilleur coup possible (victoire par alignement de 4 pions).

L'interface en console utilise des couleurs (via les codes ANSI) pour une meilleure distinction des joueurs et du curseur, et le code est conçu pour être multiplateforme (Windows, Linux, macOS).

---

## 2. Points Importants et Fonctionnalités <a name="points-importants-et-fonctionnalités"></a>

* **Jeu Complet :** Implémente les règles du Morpion (3x3, aligner 3) et une variante Gomoku (5x5, aligner 4).
* **Modes de Jeu Variés :** Permet de jouer seul contre l'ordinateur (deux niveaux) ou à deux joueurs.
* **Intelligence Artificielle :**
    * **Niveau Facile :** Sélection aléatoire parmi les cases vides. Simple mais fonctionnel.
    * **Niveau Difficile :** Utilisation de l'algorithme **Minimax** pour explorer les coups futurs jusqu'à une certaine profondeur. L'IA évalue les états du jeu pour maximiser ses chances de gagner et minimiser celles de l'adversaire.
* **Interface Console Améliorée :**
    * Affichage clair de la grille.
    * Utilisation de **couleurs ANSI** pour 'X', 'O', le curseur et les messages.
    * **Curseur interactif** contrôlable via les flèches du clavier.
* **Multiplateforme :**
    * Utilisation de directives de préprocesseur (`#ifdef _WIN32`) pour adapter les fonctions spécifiques au système d'exploitation :
        * Nettoyage de l'écran (`clear_screen`).
        * Lecture de caractère non-bufferisée (`getch_custom`).
        * Pause (`sleep_ms`).
* **Gestion Dynamique de la Mémoire :**
    * Allocation de la mémoire pour la grille de jeu via `malloc`.
    * Libération systématique de la mémoire via `free` (`free_game`) pour prévenir les fuites mémoire.
* **Structure Modulaire :** Le code est organisé en fonctions distinctes pour chaque tâche (initialisation, affichage, gestion des entrées, logique IA, vérification du gagnant, etc.), améliorant la lisibilité et la maintenance.
* **Détection de Fin de Partie :** Le jeu détecte correctement les victoires (alignements horizontaux, verticaux, diagonaux) et les matchs nuls (grille pleine sans vainqueur).

---

## 3. Mode d'Emploi <a name="mode-demploi"></a>

### Prérequis <a name="prérequis"></a>

* Un **compilateur C** (par exemple, GCC sous Linux/macOS, MinGW/MSYS2 ou le compilateur intégré à Visual Studio sous Windows).

### Compilation <a name="compilation"></a>

1.  Sauvegardez le code source dans un fichier nommé `main.c`.
2.  Ouvrez votre terminal ou invite de commande.
3.  Naviguez (`cd`) jusqu'au répertoire où vous avez enregistré `main.c`.
4.  Compilez le code à l'aide de la commande suivante :

    ```bash
    gcc main.c -o morpion
    ```
    * *(Note: Bien que ce code spécifique ne semble pas utiliser de fonctions mathématiques complexes nécessitant `-lm`, si vous rencontrez des erreurs de liaison liées à `<math.h>` ou `<limits.h>` sur certains systèmes, essayez : `gcc main.c -o morpion -lm`)*

5.  Si aucune erreur n'apparaît, un fichier exécutable sera créé :
    * `morpion` sur Linux/macOS.
    * `morpion.exe` sur Windows.

### Exécution <a name="exécution"></a>

1.  Dans le même terminal, lancez l'exécutable :

    * **Linux / macOS :**
        ```bash
        ./morpion
        ```
    * **Windows :**
        ```bash
        .\morpion.exe
        ```
        ou simplement
        ```bash
        morpion
        ```

2.  Le jeu démarrera et affichera le menu principal. Suivez les instructions à l'écran pour choisir un mode de jeu.

---

## 4. Commandes en Jeu <a name="commandes-en-jeu"></a>

Pendant une partie, utilisez les touches suivantes :

* **Flèches directionnelles** (↑, ↓, ←, →) : Déplacer le curseur (représenté par `[ ]` ou `[X]` / `[O]`) sur la case souhaitée.
* **Touche 'e'** OU **Touche 'Entrée'** : Confirmer la position du curseur et placer votre pion ('X' ou 'O') sur la case sélectionnée (si elle est vide).
* **Touche 'q'** : Quitter la partie en cours immédiatement et retourner au menu principal.

---
Fin du README
EOT;

// Affiche le contenu généré.
// Vous pouvez rediriger la sortie vers un fichier : php ce_script.php > README.md
echo $readmeContent;

?>
