char *
shmem_version(void)
{
  return "Super Happy Fun OpenSHMEM $Rev$";
}


#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_version=shmem_version")
#endif /* HAVE_PSHMEM_SUPPORT */
