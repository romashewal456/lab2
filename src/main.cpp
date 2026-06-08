#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <fstream>
#include <sstream>
#include "httplib.h" 

// ==========================================
// ПАТЕРНИ: СТРАТЕГІЯ + ШАБЛОННИЙ МЕТОД
// ==========================================
class SortStrategy {
public:
    virtual ~SortStrategy() = default;

    std::pair<std::vector<int>, long long> sort(const std::vector<int>& arr) {
        std::vector<int> data = arr;
        
        auto start = std::chrono::high_resolution_clock::now();
        doSort(data); 
        auto end = std::chrono::high_resolution_clock::now();
        
        long long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        return {data, duration};
    }

protected:
    virtual void doSort(std::vector<int>& arr) = 0; 
};

class BubbleSort : public SortStrategy {
protected:
    void doSort(std::vector<int>& arr) override {
        int n = arr.size();
        for (int i = 0; i < n - 1; i++) {
            for (int j = 0; j < n - i - 1; j++) {
                if (arr[j] > arr[j + 1]) {
                    std::swap(arr[j], arr[j + 1]);
                }
            }
        }
    }
};

class SelectionSort : public SortStrategy {
protected:
    void doSort(std::vector<int>& arr) override {
        int n = arr.size();
        for (int i = 0; i < n - 1; i++) {
            int min_idx = i;
            for (int j = i + 1; j < n; j++) {
                if (arr[j] < arr[min_idx]) min_idx = j;
            }
            std::swap(arr[i], arr[min_idx]);
        }
    }
};

// ==========================================
// ПАТЕРН: ДЕКОРАТОР
// ==========================================
class ProfilerDecorator : public SortStrategy {
private:
    std::shared_ptr<SortStrategy> wrappedStrategy;
public:
    ProfilerDecorator(std::shared_ptr<SortStrategy> strategy) : wrappedStrategy(strategy) {}

    void doSort(std::vector<int>& arr) override {
        std::cout << "[LOG]: Запуск логування через Декоратор...\n";
       wrappedStrategy->sort(arr);
        std::cout << "[LOG]: Декоратор зафіксував успішне завершення.\n";
    }
};

// ==========================================
// ПАТЕРН: ФАБРИКА
// ==========================================
class SortFactory {
public:
    static std::shared_ptr<SortStrategy> createStrategy(const std::string& type) {
        if (type == "bubble") {
            return std::make_shared<ProfilerDecorator>(std::make_shared<BubbleSort>());
        } else if (type == "selection") {
            return std::make_shared<ProfilerDecorator>(std::make_shared<SelectionSort>());
        }
        return nullptr;
    }
};

// ==========================================
// ПАТЕРН: ІТЄРАТОР
// ==========================================
std::string vectorToJson(std::vector<int>& arr) {
    std::stringstream ss;
    ss << "[";
    for (auto it = arr.begin(); it != arr.end(); ++it) {
        ss << *it;
        if (it + 1 != arr.end()) ss << ",";
    }
    ss << "]";
    return ss.str();
}

// ==========================================
// ВЕБ-СЕРВЕР
// ==========================================
int main() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream file("web/index.html"); 
        std::stringstream buffer;
        buffer << file.rdbuf();
        res.set_content(buffer.str(), "text/html; charset=utf-8");
    });

    svr.Get("/api/sort", [](const httplib::Request& req, httplib::Response& res) {
        std::string algo = req.has_param("algo") ? req.get_param_value("algo") : "bubble";
        
        std::vector<int> raw_data = {88, 45, 12, 89, 7, 54, 23, 65, 31, 90, 11, 3, 99, 76, 54, 32, 12, 8, 67, 43, 21, 9, 80, 50, 35, 17, 2, 91, 73, 61};
        std::vector<int> original_copy = raw_data; 
        
        auto strategy = SortFactory::createStrategy(algo);
        
        if (!strategy) {
            res.set_content("{\"error\":\"Unknown algorithm\"}", "application/json");
            return;
        }

        // Запуск сортування
        auto result = strategy->sort(raw_data);
        std::vector<int> sorted_data = result.first;
        long long time_taken = result.second;

        // Формуємо JSON відповідь
        std::stringstream json;
        json << "{\n"
             << "  \"original\": " << vectorToJson(original_copy) << ",\n"
             << "  \"sorted\": " << vectorToJson(sorted_data) << ",\n"
             << "  \"time\": " << time_taken << "\n"
             << "}";

        // вказуємо тип контенту application/json
        res.set_content(json.str(), "application/json; charset=utf-8");
    });

    std::cout << "Сервер запущено! Адреса: http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}