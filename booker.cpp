#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include <jsoncpp/json/json.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"


// Define data structures
struct Section {
    std::string id;
    std::vector<Section> subsections;
    std::vector<std::string>  vehicles;
};

struct Building {
    std::string id;
    std::vector<Section> sections;
    std::vector<std::string> vehicles;
};

// Map with parsed data from input json file (key: buildingId and value: Building)
std::map<std::string, Building> buildings;

// Map to track bookings (key: resourceId, value: bookedDates)
std::map<std::string, std::vector<std::string>> bookings;


void loadResources(const std::string &filename) {
    /*
        Parse resources from json file and populate 'buildings' map.
    */

    // Open input json file for reading
    std::ifstream file(filename);
    Json::CharReaderBuilder reader;
    Json::Value root;

    // Check if file is successfully opened
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open given resources file: '" + filename + "'");
        return;
    }

    // Read json file into a string buffer
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonStr = buffer.str();

    // Parse json data from string buffer
    std::string errs;
    std::istringstream jsonStream(jsonStr);
    Json::parseFromStream(reader, jsonStream, &root, &errs);

    // Extract 'buildings' json array from root object
    const Json::Value& buildingsJson = root["resources"]["buildings"];

    // Iterate through each building
    for (const auto& buildingJson : buildingsJson) {
        Building building;
        building.id = buildingJson["id"].asString();

        // Parse all sections within current building
        const Json::Value& sectionsJson = buildingJson["sections"];
        for (const auto& sectionJson : sectionsJson) {
            Section section;
            section.id = sectionJson["id"].asString();

            // Parse all subsections within current section
            const Json::Value& subsectionsJson = sectionJson["sections"];
            for (const auto& subsectionJson : subsectionsJson) {
                Section subsection;
                subsection.id = subsectionJson["id"].asString();

                // Parse all vehicles within current subsection
                const Json::Value& subsectionVehiclesJson = subsectionJson["vehicles"];
                for (const auto& vehicleJson : subsectionVehiclesJson) {
                    subsection.vehicles.push_back(vehicleJson.asString());
                }

                // Add subsection to current section
                section.subsections.push_back(subsection);
            }

            // Parse all vehicles within current section
            const Json::Value& vehiclesJson = sectionJson["vehicles"];
            for (const auto& vehicleJson : vehiclesJson) {
                section.vehicles.push_back(vehicleJson.asString());
            }

            // Add section to current building
            building.sections.push_back(section);
        }

        // Parse all vehicles within current building
        const Json::Value& vehiclesJson = buildingJson["vehicles"];
        for (const auto& vehicleJson : vehiclesJson) {
            building.vehicles.push_back(vehicleJson.asString());
        }

        // Add current building to 'buildings' map
        buildings[building.id] = building;
    }
}


template<typename T>
T checkBookingsMap(const std::string& resourceId, const std::string& date) {
    /*
        Template function that can return either a bool or a std::string.
    */

    // Check for resourceId in 'bookings' map
    if (bookings.find(resourceId) != bookings.end()) {
        const std::vector<std::string>& bookedDates = bookings.at(resourceId);
        // Check if given date is in the list of resource booked dates
        for (const std::string& bookedDate : bookedDates) {
            if (bookedDate == date) {
                if constexpr (std::is_same_v<T, bool>) {
                    return false; // Resource is NOT available
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return "yes"; // Resource is booked for given date -> NOT available
                }
            }
        }
    }

    if constexpr (std::is_same_v<T, bool>) {
        return true; // Resource is available
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "no"; // Resource is NOT booked for given date -> available
    }
}


