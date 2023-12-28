//bouguettaia imane groupe 2
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define N 10 // Taille du tampon
#define n1 5 // Nombre de lignes de B
#define m1 5 // Nombre de colonnes de B (également nombre de lignes de C)
#define n2 5 // Nombre de colonnes de C

// Matrices
int B[n1][m1], C[m1][n2], A[n1][n2];
// Tampon
int T[n1][n2];

// Synchronisation
pthread_mutex_t mutex;
sem_t empty;
sem_t full;

// Fonction pour produire une ligne de la matrice B
int produce() {
    return rand() % n1;
}

// Modifier la fonction insert_item pour utiliser un tableau bidimensionnel
void insert_item(int row, const int matrix[][n2]) {
    for (int i = 0; i < n2; ++i) {
        T[row][i] = matrix[row][i];
    }
}

// Modifier la fonction remove_item pour utiliser un tableau bidimensionnel
void remove_item(int row, int matrix[][n2]) {
    for (int i = 0; i < n2; ++i) {
        matrix[row][i] = T[row][i];
    }
}

// Producteur
void *producer(void *arg) {
    int row;

    for (int i = 0; i < n1; ++i) {
        row = produce();

        sem_wait(&empty); // Attente d'une place libre
        pthread_mutex_lock(&mutex);
        // Section critique
        for (int col = 0; col < n2; ++col) {
            int result = 0;
            for (int j = 0; j < m1; ++j) {
                result += B[row][j] * C[j][col];
            }
            T[row][col] = result;
        }
        pthread_mutex_unlock(&mutex);

        sem_post(&full); // Il y a une place remplie en plus
    }

    pthread_exit(NULL);
}

// Consommateur
void *consumer(void *arg) {
    int row;

    for (int i = 0; i < n1; ++i) {
        sem_wait(&full); // Attente d'une place remplie
        pthread_mutex_lock(&mutex);
        // Section critique
        remove_item(i, A);
        pthread_mutex_unlock(&mutex);

        sem_post(&empty); // Il y a une place libre en plus
    }

    pthread_exit(NULL);
}

int main() {
    // Initialisation des matrices B et C avec des valeurs aléatoires
    for (int i = 0; i < n1; ++i) {
        for (int j = 0; j < m1; ++j) {
            B[i][j] = rand() % 10; // Remplacez par vos valeurs
        }
    }

    for (int i = 0; i < m1; ++i) {
        for (int j = 0; j < n2; ++j) {
            C[i][j] = rand() % 10; // Remplacez par vos valeurs
        }
    }

    // Initialisation
    sem_init(&empty, 0, N);  // Buffer vide
    sem_init(&full, 0, 0);   // Buffer vide
    pthread_mutex_init(&mutex, NULL); // Exclusion mutuelle

    // Création des threads producteurs
    pthread_t producer_thread;
    pthread_create(&producer_thread, NULL, producer, NULL);

    // Création des threads consommateurs
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Attente du thread producteur
    pthread_join(producer_thread, NULL);

    // Indiquer au consommateur que la production est terminée
    sem_post(&full);

    // Attente du thread consommateur
    pthread_join(consumer_thread, NULL);

    // Afficher les matrices et le résultat à la fin du programme
    printf("Matrix B:\n");
    for (int i = 0; i < n1; ++i) {
        for (int j = 0; j < m1; ++j) {
            printf("%d ", B[i][j]);
        }
        printf("\n");
    }

    printf("\nMatrix C:\n");
    for (int i = 0; i < m1; ++i) {
        for (int j = 0; j < n2; ++j) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }

    printf("\nMatrix A (Result):\n");
    for (int i = 0; i < n1; ++i) {
        for (int j = 0; j < n2; ++j) {
            printf("%d ", A[i][j]);
        }
        printf("\n");
    }

    // Destruction des ressources
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
