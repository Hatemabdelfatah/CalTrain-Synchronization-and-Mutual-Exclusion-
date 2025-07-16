#include <pthread.h>
#include "caltrain.h"

void station_init(struct station *station) {
    station->waiting = 0;
    station->available = 0;
    station->boarding = 0;
    pthread_mutex_init(&station->lock, NULL);
    pthread_cond_init(&station->train, NULL);
    pthread_cond_init(&station->allAboard, NULL);
}

void station_wait_for_train(struct station *station) {
    pthread_mutex_lock(&station->lock);
    
    // A new passenger arrives.
    station->waiting++;
    
    // Wait until a train arrives with at least one available seat.
    while (station->available == 0)
        pthread_cond_wait(&station->train, &station->lock);
    
    // Once signaled, the passenger claims a seat.
    station->boarding++;
    station->waiting--;
    station->available--;  // Reserve the seat.
    
    pthread_mutex_unlock(&station->lock);
}

void station_on_board(struct station *station) {
    pthread_mutex_lock(&station->lock);
    
    // Passenger is now seated.
    station->boarding--;
    
    // If boarding is complete and no available seats remain,
    // signal the train that it can depart.
    if (station->boarding == 0 && station->available == 0)
        pthread_cond_signal(&station->allAboard);
    
    pthread_mutex_unlock(&station->lock);
}

void station_load_train(struct station *station, int count) {
    pthread_mutex_lock(&station->lock);

    // Return immediately if no passengers are waiting.
    if (station->waiting == 0) {
        pthread_mutex_unlock(&station->lock);
        return;
    }
    
    // Reserve as many seats as possible (up to the number waiting).
    station->available = (count < station->waiting) ? count : station->waiting;
    
    // Wake up all waiting passengers.
    if (station->available > 0)
        pthread_cond_broadcast(&station->train);
    
    // Wait until all allowed passengers have boarded.
    while (station->boarding > 0 || station->available > 0)
        pthread_cond_wait(&station->allAboard, &station->lock);
    
    pthread_mutex_unlock(&station->lock);
}
