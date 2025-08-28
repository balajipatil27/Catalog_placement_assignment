#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>

class SecretSolver {
private:
    struct DataPoint {
        int x_coord;
        long long y_value;
        
        DataPoint(int x, long long y) : x_coord(x), y_value(y) {}
    };
    
    int total_points;
    int required_points;
    std::vector<DataPoint> coordinate_pairs;
    
public:
    SecretSolver() : total_points(0), required_points(0) {}
    
    // Helper function to convert different base numbers to decimal
    long long baseToDecimal(const std::string& number_str, int base_value) {
        long long decimal_result = 0;
        long long multiplier = 1;
        
        // Work backwards through the string
        for (int idx = number_str.length() - 1; idx >= 0; idx--) {
            char current_char = number_str[idx];
            int digit_value = 0;
            
            // Handle numeric digits
            if (current_char >= '0' && current_char <= '9') {
                digit_value = current_char - '0';
            }
            // Handle lowercase letters
            else if (current_char >= 'a' && current_char <= 'z') {
                digit_value = current_char - 'a' + 10;
            }
            // Handle uppercase letters
            else if (current_char >= 'A' && current_char <= 'Z') {
                digit_value = current_char - 'A' + 10;
            }
            
            // Validate digit is valid for the given base
            if (digit_value >= base_value) {
                std::cout << "Invalid digit '" << current_char << "' for base " << base_value << std::endl;
                return -1;
            }
            
            decimal_result += digit_value * multiplier;
            multiplier *= base_value;
        }
        
        return decimal_result;
    }
    
    // Read and parse the JSON input file
    bool loadDataFromFile(const std::string& file_path) {
        std::ifstream input_file(file_path);
        if (!input_file.is_open()) {
            std::cout << "Cannot open file: " << file_path << std::endl;
            return false;
        }
        
        // Read entire file content
        std::string file_content;
        std::string line;
        while (std::getline(input_file, line)) {
            file_content += line;
        }
        input_file.close();
        
        return parseJsonContent(file_content);
    }
    
    // Simple JSON parser for our specific format
    bool parseJsonContent(const std::string& json_data) {
        // Remove unnecessary whitespace
        std::string clean_json;
        for (char c : json_data) {
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
                clean_json += c;
            }
        }
        
        // Extract n and k values
        if (!extractKeyValues(clean_json)) {
            return false;
        }
        
        // Extract all coordinate points
        extractDataPoints(clean_json);
        
