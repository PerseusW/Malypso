#include "algorithm.h"

Algorithm::Algorithm()
{
    QRandomGenerator *randomGenerator = QRandomGenerator::global();
    uint array[10];
    for (int i = 0; i < 10; ++i) {
        array[i] = randomGenerator->generate() % 10;
    }
    qDebug() << array;
}

int* Algorithm::SelectiveSort(int *array, int size)
{
    for (int toSort = 0; toSort < size - 1; ++toSort) {
        int min = toSort;
        for (int i = toSort + 1; i < size; ++i) {
            if (array[i] < array[min]) {
                min = i;
            }
        }
        if (min != toSort) {
            int tmp = array[min];
            array[min] = array[toSort];
            array[toSort] = tmp;
        }
    }
    return array;
}
