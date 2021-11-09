// followed the pinos_manual.
#define FRACTION (1 << 14)
int parseToFp(int n);
int parseToInt(int x);
int parsetToIntRound(int x);
int add_Fp(int x, int y);
int sub_Fp(int x, int y);
int add_Fp_Int(int x, int n);
int sub_Fp_Int(int x, int n);
int mul_Fp(int x, int y);
int mul_Fp_Int(int x, int n);
int div_Fp(int x, int y);
int div_Fp_Int(int x, int n);

int parseToFp(int n)
{
  return n * FRACTION;
}

int parseToInt(int x)
{
  return x / FRACTION;
}

int parsetToIntRound(int x)
{
  if (x >= 0)
    return (x + FRACTION / 2) / FRACTION;
  return (x - FRACTION / 2) / FRACTION;
}

int add_Fp(int x, int y)
{
  return x + y;
}

int sub_Fp(int x, int y)
{
  return x - y;
}

int add_Fp_Int(int x, int n)
{
  return add_Fp(x, parseToFp(n));
}

int sub_Fp_Int(int x, int n)
{
  return sub_Fp(x, parseToFp(n));
}

int mul_Fp(int x, int y)
{
  // use int64_t to prevent overflow (int 32)
  return ((int64_t)x) * y / FRACTION;
}

int mul_Fp_Int(int x, int n)
{
  return x * n;
}

int div_Fp(int x, int y)
{
  return ((int64_t)x) * FRACTION / y;
}

int div_Fp_Int(int x, int n)
{
  return x / n;
}
