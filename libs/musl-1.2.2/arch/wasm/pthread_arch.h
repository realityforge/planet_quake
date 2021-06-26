struct pthread *__pthread_self(void);

pthread_t __get_tp(void);

#define TP_ADJ(p) (p)

#define CANCEL_REG_IP 16
