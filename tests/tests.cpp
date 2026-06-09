#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <cassert>

class SortStrategy {
public:
    virtual ~SortStrategy() = default;
    
    // Шаблонний метод
    std::pair<std::vector<int>, long long> sort(const std::vector<int>& arr) {
        std::vector<int> data = arr;
        auto start = std::chrono::high_resolution_clock::now();
        doSort(data); 
        auto end = std::chrono::high_resolution_clock::now();
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        return {data, duration};
    }

    // Робимо public для декоратора в рамках тесту
    virtual void doSort(std::vector<int>& arr) = 0; 
};

class BubbleSort : public SortStrategy {
public:
    void doSort(std::vector<int>& arr) override {
        int n = arr.size();
        for (int i = 0; i < n - 1; i++) {
            for (int j = 0; j < n - i - 1; j++) {
                if (arr[j] > arr[j + 1]) std::swap(arr[j], arr[j + 1]);
            }
        }
    }
};
class SelectionSort : public SortStrategy {
public:
    void doSort(std::vector<int>& arr) override {
        int n = arr.size();
        for (int i = 0; i < n - 1; i++) {
            int min_idx = i;
            for (int j = i + 1; j < n; j++) { // <--- ТУТ ТЕПЕР ЧИСТИЙ INT!
                if (arr[j] < arr[min_idx]) min_idx = j;
            }
            std::swap(arr[i], arr[min_idx]);
        }
    }
};

// Декоратор тепер без проблем викликає doSort
class ProfilerDecorator : public SortStrategy {
private:
    std::shared_ptr<SortStrategy> wrappedStrategy;
public:
    ProfilerDecorator(std::shared_ptr<SortStrategy> strategy) : wrappedStrategy(strategy) {}
    
    void doSort(std::vector<int>& arr) override {
        wrappedStrategy->doSort(arr); // Викликає чистий алгоритм без рекурсії шаблонного методу
    }
};

class SortFactory {
public:
    static std::shared_ptr<SortStrategy> createStrategy(const std::string& type) {
        if (type == "bubble") return std::make_shared<ProfilerDecorator>(std::make_shared<BubbleSort>());
        if (type == "selection") return std::make_shared<ProfilerDecorator>(std::make_shared<SelectionSort>());
        return nullptr;
    }
};


// ЮНІТ ТЕСТИ

void testBubbleSort() {
    auto strategy = SortFactory::createStrategy("bubble");
    std::vector<int> data = {5, 3, 8, 1};
    
    // Запускаємо через загальний метод сортування
    auto result = strategy->sort(data);
    
    std::vector<int> expected = {1, 3, 5, 8};
    assert(result.first == expected);
    std::cout << "[TEST PASSED]: BubbleSort через Фабрику та Декоратор працює коректно!\n";
}

void testSelectionSort() {
    auto strategy = SortFactory::createStrategy("selection");
    std::vector<int> data = {10, -2, 4, 0};
    
    // Запускаємо через загальний метод сортування
    auto result = strategy->sort(data);
    
    std::vector<int> expected = {-2, 0, 4, 10};
    assert(result.first == expected);
    std::cout << "[TEST PASSED]: SelectionSort через Фабрику та Декоратор працює коректно!\n";
}

int main() {
    std::cout << "===============================================\n";
    std::cout << "  ЗАПУСК ЮНІТ-ТЕСТІВ ДЛЯ ЛАБОРАТОРНОЇ РОБОТИ №2\n";
    std::cout << "===============================================\n";
    
    testBubbleSort();
    testSelectionSort();
    
    std::cout << "\n>>> Всі тести для архітектури патернів успішно пройдено! <<<\n";
    return 0;
}