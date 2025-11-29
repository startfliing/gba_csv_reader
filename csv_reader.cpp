#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <filesystem>

// Trim leading and trailing whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Convert string to valid C++ identifier
std::string toIdentifier(const std::string& str) {
    std::string result = trim(str);
    if (result.empty()) return "col";
    
    // Replace spaces and hyphens with underscores
    for (char& c : result) {
        if (!std::isalnum(c) && c != '_') {
            c = '_';
        }
    }
    
    // Ensure it doesn't start with a digit
    if (std::isdigit(result[0])) {
        result = "_" + result;
    }
    
    return result;
}

// Determine C++ type from value
std::string inferType(const std::string& value) {
    std::string trimmed = trim(value);
    if (trimmed.empty()) return "const char*";
    
    // Single character should be treated as char
    if (trimmed.length() == 1) {
        return "const char";
    }
    
    // Check if it's a number (int or float)
    bool isFloat = false;
    bool isNumber = true;
    int dotCount = 0;
    
    for (size_t i = 0; i < trimmed.length(); ++i) {
        char c = trimmed[i];
        if (i == 0 && (c == '-' || c == '+')) continue;
        
        if (c == '.') {
            dotCount++;
            if (dotCount > 1) {
                isNumber = false;
                break;
            }
            isFloat = true;
        } else if (!std::isdigit(c)) {
            isNumber = false;
            break;
        }
    }
    
    if (isNumber) {
        return isFloat ? "const double" : "const int";
    }
    
    return "const char*";
}

