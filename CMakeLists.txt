# Minimum CMake version and project name
cmake_minimum_required(VERSION 3.10)
project(EdgeCloudResourceAllocation)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Manually define paths for CPLEX 12.6 ---
# Header directories (.h)
set(CPLEX_INCLUDE_DIR "/opt/ibm/ILOG/CPLEX_Studio126/cplex/include")
set(CONCERT_INCLUDE_DIR "/opt/ibm/ILOG/CPLEX_Studio126/concert/include")

# Library directories (.a)
set(CPLEX_LIB_DIR "/opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic")
set(CONCERT_LIB_DIR "/opt/ibm/ILOG/CPLEX_Studio126/concert/lib/x86-64_linux/static_pic")

# 2. Find the Threads library (for the -lpthread flag)
find_package(Threads REQUIRED)

# 3. Add the main executable
add_executable(main_app src/main.cpp)

# 4. Add include directories (from our project and from CPLEX)
target_include_directories(main_app PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    ${CPLEX_INCLUDE_DIR}
    ${CONCERT_INCLUDE_DIR}
)

# 5. Add library directories (from CPLEX)
target_link_directories(main_app PRIVATE 
    ${CPLEX_LIB_DIR}
    ${CONCERT_LIB_DIR}
)

# 6. Link the libraries in the correct order
target_link_libraries(main_app PRIVATE 
    ilocplex 
    cplex 
    concert 
    m 
    Threads::Threads
)

# 7. Add compile definitions (-D...)
target_compile_definitions(main_app PRIVATE DNDEBUG IL_STD)

# 8. Add other compile flags
target_compile_options(main_app PRIVATE -O -fPIC -g -w -fexceptions)