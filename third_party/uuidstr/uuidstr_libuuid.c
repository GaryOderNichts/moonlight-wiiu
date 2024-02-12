#include "uuidstr.h"

#include <uuid.h>

bool uuidstr_random(uuidstr_t *dest) {
    uuid_t uuid;
    uuid_generate_random(uuid);
    uuid_unparse(uuid, (char *) dest);
    return true;
}