// Parse CSV and generate assembly and header files
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: csv_reader <input.csv> [output.s]" << std::endl;
        return 1;
    }

    std::string csvFile = argv[1];
    std::string asmFile = (argc > 2) ? argv[2] : "";

    // Generate output filenames if not provided
    if (asmFile.empty()) {
        std::filesystem::path p(csvFile);
        std::string stem = p.stem().string();
        asmFile = "build/" + stem + ".s";
    }
    
    // Ensure output directory exists
    std::filesystem::path asmPath(asmFile);
    std::string headerFile = "build/" + asmPath.stem().string() + ".h";
    std::filesystem::create_directories(asmPath.parent_path());

    // Read CSV file
    std::ifstream infile(csvFile);
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open file " << csvFile << std::endl;
        return 1;
    }

    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    std::string line;

    // Read header line
    if (!std::getline(infile, line)) {
        std::cerr << "Error: CSV file is empty" << std::endl;
        return 1;
    }

    // Parse header
    std::stringstream headerStream(line);
    std::string header;
    while (std::getline(headerStream, header, ',')) {
        headers.push_back(trim(header));
    }

    // Parse data rows
    while (std::getline(infile, line)) {
        if (trim(line).empty()) continue; // Skip empty lines
        
        std::vector<std::string> row;
        std::stringstream rowStream(line);
        std::string cell;
        while (std::getline(rowStream, cell, ',')) {
            row.push_back(trim(cell));
        }

        // Pad row if necessary
        while (row.size() < headers.size()) {
            row.push_back("");
        }

        rows.push_back(row);
    }

    infile.close();

    if (headers.empty() || rows.empty()) {
        std::cerr << "Error: CSV file has no data" << std::endl;
        return 1;
    }

    // Infer types from first data row
    std::vector<std::string> types;
    bool hasStrings = false;
    for (size_t i = 0; i < headers.size(); ++i) {
        std::string type = inferType(rows[0][i]);
        types.push_back(type);
        if (type == "const char*") {
            hasStrings = true;
        }
    }

    // Generate header guard
    std::string guardName = "DATA_" + 
        std::filesystem::path(headerFile).stem().string();
    std::transform(guardName.begin(), guardName.end(), guardName.begin(), 
                   [](unsigned char c) { return std::toupper(c); });
    for (char& c : guardName) {
        if (!std::isalnum(c)) c = '_';
    }

    // Write assembly file
    std::ofstream asmout(asmFile);
    if (!asmout.is_open()) {
        std::cerr << "Error: Cannot write to file " << asmFile << std::endl;
        return 1;
    }

    std::string dataSymbolBase = std::filesystem::path(csvFile).stem().string();
    std::string dataSymbol = dataSymbolBase + "_data";
    std::string countSymbol = dataSymbolBase + "_count";
    std::string headersSymbol = dataSymbolBase + "_headers";
    std::string headersCountSymbol = dataSymbolBase + "_headers_count";

    // Write header comment block
    asmout << "@{{BLOCK(" << dataSymbolBase << ")\n\n";
    asmout << "@=======================================================================\n";
    asmout << "@\n";
    asmout << "@\t" << dataSymbolBase << " - CSV data\n";
    asmout << "@\n";
    asmout << "@\tGenerated from: " << csvFile << "\n";
    asmout << "@\tRecords: " << rows.size() << ", Fields: " << headers.size() << "\n";
    asmout << "@\n";
    asmout << "@=======================================================================\n\n";

    // Calculate data size in bytes
    size_t recordSize = headers.size() * 4; // 4 bytes per field (pointer or int/float)
    size_t totalDataSize = recordSize * rows.size();

    asmout << ".section .rodata\n";
    asmout << ".align 2\n";
    asmout << ".global " << dataSymbol << "\t\t@ " << totalDataSize << " bytes\n";
    asmout << dataSymbol << ":\n";

    // Each record as assembly data
    for (size_t i = 0; i < rows.size(); ++i) {
        for (size_t j = 0; j < headers.size(); ++j) {
            const std::string& value = rows[i][j];
            
            if (types[j] == "const char*") {
                // String data - pointer to string label
                std::string stringLabel = dataSymbolBase + "_str_" + std::to_string(i) + "_" + std::to_string(j);
                asmout << "\t.4byte " << stringLabel << "\n";
            } else if (types[j] == "const char") {
                // Single character - output as byte
                char charVal = value.empty() ? 0 : value[0];
                asmout << "\t.4byte " << (int)(unsigned char)charVal << "\n";
            } else if (types[j] == "const double") {
                // 4-byte float/double
                double dval = std::stod(value.empty() ? "0" : value);
                uint32_t ival = *(uint32_t*)&dval;
                asmout << "\t.4byte 0x" << std::hex << std::uppercase << ival << std::dec << std::nouppercase << "\n";
            } else { // int
                // Convert to integer and back to string to remove leading zeros
                int intVal = value.empty() ? 0 : std::stoi(value);
                asmout << "\t.4byte " << intVal << "\n";
            }
        }
    }

    asmout << ".size " << dataSymbol << ", . - " << dataSymbol << "\n\n";

    // Count symbol
    asmout << ".global " << countSymbol << "\t\t@ 4 bytes\n";
    asmout << countSymbol << ":\n";
    asmout << "\t.4byte " << rows.size() << "\n";
    asmout << ".size " << countSymbol << ", 4\n\n";

    // Headers array - pointers to header strings
    asmout << ".global " << headersSymbol << "\n";
    asmout << headersSymbol << ":\n";
    for (size_t j = 0; j < headers.size(); ++j) {
        std::string headerLabel = dataSymbolBase + "_header_" + std::to_string(j);
        asmout << "\t.4byte " << headerLabel << "\n";
    }
    asmout << ".size " << headersSymbol << ", . - " << headersSymbol << "\n\n";

    // Headers count symbol
    asmout << ".global " << headersCountSymbol << "\n";
    asmout << headersCountSymbol << ":\n";
    asmout << "\t.4byte " << headers.size() << "\n";
    asmout << ".size " << headersCountSymbol << ", 4\n\n";

    // String literals in rodata section
    if (hasStrings) {
        asmout << ".section .rodata\n";
        for (size_t i = 0; i < rows.size(); ++i) {
            for (size_t j = 0; j < headers.size(); ++j) {
                if (types[j] == "const char*") {
                    std::string stringLabel = dataSymbolBase + "_str_" + std::to_string(i) + "_" + std::to_string(j);
                    const std::string& value = rows[i][j];
                    asmout << stringLabel << ":\n";
                    asmout << "\t.ascii \"" << value << "\\0\"\n";
                }
            }
        }
    }

    // Header strings in rodata section
    asmout << ".section .rodata\n";
    for (size_t j = 0; j < headers.size(); ++j) {
        std::string headerLabel = dataSymbolBase + "_header_" + std::to_string(j);
        asmout << headerLabel << ":\n";
        asmout << "\t.ascii \"" << headers[j] << "\\0\"\n";
    }

    asmout << "\n@}}BLOCK(" << dataSymbolBase << ")\n";

    asmout.close();

    std::ofstream outfile(headerFile);
    if (!outfile.is_open()) {
        std::cerr << "Error: Cannot write to file " << headerFile << std::endl;
        return 1;
    }

    //std::cout << "Generated " << asmFile << " from " << csvFile << std::endl;

    // Header guard
    outfile << "#ifndef " << guardName << "_H\n";
    outfile << "#define " << guardName << "_H\n\n";

    // Struct definition
    std::string structName = std::filesystem::path(headerFile).stem().string();
    // Capitalize first letter
    if (!structName.empty()) {
        structName[0] = std::toupper(structName[0]);
    }
    
    outfile << "struct " << structName << " {\n";
    for (size_t i = 0; i < headers.size(); ++i) {
        std::string fieldName = toIdentifier(headers[i]);
        std::string fieldType = types[i];
        outfile << "    " << fieldType << " " << fieldName << ";\n";
    }
    outfile << "};\n\n";

    // Extern declarations for assembly symbols
    outfile << "extern const struct " << structName << " " << dataSymbol << "[];\n";
    outfile << "extern const int " << countSymbol << ";\n";
    outfile << "extern const char* " << headersSymbol << "[];\n";
    outfile << "extern const int " << headersCountSymbol << ";\n\n";

    outfile << "#endif // " << guardName << "_H\n";

    outfile.close();

    //std::cout << "Generated " << headerFile << " from " << csvFile << std::endl;
    //std::cout << "  Struct: " << structName << std::endl;
    //std::cout << "  Fields: " << headers.size() << std::endl;
    //std::cout << "  Records: " << rows.size() << std::endl;
    //std::cout << "  Assembly: " << asmFile << std::endl;

    return 0;
}