bool canBookResource(const std::string &resourceId, const std::string &date) {
    /*
        Recursively check resource availability for given date following 'top-down' logic.
    */

    // Check if resource (building, section, subsection or vehicle) itself is already booked
    if (!checkBookingsMap<bool>(resourceId, date)) {
        return false; // Resource is NOT available
    }

    // Resource is a building
    if (buildings.find(resourceId) != buildings.end()) {
        const Building& building = buildings[resourceId];
        // Check vehicles in building
        for (const std::string& vehicle : building.vehicles) {
            if (!canBookResource(vehicle, date)) {
                // At least one vehicle is already booked 
                return false; // Resource building is NOT available
            }
        }
        // Check sections in building
        for (const Section& section : building.sections) {
            if (!canBookResource(section.id, date)) {
                // At least one section, subsection or vehicle is already booked
                return false; // Resource building is NOT available
            }
        }
    }

    // Resource is a section
    for (const auto& buildingPair : buildings) {
        const Building& building = buildingPair.second; // Get the current Building object
        for (const Section& section : building.sections) {
            if (section.id == resourceId) {
                // Check vehicles in section
                for (const std::string& vehicle : section.vehicles) {
                    if (!canBookResource(vehicle, date)) {
                        // At least one vehicle is already booked
                        return false; // Resource section is NOT available
                    }
                }
                // Check subsections in section
                for (const Section& subsection : section.subsections) {
                    if (!canBookResource(subsection.id, date)) {
                        // At least one subsection or vehicle is already booked
                        return false; // Resource section is NOT available
                    }
                }
            }
        }
    }

    // Resource is a subsection
    for (const auto& buildingPair : buildings) {
        const Building& building = buildingPair.second; // Get the current Building object
        for (const Section& section : building.sections) {
            const Section& currentSection = section;  // Get the current Section object
            for (const Section& subsection : currentSection.subsections) {
                if (subsection.id == resourceId) {
                    // Check vehicles in subsection
                    for (const std::string& vehicle : subsection.vehicles) {
                        if (!canBookResource(vehicle, date)) {
                            // At least one vehicle is already booked
                            return false; // Resource subsection is NOT available
                        }
                    }
                }
            }
        }
    }

    // Resource and all its sub-resources are available for given date
    return true;
}


void bookAllSubResources(const std::string &resourceId, const std::string &date) {
    /*
        Recursively book resource and its sub-resources following 'top-down' logic.
    */

    // Update 'bookings' map for the current resource
    bookings[resourceId].push_back(date);

    // Resource is a building
    if (buildings.find(resourceId) != buildings.end()) {
        const Building& building = buildings[resourceId];
        // Book all vehicles within the building
        for (const std::string& vehicle : building.vehicles) {
            bookAllSubResources(vehicle, date);
        }
        // Book all sections and subsections within the building
        for (const Section& section : building.sections) {
            bookAllSubResources(section.id, date);
        }
    }

    // If the resource is a section
    for (const auto& buildingPair : buildings) {
        const Building& building = buildingPair.second;  // Get the current Building object
        for (const Section& section : building.sections) {
            if (section.id == resourceId) {
                // Book all vehicles within the section
                for (const std::string& vehicle : section.vehicles) {
                    bookAllSubResources(vehicle, date);
                }
                // Book all subsections within the section
                for (const Section& subsection : section.subsections) {
                    bookAllSubResources(subsection.id, date);
                }
            }
        }
    }

    // If the resource is a subsection
    for (const auto& buildingPair : buildings) {
        const Building& building = buildingPair.second;  // Get the current Building object
        for (const Section& section : building.sections) {
            const Section& currentSection = section;  // Get the current Section object
            for (const Section& subsection : currentSection.subsections) {
                if (subsection.id == resourceId) {
                    // Book all vehicles within the subsection
                    for (const std::string& vehicle : subsection.vehicles) {
                        bookAllSubResources(vehicle, date);
                    }
                }
            }
        }
    }
}


std::string bookResource(const std::string& resourceId, const std::string& date) {
    /*
        Book resource and all its sub-resources if available for given date.

        Assumption: All resources (including sub-resources) are initially available.
    */

    // Check if resource and all its sub-resources are available for given date
    if (canBookResource(resourceId, date)) {
        // Resource and all its sub-resources are available -> Recursive booking
        bookAllSubResources(resourceId, date);
        // Successful booking
        return "ok";
    } else {
        // Resource or one of its sub-resources is NOT available -> Unsuccessful booking
        return "failed";
    }
}


std::string isBooked(const std::string& resourceId, const std::string& date) {
    /*
        Check if resource is booked for given date.

        Assumption: Only checking if a single resource (not its sub-resources) is booked.
    */

    // Check if resource is in 'bookings' map
    return checkBookingsMap<std::string>(resourceId, date);
}


