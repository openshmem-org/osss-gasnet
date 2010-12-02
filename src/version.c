char *
shmem_version(void)
{
  return "Super Happy Fun OpenSHMEM, Revision: 1233";
}

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_version=shmem_version")
#endif /* HAVE_PSHMEM_SUPPORT */
