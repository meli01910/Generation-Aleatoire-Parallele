Simulation de Génération de Nombres Aléatoires avec Mémoire Partagée et Sémaphores

Ce projet est une application en C qui simule la génération de nombres aléatoires en utilisant des processus légers (fork) et des mécanismes de synchronisation (sémaphores) pour manipuler une mémoire partagée. L'objectif est de générer un grand nombre de valeurs aléatoires, de les stocker dans un tableau partagé, et de calculer des statistiques telles que la moyenne, la variance, l'écart-type, ainsi que les valeurs minimales et maximales.
Fonctionnalités
Gestion des processus : Le programme principal crée plusieurs processus fils en fonction du nombre de processeurs disponibles sur la machine.
Mémoire partagée : Un tableau est partagé entre tous les processus (parent et fils) pour stocker les résultats des générations de nombres aléatoires.
Synchronisation : Les accès à la mémoire partagée sont synchronisés à l'aide de sémaphores pour éviter les conditions de course.
Génération de nombres aléatoires : Chaque processus fils génère un grand nombre de nombres aléatoires et met à jour le tableau partagé.
Analyse statistique : Le programme calcule des statistiques sur les données stockées dans le tableau partagé.

Structure du Projet : 
main.c : Le programme principal qui crée les processus fils, initialise la mémoire partagée et les sémaphores, et calcule les statistiques finales.
processusFils.c : Le code exécuté par chaque processus fils pour générer des nombres aléatoires et mettre à jour la mémoire partagée.
