#include <stdio.h>
#include <wayland-client.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>

#include "global_registry_handle.h"

#define WRONG_MULTI_THREAD_SYNC_PROCESS 1

static void sync_callback( void* data, struct wl_callback* pCallback, uint32_t serial );
static void sync_callback_roundtrip(void *data, struct wl_callback * pCallback, uint32_t serial);
static void thread_job_done_callback( void* data, struct wl_callback* pCallback, uint32_t serial );

struct ClientObjState
{
    struct wl_display* pDisplay;
    struct wl_callback* pCallback1;
    struct wl_callback* pCallback2;
    struct wl_callback* pCallback3;
    struct wl_event_queue* pThreadEventQueue;
    sem_t mSem1;
    sem_t mSem2;
    sem_t mSem3;
    sem_t mSem4;
    uint8_t mbThreadCheck;
    uint8_t mbMainCheck;
    uint8_t mbThreadJobDone;
};

static const struct wl_callback_listener objSync_listener = {
    sync_callback
};

static const struct wl_callback_listener objSync_listener_roundtrip = {
    sync_callback_roundtrip
};

static const struct wl_callback_listener threadJob_done_listener = {
    thread_job_done_callback
};

static void sync_callback( void* data, struct wl_callback* pCallback, uint32_t serial )
{
    wl_callback_destroy( pCallback );

    struct ClientObjState* pObjState = data;
    pObjState->mbMainCheck = 1;
    
    printf("Performed Main Callback Sync\n");
}

static void sync_callback_roundtrip(void *data, struct wl_callback * pCallback, uint32_t serial)
{
    wl_callback_destroy(pCallback);

    struct ClientObjState* pObjState = data;
    pObjState->mbThreadCheck = 1;

    printf("Performed Thread Sync Roundtrip\n");
}

static void thread_job_done_callback( void* data, struct wl_callback* pCallback, uint32_t serial )
{
    wl_callback_destroy(pCallback);

    struct ClientObjState* pObjState = data;
    pObjState->mbThreadJobDone = 1;

    printf("Finished Thread Work\n");
}

#if WRONG_MULTI_THREAD_SYNC_PROCESS 

void* ThreadedFunc( void* pArg )
{
    struct ClientObjState* pClientObjState = (struct ClientObjState*) pArg;
    pthread_detach( pthread_self() );

    pClientObjState->pThreadEventQueue = wl_display_create_queue( pClientObjState->pDisplay );
    pClientObjState->pCallback2 = wl_display_sync( pClientObjState->pDisplay );

    
    //wl_proxy_set_queue( (struct wl_proxy*) pClientObjState->pCallback1, pClientObjState->pThreadEventQueue );

    wl_callback_add_listener( pClientObjState->pCallback2, &objSync_listener_roundtrip, pClientObjState);
    wl_proxy_set_queue( (struct wl_proxy*) pClientObjState->pCallback2, pClientObjState->pThreadEventQueue );

    int roundtrippedReq = 0;
    int processedEvents = 0;
    printf("Performing Threaded Queue Roundtrip\n");
    //roundtrippedReq = wl_display_roundtrip_queue(pClientObjState->pDisplay, pClientObjState->pThreadEventQueue);
    printf("Processed Requests After Threaded Queue Roundtrip : %d\n", roundtrippedReq);
    
    while( !pClientObjState->mbThreadCheck )
    {
        wl_display_dispatch_queue( pClientObjState->pDisplay, pClientObjState->pThreadEventQueue );
    }

    pClientObjState->pCallback3 = wl_display_sync( pClientObjState->pDisplay );
    wl_callback_add_listener( pClientObjState->pCallback3, &threadJob_done_listener, pClientObjState );
    wl_proxy_set_queue( (struct wl_proxy*) pClientObjState->pCallback3, pClientObjState->pThreadEventQueue );

    pthread_exit(NULL);
}

int main( int argc, const char* argv[] )
{
    struct ClientObjState clientObjState = {0};
    struct GlobalObjectState gObjState = {0};

    sem_init(&clientObjState.mSem1, 0, 0 );
    sem_init(&clientObjState.mSem2, 0, 0 );
    sem_init(&clientObjState.mSem3, 0, 0 );
    sem_init(&clientObjState.mSem4, 0, 0 );

    clientObjState.pDisplay = wl_display_connect( NULL );

    struct wl_registry* pRegistry = wl_display_get_registry(clientObjState.pDisplay);
    wl_registry_add_listener(pRegistry, &g_registryListener, &gObjState);

    //wl_display_roundtrip(clientObjState.pDisplay);

    pthread_t thread1;

    pthread_create(
        &thread1,
        NULL,
        &ThreadedFunc,
        &clientObjState
    );
    
    printf("Performing Main Queue Roundtrip\n");
    int roundTrippedReq = wl_display_roundtrip(clientObjState.pDisplay);
    printf("Processed Requests after Main Queue Roundtrip : %d\n", roundTrippedReq);

    clientObjState.pCallback1 = wl_display_sync( clientObjState.pDisplay );
    wl_callback_add_listener( clientObjState.pCallback1, &objSync_listener, &clientObjState );

    while( !clientObjState.mbThreadCheck || !clientObjState.mbMainCheck )
    {
        wl_display_dispatch_pending( clientObjState.pDisplay );
    }
    printf("Finshing Main Work\n");

    wl_display_roundtrip_queue( clientObjState.pDisplay, clientObjState.pThreadEventQueue );

    wl_event_queue_destroy(clientObjState.pThreadEventQueue);
    wl_display_disconnect(clientObjState.pDisplay);
}   

#endif 