// Grzegorz B. Zaleski (418494)
// Wykonano serię testów z użyciem valgrinda  

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h> 
#include <stdbool.h>

const int BUFFER_SIZE = 256;
const int TOLERANCE = 15;

int current_timestamp()
{
  return time(NULL);
}

static struct pam_conv login_conv =
{
  misc_conv,
  NULL                     
};

void display_customers_list()
{
  FILE *fp;
  fp = fopen("uzytkownicy.txt", "r");
  if (fp == NULL)
  {
    fprintf(stderr, "File reading error!\n");
    exit(4);
  }

  char line[BUFFER_SIZE];
  while(fgets(line, sizeof(line), fp) != NULL)
  {
    char *last, *token = strtok(line, " ");
    while (token != NULL)
    {
        if (strcmp(token, "client") == 0)
        {
          printf("%s", last);
          token = strtok(NULL, " ");
          while (token != NULL)
          {
            printf(" %s", token);
            token = strtok(NULL, " ");
          }
        }
        else
        {
          last = token;
          token = strtok(NULL, " ");
        }
    }
  }

  fclose(fp);
}

// Wymagane w zadaniu jest użycie struktur zamiast po prostu przeczytania pliku.
// Pierwsze 5 linii
typedef struct header
{
  char *name;
  size_t name_len; 

  int number;
  int sum; // *100 dla wygodne zapisu w incie.
  int date; // yyyy * 10^4 + mm * 10^2 + dd  dla wygodnego zapisu.
  int percentage; // *100 dla wygodnego zapisu jak suma.
} header_t;

// Kolejne informacja są podawane parami z data (lub koniec - sama data)
const char SUM = 's'; // Suma
const char PERCENTAGE = 'p'; // Procent
const char CLOSED = 'c'; // Zakończenie depozytu/kredytu.
typedef struct data
{
  char type;
  int date;
  int val;
} data_t;

// Dla numeru, sum i procenta - usuwa niecyfry
int simple_intparse(char *str)
{
  int res = 0;

  for (size_t i = 0; i < strlen(str); ++i)
    if (isdigit(str[i]))
      res = res * 10 + str[i] - '0';

  return res;
}

// Sprawdzanie czy string zawiera dany znak.
bool contains_char(char *str, char key)
{
  for (size_t i = 0; i < strlen(str); ++i)
    if (str[i] == key)
      return true;

  return false;
}

// Dla daty
int date_parse(char *str)
{
  int i = 0;
  while (!isdigit(str[i]))
    i++;

  int dd = (str[i] - '0') * 10 + str[i+1] - '0';
  i += 3;
  int mm = (str[i] - '0') * 10 + str[i+1] - '0';
  i += 3;
  int yy = (str[i] - '0') * 1000 + (str[i+1] - '0') * 100 + (str[i+2] - '0') * 10 + str[i+1] - '0';
 
  int parsed = yy * 10000 + mm * 100 + dd;
  return parsed;
}

// Wypisywanie daty z w/w systemu zapisu:
void print_date(int date)
{
  printf("Date: %s%d%c%s%d%c%d\n", 
    date % 100 > 9 ? "" : "0", date % 100, '.', 
    (date / 100) % 100 > 9 ? "" : "0", (date / 100) % 100, '.',
    date / 10000);
}

