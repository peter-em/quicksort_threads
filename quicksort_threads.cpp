#include <algorithm>
#include <iostream>
#include <vector>
#include "ctpl_stl.h"
#include "timer_milisec.h"

using namespace std;

void sorting_thread(int id, vector<int> &nvector, int left_id, int right_id,
                    int threshold, ctpl::thread_pool &tp);
void do_parallel_sort(vector<int> &numbers_vector, int size,
                      ctpl::thread_pool &tp, int available_threads);
void quick_sort(vector<int> &array, int left_id, int right_id);
void quick_sort_parallel(vector<int> &numbers_vector, int left_id, int right_id,
                         int threshold, ctpl::thread_pool &tp);

class SortDurationTests {

private:
    int array_size;
    int measure_count;
    double calculation_time;
    TimerMiliSec timer;
    vector<int> numbers_vector1;
    vector<int> numbers_vector2;
    vector<int> numbers_vector3;
    
    void randomize_vector(vector<int> &vctr) {
       srand(time(NULL));
       for (int i = 0; i < array_size; i++) {
          vctr[i] = rand() % (2 * array_size) + 1;
       }
    }
    
    bool is_sorted() {
       for (int i = 0; i < array_size - 1; i++) {
          if (numbers_vector1[i] > numbers_vector1[i+1])
             return false;
       }
       return true;
    }

public:
    SortDurationTests(int size, int count) {
       array_size = size;
       measure_count = count;
    }
    
    void perform_tests() {
        cout << "Array size: " << array_size << ", numbers range 1 - " <<
                                                                         (2 * array_size) << endl;
        cout << "\nSorting time tests begin." << endl;
        
        
        numbers_vector1.resize(array_size);
        int sort_time = 0;
        // using alternative implementation of quicksort algorithm
        cout << measure_count << " iterations of alternative quicksort algorithm" << endl;
        for (int i = 0; i < measure_count; i++) {
            cout << (i + 1) << " ";
            randomize_vector(numbers_vector1);
            timer.reset();
            quick_sort(numbers_vector1, 0, array_size - 1);
            calculation_time = timer.elapsed();
            if (!SortDurationTests::is_sorted()) {
                cerr << "ERROR: array is not sorted!" << endl;
                continue;
            }
            sort_time += calculation_time;
        }
        cout << "\nAverage sorting time: " << (sort_time / measure_count) << endl;
        cout << endl;
        
        numbers_vector1.clear();
        numbers_vector1.resize(0);
        
        numbers_vector2.resize(array_size);
        sort_time = 0;
        //using internal implementation of sorting function
        cout << measure_count << " iterations of internal sort() function" << endl;
        for (int i = 0; i < measure_count; i++) {
            cout << (i + 1) << " ";
            randomize_vector(numbers_vector2);
            timer.reset();
            sort(numbers_vector2.begin(), numbers_vector2.end());
            calculation_time = timer.elapsed();
            if (!std::is_sorted(numbers_vector2.begin(), numbers_vector2.end()
            )) {
                cerr << "ERROR: array is not sorted!" << endl;
                continue;
            }
            sort_time += calculation_time;
        }
        cout << "\nAverage sorting time: " << (sort_time / measure_count) << endl;
        cout << endl;
        
        numbers_vector2.clear();
        numbers_vector2.resize(0);
        
        numbers_vector3.resize(array_size);
        sort_time = 0;
        //using implementation of parallel sorting algorithm
        cout << measure_count << " iterations of parallel quicksort algorithm" << endl;
        unsigned int system_threads = thread::hardware_concurrency();
        ctpl::thread_pool thread_pool(system_threads);
        for (int i = 0; i < measure_count; i++) {
            cout << (i + 1) << " ";
            randomize_vector(numbers_vector3);
            timer.reset();
            do_parallel_sort(numbers_vector3, array_size, thread_pool, system_threads);
            //quick_sort(numbers_vector3, 0, array_size - 1);
            calculation_time = timer.elapsed();
            if (!std::is_sorted(numbers_vector3.begin(), numbers_vector3.end()
            )) {
                cerr << "ERROR: array is not sorted!" << endl;
                continue;
            }
            sort_time += calculation_time;
        }
        cout << "\nAverage sorting time: " << (sort_time / measure_count) << endl;
        cout << endl;
    }
   
};

int main() {
    int size = 20000000;
    int test_count = 16;
    SortDurationTests tests(size, test_count);
    tests.perform_tests();
    return 0;
}

void quick_sort(vector<int> &array, int left_id, int right_id) {
    if ((right_id - left_id) > 0) {
        int tmp;
        int pivot = array[left_id];
        int left = left_id;
        int rigth = right_id;
        
        while (left <= rigth) {
            while (array[left] < pivot)
                left++;
            while (array[rigth] > pivot)
                rigth--;
            if (left <= rigth) {
                tmp = array[left];
                array[left] = array[rigth];
                array[rigth] = tmp;
                left++;
                rigth--;
            }
        }
    
        quick_sort(array, left_id, rigth);
        quick_sort(array, left, right_id);
    }
}

void quick_sort_parallel(vector<int> &numbers_vector, int left_id, int right_id,
                         int threshold, ctpl::thread_pool &tp) {
   
    int tmp;
    int pivot = numbers_vector[left_id];
    int left = left_id;
    int right = right_id;
    
    while (left <= right) {
        while (numbers_vector[left] < pivot)
            left++;
        while (numbers_vector[right] > pivot)
            right--;
        if (left <= right) {
            tmp = numbers_vector[left];
            numbers_vector[left] = numbers_vector[right];
            numbers_vector[right] = tmp;
            left++;
            right--;
        }
    }
    
    if (right > left_id) {
        if ((right - left_id) > threshold) {
            tp.push(sorting_thread, ref(numbers_vector), left_id, right,
                            threshold, ref(tp));
        } else {
            quick_sort(numbers_vector, left_id, right);
        }
    }
    
    if (left < right_id) {
        if ((right_id - left) > threshold) {
            quick_sort_parallel(numbers_vector, left, right_id, threshold,
                                tp);
        } else {
            quick_sort(numbers_vector, left, right_id);
        }
    }
}

void sorting_thread(int id, vector<int> &nvector, int left_id, int right_id,
                    int threshold, ctpl::thread_pool &tp) {
    
    quick_sort_parallel(nvector, left_id, right_id, threshold, tp);
}

void do_parallel_sort(vector<int> &numbers_vector, int size,
                      ctpl::thread_pool &tp, int available_threads) {
   
    int minimum_size = 160000;
    if ((size - 80000) > minimum_size) {
        int threshold = (int) (size / 8 * 0.1);
        tp.push(sorting_thread, ref(numbers_vector), 0, (size - 1),
                        threshold, ref(tp));
    } else {
        quick_sort(numbers_vector, 0, (size - 1));
    }
    
    bool run = true;
    while (run) {
        this_thread::sleep_for(chrono::microseconds(80));
        if (tp.n_idle() == available_threads) {
            this_thread::sleep_for(chrono::microseconds(1000));
            if (tp.n_idle() == available_threads) {
                run = false;
            }
        }
    }
}