        return !coordinate_pairs.empty();
    }
    
    bool extractKeyValues(const std::string& json_str) {
        size_t keys_position = json_str.find("\"keys\":");
        if (keys_position == std::string::npos) return false;
        
        size_t keys_start = json_str.find("{", keys_position);
        size_t keys_end = json_str.find("}", keys_start);
        
        std::string keys_section = json_str.substr(keys_start + 1, keys_end - keys_start - 1);
        
        // Find n value
        size_t n_pos = keys_section.find("\"n\":");
        if (n_pos != std::string::npos) {
            size_t n_start = n_pos + 4;
            size_t n_end = keys_section.find_first_of(",}", n_start);
            total_points = std::stoi(keys_section.substr(n_start, n_end - n_start));
        }
        
        // Find k value
        size_t k_pos = keys_section.find("\"k\":");
        if (k_pos != std::string::npos) {
            size_t k_start = k_pos + 4;
            size_t k_end = keys_section.find_first_of(",}", k_start);
            required_points = std::stoi(keys_section.substr(k_start, k_end - k_start));
        }
        
        return true;
    }
    
    void extractDataPoints(const std::string& json_str) {
        size_t search_pos = 0;
        
        while (search_pos < json_str.length()) {
            search_pos = json_str.find("\"", search_pos);
            if (search_pos == std::string::npos) break;
            
            size_t key_end = json_str.find("\"", search_pos + 1);
            if (key_end == std::string::npos) break;
            
            std::string key = json_str.substr(search_pos + 1, key_end - search_pos - 1);
            
            // Check if this key represents a coordinate (is numeric)
            if (isNumericKey(key)) {
                int x_value = std::stoi(key);
                
                // Find the data block for this coordinate
                size_t block_start = json_str.find(":{", key_end);
                size_t block_end = json_str.find("}", block_start);
                
                if (block_start != std::string::npos && block_end != std::string::npos) {
                    std::string data_block = json_str.substr(block_start + 2, block_end - block_start - 2);
                    
                    // Extract base and value
                    std::pair<int, std::string> base_and_value = extractBaseAndValue(data_block);
                    int base_val = base_and_value.first;
                    std::string encoded_value = base_and_value.second;
                    
                    if (base_val > 0) {
                        long long y_value = baseToDecimal(encoded_value, base_val);
                        if (y_value != -1) {
                            coordinate_pairs.push_back(DataPoint(x_value, y_value));
                            std::cout << "Decoded point: (" << x_value << ", " << y_value 
                                     << ") from \"" << encoded_value << "\" (base " << base_val << ")" << std::endl;
                        }
                    }
                    search_pos = block_end;
                } else {
                    search_pos = key_end + 1;
                }
            } else {
                search_pos = key_end + 1;
            }
        }
        
        // Sort points by x-coordinate
        std::sort(coordinate_pairs.begin(), coordinate_pairs.end(), 
                 [](const DataPoint& a, const DataPoint& b) {
                     return a.x_coord < b.x_coord;
                 });
    }
    
    bool isNumericKey(const std::string& key) {
        if (key.empty() || key == "keys" || key == "base" || key == "value") return false;
        
        for (char c : key) {
            if (c < '0' || c > '9') return false;
        }
        return true;
    }
    
    std::pair<int, std::string> extractBaseAndValue(const std::string& block) {
        int base_val = 0;
        std::string value_str;
        
        // Find base
        size_t base_start = block.find("\"base\":\"");
        if (base_start != std::string::npos) {
            size_t base_value_start = base_start + 8;
            size_t base_end = block.find("\"", base_value_start);
            base_val = std::stoi(block.substr(base_value_start, base_end - base_value_start));
        }
        
        // Find value
        size_t value_start = block.find("\"value\":\"");
        if (value_start != std::string::npos) {
            size_t value_value_start = value_start + 9;
            size_t value_end = block.find("\"", value_value_start);
            value_str = block.substr(value_value_start, value_end - value_value_start);
        }
        
        return std::make_pair(base_val, value_str);
    }
    
    // Calculate the secret using Lagrange interpolation
    double calculateSecret() {
        if (coordinate_pairs.size() < static_cast<size_t>(required_points)) {
            std::cout << "Warning: Using " << coordinate_pairs.size() << " points instead of " << required_points << std::endl;
            required_points = static_cast<int>(coordinate_pairs.size());
        }
        
        double secret_value = 0.0;
        
        // Apply Lagrange interpolation formula for f(0)
        for (int i = 0; i < required_points; i++) {
            double lagrange_term = static_cast<double>(coordinate_pairs[i].y_value);
            
            // Calculate the Lagrange basis polynomial L_i(0)
            for (int j = 0; j < required_points; j++) {
                if (i != j) {
                    double numerator = 0.0 - static_cast<double>(coordinate_pairs[j].x_coord);
                    double denominator = static_cast<double>(coordinate_pairs[i].x_coord - coordinate_pairs[j].x_coord);
                    lagrange_term *= (numerator / denominator);
                }
            }
            
            secret_value += lagrange_term;
        }
        
        return secret_value;
    }
    
    void displayResults() {
        std::cout << "\n";
        for (int i = 0; i < 50; i++) std::cout << "=";
        std::cout << std::endl;
        std::cout << "BALAJI STARTED DECODING" << std::endl;
        for (int i = 0; i < 50; i++) std::cout << "=";
        std::cout << std::endl;
        
        std::cout << "\nConfiguration:" << std::endl;
        std::cout << "• Total shares (n): " << total_points << std::endl;
        std::cout << "• Required shares (k): " << required_points << std::endl;
        std::cout << "• Available points: " << coordinate_pairs.size() << std::endl;
        
        std::cout << "\nDecoded coordinates:" << std::endl;
        for (const auto& point : coordinate_pairs) {
            std::cout << "• (" << point.x_coord << ", " << point.y_value << ")" << std::endl;
        }
        
        double secret = calculateSecret();
        
        std::cout << "\n";
        for (int i = 0; i < 30; i++) std::cout << "-";
        std::cout << std::endl;
        std::cout << " THE CONSTANT  IS :   " << std::fixed << std::setprecision(0) << secret << std::endl;
        for (int i = 0; i < 30; i++) std::cout << "-";
        std::cout << std::endl;
    }
};

int main() {
    std::cout << "Welcome to the Secret Recovery Tool!" << std::endl;
    std::cout << "PROGRAM TO DECODE THE THE CONSTANT FOR A POLYNOMIAL EQUTAION \n" << std::endl;
    
    std::string filename;
    std::cout << "Please enter the JSON file path: ";
    std::getline(std::cin, filename);
    
    SecretSolver solver;
    
    if (solver.loadDataFromFile(filename)) {
        solver.displayResults();
    } else {
        std::cout << "Failed to process the file. Please check the file format and try again." << std::endl;
        return 1;
    }
    
    std::cout << "\n PROGRAM END !!!" << std::endl;
    return 0;
}

/*
How to use this program:
1. Save your JSON data to a file (e.g., "secret.json")
2. Compile: g++ -o secret_solver secret_solver.cpp
3. Run: ./secret_solver
4. Enter the filename when prompted
5. The program will decode and display the secret

Key improvements made:
- Object-oriented design with a dedicated SecretSolver class
- More descriptive variable and function names
- Better error handling and user feedback
- Cleaner separation of concerns
- More intuitive program flow
- Enhanced output formatting
*/