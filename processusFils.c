#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>

void init_Tableau(int *tab, int taille, long repetitions) {
    // Initialisation  du tableau à 0
    for (long i = 0; i < taille; i++) {
        tab[i] = 0;
    }

    // Mise à jour du tableau en fonction de nombres aléatoires
    srand(getpid());
    for (long i = 0; i < repetitions; i++) {
        int n = rand() % taille;
        tab[n] = tab[n] + 1;
    }
}

void miseAJourMemoirePartagee(int *shmaddr, int *tab, int taille) {
    for (int i = 0; i < taille; i++) {
        if (tab[i] != 0) {
            shmaddr[i] = shmaddr[i] + tab[i];
        }
    }
}

int main(int argc, char *argv[]) {

    // Vérification d'argument
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <semaphore_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    printf("je suis le fils");

    // Conversion de l'argument de la ligne de commande en entier pour obtenir l'ID du sémaphore
    int semid = atoi(argv[1]);

    // Conversion de l'argument de la ligne de commande en entier pour obtenir la taille du tableau
    int TAILLE = atoi(argv[2]);

    // Conversion de l'argument de la ligne de commande en entier pour obtenir le nombre de répétitions
    long NUM_REPEAT = atoi(argv[3]);
    

    // Conversion de l'argument de la ligne de commande en entier pour obtenir un nombre utilisé comme une clé IPC
    key_t key = atoi(argv[4]);

    
    int *tab = (int *)malloc(TAILLE * sizeof(int));
    if (tab == NULL) {
        fprintf(stderr, "Erreur lors de l'allocation de mémoire.\n");
        return 1;  // Code d'erreur
    }

    // Initialisation du générateur de nombres aléatoires en utilisant l'ID du processus courant (PID)

    // Appel de la fonction init_Tableau
    init_Tableau(tab, TAILLE, NUM_REPEAT);

    // Attente du sémaphore
    struct sembuf sem_wait = {0, -1, 0};
    struct sembuf sem_post = {0, 1, 0};

    // Accès à la mémoire partagée avec synchronisation
    semop(semid, &sem_wait, 1);

    // Attachement à la mémoire partagée
    int shmid = shmget(key, TAILLE * sizeof(int), 0666);
    int *shmaddr = shmat(shmid, NULL, 0);

    // Code pour accéder à la mémoire partagée
    printf("Programme Fils %d : Ecriture dans la memoire partagee \n", getpid());
    miseAJourMemoirePartagee(shmaddr, tab, TAILLE);

    // Détachement de la mémoire partagée
    shmdt(shmaddr);

    // Libération du sémaphore
    semop(semid, &sem_post, 1);

    // Libération de la mémoire allouée pour le tableau
    free(tab);

    return 0;
}
