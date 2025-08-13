#ifndef PER_HOSTDMA_H
#define PER_HOSTDMA_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/*----- Typedefs -----------------------------------------------------*/

typedef enum {
    HOSTDMA_SUCCESS = 0,
    HOSTDMA_UNINITIALISED,
    HOSTDMA_BUS_OCCUPIED,
} t_hostdma_status;

void per_hostdma_init();
void per_hostdma_reset();
t_hostdma_status per_hostdma_transfer(uint32_t host_address, uint16_t *words, uint16_t word_count);
bool per_hostdma_is_bus_available();

#ifdef __cplusplus
}
#endif
#endif
/*----- End of file --------------------------------------------------*/
