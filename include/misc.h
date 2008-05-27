//TODO
#define MAX2(a,b) (((a)>(b))?(a):(b))
#define ABS(a) (MAX2((a),(-(a))))
#define MAX3(a,b,c) (MAX2(MAX2(a,b),c))
#define CHOOSE_MAX(a, b, c, da, db, dc) (((a > b) && (a > c))?(da):((b>c)?(db):(dc)))

float smooth_bell(float x);
