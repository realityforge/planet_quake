#include <syscall_arch.h>
#include <math.h>
#include <stdlib.h>

CALL_JS_1_TRIPLE(cos, Math.cos)
CALL_JS_1_TRIPLE(sin, Math.sin)
CALL_JS_1_TRIPLE(tan, Math.tan)
CALL_JS_1_TRIPLE(acos, Math.acos)
CALL_JS_1_TRIPLE(asin, Math.asin)
CALL_JS_1_TRIPLE(atan, Math.atan)
CALL_JS_1_TRIPLE(exp, Math.exp)
CALL_JS_1_TRIPLE(log, Math.log)
CALL_JS_1_TRIPLE(sqrt, Math.sqrt)
CALL_JS_1_TRIPLE(fabs, Math.abs)
CALL_JS_1_TRIPLE(ceil, Math.ceil)
CALL_JS_1_TRIPLE(floor, Math.floor)

CALL_JS_2_TRIPLE(atan2, Math.atan2)
CALL_JS_2_TRIPLE(pow, Math.pow)

CALL_JS_1_IMPL_TRIPLE(round, {
  return x >= 0 ? Math.floor(x + 0.5) : Math.ceil(x - 0.5);
})
CALL_JS_1_IMPL_TRIPLE(rint,  {
  function round(x) {
    return x >= 0 ? Math.floor(x + 0.5) : Math.ceil(x - 0.5);
  }
  return (x - Math.floor(x) != .5) ? round(x) : round(x / 2) * 2;
})

EM_JS(int, JS_rand, (void), { return Math.random(x) });
int rand(void) { return JS_rand(); }

EM_JS(void, JS_srand, (unsigned s), {  });
void srand(unsigned s) { return JS_srand(s); }

EM_JS(int, JS_abs, (double x), { return Math.abs(x) });
int abs(int x) { return JS_abs((int)x); }

double nearbyint(double x) { return rint(x); }
float nearbyintf(float x) { return rintf(x); }