// Tutaj nastąpi (dosyć zbędne ale wymagana w zadaniu) przepisanie pliku na struktury a
//   następne wyprintowanie informacji ze struktur.
void load_and_print_file(char *filename)
{
  printf("Content of %s\n:", filename);
  FILE * fp;
  char* line = NULL;
  size_t len = 0;
  ssize_t read = 0;

  fp = fopen(filename, "r");

  // Wczytanie headera
  // Name
  header_t head;
  read = getline(&line, &len, fp);
  size_t head_len = len - 6;
  head.name = malloc(head_len * sizeof(char));
  if (head.name == NULL)
  {
    fprintf(stderr, "Malloc failure");
    exit(1);
  }

  for (int i = 0; i < head_len; ++i)
    head.name[i] = line[i + 6];

  // Number
  read = getline(&line, &len, fp);
  head.number = simple_intparse(line);

  // Sum
  read = getline(&line, &len, fp);
  head.sum = simple_intparse(line);
  head.sum = contains_char(line, ',') ? head.sum : head.sum * 100;

  // Date 
  read = getline(&line, &len, fp);
  head.date = date_parse(line);

  // Procent
  read = getline(&line, &len, fp);
  head.percentage = simple_intparse(line);
  head.percentage = contains_char(line, ',') ? head.percentage : head.percentage * 100;

  // Na początku mała tablica, której rozmiar będzie podwajany jak się zapełni -> amortyzowany koszt stały.
  data_t *input = malloc(BUFFER_SIZE * sizeof(data_t));
  int max_size = BUFFER_SIZE;
  int current_size = 0;

  if (input == NULL)
  {
    fprintf(stderr, "Malloc failure");
    exit(1);
  }

  // Wczytanie reszty
  while ((read = getline(&line, &len, fp)) != -1) 
  {
    if (current_size == max_size)
    {
      max_size *= 2;
      input = realloc(input, max_size * sizeof(data_t));
      if (input == NULL)
      {
        fprintf(stderr, "Malloc failure");
        exit(1);
      }
    }

    input[current_size].date = date_parse(line);

    if ((read = getline(&line, &len, fp)) != -1)
    {
      input[current_size].type = tolower(line[0]);
      input[current_size].val = simple_intparse(line);
      input[current_size].val = contains_char(line, ',') ? input[current_size].val : input[current_size].val * 100;
    }

    current_size++;
    if (read == -1)
    {
      input[current_size - 1].type = CLOSED;
      break;
    }
  } 

  // Wypisanie ma być w odwrotnej kolejności:
  printf("Name: %s", head.name);
  printf("Number: %d\n", head.number);

  for (int i = current_size - 1; i >= 0; --i)
  {
    print_date(input[i].date);
    if (input[i].type == PERCENTAGE)
    {
      printf("Procent: ");
    }
    else if (input[i].type == SUM)
    {
      printf("Sum: ");
    }
    if (input[i].type != CLOSED)
    {
      printf("%d", input[i].val / 100);
      if (input[i].val % 100)
        printf(",%d", input[i].val % 100);
    }
    printf("\n");
  }

  print_date(head.date);

  // Czyszczenie pamięci
  free(input);
  free(head.name);
  fclose(fp);
}

void display_customer_information(char *customer, size_t len)
{
  char filename[BUFFER_SIZE];
  printf("\nDeposits of %s\n", customer);
  int ind = 0;
  sprintf(filename, "deposits/%s-%d.txt", customer, ind);

  while (access(filename, F_OK) == 0)
  {
    load_and_print_file(filename);
    sprintf(filename, "deposits/%s-%d.txt", customer, ++ind);
  }

  ind = 0;
  printf("\nCredits of %s\n", customer);
  sprintf(filename, "credits/%s-%d.txt", customer, ind);

  while (access(filename, F_OK) == 0)
  {
    load_and_print_file(filename);
    sprintf(filename, "credits/%s-%d.txt", customer, ++ind);
  }
}

bool is_date(char *str)
{
  for (int i = 0; i < strlen(str) - 1; ++i)
    if (i != 2 && i != 5)
      if (!isdigit(str[i]))
          return false;

  return str[2] == '.' && str[5] == '.';
}

void add_new_credit(char *customer, size_t len, bool credit_not_deposit) // Dodatkowa flaga bo funkcje są prawie identyczne
{
  printf("For %s\n", customer);

  FILE *fp;
  fp = fopen("uzytkownicy.txt", "r");
  if (fp == NULL)
  {
    fprintf(stderr, "File reading error!\n");
    exit(4);
  }

  bool found_user = false;
  char *name = NULL;
  char line[BUFFER_SIZE];
  while(fgets(line, sizeof(line), fp) != NULL)
  {
    if (strstr(line, customer) != NULL && line[len - 1] == ' ') 
    {
      found_user = true;
      name = malloc(strlen(line) * sizeof(char));
      int i = 0, j = 0;
      while (line[i] != ' ')
        i++;
      i++;
      while (line[i] != ' ')
        i++;
      i++;

      while (i < strlen(line))
      {
        name[j++] = line[i++];
      }

      break;
    } 
  }
  fclose(fp);

  if (found_user == false)
  {
    printf("User not found!\n");
    return;
  }

  char filename[BUFFER_SIZE];
  int ind = 0;
  sprintf(filename, credit_not_deposit ? "credits/%s-%d.txt" : "deposits/%s-%d.txt", customer, ind);

  while (access(filename, F_OK) == 0)
  {
    sprintf(filename, credit_not_deposit ? "credits/%s-%d.txt" : "deposits/%s-%d.txt", customer, ++ind);
  }
  // Nowy plik

  fp = fopen(filename, "w");
  fprintf(fp, "Name: %s", name);

  size_t mem;
  char *kline = NULL;
  int klen;

  printf("Insert number: ");
  getline(&kline, &mem, stdin);
  fprintf(fp, "Number: %s", kline);

  printf("Insert sum: ");
  getline(&kline, &mem, stdin);
  fprintf(fp, "Sum: %s", kline);

  do
  {
    printf("Insert date (dd.mm.yyyy format): ");
    klen = getline(&kline, &mem, stdin);
  } while (klen != 11 || !is_date(kline));

  fprintf(fp, "Date: %s", kline);

  printf("Insert percent: ");
  getline(&kline, &mem, stdin);
  fprintf(fp, "Procent: %s", kline);

  printf("Sucessfully added!\n");
  fclose(fp);
  free(kline);
  free(name);
}

