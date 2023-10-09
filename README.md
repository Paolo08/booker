# Booker - Resource Booking System

**Booker** is a C++ program designed to manage bookings for resources within a company. It processes information about resources such as buildings, sections, subsections and vehicles from a *json* file and handles booking requests specified in a queries file. The program ensures resources are not double-booked and maintains availability records.

## Prerequisites

Before you can build and run the **Booker** program, ensure you have the following installed on your local system:

- C++ compiler (e.g., `g++`)
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) library for parsing *json* data

## How to Build and Run

### Linux/macOS

- **Install jsoncpp library**

```bash
sudo apt-get install libjsoncpp-dev
```

- **Compile C++ code**

```bash
g++ -o booker booker.cpp -ljsoncpp
```

- **Run Booker program**

```bash
./booker inputs/resource.json inputs/queries.txt outputs/results.txt
```

### Windows

- **Download and Build jsoncpp library**

Download *jsoncpp* from <https://github.com/open-source-parsers/jsoncpp> and build it using CMake.

- **Compile C++ code**

```cmd
g++ -o booker booker.cpp <path/to/jsoncpp.lib>
```

- **Run Booker program**

```cmd
booker.exe inputs/resource.json inputs/queries.txt outputs/results.txt
```

## Usage

The program accepts three file paths as command-line arguments:

- *resources.json* in *inputs* folder containing company resource information;
- *queries.txt* in *inputs* folder containing booking queries;
- *results.txt* in *outputs* folder containing the output of the program.

### Supported commands

- `book <RESOURCE_ID> <DATE>`
- `is_booked <RESOURCE_ID> <DATE>`
- `is_all_booked <RESOURCE_ID> <DATE>`
- `is_available <RESOURCE_ID> <DATE>`

## Important Notes

The program handles various booking scenarios, ensuring no resource is double-booked.
Make sure to provide the correct file paths as command-line arguments.
The program writes booking results to *results.txt* file in the same order as the queries provided in *queries.txt*.

---
