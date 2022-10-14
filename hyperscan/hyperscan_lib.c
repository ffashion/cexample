#include <hs/hs.h>
#include <hs/ch.h>
#include <string.h>
#include <stdio.h>



hs_error_t my_hs_compile(const char *expression, unsigned int flags,
                               unsigned int mode,
                               const hs_platform_info_t *platform,
                               hs_database_t **db, hs_compile_error_t **error) {

        
            return hs_compile(expression, flags, mode, platform, db, error);
}

ch_error_t HS_CDECL my_ch_compile(const char *expression, unsigned int flags,
                               unsigned int mode,
                               const hs_platform_info_t *platform,
                               ch_database_t **db,
                               ch_compile_error_t **compile_error) {
            return ch_compile(expression, flags, mode, platform, db, compile_error);

}