void modify_credit(char *customer, size_t len, bool credit_not_deposit) // Dodatkowa flaga bo funkcje są prawie identyczne
{
  size_t mem;
  char *kline = NULL;
  int klen, read;

  printf("%s index to modify: ", credit_not_deposit ? "Credit" : "Deposit");
  getline(&kline, &mem, stdin);
  int index_to_modify = simple_intparse(kline);

  char filename[BUFFER_SIZE];
  sprintf(filename, credit_not_deposit ? "credits/%s-%d.txt" : "deposits/%s-%d.txt", customer, index_to_modify);
  if (access(filename, F_OK) != 0)
  {
    printf("Document does not exist!\n");
    free(kline);
    return;
  }
  
  // Znalezienie ostatniej daty do porównania.
  FILE *fp;
  fp = fopen(filename, "r");

  bool closed_document = false;
  int last_date;
  while ((read = getline(&kline, &len, fp)) != -1) 
  {
    if (strstr(kline, "Date") != NULL) // Wiersze z datą
    {
      last_date = date_parse(kline);
      closed_document = true;
    }
    else
    {
      closed_document = false;
    }
  }

  if (closed_document)
  {
    printf("%s was closed - cannot modify.\n", credit_not_deposit ? "Credit" : "Deposit");
  }
  else
  {
    printf("For closing - press C\nFor new Sum & Percent - S\nFor just new Percent - P\n");
    getline(&kline, &mem, stdin);
    char key = toupper(kline[0]);

    if (key != 'C' && key != 'S' && key != 'P')
    {
      printf("Wrong command!\n");
    }
    else 
    {
      printf("Make sure date is after ");
      print_date(last_date);
      do
      {
        printf("Insert date (dd.mm.yyyy format): ");
        klen = getline(&kline, &mem, stdin);
      } while (klen != 11 || !is_date(kline) || date_parse(kline) < last_date);

      char date_save[11];
      for (int i = 0; i < 11; ++i)
        date_save[i] = kline[i];
      
      fclose(fp);
      fp = fopen(filename, "a+");
      fprintf(fp, "Date: %s", kline);

      if (key == 'S') // Wczytaniu nowej sumy i nowego procentu.
      {
        // Nowa suma.
        printf("Insert sum: ");
        getline(&kline, &mem, stdin);
        fprintf(fp, "Sum: %s", kline);

        // Data.
        fprintf(fp, "Date: %s", date_save);

        // Procent.
        printf("Insert percent: ");
        getline(&kline, &mem, stdin);
        fprintf(fp, "Procent: %s", kline);
      }
      else if (key == 'P') // Wczytanie tylko procentu.
      {
        printf("Insert percent: ");
        getline(&kline, &mem, stdin);
        fprintf(fp, "Procent: %s", kline);
      }
    }
  }

  fclose(fp);
  free(kline);
}