std::string isAllBooked(const std::string& resourceId, const std::string& date) {
    /*
        Check if resource and all its sub-resources are booked for given date.
    */

    if (canBookResource(resourceId, date)) {
        // Resource and all its sub-resources are NOT booked for given date -> available
        return "no";
    } else {
        // Resource or at least one of its sub-resources is booked for given date -> NOT available
        return "yes";
    }
}


std::string isAvailable(const std::string& resourceId, const std::string& date) {
    /*
        Check if resource (including its sub-resources) is available for given date.
    */

    if (canBookResource(resourceId, date)) {
        // Resource and all its sub-resources are NOT booked for given date -> available
        return "yes";
    } else {
        // Resource or at least one of its sub-resources is booked for given date -> NOT available
        return "no";
    }
}


std::vector<std::string> processQueries(const std::string& filename) {
    /*
        Process queries from input file.
    */

    // Open queries input file for reading
    std::ifstream file(filename);
    std::vector<std::string> results;
    std::string line;

    // Check if file is successfully opened
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open given queries file: '" + filename + "'");
        return results;
    }

    // Read each line from queries input file and process queries
    while (getline(file, line)) {
        // Ignore empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse single query line
        std::istringstream iss(line);
        std::string command, resourceId, date;
        iss >> command >> resourceId >> date;

        // Process query types: 'book', 'is_booked', 'is_all_booked' and 'is_available'
        if (command == "book") {
            results.push_back(bookResource(resourceId, date));
        } else if (command == "is_booked") {
            results.push_back(isBooked(resourceId, date));
        } else if (command == "is_all_booked") {
            results.push_back(isAllBooked(resourceId, date));
        } else if (command == "is_available") {
            results.push_back(isAvailable(resourceId, date));
        } else {
            std::string exception_message;
            exception_message.append("Command '" + command + "' is not supported\n");
            exception_message.append("Supported commands: 'book', 'is_booked', 'is_all_booked' and 'is_available'");
            throw std::runtime_error(exception_message);
        }
    }

    return results;
}


bool checkArguments(int argc, char* argv[]) {
    /*
        Check correct number of arguments provided.
    */

    if (argc != 4) {
        std::cerr << RED << "ERROR " << RESET << "-> Incorrect number of arguments provided (" << argc - 1 << "), should be (3)" << std::endl << std::endl;
        std::cerr << RED << "Provided execution command: ";
        for (int i = 0; i < argc; ++i) {
            std::cerr << argv[i] << " ";
        }
        std::cerr << RESET << std::endl;

        std::cerr << YELLOW << "You MUST use: ./booker <resources_path> <queries_path> <results_path>" << RESET << std::endl;

        return false;
    }

    return true;
}


void writeResultsToFile(const std::vector<std::string>& results, const std::string& resultsPath) {
    /*
        Write results to output file and handle file state correctly.
    */

    std::ofstream outputFile(resultsPath);
    if (!outputFile.is_open()) {
        throw std::runtime_error("Failed to open output file '" + resultsPath + "'");
    }
    for (const std::string& result : results) {
        outputFile << result << std::endl;
        if (!outputFile) {
            throw std::runtime_error("Failed to write to output file '" + resultsPath + "'");
        }
    }
    outputFile.close();

    std::cout << GREEN << "SUCCESS " << RESET << "-> Results written to output file: '" << resultsPath << "'" << std::endl;
}


int main(int argc, char* argv[]) {
    try {
        if (!checkArguments(argc, argv)) {
            return 1;
        }

        const std::string resourcesPath = argv[1];    // Path to 'resources.json' file
        const std::string queriesPath   = argv[2];    // Path to 'queries.txt' file
        const std::string resultsPath   = argv[3];    // Path to 'results.txt' file

        // Load resources from input json file
        loadResources(resourcesPath);
        // Process queries from input file and get results
        std::vector<std::string> results = processQueries(queriesPath);
        // Write results to output file
        writeResultsToFile(results, resultsPath);

    } catch (const std::exception& e) {
        std::cerr << RED << "ERROR " << RESET << "-> " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
