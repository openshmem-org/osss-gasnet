/*
 * so apparently these are compatibility routines
 * for older SGI architectures.  So probably fine
 * to let them all be empty
 */

void
shmem_clear_cache_inv(void)
{
}

void
shmem_set_cache_inv(void)
{
}

void
shmem_clear_cache_line_inv(void *target)
{
}

void
shmem_set_cache_line_inv(void *target)
{
}

void
shmem_udcflush(void)
{
}

void
shmem_udcflush_line(void *target)
{
}