int main()
{
  int read;
  char *line = NULL;
  size_t mem;
  
  printf("Provide current UNIX time for verification: ");
  getline(&line, &mem, stdin); // Bezpieczniejszy system.
  int user_input = simple_intparse(line);

  if (abs(current_timestamp() - user_input) >= TOLERANCE)
  {
    fprintf(stderr, "Time verification failed!\n");
    return 0;
  }

  pam_handle_t* pamh = NULL;
  int retval;
  char *username = NULL;

  retval = pam_start("login", username, &login_conv, &pamh);
  if (pamh == NULL || retval != PAM_SUCCESS) 
  {
    fprintf(stderr, "Error at PAM system: %d\n", retval);
    exit(1);
  }

  retval = pam_authenticate(pamh, 0);
  if (retval != PAM_SUCCESS) 
  {
    fprintf(stderr, "Failed to authenticate!\n");
    exit(2);
  }
  pam_end(pamh, PAM_SUCCESS);
  // UWAGA: Za pomocą valgrinda dokonałem analizy że tutaj następują wyciek (still reachable error) z funkcji bibliotecznych.

  printf("Welcome to bank system!\n");
  printf("Instructions available:\n");
  printf("0 - Exit system\n");
  printf("1 - Customers list\n");
  printf("2 - Choose customer (username) for operations\n");
  printf("3 - Display credits and deposits for a customer\n");
  printf("4 - Add new credit or deposit for a customer\n");
  printf("5 - Edit the credit or deposit for a customer\n");

  int code = -1;
  char *set_customer = NULL;
  size_t set_customer_len = 0;
  do 
  {
    printf("Insert instruction number: ");

    // Zastąpiono prosty scan na wczytanie w ten sposób które jest mniej podatne na błedny input.
    read = getline(&line, &mem, stdin);

    if (read == 2) // Liczba + koniec linii.use
    {
      if (line[0] == '0') // Wyjscie.
      {
        code = 0;
      }
      else if (line[0] == '1') // Wypisanie klientów.
      {
        display_customers_list();
      }
      else if (line[0] == '2') // Ustawienie klienta do edycji.
      {
        printf("Input username to set customer for further operations: ");
        set_customer_len = getline(&set_customer, &mem, stdin);
        set_customer[set_customer_len - 1] = '\0'; // Poprawienie końca username'a z \n na koniec stringa.
        printf("Set customer %s.\n", set_customer);
      }
      else if (line[0] == '3') // Wypisanie kredytów i depozytów tego klienta.
      {
        if (set_customer_len == 0 || set_customer == NULL)
        {
          printf("Input username to set customer for further operations: ");
          set_customer_len = getline(&set_customer, &mem, stdin);
          set_customer[set_customer_len - 1] = '\0'; // Poprawienie końca username'a z \n na koniec stringa.
        }
        else
        {
          printf("Input username to set customer for further operations (enter for %s): ", set_customer);

          char *set_customer_aux = NULL;
          size_t set_customer_aux_len = 0;
          set_customer_aux_len = getline(&set_customer_aux, &mem, stdin);
          set_customer_aux[set_customer_aux_len - 1] = '\0'; // Poprawienie końca username'a z \n na koniec stringa.

          if (set_customer_aux_len > 1)
          {
            free(set_customer);
            set_customer = set_customer_aux;
            set_customer_len = set_customer_aux_len;
          }
        }
        display_customer_information(set_customer, set_customer_len);
      }
      else if (line[0] == '4') // Dodanie nowego kredytu lub depozytu.
      {
        if (set_customer_len == 0 || set_customer == NULL)
        {
          printf("Input username to set customer for further operations: ");
          set_customer_len = getline(&set_customer, &mem, stdin);
          set_customer[set_customer_len - 1] = '\0'; // Poprawienie końca username'a z \n na koniec stringa.
        }
        else
        {
          printf("Input username to set customer for further operations (enter for %s): ", set_customer);

          char *set_customer_aux = NULL;
          size_t set_customer_aux_len = 0;
          set_customer_aux_len = getline(&set_customer_aux, &mem, stdin);
          set_customer_aux[set_customer_aux_len - 1] = '\0'; // Poprawienie końca username'a z \n na koniec stringa.

          if (set_customer_aux_len > 1)
          {
            free(set_customer);
            set_customer = set_customer_aux;
            set_customer_len = set_customer_aux_len;
          }
        }
        
        printf("For new credit - C, for new deposit - D: ");
        getline(&line, &mem, stdin);

        if (toupper(line[0]) == 'C')
        {
          add_new_credit(set_customer, set_customer_len, true);
        }
        else if (toupper(line[0]) == 'D')
        {
          add_new_credit(set_customer, set_customer_len, false);
        }
        else
        {
          printf("Wrong command!\n");
        }
      }
      else if (line[0] == '5') // Modyfikacja kredytu lub depozytu.
      {
        if (set_customer_len == 0 || set_customer == NULL)
        {
          printf("Input username to set customer for further operations: ");
          set_customer_len = getline(&set_customer, &mem, stdin);
          set_customer[set_customer_len - 1] = '\0'; // Poprawienie końca username'a z \n na koniec stringa.
        }
        else
        {
          printf("Input username to set customer for further operations (enter for %s): ", set_customer);

          char *set_customer_aux = NULL;
          size_t set_customer_aux_len = 0;
          set_customer_aux_len = getline(&set_customer_aux, &mem, stdin);
          set_customer_aux[set_customer_aux_len - 1] = '\0'; // Poprawienie końca username'a z \n na koniec stringa.

          if (set_customer_aux_len > 1)
          {
            free(set_customer);
            set_customer = set_customer_aux;
            set_customer_len = set_customer_aux_len;
          }
        }
        
        printf("For new credit - C, for new deposit - D: ");
        getline(&line, &mem, stdin);

        if (toupper(line[0]) == 'C')
        {
          modify_credit(set_customer, set_customer_len, true);
        }
        else if (toupper(line[0]) == 'D')
        {
          modify_credit(set_customer, set_customer_len, false);
        }
        else
        {
          printf("Wrong command!\n");
        }
      }
    }
    else
    {
      printf("Incorrect command!\n");
    }

  } while (code != 0);
  free(line);

  printf("Exiting system.\n");

  if (username != NULL)
    free(username);

  if (set_customer_len > 0)
    free(set_customer);

  return 0;
}
