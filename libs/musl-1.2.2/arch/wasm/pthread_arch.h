struct pthread *__pthread_self(void);

static inline uintptr_t __get_tp()
{
  return NULL;
}

#define TP_ADJ(p) (p)

#define CANCEL_REG_IP 16
