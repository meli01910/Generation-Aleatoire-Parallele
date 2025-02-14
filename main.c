#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <math.h>
#include <limits.h>
#include <sys/sysinfo.h>
#include <time.h>


int getNumChildren() {
    // Permet obtenir le nombre de processeurs disponibles
    long numProcessors = sysconf(_SC_NPROCESSORS_ONLN);

    return (int)numProcessors;
}

#define NUM_CHILDREN getNumChildren()


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
// Structure union utilisée pour différentes opérations sur les sémaphores
union semun {
    int val;              // Pour SETVAL
    struct semid_ds *buf; // Pour IPC_STAT et IPC_SET
    ushort *array;        // Pour GETALL et SETALL
};

// Fonction P pour acquérir le sémaphore
void P(int semid) {
    struct sembuf operation = {0, -1, 0};
    semop(semid, &operation, 1);
}

// Fonction V pour libérer le sémaphore
void V(int semid) {
    struct sembuf operation = {0, 1, 0};
    semop(semid, &operation, 1);
}

//Fonction pour calculer la moyenne
double calculerMoyenne(const int *donnees, int taille) {
    double somme = 0.0;
    for (int i = 0; i < taille; ++i) {
        somme += donnees[i];
    }
    return somme / taille;
}

// Fonction pour calculer la variance
double calculerVariance(const int *donnees, int taille, double moyenne) {
    double sommeCarresDifferences = 0.0;
    for (int i = 0; i < taille; ++i) {
        sommeCarresDifferences += pow((moyenne - donnees[i]), 2.0);
    }
    return sommeCarresDifferences / taille;
}

//Fonction pour calculer la variance
double calculerEcartType(double variance) {
    return sqrt(variance);
}

// Fonction pour la lecture et l'affichage de la mémoire partagée
void lireEtAfficherMemoirePartagee(int *shmaddr, int taille) {
    printf("Lecture de la mémoire partagée :\n");
    for (int i = 0; i < taille; ++i) {
        printf("Tab[%d] = %d \n", i, shmaddr[i]);
    }
}
//Renvoie les information sur la ram
void get_ram_info(double *total_ram, double *free_ram) {
    struct sysinfo si;
    sysinfo(&si);

    *total_ram = (double)si.totalram * si.mem_unit / (1024.0 * 1024.0 * 1024.0);
    *free_ram = (double)si.freeram * si.mem_unit / (1024.0 * 1024.0 * 1024.0);
}



