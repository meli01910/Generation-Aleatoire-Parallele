# Simulation de Génération de Nombres Aléatoires
Ce projet est une application en C qui simule la génération de nombres aléatoires en utilisant des processus légers (fork) et des mécanismes de synchronisation (sémaphores) pour manipuler une mémoire partagée. L'objectif est de générer un grand nombre de valeurs aléatoires, de les stocker dans un tableau partagé, et de calculer des statistiques telles que la moyenne, la variance, l'écart-type, ainsi que les valeurs minimales et maximales.
## Fonctionnalités
- **Gestion des processus** : Crée plusieurs processus fils.
- **Mémoire partagée** : Partage des données entre processus.
- **Synchronisation** : Utilise des sémaphores pour éviter les conditions de course.

## Compilation et Exécution
1. Compilez le programme :
   ```bash
   gcc main.c -o main -lm
   gcc processusFils.c -o processusFils
