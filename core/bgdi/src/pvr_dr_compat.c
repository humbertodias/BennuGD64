/*
 * kos-ports libGL 1.1.0 calls pvr_dr_init as an exported function.
 * Current KallistiOS only provides it as a static inline no-op in
 * pvr_legacy.h, so the prebuilt libGL.a needs this symbol at link time.
 */
#include <stdint.h>

typedef uint32_t pvr_dr_state_t;

void pvr_dr_init(pvr_dr_state_t *vtx_buf_ptr)
{
    (void)vtx_buf_ptr;
}