int main() {
    
    //chaine de caractere qui permetent d'envoyer des valeurs aux processus fils
    char arg_str[10]; 
    char arg_str2[10];
    char arg_str3[100];
    char arg_str4[100];
    
    /* Génération de la clé pour le sémaphore et la mémoire partagée
     ftok("./monfichier1.txt", 'P'); on peut generer la cle en combinant la clé avec l'inode d'un fichier*/
     key_t key =49;
   // Création du sémaphore
    int semid = semget(key, 1, IPC_CREAT | 0666);
    
    //Gestion d'erreur de création de sémaphore.
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // Initialisation du sémaphore à 1 (non utilisé)
    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);


   // Calcul de la taille maximale du tableau en fonction de la mémoire disponible
   double ram_size_gb;
   double ram_size_total;
   get_ram_info(&ram_size_total,&ram_size_gb);
   long max_array_size = (long)(ram_size_gb * 1024.0 * 1024.0 * 1024.0) / sizeof(int);
   // Taille du tableau pour chaque processus
   long array_size_per_child = max_array_size / (NUM_CHILDREN +2);
   long NUM_REPEAT=(long)INT_MAX*10;
  
   // Création de la mémoire partagée avec la taille maximale du tableau
   int shmid = shmget(key, array_size_per_child * sizeof(int), IPC_CREAT | 0666);
    //Gestion d'erreur de création de la mémoire partagée.
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
	
    sprintf(arg_str,"%d",semid);//converssion de semid en chaine de caractere
    sprintf(arg_str2,"%ld",array_size_per_child);//converssion de TAILLE en chaine de caractere
    sprintf(arg_str3,"%ld",NUM_REPEAT);//converssion de NUM_REPEAT en chaine de caractere
    sprintf(arg_str4,"%d",key);//converssion de key en chaine de caractere
   
    // Attachement à la mémoire partagée
    int *shmaddr = shmat(shmid, NULL, 0);
	 printf("\033[1;32m\n");
	 
    // Écriture dans la mémoire partagée
    printf("Programme Parent %d : INITIALISATION DU TABLEAU A 0\n",getpid());
    P(semid);
    for (int i = 0; i < array_size_per_child; ++i) {
        shmaddr[i] = 0;
    }
    V(semid);
    // Lancement des processus fils
    printf("Programme Parent %d : Creation de %d fils\n",getpid(),NUM_CHILDREN);
    for (int i = 0; i < NUM_CHILDREN; ++i) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Code du processus fils
            execl("./processusFils", "processusFils.c",arg_str,arg_str2, arg_str3,arg_str4,(char *)NULL);
            perror("execl"); // En cas d'échec
            exit(EXIT_FAILURE);
        }
    }
    
    
    
    int *tab = (int *)malloc(array_size_per_child * sizeof(int));
    if (tab == NULL) {
        fprintf(stderr, "Erreur lors de l'allocation de mémoire.\n");
        return 1;  // Code d'erreur
    }

    // Appel de la fonction init_Tableau
    init_Tableau(tab,array_size_per_child, NUM_REPEAT);  
    // Accès à la mémoire partagée avec synchronisation
    P(semid);
    // Code pour accéder à la mémoire partagée
    printf("Programme parent %d : Ecriture dans la memoire partagee \n", getpid());
    miseAJourMemoirePartagee(shmaddr, tab, array_size_per_child);
    V(semid);
    free(tab);
for (int i = 0; i < NUM_CHILDREN; ++i) {
    wait(NULL);
}
int minimum=shmaddr[0];
int maximum=shmaddr[0];
int diff=0;
P(semid);
for(int i=1;i<array_size_per_child;i++){
	if(shmaddr[i]<minimum){
		minimum=shmaddr[i];
	}else if(shmaddr[i]>maximum){
		maximum=shmaddr[i];
	}
}
V(semid);
double pourcen=0.;
diff=maximum-minimum;


pourcen=((double)diff/(NUM_REPEAT*(NUM_CHILDREN+1)))*100;
double moyenne, variance, ecartType;

//lireEtAfficherMemoirePartagee(shmaddr, array_size_per_child);

moyenne=calculerMoyenne(shmaddr,array_size_per_child);

variance=calculerVariance(shmaddr,array_size_per_child,moyenne);

ecartType=calculerEcartType(variance);

printf("\033[1;32m\n");
printf("Programme Parent %d : RAPPORT SUR rand()\n", getpid());
printf("Taille totale de la RAM : %.2lf Go\n", ram_size_total);
printf("Taille disponible de la RAM : %.2lf Go\n", ram_size_gb);
printf("Taille du Tableau : %ld \n", array_size_per_child);
printf("Nombre Total de Random réalisé : %ld \n", NUM_REPEAT*(NUM_CHILDREN+1) );
printf("\033[1;33m"); 
printf("Moyenne : %lf\n", moyenne);
printf("Variance : %lf\n", variance);
printf("Ecart-type : %lf\n", ecartType);
printf("Max : %d\n", maximum);
printf("Min : %d\n", minimum);
printf("diff : %d\n", diff);
printf("Observation %.15lf %% soit %.15lf %%  \n",pourcen,100-pourcen);
printf("\033[0m"); 

  

    

  //Detachement de la memoire partagée
    shmdt(shmaddr);

    // Suppression de la mémoire partagée
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    // Suppression du sémaphore
    if (semctl(semid, 0, IPC_RMID, arg) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    return 0;
}
