
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <pthread.h>
#include <cstdlib>
#include <ctime>

//using namespace std;

//#define MAX_N 10000
//#define MAX_M 10000
//#define MAX_T 10
//#define MAX_VAL 100
size_t const MAX_N(10'000);
size_t const MAX_M(10'000);
size_t const MAX_T(10);
size_t const MAX_VAL(100);

int matrix[MAX_N * MAX_N];
pthread_mutex_t locks[MAX_N];
pthread_mutex_t k_lock;
pthread_t threads[MAX_T];
//int current_k, k, t_num, n, m;
int current_k;
int k_iter;
int t_num;
int n_rows;
int m_cols;

//typedef struct arg_struct {
//  int min;
//  int max;
//} thread_args;
struct thread_args {
  int min;
  int max;
  size_t tid;
};

void * worker(void * args) {
//  int min = ((thread_args *)args)->min;
//  int max = ((thread_args *)args)->max;
  thread_args * targs = static_cast<thread_args *>(args);
  {
    std::ostringstream msg;
    msg << "\n Working thread "
        << std::setw(5) << targs->tid
        << ' '
        << targs
        << " min "
        << std::setw(10) << targs->min
        << " max "
        << std::setw(10) << targs->max
        << std::ends;
    std::cout << msg.str() << std::endl;
  }
  int min = targs->min;
  int max = targs->max;
  size_t tid = targs->tid;
  delete targs;

  for (size_t i = min; i < max; i++) {
    for (size_t j = 1; j < m_cols - 2; j++) {
      if (current_k < k_iter) {
        pthread_mutex_lock(&locks[i]);
        matrix[n_rows * i + j] = (matrix[(i - 1) * n_rows + j] +
                             matrix[(i - 1) * n_rows + j] +
                             matrix[i * n_rows + j + 1] +
                             matrix[i * n_rows + j - 1]);
        pthread_mutex_unlock(&locks[i]);
        pthread_mutex_lock(&k_lock);
        current_k++;
        pthread_mutex_unlock(&k_lock);
      }
      else {
//        std::cout << "Iteratiile s-au terminat!" << std::endl;
        std::ostringstream msg; 
        msg << "\nThread ID" << std::setw(5)
            << tid << ": "
            << "The iterations are over!"
            << std::ends;
        std::cout << msg.str() << std::endl;
        pthread_exit(0);
      }
    }
  }
  pthread_exit(0);
}

void init_matrix() {
  srand(time(NULL));
  n_rows = rand() % MAX_N + 10;
  m_cols = rand() % MAX_M + 10;
//  std::cout << "Generat : N = " << n << " si M = " << m << std::endl;
  std::cout << "Generated : N = "
            << n_rows
            << " and M = "
            << m_cols
            << std::endl;
  for (size_t i = 0; i < n_rows; i++) {
    for (size_t j = 0; j < m_cols; j++) {
      if (i == n_rows || i == 0 || j == m_cols || j == 0) {
        matrix[n_rows * i + j] = rand() % MAX_VAL + 1;
      }
      else {
        matrix[n_rows * i + j] = 0;
      }
    }
  }
//  std::cout << "Matrice generata cu " << n << " randuri si " << m << " coloane" << std::endl;
  std::cout << "Matrix generated with "
            << n_rows
            << " rows and "
            << m_cols
            << " columns"
            << std::endl;
}

int main(int argc, char const * argv[]) {
  init_matrix();

  // init locks threaduri
  for (size_t i = 0; i < n_rows; i++) {
    pthread_mutex_init(&locks[i], 0);
  }

  current_k = 0;
//  std::cout << "Introdu numar threaduri: "; std::cin >> t_num;
  std::cout << "Enter number of threads: ";
  std::cout.flush();
  std::cin >> t_num;
  t_num = t_num <= MAX_T ? t_num : MAX_T;
//  std::cout << "Introdu numar iteratii: "; std::cin >> k;
  std::cout << "Enter the number of iterations: ";
  std::cout.flush();
  std::cin >> k_iter;

  int bucket = n_rows / t_num;
  for (size_t i = 0; i < t_num; i++) {
    // setare min / max pentru randuri
    thread_args * args = new thread_args;
    if (i == 0) {
      args->min = 1;
      args->max = bucket;
    }
    else if (i == (t_num - 1)) {
      args->min = bucket * i + 1;
      args->max = n_rows;
    }
    else {
      args->min = bucket * i + 1;
      args->max = bucket * (i + 1);
    }

    args->tid = i;
    std::ostringstream msg;
    msg << "Creating thread "
        << std::setw(5)
        << args->tid
        << ' '
        << args
        << " min "
        << std::setw(10)
        << args->min
        << " max "
        << std::setw(10)
        << args->max
        << std::ends;
    std::cout << msg.str() << std::endl;

    int ret = pthread_create(
      &threads[i], NULL, worker, static_cast<void *>(args));
    if(ret != 0) {
//      perror("pthread_create a esuat\n");
      perror("pthread_create failed\n");
      exit(EXIT_FAILURE);
    }
  }

  for(size_t i = 0; i < t_num; i++) {
    pthread_join(threads[i], NULL);
  }

  // destroy locks threaduri
  for (size_t i = 0; i < n_rows; i++) {
    pthread_mutex_destroy(&locks[i]);
  }

  return 0;
}
