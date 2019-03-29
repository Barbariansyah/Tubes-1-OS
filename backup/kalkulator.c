int mod(int a, int b);
int div(int a, int b);
int isOperator (char c);
int charToint (char c);
int calculator (char *string);
void swap(char a, char b);
void reverse(char str[], int length);
char* itoa(int num, char* str, int base);

int main() {
    char ekspresi[512*20];
    char hasil[512*20];
    int result;

    interrupt(0x21,0x0,"Input dengan format seperti ini <bilangan><operator><bilangan>",0,0);
    interrupt(0x21,0x1,ekspresi,0,0);
    result = calculator(ekspresi);
    itoa(result,hasil,10);
    interrupt(0x21,0x0,hasil,0,0);
}

int mod(int a, int b) {
   while(a >= b) {
      a = a - b;
   }
   return a;
}

int div(int a, int b) {
   int q = 0;
   while(q*b <= a) {
      q = q+1;
   }
   return q-1;
}

int isOperator (char c) {
  if(c == '+' || c == '-' || c == '/' || c == '*' || c == '%') {
    return 1;
  } else {
    return 0;
  }
}

int charToint (char c) {
  int x;
  x = x*10;
  x += c - '0';

  return x;
}

int calculator (char *string) {
  int x,y,res;
  int i = 0;
  char operator = string[i];

  x = 0;
  if(string[i] = '-') {
    i++;
    while(isOperator(string[i]) == 0) {
      x = charToint(string[i]);
      i++;
    }
    x = x*(-1);
  } else {
    while(!isOperator(string[i]) == 0) {
      x = charToint(string[i]);
      i++;
    }
  }

  // untuk melewati operator
  i++;

  y = 0;
  if(string[i] = '-') {
    i++;
    while(string[i] != '\0') {
      y = charToint(string[i]);
      i++;
    }
    y = y*(-1);
  } else {
    while(string[i] != '\0') {
      y = charToint(string[i]);
      i++;
    }
  }

  // Proses perhitungan
  if(operator = '+') {
    res = x+y;
  }
  else if(operator = '-') {
    res = x-y;
  }
  else if(operator = '*') {
    res = x*y;
  }
  else if(operator = '/') {
    res = div(x,y);
  }
  else if(operator = '%') {
    res = mod(x,y);
  }

  return res;
}

void swap(char a, char b){
    a += b;
    b = a - b;
    a = a - b;
}

void reverse(char str[], int length){
    int start = 0;
    int end = length -1;
    while (start < end){
        swap(*(str+start), *(str+end));
        start++;
        end--;
    }
}

// Implementation of itoa()
char* itoa(int num, char* str, int base){
    int i = 0;
    int isNegative = 0;
    int rem;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = 1;
        num = -1*num;
    }

    // Process individual digits
    while (num != 0)
    {
        rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    // If number is negative, append '-'
    if (isNegative == 1)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse(str, i);

    return str;
}